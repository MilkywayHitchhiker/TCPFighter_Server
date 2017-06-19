#pragma once
#include"Network.h"


// ���ݹ���.
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20


// ���� ������.
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3


// ĳ���� �̵� �ӵ�
#define dfSPEED_PLAYER_X	6
#define dfSPEED_PLAYER_Y	4



// �̵� ����üũ ����
#define dfERROR_RANGE		50

#define dfSECTOR_PIXEL_WIDTH 50
#define dfSECTOR_PIXEL_HEIGHT 50

#define dfSector_Max_X		6400 / dfSECTOR_PIXEL_WIDTH
#define dfSector_Max_Y		6400 / dfSECTOR_PIXEL_HEIGHT

#define dfFrameTick 40

#define CreateRandshX 100
#define CreateRandshY 100



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
	DWORD dwActionTick;		//���� �׼��� ���� �ð�. ���巹Ŀ�׿�
	BYTE byDirection;
	BYTE MoveDirection;

	short shX;
	short shY;
	short shActionX;		//�׼��� ����������� ��ǥ
	short shActionY;

	st_SECTOR_POS CurSector;
	st_SECTOR_POS OldSector;

	char chHP;
};



extern map<DWORD, st_CHARACTER *> g_CharacterMap;


//����� �ɸ��� ����
extern list<st_CHARACTER *> g_Sector[dfSector_Max_Y][dfSector_Max_X];

//ĳ���� �˻�
st_CHARACTER *FindCharacter (DWORD dwSessionID);

//���� �浹 üũ
st_CHARACTER *AttackCheck (int iAttackType, DWORD dwSessionID);

//���ο� ����� ����
BOOL	gameCreatePlayer (st_SESSION *pSession);


void Update (void);

BOOL	MoveCheck (int iX, int iY);

//DeadReckoning (����׼�,�׼ǽ��� �ð�, �׼ǽ��� ��ġ, X,Y, (OUT)���� ��ǥ X, Y
int DeadReckoningPos (DWORD dwAction, DWORD dwActionTick, short shActionX, short shActionY, int *pPosX, int *pPosY);

//ĳ������ ���� ��ǥ shX,shY���� ���� ��ġ�� ����ؼ� �ش� ���Ϳ� ����
void Sector_AddCharacter (st_CHARACTER *pChararter);


//ĳ������ ���� ��ǥ shX,shY���� ���� ��ġ�� ����ؼ� �ش� ���Ϳ��� ����
void Sector_RemoveCharacter (st_CHARACTER *pChararter);


//���� ��ġ�� ���Ϳ��� ���� �� ������ ��ǥ�� ���͸� ���Ӱ� ����ؼ� ����
bool Sector_UpdateCharacter (st_CHARACTER *pChararter);


//Ư�� ���� ��ǥ ���� �ֺ� ����� ���� ���
void GetSectorAround (int iSectorX, int iSectorY, st_SECTOR_AROUND *pSectorAround);

//���Ϳ��� ���͸� �̵� �Ͽ����� ���� ����ǿ� ���� ����, ���� �߰��� ������ ���� ���ϴ� �Լ�
void GetUpdateSectorAround (st_CHARACTER *pChararter, st_SECTOR_AROUND *pRemoveSector, st_SECTOR_AROUND *pAddSector);

void CharacterSectorUpdatePacket (st_CHARACTER *pCharacter);