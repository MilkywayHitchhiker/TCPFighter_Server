#include"stdafx.h"
#include"Game_Proc.h"
#include "DefineEnum.h"



map<DWORD, st_CHARACTER *> g_CharacterMap;


//월드맵 케릭터 섹터
list<st_CHARACTER *> g_Sector[dfSector_Max_Y][dfSector_Max_X];


//새로운 사용자 생성
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

	//새로 만든것이므로 그냥 섹터에 집어 넣으면 된다.
	Sector_AddCharacter (pNewChar);

	_LOG (dfLog_LEVEL_DEBUG, L"Create Character SessionID:%d", pSession->dwSessionID);
	
	
	return true;
}

//공격 충돌 체크
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

	//업데이트 타이밍 계산처리

	
	//매 루프마다 업데이트 처리를 하면 부하가 너무 큼.
	//적절하게 클라이언트와 최대한 비슷한 속도로 돌아 갈 수 있게끔 해주면 됨.
	st_CHARACTER *pCharacter = NULL;

	map<DWORD, st_CHARACTER *>::iterator iter;

	for ( iter = g_CharacterMap.begin (); iter != g_CharacterMap.end (); )
	{
		pCharacter = iter->second;
		iter++;

		if ( 0 >= pCharacter->chHP )
		{
			//사망처리
			DisconnectSession (pCharacter->pSession);
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////타임아웃 막아둠.
			/*
			//일정 시간동안 수신이 없으면 종료 처리
			if ( dwCurrentTick - pCharacter->pSession->dwHeartBeat > dfNETWORK_PACKET_RECV_TIMEOUT )
			{
				DisconnectSession (pCharacter->pSession);
				continue;
			}
			*/
			//현재 동작에 따른 처리
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
				//이동인 경우 섹터 업데이트를 함
				if ( Sector_UpdateCharacter (pCharacter) )
				{
					//섹터가 변경된 경우는 클라에 관련 정보를 쏜다.
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



//DeadReckoning (현재액션,액션시작 시간, 액션시작 위치, X,Y, (OUT)계산된 좌표 X, Y
int DeadReckoningPos (DWORD dwAction, DWORD dwActionTick, short shActionX, short shActionY, int *pPosX, int *pPosY)
{
	
	//시간차를 구해서 몇 프레임이 지났는지 계산
	DWORD dwIntervalTick = timeGetTime () - dwActionTick;

	int iActionFrame = dwIntervalTick / 20;
	int iRemoveFrame = 0;

	int iVal;
	int iRPosX = shActionX;
	int iRPosY = shActionY;

	// 1. 계산된 프레임으로 X축, Y축의 좌표 이동값을 구함.
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

	//계산된 좌표가 화면의 이동 영역을 벗어난 경우 그 액션을 잘라내기 위해서 영역을 벗어난 이후의 프레임을 계산
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

	//위에서 계산된 결과 삭제 되야할 프레임이 나왔다면 좌표를 다시 재 계산
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







//캐릭터의 현재 좌표 shX,shY기준 섹터 위치를 계산해서 해당 섹터에 넣음
void Sector_AddCharacter (st_CHARACTER *pCharacter)
{
	//캐릭터에 섹터 정보가 없는 캐릭터만 신규 추가 가능
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


//캐릭터의 현재 좌표 shX,shY기준 섹터 위치를 계산해서 해당 섹터에서 삭제
void Sector_RemoveCharacter (st_CHARACTER *pCharacter)
{
	//캐릭터에 섹터 정보가 없다면 없는캐릭터로 판단
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


//현재 위치한 섹터에서 삭제 후 현재의 좌표로 섹터를 새롭게 계산해서 넣음
bool Sector_UpdateCharacter (st_CHARACTER *pCharacter)
{
	int iBeforeSectorX = pCharacter->CurSector.iX;
	int iBeforeSectorY = pCharacter->CurSector.iY;

	int iNewSectorX = pCharacter->shX / dfSECTOR_PIXEL_WIDTH;
	int iNewSectorY = pCharacter->shY / dfSECTOR_PIXEL_HEIGHT;

	//이전섹터와 현재섹터가 동일하다면 변경할 필요가 없으므로 그냥 종료
	if ( iBeforeSectorX == iNewSectorX && iBeforeSectorY == iNewSectorY )
	{
		return false;
	}

	//이전 섹터에서 캐릭터 없애고 현재 섹터에 새롭게 추가
	Sector_RemoveCharacter (pCharacter);
	Sector_AddCharacter (pCharacter);

	pCharacter->OldSector.iX = iBeforeSectorX;
	pCharacter->OldSector.iY = iBeforeSectorY;

	_LOG (dfLog_LEVEL_ERROR, L"SectorMove SessionID : %d SectorX : %d, SectorY : %d", pCharacter->dwSessionID, pCharacter->CurSector.iX, pCharacter->CurSector.iY);
	return true;

}


//특정 섹터 좌표 기준 주변 영향권 섹터 얻기
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
			//구하는 섹터가 맵을 벗어났으면 그냥 다시시작
			continue;
		}
		for ( iCntX = 0; iCntX < 3; iCntX++ )
		{
			if ( iSectorX + iCntX < 0 || iSectorX + iCntX >= dfSector_Max_X )
			{
				//구하는 섹터가 맵을 벗어났으면 그냥 다시시작
				continue;
			}
			pSectorAround->Around[pSectorAround->iCount].iX = iSectorX + iCntX;
			pSectorAround->Around[pSectorAround->iCount].iY = iSectorY + iCntY;
			pSectorAround->iCount++;

		}
	}
}

//섹터에서 섹터를 이동 하였을때 섹터 영향권에 빠진 섹터, 새로 추가된 섹터의 정보 구하는 함수
void GetUpdateSectorAround (st_CHARACTER *pCharacter, st_SECTOR_AROUND *pRemoveSector, st_SECTOR_AROUND *pAddSector)
{
	int iCntOld, iCntCur;
	bool bFind;
	st_SECTOR_AROUND OldSectorAround, CurSectorAround;

	OldSectorAround.iCount = 0;
	CurSectorAround.iCount = 0;

	pRemoveSector->iCount = 0;
	pAddSector->iCount = 0;

	//섹터 얻기
	GetSectorAround (pCharacter->OldSector.iX, pCharacter->OldSector.iY, &OldSectorAround);
	GetSectorAround (pCharacter->CurSector.iX, pCharacter->CurSector.iY, &CurSectorAround);

	//이전 섹터정보중 신규섹터에 없는 정보를 찾아서 RemoveSector에 넣음
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

	//현재 섹터 정보중 이전 섹터에 없는 정보를 찾아서 AddSector에 넣음
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

	// 1.RemoveSector에 캐릭터 삭제 패킷 보내기
	Pack_DeleteCharacter (&pack, pCharacter->dwSessionID);

	for ( iCnt = 0; iCnt < RemoveSector.iCount; iCnt++ )
	{

		//특정섹터 한 공간에만 메시지를 보내는 함수
		SendPacket_SectorOne (RemoveSector.Around[iCnt].iX, RemoveSector.Around[iCnt].iY, &pack, NULL);
	}

	// 2. 지금 움직이는 녀석에게 RemoveSector의 캐릭터들 삭제 패킷 보내기
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
			// 1.현재 캐릭터에게 삭제되는 섹터 캐릭터 삭제 패킷 보내기
			Pack_DeleteCharacter (&pack, (*iter)->dwSessionID);
			SendPacket_Unicast (pCharacter->pSession, &pack);
			iter++;
		}
	}

	// 3. AddSector에 캐릭터 생성 패킷 보내기
	Pack_CreateOtherCharacter (&pack, pCharacter->dwSessionID, pCharacter->byDirection, pCharacter->shX, pCharacter->shY, pCharacter->chHP);
	for ( iCnt = 0; iCnt < AddSector.iCount; iCnt++ )
	{
		//특정섹터 한 공간에만 메시지를 보내는 함수
		SendPacket_SectorOne (AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &pack, NULL);
	}

	// 4.이동한 녀석에게 AddSector에 있는 캐릭터들 생성 패킷 보내기
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
		//얻어진 섹터를 돌면서 섹터리스트 접근
		pSectorList = &g_Sector[AddSector.Around[iCnt].iY][AddSector.Around[iCnt].iX];


		//해당 섹터마다 등록된 캐릭터들을 뽑아서 생성패킷 만들어 보냄
		for ( iter = pSectorList->begin (); iter != pSectorList->end ();)
		{
			pExistCharacter = *iter;
			iter++;
				Pack_CreateOtherCharacter (&pack, pExistCharacter->dwSessionID, pExistCharacter->byDirection, pExistCharacter->shX, pExistCharacter->shY, pExistCharacter->chHP);
				SendPacket_Unicast (pCharacter->pSession, &pack);

				//새 AddSector의 캐릭터가 걷고 있었다면 이동 패킷 만들어서 보냄
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