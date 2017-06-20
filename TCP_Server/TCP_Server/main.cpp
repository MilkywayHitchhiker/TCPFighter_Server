// TCP_Server.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include "main.h"
#include "Network.h"
#include "Game_Proc.h"


bool g_bShutdown = false;
int g_SessionID = 0;

int g_iLogLevel=0;			//���. ���� ����� �α� ����
WCHAR g_szLogBuff[1024];	//�α� ����� �ʿ��� �ӽ� ����


//Ű���� ��Ʈ��
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


//Ű���� ��Ʈ��
void KeyProcess (void)
{
	static bool bControlMode = false;
	
	//L : ��Ʈ�� Lock, U : ��Ʈ�� Unlock, / Q : ���� ����
	//U : ��Ʈ�� Unlock
	if ( GetAsyncKeyState (0x55) )
	{
		bControlMode = true;

		wprintf (L"Control Mode : Press L - Key Lock\n");
	
	}
	//L : ��Ʈ�� Lock
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