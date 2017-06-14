#include"stdafx.h"
#include"Game_Proc.h"
#include"PacketDefine.h"
#include"Packet.h"
#include"Create_Packet.h"
#include"Network.h"


//ĳ���� ����
void	Create_Player (st_NETWORK_SESSION *pNew)
{
	st_Charactor	*pCharacter = new st_Charactor;

	Packet pack;

	//-----------------------------------------------------
	// ĳ���� ���� ���� �� ��ǥ ����.
	//-----------------------------------------------------
	pCharacter->AccountNo = pNew->AccountNo;

	pCharacter->shX = (rand () % 580) + 40;
	pCharacter->shY = (rand () % 360) + 100;

	pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;

	pCharacter->dwAction = dfACTION_NONE;


	pCharacter->chHP = 100;

	//ĳ���� map�� ����.
	g_Charactor.insert (pair<DWORD, st_Charactor *> (pCharacter->AccountNo, pCharacter));


	//�ڽ��� ĳ���� ���� ��Ŷ ����
	Create_MyCharactor (&pack, pCharacter);
	
	Send_Unicast (pCharacter->Session,&pack);

	
	pack.Clear ();

	//�ٸ�����鿡�� ĳ���� ���� ��Ŷ ����.
	Create_OtherCharactor (&pack, pCharacter);

	Send_Broadcast (&pack);

	return;
}