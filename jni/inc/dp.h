#ifndef _VRPRESENT_DP_H_
#define _VRPRESENT_DP_H_

#ifdef __linux__
#include <stdarg.h>
#endif

#ifdef MT3351_DBG
#undef _DEBUG
#undef _DEBUG_CONSOLE
#endif
//#define WRITE_FILE
#ifdef ANDROID
#ifndef ANDROID_NDK
#include "utils/Log.h"
#else
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif
#define LOG_TAG "VRP"
#endif

#ifndef __linux__
#pragma warning (disable:4995)
#pragma warning (disable:4996)
#endif

//#define DUMP_FPS

static void __cdecl DP(LPCSTR szFormat, ...)
{
#ifndef ANDROID
#ifdef __linux__
	TCHAR szBuffer[256];
	va_list vl;
	va_start(vl, szFormat);
	_vsntprintf(szBuffer,255,szFormat, vl);
	szBuffer[255]=0;
	fprintf(stderr,"%s",szBuffer);
	va_end(vl);
#else	
	char buffer[256] = {0};
	char bufferTime[256] = {0};
	SYSTEMTIME time;	
	va_list ap;
	va_start(ap, szFormat);
	_vsnprintf(buffer, 255, (char*)szFormat, ap);
	GetSystemTime(&time);
	sprintf(bufferTime, "%d/%d/%d, %d:%d:%d ", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

#ifndef MT3351_DBG
#ifdef _DEBUG_CONSOLE
	TCHAR tBufferTime[MAX_PATH];
	TCHAR tBuffer[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, bufferTime, -1, tBufferTime, sizeof(tBufferTime));
	MultiByteToWideChar(CP_UTF8, 0, buffer, -1, tBuffer, sizeof(tBuffer));

	OutputDebugString(tBufferTime);
	OutputDebugString(tBuffer);
#endif
#endif
#ifdef WRITE_FILE
	FILE* f_txt = fopen(".\\vrpresent_log.txt", "a+");
	if(f_txt)
	{
		fprintf(f_txt, bufferTime);
		fprintf(f_txt, buffer);
		fclose(f_txt);
	}
#endif

	va_end(ap);
#endif
#else
#if 1

	char szBuffer[256]; 
	va_list vl; 
	va_start(vl, szFormat);

	memset(szBuffer, 0, sizeof(szBuffer));
	
	vsnprintf(szBuffer,255,szFormat,vl);
	szBuffer[255]=0;
	
	LOGD("%s", szBuffer);
	va_end(vl);
#endif
#endif
	return;
}


#if 0
#include <io.h>
#include <fcntl.h>

static void InitConsoleWindow(void)
{

	int hCrt;
	FILE *hf;
	AllocConsole();
	hCrt = _open_osfhandle((long)(GetStdHandle(STD_OUTPUT_HANDLE)),_O_TEXT);
	hf = _fdopen(hCrt,"w");
	*stdout = *hf;
	setvbuf(stdout,NULL,_IONBF,	0);
}
#endif

#endif
