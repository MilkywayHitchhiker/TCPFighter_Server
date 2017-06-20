#pragma once
#include"Network.h"
#include"DefineEnum.h"

// �̵� ����üũ ����
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