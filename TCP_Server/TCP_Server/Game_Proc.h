#pragma once
#include <Windows.h>
#include"Network.h"

// 캐릭터 동작 플래그
#define dfACTION_MOVE_LL		0
#define dfACTION_MOVE_LU		1
#define dfACTION_MOVE_UU		2
#define dfACTION_MOVE_RU		3
#define dfACTION_MOVE_RR		4
#define dfACTION_MOVE_RD		5
#define dfACTION_MOVE_DD		6
#define dfACTION_MOVE_LD		7
#define dfACTION_ATTACK1		8
#define dfACTION_ATTACK2		9
#define dfACTION_ATTACK3		10
#define dfACTION_DAMAGE			11
#define dfACTION_NONE			255


// 공격범위.
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20


// 공격 데미지.
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3


// 캐릭터 이동 속도
#define dfSPEED_PLAYER_X	3
#define dfSPEED_PLAYER_Y	2

// 화면 이동 범위.
#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470


// 이동 오류체크 범위
#define dfERROR_RANGE		50

#include<map>
using namespace std;

struct st_Sector_Pos
{
	int X;
	int Y;
};

struct st_Charactor
{
	DWORD	AccountNo;
	st_NETWORK_SESSION * Session;
	DWORD	dwAction;
	BYTE	byDirection;

	short	shX;
	short	shY;

	char	chHP;
	st_Sector_Pos CurPos;
	st_Sector_Pos OldPos;
};


extern map<DWORD, st_Charactor *> g_Charactor;


//캐릭터 생성
void	Create_Player (st_NETWORK_SESSION *pNew);
