#pragma once

#include"Game_Proc.h"
#include<list>
using namespace std;

#define dfSector_Size 150

#define dfSector_Max_X 60
#define dfSector_Max_Y 60


//캐릭터 섹터 업데이트
void Sector_Update (st_Charactor *pChar);

//현재섹터와 이전섹터를 비교해서 8방향 섹터 Send처리
void Sector_Send (st_Charactor *pChar);


extern list<st_Charactor *> g_Sector[dfSector_Max_Y][dfSector_Max_X];
