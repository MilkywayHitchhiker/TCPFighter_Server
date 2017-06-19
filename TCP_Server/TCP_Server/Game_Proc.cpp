#include"stdafx.h"
#include"Game_Proc.h"
#include "DefineEnum.h"



map<DWORD, st_CHARACTER *> g_CharacterMap;


//����� �ɸ��� ����
list<st_CHARACTER *> g_Sector[dfSector_Max_Y][dfSector_Max_X];


//���ο� ����� ����
BOOL	gameCreatePlayer (st_SESSION *pSession)
{
	st_CHARACTER *pNewChar;
	pNewChar = new st_CHARACTER;

	pNewChar->byDirection = dfACTION_MOVE_LL;
	pNewChar->chHP = 100;
	pNewChar->shX = (rand () % CreateRandshX) + 3000;
	pNewChar->shY = (rand () % CreateRandshY) + 3000;
	pNewChar->dwAction = dfACTION_STAND;
	pNewChar->shActionX = pNewChar->shX;
	pNewChar->shActionY = pNewChar->shY;
	pNewChar->dwSessionID = pSession->dwSessionID;
	pNewChar->pSession = pSession;
	pNewChar->CurSector.iX = -1;
	pNewChar->CurSector.iY = -1;
	
	g_CharacterMap.insert (pair<DWORD, st_CHARACTER *> (pNewChar->dwSessionID, pNewChar));

	//���� ������̹Ƿ� �׳� ���Ϳ� ���� ������ �ȴ�.
	Sector_AddCharacter (pNewChar);

	_LOG (dfLog_LEVEL_DEBUG, L"Create Character SessionID:%d", pSession->dwSessionID);
	
	
	return true;
}

//���� �浹 üũ
st_CHARACTER *AttackCheck (int iAttackType, DWORD dwSessionID)
{
	st_CHARACTER	*pAttackChar = FindCharacter (dwSessionID);
	st_CHARACTER	*pDamageChar;

	bool attack = false;
	return NULL;
}


st_CHARACTER *FindCharacter (DWORD dwSessionID)
{
	map<DWORD, st_CHARACTER *>::iterator iter;

	iter = g_CharacterMap.find (dwSessionID);

	if ( iter == g_CharacterMap.end () )
	{
		return NULL;
	}

	return iter->second;
}




