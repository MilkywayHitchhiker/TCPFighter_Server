#include"stdafx.h"
#include"Network.h"
#include"Game_Proc.h"
#include"PacketDefine.h"
#include"DefineEnum.h"

SOCKET g_ListenSock;		//리슨 소켓

map<SOCKET, st_SESSION *> g_SessionMap;		//세션 구조체를 관리하기 위한 맵

//Socket으로 세션 구조체 찾아서 반환 없는 유저일 경우 NULL 반환
st_SESSION *FindSession (SOCKET sock)
{
	map<SOCKET, st_SESSION *>::iterator iter;

	iter = g_SessionMap.find (sock);

	if ( iter == g_SessionMap.end () )
	{
		return NULL;
	}

	return iter->second;

}


//새로운 세션 생성 및 등록
st_SESSION *CreateSession (SOCKET Sock)
{
	st_SESSION *pNewSession;
	pNewSession = new st_SESSION;

	pNewSession->Sock = Sock;
	pNewSession->dwSessionID = g_SessionID;
	pNewSession->dwHeartBeat = timeGetTime ();
	pNewSession->RecvQ.ClearBuffer ();
	pNewSession->SendQ.ClearBuffer ();

	g_SessionMap.insert (pair<SOCKET,st_SESSION *>(Sock,pNewSession));
	g_SessionID++;
	return pNewSession;
}

//해당 세션 종료처리
void DisconnectSession (st_SESSION *pSession)
{
	st_CHARACTER *pChar;
	Packet pack;

	_LOG (dfLog_LEVEL_DEBUG, L"Disconnect SessionID : %d", pSession->dwSessionID);

	pChar = FindCharacter (pSession->dwSessionID);
	
	//캐릭터 삭제 패킷 생성
	Pack_DeleteCharacter (&pack, pSession->dwSessionID);

	//주변타일에 삭제 요청 뿌림
	SendPacket_Around (pSession, &pack,true);

	//섹터에서 삭제
	Sector_RemoveCharacter (pChar);

	//캐릭터 맵에서 삭제
	g_CharacterMap.erase (pSession->dwSessionID);

	//세션 맵에서 삭제
	g_SessionMap.erase (pSession->Sock);

	delete pChar;

	//메모리 할당 해제.
	delete pSession;

	return;

}

//서버 메인 네트워크 처리 함수
void NetworkProcess (void)
{
	st_SESSION *pSession;
	SOCKET SockTable[FD_SETSIZE] = { INVALID_SOCKET, };	//소켓테이블 생성 및 초기화
	int iSocketCount = 0;

	FD_SET ReadSet;
	FD_SET WriteSet;

	//초기화
	FD_ZERO (&ReadSet);
	FD_ZERO (&WriteSet);

	//리슨소켓을 Readset에 넣고 시작
	FD_SET (g_ListenSock, &ReadSet);
	SockTable[iSocketCount] = g_ListenSock;
	iSocketCount++;

	map<SOCKET, st_SESSION *>::iterator iter;
	for ( iter = g_SessionMap.begin (); iter != g_SessionMap.end ();)
	{
		pSession = iter->second;

		SockTable[iSocketCount] = pSession->Sock;

		//ReadSet에 해당 유저 소켓 등록
		FD_SET (pSession->Sock, &ReadSet);
		//SendQ에 데이터가 있다면 보낼 데이터가 있는것이므로 WriteSet에 등록
		if ( pSession->SendQ.GetUseSize () > 0 )
		{
			FD_SET (pSession->Sock, &WriteSet);
		}

		iSocketCount++;
		iter++;



		//Select가 FD_SETSIZE 에 도달했다면 Select호출 후 정리
		if ( FD_SETSIZE <= iSocketCount )
		{
			NetworkSelectProc (SockTable, &ReadSet, &WriteSet);

			//초기화
			FD_ZERO (&ReadSet);
			FD_ZERO (&WriteSet);

			memset (SockTable, INVALID_SOCKET, sizeof (SOCKET) * FD_SETSIZE);


			//단일 스레드에서 select 방식의 경우 소켓이 천개 이상일 경우와 서버의 로직에 부하가 걸리는 경우 접속자 처리가 느려지기때문에
			//한번 루프 돌때마다 리슨소켓을 매번 체크해 준다.
			FD_SET (g_ListenSock, &ReadSet);
			SockTable[0] = g_ListenSock;
			iSocketCount = 1;
		}



	}

	//for문을 다 돌았는데 iSocketCount가 0보다 크다면 검사해야될 소켓이 남아있는것이므로 마지막으로 한번더 Select함수 호출
	if ( iSocketCount > 0 )
	{
		NetworkSelectProc (SockTable, &ReadSet, &WriteSet);
	}

}

