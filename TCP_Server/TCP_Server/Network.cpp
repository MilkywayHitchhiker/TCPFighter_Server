#include"stdafx.h"
#include"Network.h"
#include"Packet.h"
#include"PacketDefine.h"
#include"Game_Proc.h"
#include"Create_Packet.h"
#include"Sector.h"




#include<map>

using namespace std;

map<SOCKET, st_NETWORK_SESSION *> g_Session;	//리슨소켓 접속시 이곳에서 대기



//네트워크 초기화
bool Network_Init (void)
{
	WSAData wsa;
	int retval;

	//윈속 초기화
	if ( WSAStartup (MAKEWORD (2, 2), &wsa) != 0 )
	{
		return false;
	}

	//소켓 초기화
	ListenSocket = socket (AF_INET, SOCK_STREAM, 0);
	if ( ListenSocket == INVALID_SOCKET )
	{
		return false;
	}

	//bind
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	InetPton (AF_INET, ServerIP, &addr.sin_addr);
	addr.sin_port = htons (dfNETWORK_PORT);

	retval = bind (ListenSocket, ( SOCKADDR * )&addr, sizeof (addr));
	if ( retval == SOCKET_ERROR )
	{
		return false;
	}

	//listen
	retval = listen (ListenSocket, SOMAXCONN);
	if ( retval == SOCKET_ERROR )
	{
		return false;
	}

	int opt_val = TRUE;
	setsockopt (ListenSocket, IPPROTO_TCP, TCP_NODELAY, ( char * )&opt_val, sizeof (opt_val));

	//네트워크 셋팅완료.
	return true;
}


//유저번호를 받아서 클라이언트를 찾아서 return, 해당 클라 없을 시 NULL리턴.
st_NETWORK_SESSION *FindSession (SOCKET sock)
{
	map<SOCKET, st_NETWORK_SESSION *>::iterator iter;
	iter = g_Session.find (sock);

	//Client검색 했는데 해당 유저가 없다면 NULL리턴.
	if ( iter == g_Session.end () )
	{
		return NULL;
	}

	return iter->second;
}


//Select모델사용
void NetworkProcess (void)
{
	SOCKET UserTable_SOCKET[FD_SETSIZE];

	FD_SET ReadSet;
	FD_SET WriteSet;
	int iSocketCount = 0;

	//초기화
	FD_ZERO (&ReadSet);
	FD_ZERO (&WriteSet);

	memset (UserTable_SOCKET, INVALID_SOCKET, sizeof (SOCKET) * FD_SETSIZE);

	//리슨소켓을 Readset에 넣고 시작
	FD_SET (ListenSocket, &ReadSet);
	UserTable_SOCKET[iSocketCount] = ListenSocket;
	iSocketCount++;

	map<SOCKET, st_NETWORK_SESSION *>::iterator iter;
	for ( iter = g_Session.begin (); iter != g_Session.end ();)
	{

		UserTable_SOCKET[iSocketCount] = iter->first;

		//ReadSet에 수신된 데이터가 있는지 확인하기 위해서 ReadSet에 소켓 삽입

		FD_SET (UserTable_SOCKET[iSocketCount], &ReadSet);

		//현재 유저의 SendQ에 UseSize가 0보다 크다면 보낼 데이터가 있는 것으로 판단. WriteSet에 소켓 삽입
		if ( iter->second->SendQ.GetUseSize () > 0 )
		{
			FD_SET (UserTable_SOCKET[iSocketCount], &WriteSet);
		}

		iSocketCount++;

		iter++;


		//ReadSet에 소켓이 64개가 들어갔다면 Select모델 사용. Accept,Send,Recv 처리
		if ( FD_SETSIZE <= iSocketCount )
		{
			SelectSocket (UserTable_SOCKET, &ReadSet, &WriteSet);


			//소켓 초기화
			FD_ZERO (&ReadSet);
			FD_ZERO (&WriteSet);

			memset (UserTable_SOCKET, INVALID_SOCKET, sizeof (SOCKET) * FD_SETSIZE);

			iSocketCount = 0;


		}

	}

	//iSocketCount가 0보다 크다면 처리되지 못한 사용자들이 남았으므로 추가 Select 처리 후 종료.
	if ( iSocketCount > 0 )
	{
		SelectSocket (UserTable_SOCKET, &ReadSet, &WriteSet);

	}


	//모든 접속자 목록을 돌면서 INVALID_SOCKET으로 만든 접속자들 삭제.
	Disconnect_Client (-1);


}


