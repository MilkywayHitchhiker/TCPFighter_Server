#pragma once

#include"Game_Proc.h"
#include<list>
using namespace std;

#define dfSector_Size 150

#define dfSector_Max_X 60
#define dfSector_Max_Y 60


//ĳ���� ���� ������Ʈ
void Sector_Update (st_Charactor *pChar);

//���缽�Ϳ� �������͸� ���ؼ� 8���� ���� Sendó��
void Sector_Send (st_Charactor *pChar);


extern list<st_Charactor *> g_Sector[dfSector_Max_Y][dfSector_Max_X];