//Seslct 모델 체크 함수
void NetworkSelectProc (SOCKET *pSockTable, FD_SET *pReadSet, FD_SET *pWriteSet)
{
	timeval Time;

	int iResult;
	int iCnt;
	bool ProcFlag;

	Time.tv_sec = 0;
	Time.tv_usec = 0;

	iResult = select (0, pReadSet, pWriteSet, 0, &Time);


	//리턴값이 0보다 크다면 데이터가 수신된 것이다.
	if ( iResult > 0 )
	{
		for ( iCnt = 0; iResult > 0 && iCnt < FD_SETSIZE; iCnt++ )
		{
			ProcFlag = true;
			if ( pSockTable[iCnt] == INVALID_SOCKET )
			{
				continue;
			}

			//WriteSet 체크
			if ( FD_ISSET (pSockTable[iCnt], pWriteSet) )
			{
				ProcFlag = NetworkSend (pSockTable[iCnt]);
				iResult--;
			}

			if ( FD_ISSET (pSockTable[iCnt], pReadSet) )
			{

				//NetworkSend 부분에서 에러의 상황으로 해당 클라이언트가 접속종료를 한 경우가 있기에 ProcFlag로 확인 후 Recv 진행
				if ( ProcFlag )
				{
					//리슨소켓일 경우 접속 처리
					if ( pSockTable[iCnt] == g_ListenSock )
					{
						NetworkAccept ();
					}
					else if ( pSockTable[iCnt] != g_ListenSock )
					{
						NetworkRecv (pSockTable[iCnt]);
					}
				}


				iResult--;
			}
		}

	}
	else if ( iResult == SOCKET_ERROR )
	{
		//에러 처리
		wprintf (L"select Error\n");
	}



}

//Recv처리
void NetworkRecv (SOCKET sock)
{
	st_SESSION *pSession;
	int iBuffSize;
	int iResult;

	pSession = FindSession (sock);

	if ( pSession == NULL )
	{
		return;
	}

	//마지막으로 받은 메세지 타임
	pSession->dwHeartBeat = timeGetTime ();

	//받기 작업

	iBuffSize = pSession->RecvQ.GetNotBrokenPutSize ();

	iResult = recv (pSession->Sock, pSession->RecvQ.GetWriteBufferPtr (), iBuffSize, 0);

	//recv결과가 소켓 에러일 경우 연결 끊기.
	if ( SOCKET_ERROR == iResult || 0 == iResult)
	{
		DisconnectSession (pSession);
		return;
	}

	//받은 데이터가 있다면
	if ( 0 < iResult )
	{
		//RecvQ에 데이터를 넣었으니 WritePos를 이동시켜줘야 된다.
		pSession->RecvQ.MoveWritePos (iResult);

		//패킷이 하나 이상 수신되었을 수 있으므로 모두 처리될 때 까지 계속 반복한다.
		while ( 1 )
		{
			iResult = RecvPacket (pSession);
			if ( 1 == iResult )
			{
				break;
			}

			if ( -1 == iResult )
			{
				_LOG (dfLog_LEVEL_ERROR, L"PRError SessionID : %d", pSession->dwSessionID);
				return;
			}

		}
	}



}

