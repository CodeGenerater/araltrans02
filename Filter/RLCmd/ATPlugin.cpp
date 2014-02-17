// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"

#include "resource.h"

#include "RLCmd.h"
#include "OptionDialog.h"

#ifdef _UNICODE
#include "tstring.h"
#endif

// ATCApi ����Ϸ��� include
//#include "ATCApi.h"

// Util ����Ϸ��� include
#include "Util.h"

#include "tstring.h"

#include <stdio.h>



/*
** �������� �����
*/

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080704 },		// �÷����� ���۽� AT ����
	{ 1, 0, 20081005 },		// �÷����� ����
	"whoami",	// ������
	"RLCmd",	// �÷����� �̸�
	"RealLive Ŀ�ǵ� ����"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

// debug
tstring g_strLogFile;

void ApplyOption();

void TrimCommand(const char *cszOriginal, string &strPrefix, string &strBody, string &strPostfix);

void RestoreCommand(string &strPrefix, const char *cszTranslated, string &strPostfix, string &strRestored, int nMaxSize);

void SqueezeAndReplaceSpecialString(string &strData, bool bCutSpace=false);

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
	
	ATOPTION_ARRAY aOptions;

	GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	for(int i=0; i<aOptions.size(); i++)
	{
		if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("DEBUG")))
		{
			FILE *fp;
			g_strLogFile=aOptions[i].strValue;
			fp=_tfopen(g_strLogFile.c_str(), _T("w"));
			fclose(fp);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("OVERWRITE")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cRLCmd.SetOverwriteSetting(false);
			else
				g_cRLCmd.SetOverwriteSetting(true);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("REMOVESPACE")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cRLCmd.SetRemoveSpaceSetting(false);
			else
				g_cRLCmd.SetRemoveSpaceSetting(true);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("BRACKET")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cRLCmd.SetBracketSetting(false);
			else
				g_cRLCmd.SetBracketSetting(true);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("NOFREEZE")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cRLCmd.SetNoFreezeSetting(false);
			else
				g_cRLCmd.SetNoFreezeSetting(true);
		}
		
	}
	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		
		if (DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_OPTIONDIALOG), g_hSettingWnd, OptionDialogProc) == IDOK)
			ApplyOption();
		

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

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_FILTER
BOOL  __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return g_cRLCmd.PreTranslate(cszInJapanese, szOutJapanese, nBufSize);
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return g_cRLCmd.PostTranslate(cszInKorean, szOutKorean, nBufSize);
}
#endif

void ApplyOption()
{
	ATOPTION_ARRAY aOptions;
	ATOPTION_PAIR option;

	if (!g_strLogFile.empty())
	{
		option.strKey=_T("DEBUG");
		option.strValue = g_strLogFile;
		aOptions.push_back(option);
	}
	if (g_cRLCmd.GetOverwriteSetting())
	{
		option.strKey=_T("OVERWRITE");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cRLCmd.GetRemoveSpaceSetting())
	{
		option.strKey=_T("REMOVESPACE");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cRLCmd.GetBracketSetting())
	{
		option.strKey=_T("BRACKET");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cRLCmd.GetNoFreezeSetting())
	{
		option.strKey=_T("NOFREEZE");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);

}
