#include"stdafx.h"
#include"Network.h"
#include"Packet.h"
#include"PacketDefine.h"
#include"Game_Proc.h"
#include"Create_Packet.h"
#include"Sector.h"




#include<map>

using namespace std;

map<SOCKET, st_NETWORK_SESSION *> g_Session;	//�������� ���ӽ� �̰����� ���



//��Ʈ��ũ �ʱ�ȭ
bool Network_Init (void)
{
	WSAData wsa;
	int retval;

	//���� �ʱ�ȭ
	if ( WSAStartup (MAKEWORD (2, 2), &wsa) != 0 )
	{
		return false;
	}

	//���� �ʱ�ȭ
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

	//��Ʈ��ũ ���ÿϷ�.
	return true;
}


//������ȣ�� �޾Ƽ� Ŭ���̾�Ʈ�� ã�Ƽ� return, �ش� Ŭ�� ���� �� NULL����.
st_NETWORK_SESSION *FindSession (SOCKET sock)
{
	map<SOCKET, st_NETWORK_SESSION *>::iterator iter;
	iter = g_Session.find (sock);

	//Client�˻� �ߴµ� �ش� ������ ���ٸ� NULL����.
	if ( iter == g_Session.end () )
	{
		return NULL;
	}

	return iter->second;
}


//Select�𵨻��
void NetworkProcess (void)
{
	SOCKET UserTable_SOCKET[FD_SETSIZE];

	FD_SET ReadSet;
	FD_SET WriteSet;
	int iSocketCount = 0;

	//�ʱ�ȭ
	FD_ZERO (&ReadSet);
	FD_ZERO (&WriteSet);

	memset (UserTable_SOCKET, INVALID_SOCKET, sizeof (SOCKET) * FD_SETSIZE);

	//���������� Readset�� �ְ� ����
	FD_SET (ListenSocket, &ReadSet);
	UserTable_SOCKET[iSocketCount] = ListenSocket;
	iSocketCount++;

	map<SOCKET, st_NETWORK_SESSION *>::iterator iter;
	for ( iter = g_Session.begin (); iter != g_Session.end ();)
	{

		UserTable_SOCKET[iSocketCount] = iter->first;

		//ReadSet�� ���ŵ� �����Ͱ� �ִ��� Ȯ���ϱ� ���ؼ� ReadSet�� ���� ����

		FD_SET (UserTable_SOCKET[iSocketCount], &ReadSet);

		//���� ������ SendQ�� UseSize�� 0���� ũ�ٸ� ���� �����Ͱ� �ִ� ������ �Ǵ�. WriteSet�� ���� ����
		if ( iter->second->SendQ.GetUseSize () > 0 )
		{
			FD_SET (UserTable_SOCKET[iSocketCount], &WriteSet);
		}

		iSocketCount++;

		iter++;


		//ReadSet�� ������ 64���� ���ٸ� Select�� ���. Accept,Send,Recv ó��
		if ( FD_SETSIZE <= iSocketCount )
		{
			SelectSocket (UserTable_SOCKET, &ReadSet, &WriteSet);


			//���� �ʱ�ȭ
			FD_ZERO (&ReadSet);
			FD_ZERO (&WriteSet);

			memset (UserTable_SOCKET, INVALID_SOCKET, sizeof (SOCKET) * FD_SETSIZE);

			iSocketCount = 0;


		}

	}

	//iSocketCount�� 0���� ũ�ٸ� ó������ ���� ����ڵ��� �������Ƿ� �߰� Select ó�� �� ����.
	if ( iSocketCount > 0 )
	{
		SelectSocket (UserTable_SOCKET, &ReadSet, &WriteSet);

	}


	//��� ������ ����� ���鼭 INVALID_SOCKET���� ���� �����ڵ� ����.
	Disconnect_Client (-1);


}