//Select함수 사용 Accept,Send,Recv처리
void SelectSocket (SOCKET *UserSockTable, FD_SET *ReadSet, FD_SET *WriteSet)
{
	timeval Time;
	int iResult, iCnt;
	SOCKET Table[FD_SETSIZE];
	memcpy (Table, UserSockTable, FD_SETSIZE);

	//Select 대기시간 설정
	Time.tv_sec = 0;
	Time.tv_usec = 0;
	//접속자 요청과 접속중인 클라이언트들의 메세지 체크
	iResult = select (0, ReadSet, WriteSet, 0, &Time);

	//iResult가 0보다 크다면 클라이언트가 보낸 데이터가 수신. 혹은 보내야될 데이터가 있는것이므로 체크
	if ( 0 < iResult )
	{

		//최대 0~63까지 돌면서 수신 체크
		for ( iCnt = 0; iCnt < FD_SETSIZE; iCnt++ )
		{
			if ( Table[iCnt] == INVALID_SOCKET )
			{
				continue;
			}

			//ReadSet 체크

			if ( FD_ISSET (Table[iCnt], ReadSet) )
			{
				//ListenSocket이 아니면 새로운 접속자 이므로 Accept처리
				if ( Table[iCnt] == ListenSocket )
				{
					Accept ();

				}
				else
				{
					//Recv 처리
					Network_Recv (Table[iCnt]);
				}

			}




			//WriteSet 체크
			if ( FD_ISSET (Table[iCnt], WriteSet) )
			{
				//Send 처리
				Network_Send (Table[iCnt]);

			}



		}


	}
	else if ( iResult == SOCKET_ERROR )
	{
		wprintf (L"Select Error\n");
	}


	return;
}


//신규 유저 Accept처리
void Accept (void)
{
	SOCKET NewUser;
	SOCKADDR_IN addr;


	int addrlen = sizeof (addr);


	//accept처리
	NewUser = accept (ListenSocket, ( SOCKADDR * )&addr, &addrlen);


	//신규접속이 제대로된 소켓이 아니라면 정리하고 종료
	if ( NewUser == INVALID_SOCKET )
	{
		return;
	}

	st_NETWORK_SESSION *pNew = new st_NETWORK_SESSION;
	pNew->AccountNo = AccountNo;		//나중에 로그인 요청시 처리할 부분.
	pNew->Socket = NewUser;

	pNew->RecvQ.ClearBuffer ();
	pNew->SendQ.ClearBuffer ();


	AccountNo++;


	g_Session.insert (pair<SOCKET, st_NETWORK_SESSION *> (NewUser, pNew));	//키로 AcceptNo를 넣고 value로 st_Client 주소 저장. 

	//신규접속 알림처리
	Proc_Connect (pNew);

	wprintf (L"Accept = IP : %d.%d.%d.%d Port : %d\n", addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2, addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4, addr.sin_port);

	return;
}


// 사용자 접속/해지 알림.
BOOL	Proc_Connect (st_NETWORK_SESSION *pNew)
{
	Packet pack;
	
	//-----------------------------------------------------
	// 새로운 사용자 접속을 처리한다.
	//-----------------------------------------------------
	Create_Player (pNew);




	// 기존에 있던 사용자들의 목록을 신규 유저에게 전송
	map<DWORD, st_Charactor *>::iterator iter;
	for ( iter = g_Charactor.begin(); iter != g_Charactor.end();  )
	{
		//-----------------------------------------------------
		// 사용자 목록을 모두 뽑아서, 패킷을 만들어 해당 사용자에게 전송 !
		// 단, 방금 접속한 사용자 에게는 보내지 않는다.
		//-----------------------------------------------------
		st_Charactor *pCharactor = iter->second;

		if ( pCharactor->AccountNo != pNew->AccountNo )
		{
			pack.Clear ();

			Create_OtherCharactor (&pack, pCharactor);

			pNew->SendQ.Put (pack.GetBufferPtr (), pack.GetDataSize ());
		}
		iter++;
	}
	return TRUE;
}

//1명에게만 보냄.
BOOL	Send_Unicast (st_NETWORK_SESSION *pSession, Packet *pack)
{
	if ( NULL == pSession )
		return FALSE;

	pSession->SendQ.Put (pack->GetBufferPtr (), pack->GetDataSize ());

	return TRUE;
}

//모두에게 송신
BOOL Send_Broadcast (Packet *pack)
{
	st_NETWORK_SESSION *pSession;

	map<SOCKET, st_NETWORK_SESSION *>::iterator iter;
	for ( iter = g_Session.begin (); iter != g_Session.end (); )
	{
		pSession = iter->second;
		pSession->SendQ.Put (pack->GetBufferPtr (), pack->GetDataSize ());
		iter++;
	}
}

//섹터 송신
BOOL Send_Sector (int X, int Y, st_Charactor *pClient, Packet *pack)
{
	if ( X > 0 || X <= dfSector_Max_X )
	{
		return false;
	}
	if ( Y > 0 || Y <= dfSector_Max_Y )
	{
		return false;
	}
	
	st_Charactor *pSession;

	list<st_Charactor *>::iterator iter;
	for ( iter = g_Sector[Y][X].begin (); iter != g_Sector[Y][X].end ();)
	{
		pSession = *iter;
		pSession->Session->SendQ.Put (pack->GetBufferPtr (), pack->GetDataSize ());
	}
	return true;
}