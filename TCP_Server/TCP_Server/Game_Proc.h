#pragma once
#include"Network.h"
#include"DefineEnum.h"

// 이동 오류체크 범위
#define dfERROR_RANGE		50

#define dfSECTOR_PIXEL_WIDTH 80
#define dfSECTOR_PIXEL_HEIGHT 80

#define dfSector_Max_X		6400 / dfSECTOR_PIXEL_WIDTH
#define dfSector_Max_Y		6400 / dfSECTOR_PIXEL_HEIGHT

#define dfFrameTick 40

#define CreateRandshX 100
#define CreateRandshY 100
#define CreateCharacterCenterX 3000
#define CreateCharacterCenterY 3000




struct st_SECTOR_POS
{
	int iX;
	int iY;
};

struct st_SECTOR_AROUND
{
	int iCount;
	st_SECTOR_POS Around[9];
};


struct st_CHARACTER
{
	st_SESSION *pSession;
	DWORD dwSessionID;

	DWORD dwAction;
	DWORD dwActionTick;		//현재 액션을 취한 시간. 데드레커닝용
	BYTE byDirection;
	BYTE MoveDirection;

	short shX;
	short shY;
	short shActionX;		//액션이 변경됬을때의 좌표
	short shActionY;

	st_SECTOR_POS CurSector;
	st_SECTOR_POS OldSector;

	char chHP;
};



extern map<DWORD, st_CHARACTER *> g_CharacterMap;


//월드맵 케릭터 섹터
extern list<st_CHARACTER *> g_Sector[dfSector_Max_Y][dfSector_Max_X];

//캐릭터 검색
st_CHARACTER *FindCharacter (DWORD dwSessionID);

//공격 충돌 체크
st_CHARACTER *AttackCheck (int iAttackType, DWORD dwSessionID);

//새로운 사용자 생성
BOOL	gameCreatePlayer (st_SESSION *pSession);


void Update (void);

BOOL	MoveCheck (int iX, int iY);

//DeadReckoning (현재액션,액션시작 시간, 액션시작 위치, X,Y, (OUT)계산된 좌표 X, Y
int DeadReckoningPos (DWORD dwAction, DWORD dwActionTick, short shActionX, short shActionY, int *pPosX, int *pPosY);

//캐릭터의 현재 좌표 shX,shY기준 섹터 위치를 계산해서 해당 섹터에 넣음
void Sector_AddCharacter (st_CHARACTER *pChararter);


//캐릭터의 현재 좌표 shX,shY기준 섹터 위치를 계산해서 해당 섹터에서 삭제
void Sector_RemoveCharacter (st_CHARACTER *pChararter);


//현재 위치한 섹터에서 삭제 후 현재의 좌표로 섹터를 새롭게 계산해서 넣음
bool Sector_UpdateCharacter (st_CHARACTER *pChararter);


//특정 섹터 좌표 기준 주변 영향권 섹터 얻기
void GetSectorAround (int iSectorX, int iSectorY, st_SECTOR_AROUND *pSectorAround);

//섹터에서 섹터를 이동 하였을때 섹터 영향권에 빠진 섹터, 새로 추가된 섹터의 정보 구하는 함수
void GetUpdateSectorAround (st_CHARACTER *pChararter, st_SECTOR_AROUND *pRemoveSector, st_SECTOR_AROUND *pAddSector);

void CharacterSectorUpdatePacket (st_CHARACTER *pCharacter);