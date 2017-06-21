#include"stdafx.h"
#include"Network.h"
#include"Game_Proc.h"
#include"PacketDefine.h"
#include"DefineEnum.h"

SOCKET g_ListenSock;		//���� ����

map<SOCKET, st_SESSION *> g_SessionMap;		//���� ����ü�� �����ϱ� ���� ��

//Socket���� ���� ����ü ã�Ƽ� ��ȯ ���� ������ ��� NULL ��ȯ
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


//���ο� ���� ���� �� ���
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

//�ش� ���� ����ó��
void DisconnectSession (st_SESSION *pSession)
{
	st_CHARACTER *pChar;
	Packet pack;

	_LOG (dfLog_LEVEL_DEBUG, L"Disconnect SessionID : %d", pSession->dwSessionID);

	pChar = FindCharacter (pSession->dwSessionID);
	
	//ĳ���� ���� ��Ŷ ����
	Pack_DeleteCharacter (&pack, pSession->dwSessionID);

	//�ֺ�Ÿ�Ͽ� ���� ��û �Ѹ�
	SendPacket_Around (pSession, &pack,true);

	//���Ϳ��� ����
	Sector_RemoveCharacter (pChar);

	//ĳ���� �ʿ��� ����
	g_CharacterMap.erase (pSession->dwSessionID);

	//���� �ʿ��� ����
	g_SessionMap.erase (pSession->Sock);

	delete pChar;

	//�޸� �Ҵ� ����.
	delete pSession;

	return;

}

//���� ���� ��Ʈ��ũ ó�� �Լ�
void NetworkProcess (void)
{
	st_SESSION *pSession;
	SOCKET SockTable[FD_SETSIZE] = { INVALID_SOCKET, };	//�������̺� ���� �� �ʱ�ȭ
	int iSocketCount = 0;

	FD_SET ReadSet;
	FD_SET WriteSet;

	//�ʱ�ȭ
	FD_ZERO (&ReadSet);
	FD_ZERO (&WriteSet);

	//���������� Readset�� �ְ� ����
	FD_SET (g_ListenSock, &ReadSet);
	SockTable[iSocketCount] = g_ListenSock;
	iSocketCount++;

	map<SOCKET, st_SESSION *>::iterator iter;
	for ( iter = g_SessionMap.begin (); iter != g_SessionMap.end ();)
	{
		pSession = iter->second;

		SockTable[iSocketCount] = pSession->Sock;

		//ReadSet�� �ش� ���� ���� ���
		FD_SET (pSession->Sock, &ReadSet);
		//SendQ�� �����Ͱ� �ִٸ� ���� �����Ͱ� �ִ°��̹Ƿ� WriteSet�� ���
		if ( pSession->SendQ.GetUseSize () > 0 )
		{
			FD_SET (pSession->Sock, &WriteSet);
		}

		iSocketCount++;
		iter++;



		//Select�� FD_SETSIZE �� �����ߴٸ� Selectȣ�� �� ����
		if ( FD_SETSIZE <= iSocketCount )
		{
			NetworkSelectProc (SockTable, &ReadSet, &WriteSet);

			//�ʱ�ȭ
			FD_ZERO (&ReadSet);
			FD_ZERO (&WriteSet);

			memset (SockTable, INVALID_SOCKET, sizeof (SOCKET) * FD_SETSIZE);


			//���� �����忡�� select ����� ��� ������ õ�� �̻��� ���� ������ ������ ���ϰ� �ɸ��� ��� ������ ó���� �������⶧����
			//�ѹ� ���� �������� ���������� �Ź� üũ�� �ش�.
			FD_SET (g_ListenSock, &ReadSet);
			SockTable[0] = g_ListenSock;
			iSocketCount = 1;
		}



	}

	//for���� �� ���Ҵµ� iSocketCount�� 0���� ũ�ٸ� �˻��ؾߵ� ������ �����ִ°��̹Ƿ� ���������� �ѹ��� Select�Լ� ȣ��
	if ( iSocketCount > 0 )
	{
		NetworkSelectProc (SockTable, &ReadSet, &WriteSet);
	}

}