//Send처리
bool NetworkSend (SOCKET sock)
{
	st_SESSION *pSession;
	int iResult;
	int iSendSize;

	//해당 사용자 세션 찾기

	pSession = FindSession (sock);
	//찾는 사용자가 없다면 그냥 나옴.
	if ( NULL == pSession )
	{
		return false;
	}

	//SendQ에 있는 데이터들을 최대 dfMaxSendSize 이하로 보낸다.
	//이는 한개의 패킷에 담을 수 있는 최대 MTU사이즈이기 때문이다.
	//이를 넘어서면 전송중간에 패킷이 분리되며 데이터가 깨질 우려가 있다.
	iSendSize = pSession->SendQ.GetNotBrokenGetSize ();
	//iSendSize = min (dfMaxSendSize, iSendSize);

	//SendQ에 보낼 데이터가 없으면 그냥 빠져나옴.
	if ( 0 >= iSendSize )
	{
		return true;
	}

	//send처리
	iResult = send (pSession->Sock, pSession->SendQ.GetReadBufferPtr (), iSendSize, 0);
	pSession->SendQ.RemoveData (iResult);

	//send에서 소켓에러가 뜬다면 해당 클라이언트 디스커넥트 처리
	if ( iResult == SOCKET_ERROR )
	{
		_LOG (dfLog_LEVEL_ERROR, L"##Send_SOCKET_ERROR ID : %d", pSession->dwSessionID);
		DisconnectSession (pSession);
		return false;
	}
	else if ( iSendSize < iResult )
	{
		//보낸 보낼 사이즈보다 오히려 더 크다면 오류기때문에 무조건 끊어주면 된다.
		//생기면 안되는 상황이지만 가끔 이런 경우가 생길때가 있다.
		DisconnectSession (pSession);
		return false;
	}

	return true;
}

//네트워크 초기화
void Network_Init (void)
{
	WSAData wsa;
	int retval;

	//윈속 초기화
	if ( WSAStartup (MAKEWORD (2, 2), &wsa) != 0 )
	{
		return;
	}

	//소켓 초기화
	g_ListenSock = socket (AF_INET, SOCK_STREAM, 0);
	if ( g_ListenSock == INVALID_SOCKET )
	{
		return;
	}

	//bind
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	InetPton (AF_INET, ServerIP, &addr.sin_addr);
	addr.sin_port = htons (dfNETWORK_PORT);

	retval = bind (g_ListenSock, ( SOCKADDR * )&addr, sizeof (addr));
	if ( retval == SOCKET_ERROR )
	{
		return;
	}

	//listen
	retval = listen (g_ListenSock, SOMAXCONN);
	if ( retval == SOCKET_ERROR )
	{
		return;
	}

	int opt_val = TRUE;
	setsockopt (g_ListenSock, IPPROTO_TCP, TCP_NODELAY, ( char * )&opt_val, sizeof (opt_val));

	_LOG (dfLog_LEVEL_WARNING, L"Server Start IP:%s\n",ServerIP);
	//wprintf (L"Server Start IP:%s", ServerIP);
	//네트워크 셋팅완료.
	return;
}

//사용자 접속 이벤트 처리
bool NetworkAccept (void)
{
	SOCKADDR_IN addr;
	SOCKET sock;
	int iSize = sizeof (addr);

	sock = accept (g_ListenSock, ( SOCKADDR * )&addr, &iSize);

	if ( INVALID_SOCKET == sock )
	{
		return false;
	}

	st_SESSION *pSession = CreateSession (sock);

	PacketProc_Connect (pSession);

	//inet_ntoa는 멀티바이트문자열이기 때문에 유니코드로 바꿔줘야 된다.
	wchar_t strUnicode[256] = { 0, };
	char	*strMultibyte;
	strMultibyte = inet_ntoa (addr.sin_addr);

	int nLen = MultiByteToWideChar (CP_ACP, 0, strMultibyte, strlen (strMultibyte), NULL, NULL);
	MultiByteToWideChar (CP_ACP, 0, strMultibyte, strlen (strMultibyte), strUnicode, nLen);


	_LOG (dfLog_LEVEL_DEBUG,L"Connect # IP:%s SessionID:%d", strUnicode, g_SessionID);


	return true;

}

