// TCP_Server.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "main.h"
#include "Network.h"
#include "Game_Proc.h"


bool g_bShutdown = false;
int g_SessionID = 0;

int g_iLogLevel=0;			//출력. 저장 대상의 로그 레벨
WCHAR g_szLogBuff[1024];	//로그 저장시 필요한 임시 버퍼


//키보드 컨트롤
void KeyProcess (void);

int main()
{
	timeBeginPeriod (1);



	Network_Init ();

	while ( 1 )
	{
		NetworkProcess ();

		Update ();

//		KeyProcess ();
	}



	timeEndPeriod (1);

    return 0;
}


//키보드 컨트롤
void KeyProcess (void)
{
	static bool bControlMode = false;
	
	//L : 컨트롤 Lock, U : 컨트롤 Unlock, / Q : 서버 종료
	//U : 컨트롤 Unlock
	if ( GetAsyncKeyState (0x55) )
	{
		bControlMode = true;

		wprintf (L"Control Mode : Press L - Key Lock\n");
	
	}
	//L : 컨트롤 Lock
	if ( bControlMode == true && GetAsyncKeyState (0x4C) )
	{
		bControlMode = false;

		wprintf (L"Control Mode : Press U - Key UnLock\n");
	}

}

void Log (WCHAR *szString, int iLogLevel)
{
	wprintf (L"%s \n",szString);
}