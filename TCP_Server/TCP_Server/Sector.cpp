#include"stdafx.h"
#include"Sector.h"

list<st_Charactor *> g_Sector[dfSector_Max_Y][dfSector_Max_X];


void Sector_Update (st_Charactor *pChar)
{
	int X;
	int Y;

	//���� ĳ������ ���͸� ���Ѵ�.
	X = pChar->shX / dfSector_Size;
	Y = pChar->shY / dfSector_Size;


	//�ش� ���Ϳ� �ش� ĳ���͸� ����.
	g_Sector[Y][X].push_front (pChar);


	//������ �̵��� �־������
	if ( pChar->CurPos.X != X || pChar->CurPos.Y != Y)
	{
		pChar->OldPos = pChar->CurPos;

		pChar->CurPos.X = X;
		pChar->CurPos.Y = Y;
	
		//���� Send
		Sector_Send (pChar);
	}

}

//���缽�Ϳ� �������͸� ���ؼ� 8���� ���� Sendó��
void Sector_Send (st_Charactor *pChar)
{
	int X;
	int Y;
	X = pChar->CurPos.X;
	Y = pChar->CurPos.Y;
	
}