#include"stdafx.h"
#include"Sector.h"

list<st_Charactor *> g_Sector[dfSector_Max_Y][dfSector_Max_X];


void Sector_Update (st_Charactor *pChar)
{
	int X;
	int Y;

	//받은 캐릭터의 섹터를 구한다.
	X = pChar->shX / dfSector_Size;
	Y = pChar->shY / dfSector_Size;


	//해당 섹터에 해당 캐릭터를 삽입.
	g_Sector[Y][X].push_front (pChar);


	//섹터의 이동이 있었더라면
	if ( pChar->CurPos.X != X || pChar->CurPos.Y != Y)
	{
		pChar->OldPos = pChar->CurPos;

		pChar->CurPos.X = X;
		pChar->CurPos.Y = Y;
	
		//섹터 Send
		Sector_Send (pChar);
	}

}

//현재섹터와 이전섹터를 비교해서 8방향 섹터 Send처리
void Sector_Send (st_Charactor *pChar)
{
	int X;
	int Y;
	X = pChar->CurPos.X;
	Y = pChar->CurPos.Y;
	
}