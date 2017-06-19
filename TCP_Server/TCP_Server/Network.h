#pragma once
#include"RingBuffer.h"
#include"Packet.h"
#include"main.h"
#include"CreatePacket.h"



using namespace std;

#define dfNETWORK_PACKET_RECV_TIMEOUT 20000
#define ServerIP L"127.0.0.1"
#define dfNETWORK_PORT 20000

#define dfMaxSendSize 1460

struct st_SESSION
{
	SOCKET Sock;			//������ ����
	DWORD dwSessionID;		//������ ���� ���� ID
	CRingbuffer SendQ;		
	CRingbuffer RecvQ;		
	DWORD dwHeartBeat;		//������ ��Ŷ ���Žð�
};

extern SOCKET g_ListenSock;		//���� ����

extern map<SOCKET, st_SESSION *> g_SessionMap;		//���� ����ü�� �����ϱ� ���� ��

//Socket���� ���� ����ü ã�Ƽ� ��ȯ ���� ������ ��� NULL ��ȯ
st_SESSION *FindSession (SOCKET sock);


//���ο� ���� ���� �� ���
st_SESSION *CreateSession (SOCKET Sock);

//�ش� ���� ����ó��
void DisconnectSession (st_SESSION *pSesssion);

//��Ʈ��ũ �ʱ�ȭ
void Network_Init (void);

//���� ���� ��Ʈ��ũ ó�� �Լ�
void NetworkProcess (void);

//Seslct �� üũ �Լ�
void NetworkSelectProc (SOCKET *pSockTable, FD_SET *pReadSet, FD_SET *pWriteSet);

//Recvó��
void NetworkRecv (SOCKET sock);

//Sendó��
bool NetworkSend (SOCKET sock);

//����� ���� �̺�Ʈ ó��
bool NetworkAccept (void);

//��Ŷ�� �Ϸ�Ǿ����� �˻� �� ��Ŷ ó��
int RecvPacket (st_SESSION *pSession);

//��Ŷ Ÿ�Կ� ���� ó�� �Լ� ȣ��
bool PacketProc (st_SESSION *pSession, BYTE PacketType, Packet *pPack);



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//���⼭ ���ʹ� ��Ŷ ó�� ���ν���
//////////////////////////////////////////////////////////////////////////
// ����� ����/���� �˸�.
BOOL	PacketProc_Connect (st_SESSION *pSession);


//////////////////////////////////////////////////////////////////////////
// �� ��Ŷ Ÿ�Կ� ���� ��Ŷ ó�� �Լ�.
//
// Parameters: (DWORD)SessionID. (CAyaPacket *)PacketClass.
// Return: (BOOL)TRUE, FALSE.
//////////////////////////////////////////////////////////////////////////
BOOL	PacketProc_MoveStart (st_SESSION *pSession, Packet *pack);

BOOL	PacketProc_MoveStop (st_SESSION *pSession, Packet *pack);

BOOL	PacketProc_Attack1 (st_SESSION *pSession, Packet *pack);

BOOL	PacketProc_Attack2 (st_SESSION *pSession, Packet *pack);

BOOL	PacketProc_Attack3 (st_SESSION *pSession, Packet *pack);

BOOL	PacketProc_ECHO (st_SESSION *pSession, Packet *pack);

/////////////////////////////////////////////////////////////////
//Ư������ �� �������� �޽����� ������ �Լ�
void SendPacket_SectorOne (int SectoriX, int SectoriY, Packet *pack, st_SESSION *pExceptSession);

//�Ѹ��Ը� ����
void SendPacket_Unicast (st_SESSION *pSession, Packet *pack);

//Ŭ���̾�Ʈ ���� �ֺ� ���Ϳ� �޼��� ������ (�ִ� 9�� ����)
void SendPacket_Around (st_SESSION *pSession, Packet *pack, bool bSendMe = false);

//��� �����ڿ��� �� ������ (�ý��� �޼����� ������� ����)
void SendPacket_Broadcast (st_SESSION *pSession, Packet *pack);