//Seslct �� üũ �Լ�
void NetworkSelectProc (SOCKET *pSockTable, FD_SET *pReadSet, FD_SET *pWriteSet)
{
	timeval Time;

	int iResult;
	int iCnt;
	bool ProcFlag;

	Time.tv_sec = 0;
	Time.tv_usec = 0;

	iResult = select (0, pReadSet, pWriteSet, 0, &Time);


	//���ϰ��� 0���� ũ�ٸ� �����Ͱ� ���ŵ� ���̴�.
	if ( iResult > 0 )
	{
		for ( iCnt = 0; iResult > 0 && iCnt < FD_SETSIZE; iCnt++ )
		{
			ProcFlag = true;
			if ( pSockTable[iCnt] == INVALID_SOCKET )
			{
				continue;
			}

			//WriteSet üũ
			if ( FD_ISSET (pSockTable[iCnt], pWriteSet) )
			{
				ProcFlag = NetworkSend (pSockTable[iCnt]);
				iResult--;
			}

			if ( FD_ISSET (pSockTable[iCnt], pReadSet) )
			{

				//NetworkSend �κп��� ������ ��Ȳ���� �ش� Ŭ���̾�Ʈ�� �������Ḧ �� ��찡 �ֱ⿡ ProcFlag�� Ȯ�� �� Recv ����
				if ( ProcFlag )
				{
					//���������� ��� ���� ó��
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
		//���� ó��
		wprintf (L"select Error\n");
	}



}

//Recvó��
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

	//���������� ���� �޼��� Ÿ��
	pSession->dwHeartBeat = timeGetTime ();

	//�ޱ� �۾�

	iBuffSize = pSession->RecvQ.GetNotBrokenPutSize ();

	iResult = recv (pSession->Sock, pSession->RecvQ.GetWriteBufferPtr (), iBuffSize, 0);

	//recv����� ���� ������ ��� ���� ����.
	if ( SOCKET_ERROR == iResult || 0 == iResult)
	{
		DisconnectSession (pSession);
		return;
	}

	//���� �����Ͱ� �ִٸ�
	if ( 0 < iResult )
	{
		//RecvQ�� �����͸� �־����� WritePos�� �̵�������� �ȴ�.
		pSession->RecvQ.MoveWritePos (iResult);

		//��Ŷ�� �ϳ� �̻� ���ŵǾ��� �� �����Ƿ� ��� ó���� �� ���� ��� �ݺ��Ѵ�.
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

//Sendó��
bool NetworkSend (SOCKET sock)
{
	st_SESSION *pSession;
	int iResult;
	int iSendSize;

	//�ش� ����� ���� ã��

	pSession = FindSession (sock);
	//ã�� ����ڰ� ���ٸ� �׳� ����.
	if ( NULL == pSession )
	{
		return false;
	}

	//SendQ�� �ִ� �����͵��� �ִ� dfMaxSendSize ���Ϸ� ������.
	//�̴� �Ѱ��� ��Ŷ�� ���� �� �ִ� �ִ� MTU�������̱� �����̴�.
	//�̸� �Ѿ�� �����߰��� ��Ŷ�� �и��Ǹ� �����Ͱ� ���� ����� �ִ�.
	iSendSize = pSession->SendQ.GetNotBrokenGetSize ();
	//iSendSize = min (dfMaxSendSize, iSendSize);

	//SendQ�� ���� �����Ͱ� ������ �׳� ��������.
	if ( 0 >= iSendSize )
	{
		return true;
	}

	//sendó��
	iResult = send (pSession->Sock, pSession->SendQ.GetReadBufferPtr (), iSendSize, 0);
	pSession->SendQ.RemoveData (iResult);

	//send���� ���Ͽ����� ��ٸ� �ش� Ŭ���̾�Ʈ ��Ŀ��Ʈ ó��
	if ( iResult == SOCKET_ERROR )
	{
		_LOG (dfLog_LEVEL_ERROR, L"##Send_SOCKET_ERROR ID : %d", pSession->dwSessionID);
		DisconnectSession (pSession);
		return false;
	}
	else if ( iSendSize < iResult )
	{
		//���� ���� ������� ������ �� ũ�ٸ� �����⶧���� ������ �����ָ� �ȴ�.
		//����� �ȵǴ� ��Ȳ������ ���� �̷� ��찡 ���涧�� �ִ�.
		DisconnectSession (pSession);
		return false;
	}

	return true;
}

//��Ʈ��ũ �ʱ�ȭ
void Network_Init (void)
{
	WSAData wsa;
	int retval;

	//���� �ʱ�ȭ
	if ( WSAStartup (MAKEWORD (2, 2), &wsa) != 0 )
	{
		return;
	}

	//���� �ʱ�ȭ
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
	//��Ʈ��ũ ���ÿϷ�.
	return;
}

//����� ���� �̺�Ʈ ó��
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

	//inet_ntoa�� ��Ƽ����Ʈ���ڿ��̱� ������ �����ڵ�� �ٲ���� �ȴ�.
	wchar_t strUnicode[256] = { 0, };
	char	*strMultibyte;
	strMultibyte = inet_ntoa (addr.sin_addr);

	int nLen = MultiByteToWideChar (CP_ACP, 0, strMultibyte, strlen (strMultibyte), NULL, NULL);
	MultiByteToWideChar (CP_ACP, 0, strMultibyte, strlen (strMultibyte), strUnicode, nLen);


	_LOG (dfLog_LEVEL_DEBUG,L"Connect # IP:%s SessionID:%d", strUnicode, g_SessionID);


	return true;

}

//��Ŷ�� �Ϸ�Ǿ����� �˻� �� ��Ŷ ó��
int RecvPacket (st_SESSION *pSession)
{
	st_PACK_HEADER PacketHeader;
	int iRecvQSize;
	BYTE byEndCode;

	iRecvQSize = pSession->RecvQ.GetUseSize ();

	//���� ����� ��Ŷ������� �۴ٸ� �׳� ����
	if ( sizeof (st_PACK_HEADER) > iRecvQSize )
	{
		return 1;
	}

	// 1. PacketCode�˻�
	pSession->RecvQ.Peek (( char * )&PacketHeader, sizeof (st_PACK_HEADER));

	//Peek�� �̾Ƽ� ��� �ڵ尡 �´��� �˻�. Ʋ���� ����
	if ( dfPACKET_CODE != PacketHeader.byCode )
	{
		return -1;
	}

	// 2. ť�� ����� �����Ͱ� ��Ŷ�� ũ�⸸ŭ �ִ��� Ȯ�� EndCode ũ�� ���ؼ� ����Ұ�.
	if ( PacketHeader.bySize + sizeof (st_PACK_HEADER) + 1 > iRecvQSize )
	{
		return 1;
	}

	//������ ����� Peek�� �̾����Ƿ� ť���� ��� ������ ��ŭ ������ ��.
	pSession->RecvQ.RemoveData (sizeof (st_PACK_HEADER));
	
	Packet pack;
	pack.Clear ();
	//Payload �κ��� ���۷� ����.
	if ( !pSession->RecvQ.Get (pack.GetBufferPtr (), PacketHeader.bySize))
	{
		return -1;
	}

	//EndCode�� ���۷� ���� Ȯ��.
	if ( !pSession->RecvQ.Get (( char * )&byEndCode, 1) )
	{
		return -1;
	}
	if ( byEndCode != dfNETWORK_PACKET_END )
	{
		return -1;
	}

	//��Ŷ ���� �����͸� ��� ���Ƿ� �����͸� �־����Ƿ� ������ �̵�ó��
	pack.MoveWritePos (PacketHeader.bySize);

	//��Ŷó�� �Լ� ȣ��
	if ( !PacketProc (pSession, PacketHeader.byType, &pack) )
	{
		return -1;
	}
	return 0;


}

//��Ŷ Ÿ�Կ� ���� ó�� �Լ� ȣ��
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


// ����� ����/���� �˸�.
BOOL	PacketProc_Connect (st_SESSION *pSession)
{
	Packet	pack;
	pack.Clear ();
	_LOG (dfLog_LEVEL_DEBUG,L"# PACKET_CONNECT # SessionID:%d", pSession->dwSessionID);


	//-----------------------------------------------------
	// ���ο� ����� ������ ó���Ѵ�.
	//-----------------------------------------------------
	gameCreatePlayer (pSession);

	st_SECTOR_AROUND Pos;
	st_CHARACTER *pChar = FindCharacter (pSession->dwSessionID);
	st_CHARACTER *pOtherChar;


	list<st_CHARACTER *> *pSectorList;
	list<st_CHARACTER *>::iterator iter;
	//�ش� Ŭ���̾�Ʈ �ֺ� ���� �˻�
	GetSectorAround (pChar->CurSector.iX, pChar->CurSector.iY, &Pos);
	
	//�� ���������� ������ �޼��� ����


	pack.Clear ();
	//������ �� �������� �˸�
	Pack_CreateMyCharacter (&pack, pChar->dwSessionID, pChar->byDirection, pChar->shX, pChar->shY, pChar->chHP);
	SendPacket_Unicast (pSession, &pack);

	
	//�ڽ����� ���� �� �ٸ� �������� ��� ����
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
	//�ٸ� ���������� ���� �������� �˸�
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

	//ID�� ĳ���͸� �˻�

	st_CHARACTER *pCharacter = FindCharacter (pSession->dwSessionID);
	if ( pCharacter == NULL )
	{
		_LOG (dfLog_LEVEL_ERROR, L"MoveStart SessionID:%d Charactor Not Found!", pSession->dwSessionID);
		return false;
	}

	//������ ��ġ�� ���� ��Ŷ�� ��ġ���� ũ�� �ٸ��ٸ� ���巹Ŀ������ ����ġ Ȯ��.
	//�׷��� ��ǥ�� �ٸ��ٸ� ��ũ��Ŷ�� Ŭ��� ������ ��ǥ����.
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


		//���巹Ŀ�� ��ǥ ĳ���Ϳ� ����.
		shX = iDirX;
		shY = iDirY;


	}

	//������ ����. ���۹�ȣ�� ���Ⱚ�� ����.
	pCharacter->dwAction = byDirection;

	//�̵�����üũ��
	pCharacter->MoveDirection = byDirection;

	//���� ����
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

	//�̵��� �ϸ鼭 ��ǥ�� �ణ ������ ��켽�� ������Ʈ�� ��.
	if ( Sector_UpdateCharacter (pCharacter) )
	{
		//���Ͱ� ����� ��� Ŭ�󿡰� ���� ������ ���
		CharacterSectorUpdatePacket (pCharacter);
	}

	//�̵� �׼��� ����Ǵ� �������� ���巹Ŀ���� ���� ���� ����.

	pCharacter->dwActionTick = timeGetTime ();
	pCharacter->shActionX = pCharacter->shX;
	pCharacter->shActionY = pCharacter->shY;

	Packet StartPack;
	StartPack.Clear ();
	
	Pack_MoveStart (&StartPack, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);
	
	//���ʹ����� �������� ����ڿ��� ��Ŷ�� �Ѹ���.
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

	//ID�� ĳ���͸� �˻�

	st_CHARACTER *pCharacter = FindCharacter (pSession->dwSessionID);
	if ( pCharacter == NULL )
	{
		_LOG (dfLog_LEVEL_ERROR, L"MoveStop SessionID:%d Charactor Not Found!", pSession->dwSessionID);
		return false;
	}

	//������ ��ġ�� ���� ��Ŷ�� ��ġ���� ũ�� �ٸ��ٸ� ���巹Ŀ������ ����ġ Ȯ��.
	//�׷��� ��ǥ�� �ٸ��ٸ� ��ũ��Ŷ�� Ŭ��� ������ ��ǥ����.
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

	//������ ����.
	pCharacter->dwAction = dfACTION_STAND;

	//�̵�����üũ��
	pCharacter->MoveDirection = byDirection;

	//ĳ���Ͱ� ���ִ� ����
	pCharacter->byDirection = byDirection;

	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//������ �ϸ鼭 ��ǥ�� �ణ ������ ��켽�� ������Ʈ�� ��.
	if ( Sector_UpdateCharacter (pCharacter) )
	{
		//���Ͱ� ����� ��� Ŭ�󿡰� ���� ������ ���
		CharacterSectorUpdatePacket (pCharacter);
	}

	//�̵� �׼��� ����Ǵ� �������� ���巹Ŀ���� ���� ���� ����.

	pCharacter->dwActionTick = timeGetTime ();
	pCharacter->shActionX = pCharacter->shX;
	pCharacter->shActionY = pCharacter->shY;

	Packet StopPack;
	StopPack.Clear ();

	Pack_MoveStop (&StopPack, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//���ʹ����� �������� ����ڿ��� ��Ŷ�� �Ѹ���.
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


	//Ŭ���̾�Ʈ�� ������ ��ġ���̰� ũ�� ���ٸ� Ŭ���̾�Ʈ�� �����̹Ƿ� �������.
	if ( abs (pAttacker->shX - shX) > dfERROR_RANGE || abs (pAttacker->shY - shY) > dfERROR_RANGE )
	{
		DisconnectSession (pSession);
		return true;
	}

	//���� ����
	pAttacker->byDirection = byDirection;
	pAttacker->dwAction = dfACTION_ATTACK1;
	pAttacker->shX = shX;
	pAttacker->shY = shY;



	Packet Damege;
	Damege.Clear ();
	//���� ��Ŷ ���� ����.
	Pack_Attack1 (&Damege, pAttacker->dwSessionID, pAttacker->byDirection, pAttacker->shX, pAttacker->shY);
	SendPacket_Around (pSession, &Damege);


	st_CHARACTER *pCharDamege = AttackCheck (dfACTION_ATTACK1, pSession->dwSessionID);
	//������ �Դ� ��� ��Ŷ ���� ����.
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


	//Ŭ���̾�Ʈ�� ������ ��ġ���̰� ũ�� ���ٸ� Ŭ���̾�Ʈ�� �����̹Ƿ� �������.
	if ( abs (pAttacker->shX - shX) > dfERROR_RANGE || abs (pAttacker->shY - shY) > dfERROR_RANGE )
	{
		DisconnectSession (pSession);
		return true;
	}

	//���� ����
	pAttacker->byDirection = byDirection;
	pAttacker->dwAction = dfACTION_ATTACK2;
	pAttacker->shX = shX;
	pAttacker->shY = shY;



	Packet Damege;
	Damege.Clear ();
	//���� ��Ŷ ���� ����.
	Pack_Attack2 (&Damege, pAttacker->dwSessionID, pAttacker->byDirection, pAttacker->shX, pAttacker->shY);
	SendPacket_Around (pSession, &Damege);


	st_CHARACTER *pCharDamege = AttackCheck (dfACTION_ATTACK2, pSession->dwSessionID);
	//������ �Դ� ��� ��Ŷ ���� ����.
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


	//Ŭ���̾�Ʈ�� ������ ��ġ���̰� ũ�� ���ٸ� Ŭ���̾�Ʈ�� �����̹Ƿ� �������.
	if ( abs (pAttacker->shX - shX) > dfERROR_RANGE || abs (pAttacker->shY - shY) > dfERROR_RANGE )
	{
		DisconnectSession (pSession);
		return true;
	}

	//���� ����
	pAttacker->byDirection = byDirection;
	pAttacker->dwAction = dfACTION_ATTACK3;
	pAttacker->shX = shX;
	pAttacker->shY = shY;



	Packet Damege;
	Damege.Clear ();
	//���� ��Ŷ ���� ����.
	Pack_Attack3 (&Damege, pAttacker->dwSessionID, pAttacker->byDirection, pAttacker->shX, pAttacker->shY);
	SendPacket_Around (pSession, &Damege);


	st_CHARACTER *pCharDamege = AttackCheck (dfACTION_ATTACK3, pAttacker->dwSessionID);
	//������ �Դ� ��� ��Ŷ ���� ����.
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



//Ư������ �� �������� �޽����� ������ �Լ�
void SendPacket_SectorOne (int SectoriX, int SectoriY, Packet *pack, st_SESSION *pExceptSession)
{
	st_SESSION *pSession;

	//���� ���ο����� ������.
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

		// �ش� ���Ϳ� ��� �����鿡�� �ش� ��Ŷ �߼�.
		pSession = (*iter)->pSession;
		//pExceptSession�� �����ϰ� ���������׸� ����.
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

//�Ѹ��Ը� ����
void SendPacket_Unicast (st_SESSION *pSession, Packet *pack)
{
	pSession->SendQ.Put ((char *)pack->GetBufferPtr(), pack->GetDataSize());
	return;
}

//Ŭ���̾�Ʈ ���� �ֺ� ���Ϳ� �޼��� ������ (�ִ� 9�� ����)
void SendPacket_Around (st_SESSION *pSession, Packet *pack, bool bSendMe)
{
	//�������� Ŭ���̾�Ʈ ã��
	st_CHARACTER *pCharacter;
	pCharacter = FindCharacter (pSession->dwSessionID);

	//�ش� Ŭ���̾�Ʈ �ֺ� ���� �˻�
	st_SECTOR_AROUND Pos;
	GetSectorAround (pCharacter->CurSector.iX, pCharacter->CurSector.iY, &Pos);

	//�� ���������� ������ �޼��� ����
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

//��� �����ڿ��� �� ������ (�ý��� �޼����� ������� ����)
void SendPacket_Broadcast (st_SESSION *pSession, Packet *pack)
{
}