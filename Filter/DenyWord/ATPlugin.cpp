// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "resource.h"

#include "OptionDlg.h"

#ifdef _UNICODE
#include "tstring.h"
#endif

// ATCApi ����Ϸ��� include
//#include "ATCApi.h"

// Util ����Ϸ��� include
#include "Util.h"

#include "DenyWord.h"

tstring g_strDebugFile;

/*
** �������� �����
*/

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080704 },		// �÷����� ���۽� AT ����
	{ 0, 1, 20080811 },		// �÷����� ����
	"whoami",	// ������
	"DenyWord",	// �÷����� �̸�
	"Ư�� �ܾ� �����ź� ����"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

void ApplyOption();

// DLLMain
// ��ü ���ҽ��� �ִ� ��� ��� �ڵ��� �˾ƿ��� ���� �ʿ�
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

	// TODO : �÷������� �ʱ�ȭ, �ڿ� Ȯ��, �ɼ� �м����� �Ѵ�.

	// ATCApi ����
	// if ( !g_cATCApi.IsValid() ) return FALSE;

	// Util �� �ɼ� �ļ� ��� ��
	ATOPTION_ARRAY aOptions;
	GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	for (int i=0; i<aOptions.size(); i++)
	{
		if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("DEBUG")) )
			g_strDebugFile = aOptions[i].strValue;
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("CUT")) )
			g_cDenyWord.SetCutSetting(_ttoi(aOptions[i].strValue.c_str()));
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("PASS")) )
			g_cDenyWord.SetPassKanjiSetting(_ttoi(aOptions[i].strValue.c_str()));
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("NOTKANJI")) )
			g_cDenyWord.SetNotKanjiSetting(true);
	}


	g_cDenyWord.Load();
	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.
		g_cDenyWord.Load();

		//���̾�α�
		if (DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_DIALOG1), g_hSettingWnd, OptionDialogProc) == IDOK)
		{
			ApplyOption();
		}
		
		// Util �� �ɼ� �ļ� ��� ��
		// ATOPTION_ARRAY aOptions;
		// ReadOptionsFromSomewhere(aOptions); // ��ܰ����� �ɼ��� �о�´�
		// GetOptionStringFromATPluginArgs(aOptions, g_szOptionStringBuffer, 4096);	// �ɼ��� ���ڿ��� �ٲ� �ƶ�Ʈ������ �ѱ��.

	}
	return TRUE;
}
BOOL  __stdcall OnPluginClose()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: �÷����� ���� �غ�, �ڿ� ��ȯ ���� �Ѵ�.

	return TRUE;
}
const ATPLUGIN_VERSION * __stdcall OnPluginVersion()
{
	// �÷����� ���� ��ȯ

	return &g_pluginVer;
}

// ��ŷ �÷����� ����
#ifdef ATPLUGIN_HOOKER
BOOL  __stdcall OnPluginStart()
{
	// TODO: ��ŷ ���� �۾�.

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
BOOL  __stdcall OnPluginStop()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: ��ŷ ���� �۾�.

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
BOOL  __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
{
	// TODO: �����۾��� �Ѵ�.

	lstrcpyA(szKorean, cszJapanese);

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_FILTER
BOOL  __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutJapanese, cszInJapanese);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 

	if (g_cDenyWord.IsDenied(cszInJapanese))
		return FALSE;
	return TRUE;
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutKorean, cszInKorean);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
#endif



void ApplyOption()
{
	ATOPTION_ARRAY aOptions;
	ATOPTION_PAIR option;
	tstring strBoundary;

	if (!g_strDebugFile.empty())
	{
		option.strKey=_T("DEBUG");
		option.strValue = g_strDebugFile;
		aOptions.push_back(option);
	}
	if (g_cDenyWord.GetPassKanjiSetting()>0)
	{
		TCHAR szbuf[10];
		option.strKey=_T("PASS");
		option.strValue=_itot(g_cDenyWord.GetPassKanjiSetting(),szbuf,10);
		aOptions.push_back(option);
	}
	if (g_cDenyWord.GetNotKanjiSetting())
	{
		option.strKey=_T("NOTKANJI");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cDenyWord.GetCutSetting()>0)
	{
		TCHAR szbuf[10];
		option.strKey=_T("CUT");
		option.strValue=_itot(g_cDenyWord.GetCutSetting(),szbuf,10);
		aOptions.push_back(option);
	}


	GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);

}
