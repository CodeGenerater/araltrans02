// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"

#include "resource.h"

#include "CmdFilter.h"
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
	{ 0, 3, 20080807 },		// �÷����� ����
	"whoami",	// ������
	"CmdFilter",	// �÷����� �̸�
	"Ŀ�ǵ� ����"	// �÷����� ����
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

	FILE *fp=NULL;

	GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	for(int i=0; i<aOptions.size(); i++)
	{
		if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("DEBUG")))
		{
			g_strLogFile=aOptions[i].strValue;
			fp=_tfopen(g_strLogFile.c_str(), _T("w"));
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("OVERWRITE")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cCmdFilter.SetOverwriteSetting(false);
			else
				g_cCmdFilter.SetOverwriteSetting(true);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("REMOVESPACE")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cCmdFilter.SetRemoveSpaceSetting(false);
			else
				g_cCmdFilter.SetRemoveSpaceSetting(true);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("TWOBYTE")))
		{
			if (!lstrcmpi(aOptions[i].strValue.c_str(), _T("FALSE")))
				g_cCmdFilter.SetTwoByteSetting(false);
			else
				g_cCmdFilter.SetTwoByteSetting(true);
		}
		else if (!lstrcmpi(aOptions[i].strKey.c_str(), _T("CUT")) )
		{
			int n=_ttoi(aOptions[i].strValue.c_str());

			if (n<0) n=0;

			if (fp) fprintf(fp, "CUT=%d\n", n);

			g_cCmdFilter.SetCutSizeSetting(n);
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("DENY")) )
		{
			g_cCmdFilter.SetDenySetting(aOptions[i].strValue);

			if (fp)
			{
#ifdef _UNICODE
				char strTemp[1024];
				Wide2Kor(aOptions[i].strValue.c_str(), strTemp, 1024);
				fprintf(fp, "Deny=%s\n", strTemp);
#else
				fprintf(fp, "Deny=%s\n", aOptions[i].strValue.c_str());
#endif
			}
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("PREINC")) )
		{
			g_cCmdFilter.SetPrefixIncludeSetting(aOptions[i].strValue);
			
			if (fp)
			{
#ifdef _UNICODE
				char strTemp[1024];
				Wide2Kor(aOptions[i].strValue.c_str(), strTemp, 1024);
				fprintf(fp, "PreInc=%s\n", strTemp);
#else
				fprintf(fp, "PreInc=%s\n", aOptions[i].strValue.c_str());
#endif
			}
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("PREEXC")) )
		{
			g_cCmdFilter.SetPrefixExcludeSetting(aOptions[i].strValue);
			
			if (fp)
			{
#ifdef _UNICODE
				char strTemp[1024];
				Wide2Kor(aOptions[i].strValue.c_str(), strTemp, 1024);
				fprintf(fp, "PreExc=%s\n", strTemp);
#else
				fprintf(fp, "PreExc=%s\n", aOptions[i].strValue.c_str());
#endif
			}
		}		
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("BODYINC")) )
		{
			g_cCmdFilter.SetBodyIncludeSetting(aOptions[i].strValue);
			
			if (fp)
			{
#ifdef _UNICODE
				char strTemp[1024];
				Wide2Kor(aOptions[i].strValue.c_str(), strTemp, 1024);
				fprintf(fp, "BodyInc=%s\n", strTemp);
#else
				fprintf(fp, "BodyInc=%s\n", aOptions[i].strValue.c_str());
#endif
			}
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("BODYEXC")) )
		{
			g_cCmdFilter.SetBodyExcludeSetting(aOptions[i].strValue);
			
			if (fp)
			{
#ifdef _UNICODE
				char strTemp[1024];
				Wide2Kor(aOptions[i].strValue.c_str(), strTemp, 1024);
				fprintf(fp, "BodyExc=%s\n", strTemp);
#else
				fprintf(fp, "BodyExc=%s\n", aOptions[i].strValue.c_str());
#endif
			}
		}		
	}	// for (..)
	if (fp)
		fclose(fp);

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
	return g_cCmdFilter.PreTranslate(cszInJapanese, szOutJapanese, nBufSize);
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return g_cCmdFilter.PostTranslate(cszInKorean, szOutKorean, nBufSize);
}
#endif

void ApplyOption()
{
	ATOPTION_ARRAY aOptions;
	ATOPTION_PAIR option;
	tstring strBoundary;

	if (!g_strLogFile.empty())
	{
		option.strKey=_T("DEBUG");
		option.strValue = g_strLogFile;
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetOverwriteSetting())
	{
		option.strKey=_T("OVERWRITE");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetRemoveSpaceSetting())
	{
		option.strKey=_T("REMOVESPACE");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetTwoByteSetting())
	{
		option.strKey=_T("TWOBYTE");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetCutSizeSetting())
	{
		TCHAR szBuf[10];
		option.strKey=_T("CUT");
		option.strValue=_itot(g_cCmdFilter.GetCutSizeSetting(), szBuf, 10);
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetDenySetting(strBoundary))
	{
		option.strKey=_T("DENY");
		option.strValue=strBoundary;
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetPrefixIncludeSetting(strBoundary))
	{
		option.strKey=_T("PREINC");
		option.strValue=strBoundary;
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetPrefixExcludeSetting(strBoundary))
	{
		option.strKey=_T("PREEXC");
		option.strValue=strBoundary;
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetBodyIncludeSetting(strBoundary))
	{
		option.strKey=_T("BODYINC");
		option.strValue=strBoundary;
		aOptions.push_back(option);
	}
	if (g_cCmdFilter.GetBodyExcludeSetting(strBoundary))
	{
		option.strKey=_T("BODYEXC");
		option.strValue=strBoundary;
		aOptions.push_back(option);
	}


	GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);

}