//패킷이 완료되었는지 검사 후 패킷 처리
int RecvPacket (st_SESSION *pSession)
{
	st_PACK_HEADER PacketHeader;
	int iRecvQSize;
	BYTE byEndCode;

	iRecvQSize = pSession->RecvQ.GetUseSize ();

	//받은 사이즈가 패킷헤더보다 작다면 그냥 종료
	if ( sizeof (st_PACK_HEADER) > iRecvQSize )
	{
		return 1;
	}

	// 1. PacketCode검사
	pSession->RecvQ.Peek (( char * )&PacketHeader, sizeof (st_PACK_HEADER));

	//Peek로 뽑아서 헤더 코드가 맞는지 검사. 틀리면 리턴
	if ( dfPACKET_CODE != PacketHeader.byCode )
	{
		return -1;
	}

	// 2. 큐에 저장된 데이터가 패킷의 크기만큼 있는지 확인 EndCode 크기 더해서 계산할것.
	if ( PacketHeader.bySize + sizeof (st_PACK_HEADER) + 1 > iRecvQSize )
	{
		return 1;
	}

	//위에서 헤더를 Peek로 뽑았으므로 큐에서 헤더 사이즈 만큼 지워야 됨.
	pSession->RecvQ.RemoveData (sizeof (st_PACK_HEADER));
	
	Packet pack;
	pack.Clear ();
	//Payload 부분을 버퍼로 빼옴.
	if ( !pSession->RecvQ.Get (pack.GetBufferPtr (), PacketHeader.bySize))
	{
		return -1;
	}

	//EndCode를 버퍼로 빼서 확인.
	if ( !pSession->RecvQ.Get (( char * )&byEndCode, 1) )
	{
		return -1;
	}
	if ( byEndCode != dfNETWORK_PACKET_END )
	{
		return -1;
	}

	//패킷 버퍼 포인터를 얻어 임의로 데이터를 넣었으므로 사이즈 이동처리
	pack.MoveWritePos (PacketHeader.bySize);

	//패킷처리 함수 호출
	if ( !PacketProc (pSession, PacketHeader.byType, &pack) )
	{
		return -1;
	}
	return 0;


}

//패킷 타입에 따른 처리 함수 호출
bool PacketProc (st_SESSION *pSession, BYTE PacketType, Packet *pPack)
{
	switch ( PacketType )
	{
	case dfPACKET_CS_MOVE_START :
		PacketProc_MoveStart (pSession, pPack);
		break;
	case dfPACKET_CS_MOVE_STOP :
		PacketProc_MoveStop (pSession, pPack);
		break;
	case dfPACKET_CS_ATTACK1 :
		PacketProc_Attack1 (pSession, pPack);
		break;
	case dfPACKET_CS_ATTACK2 :
		PacketProc_Attack2 (pSession, pPack);
		break;
	case dfPACKET_CS_ATTACK3 :
		PacketProc_Attack3 (pSession, pPack);
		break;
	case dfPACKET_CS_ECHO :
		PacketProc_ECHO (pSession, pPack);
		break;
	default : 
		return false;
		break;
	}
	return true;
}