//Select�Լ� ��� Accept,Send,Recvó��
void SelectSocket (SOCKET *UserSockTable, FD_SET *ReadSet, FD_SET *WriteSet)
{
	timeval Time;
	int iResult, iCnt;
	SOCKET Table[FD_SETSIZE];
	memcpy (Table, UserSockTable, FD_SETSIZE);

	//Select ���ð� ����
	Time.tv_sec = 0;
	Time.tv_usec = 0;
	//������ ��û�� �������� Ŭ���̾�Ʈ���� �޼��� üũ
	iResult = select (0, ReadSet, WriteSet, 0, &Time);

	//iResult�� 0���� ũ�ٸ� Ŭ���̾�Ʈ�� ���� �����Ͱ� ����. Ȥ�� �����ߵ� �����Ͱ� �ִ°��̹Ƿ� üũ
	if ( 0 < iResult )
	{

		//�ִ� 0~63���� ���鼭 ���� üũ
		for ( iCnt = 0; iCnt < FD_SETSIZE; iCnt++ )
		{
			if ( Table[iCnt] == INVALID_SOCKET )
			{
				continue;
			}

			//ReadSet üũ

			if ( FD_ISSET (Table[iCnt], ReadSet) )
			{
				//ListenSocket�� �ƴϸ� ���ο� ������ �̹Ƿ� Acceptó��
				if ( Table[iCnt] == ListenSocket )
				{
					Accept ();

				}
				else
				{
					//Recv ó��
					Network_Recv (Table[iCnt]);
				}

			}




			//WriteSet üũ
			if ( FD_ISSET (Table[iCnt], WriteSet) )
			{
				//Send ó��
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


//�ű� ���� Acceptó��
void Accept (void)
{
	SOCKET NewUser;
	SOCKADDR_IN addr;


	int addrlen = sizeof (addr);


	//acceptó��
	NewUser = accept (ListenSocket, ( SOCKADDR * )&addr, &addrlen);


	//�ű������� ����ε� ������ �ƴ϶�� �����ϰ� ����
	if ( NewUser == INVALID_SOCKET )
	{
		return;
	}

	st_NETWORK_SESSION *pNew = new st_NETWORK_SESSION;
	pNew->AccountNo = AccountNo;		//���߿� �α��� ��û�� ó���� �κ�.
	pNew->Socket = NewUser;

	pNew->RecvQ.ClearBuffer ();
	pNew->SendQ.ClearBuffer ();


	AccountNo++;


	g_Session.insert (pair<SOCKET, st_NETWORK_SESSION *> (NewUser, pNew));	//Ű�� AcceptNo�� �ְ� value�� st_Client �ּ� ����. 

	//�ű����� �˸�ó��
	Proc_Connect (pNew);

	wprintf (L"Accept = IP : %d.%d.%d.%d Port : %d\n", addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2, addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4, addr.sin_port);

	return;
}


// ����� ����/���� �˸�.
BOOL	Proc_Connect (st_NETWORK_SESSION *pNew)
{
	Packet pack;
	
	//-----------------------------------------------------
	// ���ο� ����� ������ ó���Ѵ�.
	//-----------------------------------------------------
	Create_Player (pNew);




	// ������ �ִ� ����ڵ��� ����� �ű� �������� ����
	map<DWORD, st_Charactor *>::iterator iter;
	for ( iter = g_Charactor.begin(); iter != g_Charactor.end();  )
	{
		//-----------------------------------------------------
		// ����� ����� ��� �̾Ƽ�, ��Ŷ�� ����� �ش� ����ڿ��� ���� !
		// ��, ��� ������ ����� ���Դ� ������ �ʴ´�.
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

//1���Ը� ����.
BOOL	Send_Unicast (st_NETWORK_SESSION *pSession, Packet *pack)
{
	if ( NULL == pSession )
		return FALSE;

	pSession->SendQ.Put (pack->GetBufferPtr (), pack->GetDataSize ());

	return TRUE;
}

//��ο��� �۽�
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

//���� �۽�
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