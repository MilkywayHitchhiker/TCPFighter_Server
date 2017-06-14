#pragma once

#include<Windows.h>
#include"RingBuffer.h"
#include"Packet.h"

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <WS2tcpip.h>

//���� IP����
#define ServerIP L"127.0.0.1"

//��Ʈ ��ȣ
#define dfNETWORK_PORT		20000



//---------------------------------------------------------------
// ��Ŷ�� ���� �տ� �� ��Ŷ�ڵ�.
//---------------------------------------------------------------
#define dfNETWORK_PACKET_CODE	((BYTE)0x89)
//---------------------------------------------------------------
// ��Ŷ�� ���� �ڿ� �� ��Ŷ�ڵ�.
//---------------------------------------------------------------
#define dfNETWORK_PACKET_END	((BYTE)0x79)

//��Ŷ �ش� ����ü
struct st_NETWORK_PACKET_HEADER
{
	BYTE	byCode;			// ��Ŷ�ڵ� 0x89 ����.
	BYTE	bySize;			// ��Ŷ ������.
	BYTE	byType;			// ��ŶŸ��.
	BYTE	byTemp;			// ������.
};

#define dfNETWORK_PACKET_HEADER_SIZE	4




//���� ����ü
struct st_NETWORK_SESSION
{
	SOCKET			Socket;			// �� ������ TCP ����.
	DWORD			AccountNo;	// �������� ���� ���� ID.

	CRingbuffer	RecvQ;		// ���� ť.
	CRingbuffer	SendQ;		// �۽� ť.

	DWORD			last_HeartBeat;	// Ʈ���� üũ�� ���� ƽ.  

};


DWORD AccountNo = 1;

//���� ���� ����
extern SOCKET ListenSocket;

//��Ʈ��ũ �ʱ�ȭ
bool Network_Init (void);

//������ȣ�� �޾Ƽ� Ŭ���̾�Ʈ�� ã�Ƽ� return, �ش� Ŭ�� ���� �� NULL����.
st_NETWORK_SESSION *FindSession (SOCKET User);

//Select�𵨻��
void NetworkProcess (void);

//Select�Լ� ��� Accept,Send,Recvó��
void SelectSocket (SOCKET *UserSockTable, FD_SET *ReadSet, FD_SET *WriteSet);

//�ű� ���� Acceptó��
void Accept (void);

// ����� ����/���� �˸�.
BOOL Proc_Connect (st_NETWORK_SESSION *pNew);


//1���� �۽�
BOOL Send_Unicast (st_NETWORK_SESSION *pSession, Packet *pack);

//��ο��� �۽�
void Send_Broadcast (Packet *pack);

//���� �۽�
BOOL Send_Sector (int X, int Y, st_Charactor *pClient, Packet *pack);