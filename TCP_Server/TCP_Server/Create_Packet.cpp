#include"stdafx.h"
#include"Create_Packet.h"
#include"Network.h"
#include"PacketDefine.h"


//자신의 캐릭터를 생성
void Create_MyCharactor (Packet *Pack, st_Charactor *pChar)
{
	st_NETWORK_PACKET_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = 10;
	stPacketHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	Pack->PutData (( char * )&stPacketHeader, dfNETWORK_PACKET_HEADER_SIZE);

	*Pack << pChar->AccountNo;
	*Pack << pChar->byDirection;
	*Pack << pChar->shX;
	*Pack << pChar->shY;
	*Pack << pChar->chHP;

	*Pack << ( BYTE )dfNETWORK_PACKET_END;

	return;
}

//다른사람의 캐릭터 생성
void Create_OtherCharactor (Packet *Pack, st_Charactor *pChar)
{
	st_NETWORK_PACKET_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
	stPacketHeader.bySize = 10;
	stPacketHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	Pack->PutData (( char * )&stPacketHeader, dfNETWORK_PACKET_HEADER_SIZE);

	*Pack << pChar->AccountNo;
	*Pack << pChar->byDirection;
	*Pack << pChar->shX;
	*Pack << pChar->shY;
	*Pack << pChar->chHP;

	*Pack << ( BYTE )dfNETWORK_PACKET_END;

	return;
}