// 사용자 접속/해지 알림.
BOOL	PacketProc_Connect (st_SESSION *pSession)
{
	Packet	pack;
	pack.Clear ();
	_LOG (dfLog_LEVEL_DEBUG,L"# PACKET_CONNECT # SessionID:%d", pSession->dwSessionID);


	//-----------------------------------------------------
	// 새로운 사용자 접속을 처리한다.
	//-----------------------------------------------------
	gameCreatePlayer (pSession);

	st_SECTOR_AROUND Pos;
	st_CHARACTER *pChar = FindCharacter (pSession->dwSessionID);
	st_CHARACTER *pOtherChar;


	list<st_CHARACTER *> *pSectorList;
	list<st_CHARACTER *>::iterator iter;
	//해당 클라이언트 주변 섹터 검색
	GetSectorAround (pChar->CurSector.iX, pChar->CurSector.iY, &Pos);
	
	//얻어낸 섹터정보를 가지고 메세지 전송


	pack.Clear ();
	//나한테 내 생성정보 알림
	Pack_CreateMyCharacter (&pack, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
	SendPacket_Unicast (pSession, &pack);

	
	//자신한테 섹터 내 다른 유저들의 목록 전송
	int iCnt;
	for ( iCnt = 0; iCnt < Pos.iCount; iCnt++ )
	{

		pSectorList = &g_Sector[Pos.Around[iCnt].iY][Pos.Around[iCnt].iX];
		for ( iter = pSectorList->begin (); iter != pSectorList->end ();)
		{
			pOtherChar = *iter;
			if ( pOtherChar != pChar )
			{
				pack.Clear ();
				Pack_CreateOtherCharacter (&pack, pOtherChar->dwSessionID, pOtherChar->byDirection, pOtherChar->shX, pOtherChar->shY, pOtherChar->chHP);
				SendPacket_Unicast (pSession, &pack);
			}
			iter++;
		}
	}

	pack.Clear ();
	//다른 유저들한테 나의 생성정보 알림
	Pack_CreateOtherCharacter (&pack, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
	SendPacket_Around (pSession, &pack,false);


	
	return true;
}





BOOL PacketProc_MoveStart (st_SESSION *pSession, Packet *pack)
{
	BYTE byDirection;
	short shX, shY;
	*pack >> byDirection;
	*pack >> shX;
	*pack >> shY;

	_LOG (dfLog_LEVEL_DEBUG, L"MoveStart SessionID:%d / Direction: %d / X: %d / Y: %d", pSession->dwSessionID, byDirection, shX, shY);

	//ID로 캐릭터를 검색

	st_CHARACTER *pCharacter = FindCharacter (pSession->dwSessionID);
	if ( pCharacter == NULL )
	{
		_LOG (dfLog_LEVEL_ERROR, L"MoveStart SessionID:%d Charactor Not Found!", pSession->dwSessionID);
		return false;
	}

	//서버의 위치와 받은 패킷의 위치값이 크게 다르다면 데드레커닝으로 재위치 확인.
	//그래도 좌표가 다르다면 싱크패킷을 클라로 보내서 좌표보정.
	if ( abs (pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		int iDirX, iDirY;
		int iDeadFrame = DeadReckoningPos (pCharacter->dwAction, pCharacter->dwActionTick, pCharacter->shActionX, pCharacter->shActionY, &iDirX, &iDirY);
		
		if ( abs (iDirX - shX) > dfERROR_RANGE || abs (iDirY - shY) > dfERROR_RANGE )
		{
			pack->Clear ();
			Pack_Sync (pack, pCharacter->dwSessionID, iDirX, iDirY);
			SendPacket_Around (pCharacter->pSession, pack, true);

			_LOG (dfLog_LEVEL_ERROR, L"SYNC SessionID : %d , iDirX : %d iDirY : %d", pCharacter->dwSessionID, iDirX, iDirY);
		}


		//데드레커닝 좌표 캐릭터에 저장.
		shX = iDirX;
		shY = iDirY;


	}

	//동작을 변경. 동작번호와 방향값이 같다.
	pCharacter->dwAction = byDirection;

	//이동방향체크용
	pCharacter->MoveDirection = byDirection;

	//방향 변경
	switch ( byDirection )
	{
	case dfPACKET_MOVE_DIR_RR :
	case dfPACKET_MOVE_DIR_RU :
	case dfPACKET_MOVE_DIR_RD :
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_DD:
		pCharacter->byDirection;
		break;
	case dfPACKET_MOVE_DIR_UU:
		pCharacter->byDirection;
		break;
	}
	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//이동을 하면서 좌표가 약간 조절된 경우섹터 업데이트를 함.
	if ( Sector_UpdateCharacter (pCharacter) )
	{
		//섹터가 변경된 경우 클라에게 관련 정보를 쏜다
		CharacterSectorUpdatePacket (pCharacter);
	}

	//이동 액션이 변경되는 시점에서 데드레커닝을 위한 정보 저장.

	pCharacter->dwActionTick = timeGetTime ();
	pCharacter->shActionX = pCharacter->shX;
	pCharacter->shActionY = pCharacter->shY;

	Packet StartPack;
	StartPack.Clear ();
	
	Pack_MoveStart (&StartPack, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);
	
	//섹터단위로 접속중인 사용자에게 패킷을 뿌린다.
	SendPacket_Around (pSession, &StartPack,false);
	
	return true;

}

BOOL	PacketProc_MoveStop (st_SESSION *pSession, Packet *pack)
{
	BYTE byDirection;
	short shX, shY;

	*pack >> byDirection;
	*pack >> shX;
	*pack >> shY;

	_LOG (dfLog_LEVEL_DEBUG,L"# PACKET_MOVESTOP # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);

	//ID로 캐릭터를 검색

	st_CHARACTER *pCharacter = FindCharacter (pSession->dwSessionID);
	if ( pCharacter == NULL )
	{
		_LOG (dfLog_LEVEL_ERROR, L"MoveStop SessionID:%d Charactor Not Found!", pSession->dwSessionID);
		return false;
	}

	//서버의 위치와 받은 패킷의 위치값이 크게 다르다면 데드레커닝으로 재위치 확인.
	//그래도 좌표가 다르다면 싱크패킷을 클라로 보내서 좌표보정.
	if ( abs (pCharacter->shX - shX) > dfERROR_RANGE || abs (pCharacter->shY - shY) > dfERROR_RANGE )
	{
		int iDirX, iDirY;
		int iDeadFrame = DeadReckoningPos (pCharacter->dwAction, pCharacter->dwActionTick, pCharacter->shActionX, pCharacter->shActionY, &iDirX, &iDirY);

		if ( abs (iDirX - shX) > dfERROR_RANGE || abs (iDirY - shY) > dfERROR_RANGE )
		{
			pack->Clear ();
			Pack_Sync (pack, pCharacter->dwSessionID, iDirX, iDirY);
			SendPacket_Around (pCharacter->pSession, pack, true);

			_LOG (dfLog_LEVEL_ERROR, L"SYNC SessionID : %d , iDirX : %d iDirY : %d", pCharacter->dwSessionID, iDirX, iDirY);
		}
		shX = iDirX;
		shY = iDirY;
	}

	//동작을 변경.
	pCharacter->dwAction = dfACTION_STAND;

	//이동방향체크용
	pCharacter->MoveDirection = byDirection;

	//캐릭터가 서있는 방향
	pCharacter->byDirection = byDirection;

	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//정지를 하면서 좌표가 약간 조절된 경우섹터 업데이트를 함.
	if ( Sector_UpdateCharacter (pCharacter) )
	{
		//섹터가 변경된 경우 클라에게 관련 정보를 쏜다
		CharacterSectorUpdatePacket (pCharacter);
	}

	//이동 액션이 변경되는 시점에서 데드레커닝을 위한 정보 저장.

	pCharacter->dwActionTick = timeGetTime ();
	pCharacter->shActionX = pCharacter->shX;
	pCharacter->shActionY = pCharacter->shY;

	Packet StopPack;
	StopPack.Clear ();

	Pack_MoveStop (&StopPack, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//섹터단위로 접속중인 사용자에게 패킷을 뿌린다.
	SendPacket_Around (pSession, &StopPack,false);

	return true;



}

BOOL	PacketProc_Attack1 (st_SESSION *pSession, Packet *pack)
{
	BYTE byDirection;
	short shX, shY;

	*pack >> byDirection;
	*pack >> shX;
	*pack >> shY;

	_LOG (dfLog_LEVEL_DEBUG, L"# PACKET_ATTACK1 # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);

	st_CHARACTER *pAttacker = FindCharacter (pSession->dwSessionID);


	//클라이언트와 서버의 위치차이가 크게 난다면 클라이언트의 문제이므로 끊어버림.
	if ( abs (pAttacker->shX - shX) > dfERROR_RANGE || abs (pAttacker->shY - shY) > dfERROR_RANGE )
	{
		DisconnectSession (pSession);
		return true;
	}

	//동작 변경
	pAttacker->byDirection = byDirection;
	pAttacker->dwAction = dfACTION_ATTACK1;
	pAttacker->shX = shX;
	pAttacker->shY = shY;



	Packet Damege;
	Damege.Clear ();
	//공격 패킷 만들어서 전송.
	Pack_Attack1 (&Damege, pAttacker->dwSessionID, pAttacker->byDirection, pAttacker->shX, pAttacker->shY);
	SendPacket_Around (pSession, &Damege);


	st_CHARACTER *pCharDamege = AttackCheck (dfACTION_ATTACK1, pSession->dwSessionID);
	//데미지 입는 사람 패킷 만들어서 전송.
	if ( pCharDamege == NULL )
	{
		return true;
	}

	Damege.Clear ();
	pCharDamege->chHP -= dfATTACK1_DAMAGE;
	Pack_Damage (&Damege, pAttacker->dwSessionID, pCharDamege->dwSessionID, pCharDamege->chHP);

	SendPacket_Around (pCharDamege->pSession, &Damege, true);

	return true;
}

BOOL	PacketProc_Attack2 (st_SESSION *pSession, Packet *pack)
{
	BYTE byDirection;
	short shX, shY;

	*pack >> byDirection;
	*pack >> shX;
	*pack >> shY;

	_LOG (dfLog_LEVEL_DEBUG, L"# PACKET_ATTACK2 # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);

	st_CHARACTER *pAttacker = FindCharacter (pSession->dwSessionID);


	//클라이언트와 서버의 위치차이가 크게 난다면 클라이언트의 문제이므로 끊어버림.
	if ( abs (pAttacker->shX - shX) > dfERROR_RANGE || abs (pAttacker->shY - shY) > dfERROR_RANGE )
	{
		DisconnectSession (pSession);
		return true;
	}

	//동작 변경
	pAttacker->byDirection = byDirection;
	pAttacker->dwAction = dfACTION_ATTACK2;
	pAttacker->shX = shX;
	pAttacker->shY = shY;



	Packet Damege;
	Damege.Clear ();
	//공격 패킷 만들어서 전송.
	Pack_Attack2 (&Damege, pAttacker->dwSessionID, pAttacker->byDirection, pAttacker->shX, pAttacker->shY);
	SendPacket_Around (pSession, &Damege);


	st_CHARACTER *pCharDamege = AttackCheck (dfACTION_ATTACK2, pSession->dwSessionID);
	//데미지 입는 사람 패킷 만들어서 전송.
	if ( pCharDamege == NULL )
	{
		return true;
	}
	Damege.Clear ();
	pCharDamege->chHP -= dfATTACK2_DAMAGE;
	Pack_Damage (&Damege, pAttacker->dwSessionID, pCharDamege->dwSessionID, pCharDamege->chHP);

	SendPacket_Around (pCharDamege->pSession, &Damege, true);

	return true;
}

BOOL	PacketProc_Attack3 (st_SESSION *pSession, Packet *pack)
{
	BYTE byDirection;
	short shX, shY;

	*pack >> byDirection;
	*pack >> shX;
	*pack >> shY;

	_LOG (dfLog_LEVEL_DEBUG, L"# PACKET_ATTACK3 # SessionID:%d / Direction:%d / X:%d / Y:%d", pSession->dwSessionID, byDirection, shX, shY);

	st_CHARACTER *pAttacker = FindCharacter (pSession->dwSessionID);


	//클라이언트와 서버의 위치차이가 크게 난다면 클라이언트의 문제이므로 끊어버림.
	if ( abs (pAttacker->shX - shX) > dfERROR_RANGE || abs (pAttacker->shY - shY) > dfERROR_RANGE )
	{
		DisconnectSession (pSession);
		return true;
	}

	//동작 변경
	pAttacker->byDirection = byDirection;
	pAttacker->dwAction = dfACTION_ATTACK3;
	pAttacker->shX = shX;
	pAttacker->shY = shY;



	Packet Damege;
	Damege.Clear ();
	//공격 패킷 만들어서 전송.
	Pack_Attack3 (&Damege, pAttacker->dwSessionID, pAttacker->byDirection, pAttacker->shX, pAttacker->shY);
	SendPacket_Around (pSession, &Damege);


	st_CHARACTER *pCharDamege = AttackCheck (dfACTION_ATTACK3, pAttacker->dwSessionID);
	//데미지 입는 사람 패킷 만들어서 전송.
	if ( pCharDamege == NULL )
	{
		return true;
	}
	Damege.Clear ();
	pCharDamege->chHP -= dfATTACK3_DAMAGE;
	Pack_Damage (&Damege, pAttacker->dwSessionID, pCharDamege->dwSessionID, pCharDamege->chHP);

	SendPacket_Around (pCharDamege->pSession, &Damege, true);

	return true;
}


BOOL	PacketProc_ECHO (st_SESSION *pSession, Packet *pack)
{
	st_PACK_HEADER Header;
	Packet TimePack;
	int Time;
	
	*pack >> Time;

	TimePack.Clear ();
	Pack_ECHO (&TimePack, Time);
	SendPacket_Unicast (pSession, &TimePack);
	return true;
}



//특정섹터 한 공간에만 메시지를 보내는 함수
void SendPacket_SectorOne (int SectoriX, int SectoriY, Packet *pack, st_SESSION *pExceptSession)
{
	st_SESSION *pSession;

	//섹터 내부에서만 돌도록.
	if ( SectoriY < 0 || SectoriY >= dfSector_Max_Y )
	{
		return;
	}
	if ( SectoriX < 0 || SectoriX >= dfSector_Max_X )
	{
		return;
	}

	list<st_CHARACTER *> *pSectorList = &g_Sector[SectoriY][SectoriX];
	list<st_CHARACTER *>::iterator iter;
	for ( iter = pSectorList->begin (); iter != pSectorList->end ();)
	{

		// 해당 섹터에 모든 유저들에게 해당 패킷 발송.
		pSession = (*iter)->pSession;
		//pExceptSession은 제외하고 나머지한테만 보냄.
		if ( pSession == pExceptSession )
		{

		}
		else
		{
			pSession->SendQ.Put (( char * )pack->GetBufferPtr (), pack->GetDataSize());
		}
		iter++;
	}
	return;
}

//한명에게만 보냄
void SendPacket_Unicast (st_SESSION *pSession, Packet *pack)
{
	pSession->SendQ.Put ((char *)pack->GetBufferPtr(), pack->GetDataSize());
	return;
}

//클라이언트 기준 주변 섹터에 메세지 보내기 (최대 9개 영역)
void SendPacket_Around (st_SESSION *pSession, Packet *pack, bool bSendMe)
{
	//세션으로 클라이언트 찾기
	st_CHARACTER *pCharacter;
	pCharacter = FindCharacter (pSession->dwSessionID);

	//해당 클라이언트 주변 섹터 검색
	st_SECTOR_AROUND Pos;
	GetSectorAround (pCharacter->CurSector.iX, pCharacter->CurSector.iY, &Pos);

	//얻어낸 섹터정보를 가지고 메세지 전송
	int iCnt;
	if ( bSendMe == false )
	{
		for ( iCnt = 0; iCnt < Pos.iCount; iCnt++ )
		{
			SendPacket_SectorOne (Pos.Around[iCnt].iX, Pos.Around[iCnt].iY, pack, pSession);
		}
	}
	else
	{
		for ( iCnt = 0; iCnt < Pos.iCount; iCnt++ )
		{
			SendPacket_SectorOne (Pos.Around[iCnt].iX, Pos.Around[iCnt].iY, pack, NULL);
		}
	}
}

//모든 접속자에게 다 보내기 (시스템 메세지외 사용하지 않음)
void SendPacket_Broadcast (st_SESSION *pSession, Packet *pack)
{
}