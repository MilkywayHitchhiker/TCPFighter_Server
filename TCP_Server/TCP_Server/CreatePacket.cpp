#include"stdafx.h"
#include"CreatePacket.h"
#include"Packet.h"
#include"RingBuffer.h"
#include"PacketDefine.h"

void	Pack_CreateMyCharacter (Packet *pack, DWORD dwSessionID, BYTE byDirection, short shX, short shY, char chHP)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 10;
	stPacketHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	pack->PutData (( char * )&stPacketHeader, sizeof(st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << byDirection;
	*pack << shX;
	*pack << shY;
	*pack << chHP;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_CreateOtherCharacter (Packet *pack, DWORD dwSessionID, BYTE byDirection, short shX, short shY, char chHP)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 10;
	stPacketHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	pack->PutData (( char * )&stPacketHeader, sizeof(st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << byDirection;
	*pack << shX;
	*pack << shY;
	*pack << chHP;


	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_DeleteCharacter (Packet *pack, DWORD dwSessionID)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 4;
	stPacketHeader.byType = dfPACKET_SC_DELETE_CHARACTER;

	pack->PutData (( char * )&stPacketHeader, sizeof (st_PACK_HEADER));

	*pack << dwSessionID;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_MoveStart (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACK_HEADER Header;
	Header.byCode = dfPACKET_CODE;
	Header.bySize = 9;
	Header.byType = dfPACKET_SC_MOVE_START;

	pack->Clear ();
	pack->PutData (( char * )&Header, sizeof (st_PACK_HEADER));
	*pack << dwSessionID;
	*pack << byDir;
	*pack << shX;
	*pack << shY;
	*pack << dfNETWORK_PACKET_END;

	return;
}

void	Pack_MoveStop (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_MOVE_STOP;

	pack->PutData (( char * )&stPacketHeader, sizeof (st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << byDir;
	*pack << shX;
	*pack << shY;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_Attack1 (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_ATTACK1;

	pack->PutData (( char * )&stPacketHeader, sizeof(st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << byDir;
	*pack << shX;
	*pack << shY;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_Attack2 (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_ATTACK2;

	pack->PutData (( char * )&stPacketHeader, sizeof (st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << byDir;
	*pack << shX;
	*pack << shY;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_Attack3 (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_ATTACK3;

	pack->PutData (( char * )&stPacketHeader, sizeof(st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << byDir;
	*pack << shX;
	*pack << shY;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_Damage (Packet *pack, DWORD dwAttackID, DWORD dwDamageID, short shHP)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_DAMAGE;

	pack->PutData (( char * )&stPacketHeader, sizeof (st_PACK_HEADER));

	*pack << dwAttackID;
	*pack << dwDamageID;
	*pack << ( char )shHP;

	*pack << ( BYTE )dfNETWORK_PACKET_END;
}

void	Pack_Sync (Packet *pack, DWORD dwSessionID, short shX, short shY)
{
	st_PACK_HEADER	stPacketHeader;

	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 6;
	stPacketHeader.byType = dfPACKET_SC_DAMAGE;

	pack->PutData (( char * )&stPacketHeader, sizeof (st_PACK_HEADER));

	*pack << dwSessionID;
	*pack << shX;
	*pack << shY;
	*pack << ( BYTE )dfNETWORK_PACKET_END;

	
}