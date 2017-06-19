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
	SOCKET Sock;			//접속자 소켓
	DWORD dwSessionID;		//접속자 고유 세션 ID
	CRingbuffer SendQ;		
	CRingbuffer RecvQ;		
	DWORD dwHeartBeat;		//마지막 패킷 수신시간
};

extern SOCKET g_ListenSock;		//리슨 소켓

extern map<SOCKET, st_SESSION *> g_SessionMap;		//세션 구조체를 관리하기 위한 맵

//Socket으로 세션 구조체 찾아서 반환 없는 유저일 경우 NULL 반환
st_SESSION *FindSession (SOCKET sock);


//새로운 세션 생성 및 등록
st_SESSION *CreateSession (SOCKET Sock);

//해당 세션 종료처리
void DisconnectSession (st_SESSION *pSesssion);

//네트워크 초기화
void Network_Init (void);

//서버 메인 네트워크 처리 함수
void NetworkProcess (void);

//Seslct 모델 체크 함수
void NetworkSelectProc (SOCKET *pSockTable, FD_SET *pReadSet, FD_SET *pWriteSet);

//Recv처리
void NetworkRecv (SOCKET sock);

//Send처리
bool NetworkSend (SOCKET sock);

//사용자 접속 이벤트 처리
bool NetworkAccept (void);

//패킷이 완료되었는지 검사 후 패킷 처리
int RecvPacket (st_SESSION *pSession);

//패킷 타입에 따른 처리 함수 호출
bool PacketProc (st_SESSION *pSession, BYTE PacketType, Packet *pPack);



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//여기서 부터는 패킷 처리 프로시저
//////////////////////////////////////////////////////////////////////////
// 사용자 접속/해지 알림.
BOOL	PacketProc_Connect (st_SESSION *pSession);


//////////////////////////////////////////////////////////////////////////
// 각 패킷 타입에 따른 패킷 처리 함수.
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
//특정섹터 한 공간에만 메시지를 보내는 함수
void SendPacket_SectorOne (int SectoriX, int SectoriY, Packet *pack, st_SESSION *pExceptSession);

//한명에게만 보냄
void SendPacket_Unicast (st_SESSION *pSession, Packet *pack);

//클라이언트 기준 주변 섹터에 메세지 보내기 (최대 9개 영역)
void SendPacket_Around (st_SESSION *pSession, Packet *pack, bool bSendMe = false);

//모든 접속자에게 다 보내기 (시스템 메세지외 사용하지 않음)
void SendPacket_Broadcast (st_SESSION *pSession, Packet *pack);