void Update (void)
{
	DWORD dwCurrentTick = timeGetTime ();

	static DWORD	dwOldTick = timeGetTime ();
	static DWORD	dwNowTick = 0;
	static DWORD	dwUseTick = 0;

	dwNowTick = timeGetTime ();
	dwUseTick = dwNowTick - dwOldTick;
	if ( dwUseTick <= dfFrameTick )
	{
		return;
	}

	dwOldTick = dwNowTick - (dwUseTick - dfFrameTick);

	//������Ʈ Ÿ�̹� ���ó��

	
	//�� �������� ������Ʈ ó���� �ϸ� ���ϰ� �ʹ� ŭ.
	//�����ϰ� Ŭ���̾�Ʈ�� �ִ��� ����� �ӵ��� ���� �� �� �ְԲ� ���ָ� ��.
	st_CHARACTER *pCharacter = NULL;

	map<DWORD, st_CHARACTER *>::iterator iter;

	for ( iter = g_CharacterMap.begin (); iter != g_CharacterMap.end (); )
	{
		pCharacter = iter->second;
		iter++;

		if ( 0 >= pCharacter->chHP )
		{
			//���ó��
			DisconnectSession (pCharacter->pSession);
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Ÿ�Ӿƿ� ���Ƶ�.
			/*
			//���� �ð����� ������ ������ ���� ó��
			if ( dwCurrentTick - pCharacter->pSession->dwHeartBeat > dfNETWORK_PACKET_RECV_TIMEOUT )
			{
				DisconnectSession (pCharacter->pSession);
				continue;
			}
			*/
			//���� ���ۿ� ���� ó��
			switch ( pCharacter->dwAction )
			{
			case dfACTION_MOVE_LL:
				if ( MoveCheck (pCharacter->shX - dfSPEED_PLAYER_X, pCharacter->shY) )
				{
					pCharacter->shX -= dfSPEED_PLAYER_X;
					_LOG (dfLog_LEVEL_ERROR, L"Move_LL SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_LU:
				if ( MoveCheck (pCharacter->shX - dfSPEED_PLAYER_X, pCharacter->shY - dfSPEED_PLAYER_Y) )
				{
					pCharacter->shX -= dfSPEED_PLAYER_X;
					pCharacter->shY -= dfSPEED_PLAYER_Y;
					_LOG (dfLog_LEVEL_ERROR, L"Move_LU SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_UU:
				if ( MoveCheck (pCharacter->shX, pCharacter->shY - dfSPEED_PLAYER_Y) )
				{
					pCharacter->shX;
					pCharacter->shY -= dfSPEED_PLAYER_Y;
					_LOG (dfLog_LEVEL_ERROR, L"Move_UU SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_RU:
				if ( MoveCheck (pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY - dfSPEED_PLAYER_Y) )
				{
					pCharacter->shX += dfSPEED_PLAYER_X;
					pCharacter->shY -= dfSPEED_PLAYER_Y;
					_LOG (dfLog_LEVEL_ERROR, L"Move_RU SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_RR:
				if ( MoveCheck (pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY) )
				{
					pCharacter->shX += dfSPEED_PLAYER_X;
					_LOG (dfLog_LEVEL_ERROR, L"Move_RR SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_RD:
				if ( MoveCheck (pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY + dfSPEED_PLAYER_Y) )
				{
					pCharacter->shX += dfSPEED_PLAYER_X;
					pCharacter->shY += dfSPEED_PLAYER_Y;
					_LOG (dfLog_LEVEL_ERROR, L"Move_RD SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_DD:
				if ( MoveCheck (pCharacter->shX, pCharacter->shY + dfSPEED_PLAYER_Y) )
				{
					pCharacter->shY -= dfSPEED_PLAYER_Y;
					_LOG (dfLog_LEVEL_ERROR, L"Move_DD SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			case dfACTION_MOVE_LD:
				if ( MoveCheck (pCharacter->shX - dfSPEED_PLAYER_X, pCharacter->shY + dfSPEED_PLAYER_Y) )
				{
					pCharacter->shX -= dfSPEED_PLAYER_X;
					pCharacter->shY += dfSPEED_PLAYER_Y;
					_LOG (dfLog_LEVEL_ERROR, L"Move_LD SessionID : %d PosX : %d, PosY : %d", pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
				}
				break;
			}
			if ( pCharacter->dwAction >= dfACTION_MOVE_LL && pCharacter->dwAction <= dfACTION_MOVE_LD )
			{
				//�̵��� ��� ���� ������Ʈ�� ��
				if ( Sector_UpdateCharacter (pCharacter) )
				{
					//���Ͱ� ����� ���� Ŭ�� ���� ������ ���.
					CharacterSectorUpdatePacket (pCharacter);
				}
			}
		}
	}

}



BOOL	MoveCheck (int iX, int iY)
{
	if ( dfRANGE_MOVE_LEFT < iX && dfRANGE_MOVE_RIGHT > iX &&
		dfRANGE_MOVE_TOP < iY && dfRANGE_MOVE_BOTTOM > iY )
		return TRUE;

	return FALSE;
}



//DeadReckoning (����׼�,�׼ǽ��� �ð�, �׼ǽ��� ��ġ, X,Y, (OUT)���� ��ǥ X, Y
int DeadReckoningPos (DWORD dwAction, DWORD dwActionTick, short shActionX, short shActionY, int *pPosX, int *pPosY)
{
	
	//�ð����� ���ؼ� �� �������� �������� ���
	DWORD dwIntervalTick = timeGetTime () - dwActionTick;

	int iActionFrame = dwIntervalTick / 20;
	int iRemoveFrame = 0;

	int iVal;
	int iRPosX = shActionX;
	int iRPosY = shActionY;

	// 1. ���� ���������� X��, Y���� ��ǥ �̵����� ����.
	int iDX = iActionFrame * dfSPEED_PLAYER_X;
	int iDY = iActionFrame * dfSPEED_PLAYER_Y;

	switch ( dwAction )
	{
	case dfACTION_MOVE_LL :
		iRPosX = shActionX - iDX;
		iRPosY = shActionY;
		break;
	case dfACTION_MOVE_LU:
		iRPosX = shActionX - iDX;
		iRPosY = shActionY - iDY;
		break;
	case dfACTION_MOVE_UU:
		iRPosX = shActionX;
		iRPosY = shActionY - iDY;
		break;
	case dfACTION_MOVE_RU:
		iRPosX = shActionX + iDX;
		iRPosY = shActionY - iDY;
		break;
	case dfACTION_MOVE_RR:
		iRPosX = shActionX + iDX;
		iRPosY = shActionY;
		break;
	case dfACTION_MOVE_RD:
		iRPosX = shActionX + iDX;
		iRPosY = shActionY + iDY;
		break;
	case dfACTION_MOVE_DD:
		iRPosX = shActionX;
		iRPosY = shActionY + iDY;
		break;
	case dfACTION_MOVE_LD:
		iRPosX = shActionX - iDX;
		iRPosY = shActionY + iDY;
		break;
	}

	//���� ��ǥ�� ȭ���� �̵� ������ ��� ��� �� �׼��� �߶󳻱� ���ؼ� ������ ��� ������ �������� ���
	if ( iRPosX <= dfRANGE_MOVE_LEFT )
	{
		iVal = abs (dfRANGE_MOVE_LEFT - abs (iRPosX) / dfSPEED_PLAYER_X);
		iRemoveFrame = max (iVal, iRemoveFrame);
	}
	if ( iRPosX >= dfRANGE_MOVE_RIGHT )
	{
		iVal = abs (dfRANGE_MOVE_RIGHT - abs (iRPosX) / dfSPEED_PLAYER_X);
		iRemoveFrame = max (iVal, iRemoveFrame);
	}
	if ( iRPosY <= dfRANGE_MOVE_TOP )
	{
		iVal = abs (dfRANGE_MOVE_TOP - abs (iRPosY) / dfSPEED_PLAYER_Y);
		iRemoveFrame = max (iVal, iRemoveFrame);
	}
	if ( iRPosY >= dfRANGE_MOVE_BOTTOM )
	{
		iVal = abs (dfRANGE_MOVE_BOTTOM - abs (iRPosY) / dfSPEED_PLAYER_Y);
		iRemoveFrame = max (iVal, iRemoveFrame);
	}

	//������ ���� ��� ���� �Ǿ��� �������� ���Դٸ� ��ǥ�� �ٽ� �� ���
	if ( iRemoveFrame > 0 )
	{
		iActionFrame -= iRemoveFrame;
		int iDX = iActionFrame * dfSPEED_PLAYER_X;
		int iDY = iActionFrame * dfSPEED_PLAYER_Y;

		switch ( dwAction )
		{
		case dfACTION_MOVE_LL:
			iRPosX = shActionX - iDX;
			iRPosY = shActionY;
			break;
		case dfACTION_MOVE_LU:
			iRPosX = shActionX - iDX;
			iRPosY = shActionY - iDY;
			break;
		case dfACTION_MOVE_UU:
			iRPosX = shActionX;
			iRPosY = shActionY - iDY;
			break;
		case dfACTION_MOVE_RU:
			iRPosX = shActionX + iDX;
			iRPosY = shActionY - iDY;
			break;
		case dfACTION_MOVE_RR:
			iRPosX = shActionX + iDX;
			iRPosY = shActionY;
			break;
		case dfACTION_MOVE_RD:
			iRPosX = shActionX + iDX;
			iRPosY = shActionY + iDY;
			break;
		case dfACTION_MOVE_DD:
			iRPosX = shActionX;
			iRPosY = shActionY + iDY;
			break;
		case dfACTION_MOVE_LD:
			iRPosX = shActionX - iDX;
			iRPosY = shActionY + iDY;
			break;
		}
	}

	iRPosX = min (iRPosX, dfRANGE_MOVE_RIGHT);
	iRPosX = max (iRPosX, dfRANGE_MOVE_LEFT);
	iRPosY = min (iRPosY, dfRANGE_MOVE_BOTTOM);
	iRPosY = max (iRPosY, dfRANGE_MOVE_TOP);

	*pPosX = iRPosX;
	*pPosY = iRPosY;

	return iActionFrame;
}







//ĳ������ ���� ��ǥ shX,shY���� ���� ��ġ�� ����ؼ� �ش� ���Ϳ� ����
void Sector_AddCharacter (st_CHARACTER *pCharacter)
{
	//ĳ���Ϳ� ���� ������ ���� ĳ���͸� �ű� �߰� ����
	if ( pCharacter->CurSector.iX != -1 || pCharacter->CurSector.iY != -1 )
	{
		return;
	}
	
	int iSectorX = pCharacter->shX / dfSECTOR_PIXEL_WIDTH;
	int iSectorY = pCharacter->shY / dfSECTOR_PIXEL_HEIGHT;
	
	if ( iSectorX >= dfSector_Max_X || iSectorY >= dfSector_Max_Y )
	{
		return;
	}

	g_Sector[iSectorY][iSectorX].push_back (pCharacter);
	pCharacter->OldSector.iX = pCharacter->CurSector.iX = iSectorX;
	pCharacter->OldSector.iY = pCharacter->CurSector.iY = iSectorY;

}


//ĳ������ ���� ��ǥ shX,shY���� ���� ��ġ�� ����ؼ� �ش� ���Ϳ��� ����
void Sector_RemoveCharacter (st_CHARACTER *pCharacter)
{
	//ĳ���Ϳ� ���� ������ ���ٸ� ����ĳ���ͷ� �Ǵ�
	if ( pCharacter->CurSector.iX == -1 || pCharacter->CurSector.iY == -1 )
	{
		return;
	}

	list<st_CHARACTER *>&SectorList = g_Sector[pCharacter->CurSector.iY][pCharacter->CurSector.iX];
	list<st_CHARACTER *>::iterator iter;
	for ( iter = SectorList.begin (); iter != SectorList.end ();)
	{
		if ( pCharacter == *iter )
		{
			SectorList.erase (iter);
			break;
		}
		iter++;
	}
	pCharacter->OldSector.iX = pCharacter->CurSector.iX;
	pCharacter->OldSector.iY = pCharacter->CurSector.iY;
	pCharacter->CurSector.iX = -1;
	pCharacter->CurSector.iY = -1;

}


//���� ��ġ�� ���Ϳ��� ���� �� ������ ��ǥ�� ���͸� ���Ӱ� ����ؼ� ����
bool Sector_UpdateCharacter (st_CHARACTER *pCharacter)
{
	int iBeforeSectorX = pCharacter->CurSector.iX;
	int iBeforeSectorY = pCharacter->CurSector.iY;

	int iNewSectorX = pCharacter->shX / dfSECTOR_PIXEL_WIDTH;
	int iNewSectorY = pCharacter->shY / dfSECTOR_PIXEL_HEIGHT;

	//�������Ϳ� ���缽�Ͱ� �����ϴٸ� ������ �ʿ䰡 �����Ƿ� �׳� ����
	if ( iBeforeSectorX == iNewSectorX && iBeforeSectorY == iNewSectorY )
	{
		return false;
	}

	//���� ���Ϳ��� ĳ���� ���ְ� ���� ���Ϳ� ���Ӱ� �߰�
	Sector_RemoveCharacter (pCharacter);
	Sector_AddCharacter (pCharacter);

	pCharacter->OldSector.iX = iBeforeSectorX;
	pCharacter->OldSector.iY = iBeforeSectorY;

	_LOG (dfLog_LEVEL_ERROR, L"SectorMove SessionID : %d SectorX : %d, SectorY : %d", pCharacter->dwSessionID, pCharacter->CurSector.iX, pCharacter->CurSector.iY);
	return true;

}


//Ư�� ���� ��ǥ ���� �ֺ� ����� ���� ���
void GetSectorAround (int iSectorX, int iSectorY, st_SECTOR_AROUND *pSectorAround)
{
	int iCntX;
	int iCntY;
	iSectorX--;
	iSectorY--;
	pSectorAround->iCount = 0;

	for ( iCntY = 0; iCntY < 3; iCntY++ )
	{
		if ( iSectorY + iCntY < 0 || iSectorY + iCntY >= dfSector_Max_Y )
		{
			//���ϴ� ���Ͱ� ���� ������� �׳� �ٽý���
			continue;
		}
		for ( iCntX = 0; iCntX < 3; iCntX++ )
		{
			if ( iSectorX + iCntX < 0 || iSectorX + iCntX >= dfSector_Max_X )
			{
				//���ϴ� ���Ͱ� ���� ������� �׳� �ٽý���
				continue;
			}
			pSectorAround->Around[pSectorAround->iCount].iX = iSectorX + iCntX;
			pSectorAround->Around[pSectorAround->iCount].iY = iSectorY + iCntY;
			pSectorAround->iCount++;

		}
	}
}

//���Ϳ��� ���͸� �̵� �Ͽ����� ���� ����ǿ� ���� ����, ���� �߰��� ������ ���� ���ϴ� �Լ�
void GetUpdateSectorAround (st_CHARACTER *pCharacter, st_SECTOR_AROUND *pRemoveSector, st_SECTOR_AROUND *pAddSector)
{
	int iCntOld, iCntCur;
	bool bFind;
	st_SECTOR_AROUND OldSectorAround, CurSectorAround;

	OldSectorAround.iCount = 0;
	CurSectorAround.iCount = 0;

	pRemoveSector->iCount = 0;
	pAddSector->iCount = 0;

	//���� ���
	GetSectorAround (pCharacter->OldSector.iX, pCharacter->OldSector.iY, &OldSectorAround);
	GetSectorAround (pCharacter->CurSector.iX, pCharacter->CurSector.iY, &CurSectorAround);

	//���� ���������� �űԼ��Ϳ� ���� ������ ã�Ƽ� RemoveSector�� ����
	for ( iCntOld = 0; iCntOld < OldSectorAround.iCount; iCntOld++ )
	{
		bFind = false;
		for ( iCntCur = 0; iCntCur < CurSectorAround.iCount; iCntCur++ )
		{
			if ( OldSectorAround.Around[iCntOld].iX == CurSectorAround.Around[iCntCur].iX && OldSectorAround.Around[iCntOld].iY == CurSectorAround.Around[iCntCur].iY )
			{
				bFind = true;
				break;
			}
		}
		if ( bFind == false )
		{
			pRemoveSector->Around[pRemoveSector->iCount] = OldSectorAround.Around[iCntOld];
			pRemoveSector->iCount++;
		}
	}

	//���� ���� ������ ���� ���Ϳ� ���� ������ ã�Ƽ� AddSector�� ����
	for ( iCntCur = 0; iCntCur < CurSectorAround.iCount; iCntCur++ )
	{
		bFind = false;
		for ( iCntOld = 0; iCntOld < OldSectorAround.iCount; iCntOld++ )
		{
			if ( OldSectorAround.Around[iCntOld].iX == CurSectorAround.Around[iCntCur].iX && OldSectorAround.Around[iCntOld].iY == CurSectorAround.Around[iCntCur].iY )
			{
				bFind = true;
				break;
			}
		}
		if ( bFind == false )
		{
			pAddSector->Around[pAddSector->iCount] = CurSectorAround.Around[iCntOld];
			pAddSector->iCount++;
		}
	}

	return;

}



void CharacterSectorUpdatePacket (st_CHARACTER *pCharacter)
{
	st_SECTOR_AROUND RemoveSector, AddSector;
	st_CHARACTER *pExistCharacter;

	list<st_CHARACTER *> *pSectorList;
	list<st_CHARACTER *>::iterator iter;
	Packet pack;
	int iCnt;


	GetUpdateSectorAround (pCharacter, &RemoveSector, &AddSector);

	// 1.RemoveSector�� ĳ���� ���� ��Ŷ ������
	Pack_DeleteCharacter (&pack, pCharacter->dwSessionID);

	for ( iCnt = 0; iCnt < RemoveSector.iCount; iCnt++ )
	{

		//Ư������ �� �������� �޽����� ������ �Լ�
		SendPacket_SectorOne (RemoveSector.Around[iCnt].iX, RemoveSector.Around[iCnt].iY, &pack, NULL);
	}

	// 2. ���� �����̴� �༮���� RemoveSector�� ĳ���͵� ���� ��Ŷ ������
	for ( iCnt = 0; iCnt < RemoveSector.iCount; iCnt++ )
	{
		if ( RemoveSector.Around[iCnt].iY < 0 || RemoveSector.Around[iCnt].iY >= dfSector_Max_Y )
		{
			continue;
		}
		if ( RemoveSector.Around[iCnt].iX < 0 || RemoveSector.Around[iCnt].iX >= dfSector_Max_X )
		{
			continue;
		}
		pSectorList = &g_Sector[RemoveSector.Around[iCnt].iY][RemoveSector.Around[iCnt].iX];
		for ( iter = pSectorList->begin (); iter != pSectorList->end ();)
		{
			// 1.���� ĳ���Ϳ��� �����Ǵ� ���� ĳ���� ���� ��Ŷ ������
			Pack_DeleteCharacter (&pack, (*iter)->dwSessionID);
			SendPacket_Unicast (pCharacter->pSession, &pack);
			iter++;
		}
	}

	// 3. AddSector�� ĳ���� ���� ��Ŷ ������
	Pack_CreateOtherCharacter (&pack, pCharacter->dwSessionID, pCharacter->byDirection, pCharacter->shX, pCharacter->shY, pCharacter->chHP);
	for ( iCnt = 0; iCnt < AddSector.iCount; iCnt++ )
	{
		//Ư������ �� �������� �޽����� ������ �Լ�
		SendPacket_SectorOne (AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &pack, NULL);
	}

	// 4.�̵��� �༮���� AddSector�� �ִ� ĳ���͵� ���� ��Ŷ ������
	for ( iCnt = 0; iCnt < AddSector.iCount; iCnt++ )
	{
		if ( AddSector.Around[iCnt].iY < 0 || AddSector.Around[iCnt].iY >= dfSector_Max_Y )
		{
			continue;
		}
		if ( AddSector.Around[iCnt].iX < 0 || AddSector.Around[iCnt].iX >= dfSector_Max_X )
		{
			continue;
		}
		//����� ���͸� ���鼭 ���͸���Ʈ ����
		pSectorList = &g_Sector[AddSector.Around[iCnt].iY][AddSector.Around[iCnt].iX];


		//�ش� ���͸��� ��ϵ� ĳ���͵��� �̾Ƽ� ������Ŷ ����� ����
		for ( iter = pSectorList->begin (); iter != pSectorList->end ();)
		{
			pExistCharacter = *iter;
			iter++;
				Pack_CreateOtherCharacter (&pack, pExistCharacter->dwSessionID, pExistCharacter->byDirection, pExistCharacter->shX, pExistCharacter->shY, pExistCharacter->chHP);
				SendPacket_Unicast (pCharacter->pSession, &pack);

				//�� AddSector�� ĳ���Ͱ� �Ȱ� �־��ٸ� �̵� ��Ŷ ���� ����
				switch ( pExistCharacter->dwAction )
				{
				case dfACTION_MOVE_DD :
				case dfACTION_MOVE_LD :
				case dfACTION_MOVE_LL :
				case dfACTION_MOVE_LU :
				case dfACTION_MOVE_UU :
				case dfACTION_MOVE_RU :
				case dfACTION_MOVE_RR :
				case dfACTION_MOVE_RD :
					Pack_MoveStart (&pack, pExistCharacter->dwSessionID, pExistCharacter->MoveDirection, pExistCharacter->shX, pExistCharacter->shY);
					SendPacket_Unicast (pCharacter->pSession, &pack);
					break;
				case dfACTION_ATTACK1 :
					Pack_Attack1 (&pack, pExistCharacter->dwSessionID, pExistCharacter->byDirection, pExistCharacter->shX, pExistCharacter->shY);
					SendPacket_Unicast (pCharacter->pSession, &pack);
					break;
				case dfACTION_ATTACK2 :
					Pack_Attack2 (&pack, pExistCharacter->dwSessionID, pExistCharacter->byDirection, pExistCharacter->shX, pExistCharacter->shY);
					SendPacket_Unicast (pCharacter->pSession, &pack);
					break;
				case dfACTION_ATTACK3 :
					Pack_Attack3 (&pack, pExistCharacter->dwSessionID, pExistCharacter->byDirection, pExistCharacter->shX, pExistCharacter->shY);
					SendPacket_Unicast (pCharacter->pSession, &pack);
					break;
				}
			

		}
	}
}