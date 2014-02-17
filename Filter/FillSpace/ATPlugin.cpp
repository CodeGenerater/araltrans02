// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"

HWND g_hSettingWnd=NULL;
HINSTANCE g_hThisModule;
LPSTR g_szOptionStringBuffer=NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hThisModule = (HINSTANCE) hModule;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// �÷����� export �Լ�

// ����
BOOL  __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer)
{
	g_hSettingWnd = hSettingWnd;
	g_szOptionStringBuffer=cszOptionStringBuffer;

	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	
	return TRUE;
}
BOOL  __stdcall OnPluginClose()
{

	return TRUE;
}

//���⼭���� ����
int nOriginalLength=0;

BOOL  __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	nOriginalLength = strlen(cszInJapanese);
	memcpy_s(szOutJapanese,nBufSize,cszInJapanese,nOriginalLength+1);

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	int nNowLength=strlen(cszInKorean);
	memcpy_s(szOutKorean,nBufSize,cszInKorean,nNowLength+1);

	if(nNowLength<nOriginalLength)
	{
		memset(szOutKorean+nNowLength,nOriginalLength-nNowLength,' ');
		szOutKorean[nOriginalLength+1]='\0';
	}

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}