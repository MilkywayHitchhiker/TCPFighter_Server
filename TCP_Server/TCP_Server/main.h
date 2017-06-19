#pragma once
#include<Windows.h>
#include<list>
#include<map>
using namespace std;


extern int g_SessionID;
extern bool g_bShutdown;

//로그용 함수들

#define dfLog_LEVEL_DEBUG 0
#define dfLog_LEVEL_WARNING 1
#define dfLog_LEVEL_ERROR 2

extern int g_iLogLevel;			//출력. 저장 대상의 로그 레벨
extern WCHAR g_szLogBuff[1024];	//로그 저장시 필요한 임시 버퍼
void Log (WCHAR *szString, int iLogLevel);


#define _LOG(LogLevel,fmt,...)							\
do{														\
	if ( g_iLogLevel <= LogLevel )						\
	{													\
		wsprintf (g_szLogBuff, fmt, ##__VA_ARGS__);		\
		Log(g_szLogBuff,LogLevel);						\
	}													\
														\
}while(0)												