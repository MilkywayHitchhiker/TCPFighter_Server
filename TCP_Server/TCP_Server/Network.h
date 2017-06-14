#pragma once

#include<Windows.h>
#include"RingBuffer.h"
#include"Packet.h"

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <WS2tcpip.h>

//서버 IP설정
#define ServerIP L"127.0.0.1"

//포트 번호
#define dfNETWORK_PORT		20000



//---------------------------------------------------------------
// 패킷의 가장 앞에 들어갈 패킷코드.
//---------------------------------------------------------------
#define dfNETWORK_PACKET_CODE	((BYTE)0x89)
//---------------------------------------------------------------
// 패킷의 가장 뒤에 들어갈 패킷코드.
//---------------------------------------------------------------
#define dfNETWORK_PACKET_END	((BYTE)0x79)

//패킷 해더 구조체
struct st_NETWORK_PACKET_HEADER
{
	BYTE	byCode;			// 패킷코드 0x89 고정.
	BYTE	bySize;			// 패킷 사이즈.
	BYTE	byType;			// 패킷타입.
	BYTE	byTemp;			// 사용안함.
};

#define dfNETWORK_PACKET_HEADER_SIZE	4




//세션 구조체
struct st_NETWORK_SESSION
{
	SOCKET			Socket;			// 현 접속의 TCP 소켓.
	DWORD			AccountNo;	// 접속자의 고유 세션 ID.

	CRingbuffer	RecvQ;		// 수신 큐.
	CRingbuffer	SendQ;		// 송신 큐.

	DWORD			last_HeartBeat;	// 트래픽 체크를 위한 틱.  

};


DWORD AccountNo = 1;

//전역 리슨 소켓
extern SOCKET ListenSocket;

//네트워크 초기화
bool Network_Init (void);

//유저번호를 받아서 클라이언트를 찾아서 return, 해당 클라 없을 시 NULL리턴.
st_NETWORK_SESSION *FindSession (SOCKET User);

//Select모델사용
void NetworkProcess (void);

//Select함수 사용 Accept,Send,Recv처리
void SelectSocket (SOCKET *UserSockTable, FD_SET *ReadSet, FD_SET *WriteSet);

//신규 유저 Accept처리
void Accept (void);

// 사용자 접속/해지 알림.
BOOL Proc_Connect (st_NETWORK_SESSION *pNew);


//1명에게 송신
BOOL Send_Unicast (st_NETWORK_SESSION *pSession, Packet *pack);

//모두에게 송신
void Send_Broadcast (Packet *pack);

//섹터 송신
BOOL Send_Sector (int X, int Y, st_Charactor *pClient, Packet *pack);