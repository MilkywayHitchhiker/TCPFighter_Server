#include"stdafx.h"
#include"Game_Proc.h"
#include"PacketDefine.h"
#include"Packet.h"
#include"Create_Packet.h"
#include"Network.h"


//캐릭터 생성
void	Create_Player (st_NETWORK_SESSION *pNew)
{
	st_Charactor	*pCharacter = new st_Charactor;

	Packet pack;

	//-----------------------------------------------------
	// 캐릭터 정보 저장 및 좌표 생성.
	//-----------------------------------------------------
	pCharacter->AccountNo = pNew->AccountNo;

	pCharacter->shX = (rand () % 580) + 40;
	pCharacter->shY = (rand () % 360) + 100;

	pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;

	pCharacter->dwAction = dfACTION_NONE;


	pCharacter->chHP = 100;

	//캐릭터 map에 저장.
	g_Charactor.insert (pair<DWORD, st_Charactor *> (pCharacter->AccountNo, pCharacter));


	//자신의 캐릭터 생성 패킷 전송
	Create_MyCharactor (&pack, pCharacter);
	
	Send_Unicast (pCharacter->Session,&pack);

	
	pack.Clear ();

	//다른사람들에게 캐릭터 생성 패킷 전송.
	Create_OtherCharactor (&pack, pCharacter);

	Send_Broadcast (&pack);

	return;
}