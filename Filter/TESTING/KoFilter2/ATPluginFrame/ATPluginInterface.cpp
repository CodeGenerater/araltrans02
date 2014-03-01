// ATPluginInterface - �ƶ�Ʈ���� �÷����� �������̽� �Ծ�
//

#include "StdAfx.h"
#include "ATPluginInterface.h"
#include "../ATPlugin.h"

/*
** �÷����� export �Լ�
*/

// ����
BOOL  __stdcall OnPluginInit(HWND hAralWnd, LPVOID pvPluginOption)
{
	BOOL bRetVal;

	bRetVal = theApp._PrePluginInit(hAralWnd, pvPluginOption);

#ifdef ATPLUGIN_ALGORITHM
	if ( bRetVal && theApp.m_dwAralVersion == MAKELONG(0003,0000) ) theApp.OnPluginStart();
#endif
	
	return bRetVal;
}

BOOL  __stdcall OnPluginOption()
{
	return theApp.OnPluginOption();
}

BOOL  __stdcall OnPluginClose()
{
#ifdef ATPLUGIN_ALGORITHM
	if ( theApp.m_dwAralVersion == MAKELONG(0003,0000) ) theApp.OnPluginStop();
#endif

	return theApp.OnPluginClose();
}

#ifdef USE_ARALTRANS_0_3	// �ƶ�Ʈ���� 0.3 ����
BOOL __stdcall GetPluginInfo(PLUGIN_INFO * pPluginInfo)
{
	return theApp.GetPluginInfo(pPluginInfo);
}
#endif	// USE_ARALTRANS_0_3


#ifdef USE_ARALTRANS_0_2	// �ƶ�Ʈ���� 0.2 ����

// ��ŷ �÷����� ����
#ifdef ATPLUGIN_ALGORITHM
BOOL  __stdcall OnPluginStart()
{
	return theApp.OnPluginStart();
}
BOOL  __stdcall OnPluginStop()
{
	return theApp.OnPluginStop();
}
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
BOOL  __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
{
	return theApp.Translate(cszJapanese, szKorean, nBufSize);
}
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_FILTER
BOOL  __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	return theApp.PreTranslate(cszInJapanese,szOutJapanese,nBufSize);
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	return theApp.PostTranslate(cszInKorean, szOutKorean, nBufSize);
}
#endif
#endif // USE_ARALTRANS_0_2


#ifdef USE_ARALTRANS_0_3
// ���� �� ���� �÷����� ����
#if defined(ATPLUGIN_FILTER) || defined(ATPLUGIN_TRANSLATOR)
BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectInit(pTransObj);
}
BOOL __stdcall OnObjectClose(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectClose(pTransObj);
}
BOOL __stdcall OnObjectMove(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectMove(pTransObj);
}
BOOL __stdcall OnObjectOption(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.OnObjectOption(pTransObj);
}

BOOL __stdcall MainTranslateProcedure(TRANSLATION_OBJECT* pTransObj)
{
	return theApp.MainTranslateProcedure(pTransObj);
}

#endif
#endif // USE_ARALTRANS_0_3

#ifndef USE_MFC

// DLLMain
// ��ü ���ҽ��� �ִ� ��� ��� �ڵ��� �˾ƿ��� ���� �ʿ�
// MFC ������ CWinApp �� ���� �� �ֱ� ������ �ʿ����..
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		theApp.m_hInstance = (HINSTANCE) hModule;
		theApp.InitInstance();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}
#endif // USE_MFC
