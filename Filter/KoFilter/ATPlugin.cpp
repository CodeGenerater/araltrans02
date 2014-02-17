// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"

#ifdef _UNICODE
#include "tstring.h"
#endif

// ATCApi ����Ϸ��� include
//#include "ATCApi.h"

// Util ����Ϸ��� include
#include "Util.h"

#include "resource.h"
#include "OptionDialog.h"
#include "FileOptionParser.h"
#include "SubFunc.h"
#include "hash.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <set>

using namespace std;


/*
** �������� �����
*/

// �⺻ ���� ���� = �ѱ� > �Ͼ�x2
#define LEVEL_DEFAULT 10

// ���� ����
int g_nLevel=LEVEL_DEFAULT;

// Ư������ ��� ����
bool g_bNoRoman = false;
bool g_bNoRound = false;

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20081207 },		// �÷����� ���۽� AT ����
	{ 1, 0, 20090114 },		// �÷����� ����
	"whoami",	// ������
	"KoFilter",	// �÷����� �̸�
	"�ѱ� �ߺ����� ���� �÷�����"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

bool g_bLogJp=false;
bool g_bLogKo=false;

set <UINT> g_Deny;
set <UINT> g_Allow;

int g_nAllow = 0;
int g_nDeny = 0;

int IsDenyAllow(const char *pszTestString);
BOOL IsKorean(const char *pszTestString, bool bCheckEncodeKor=true);
BOOL DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode);

void OpenDenyAllow();
void ApplySetting();

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
		if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("LOG")) )
		{
			if ( !lstrcmpi(aOptions[i].strValue.c_str(), _T("JP")) )
				g_bLogJp=true;
			else if ( !lstrcmpi(aOptions[i].strValue.c_str(), _T("KO")) )
				g_bLogKo=true;

			FILE *fp=fopen("c:\\KoFilter.log", "w");
			fclose(fp);
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("LEVEL")) )
			g_nLevel= _ttoi(aOptions[i].strValue.c_str());
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("NOROMAN")) )
			g_bNoRoman=true;
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("NOROUND")) )
			g_bNoRound=true;

	}

	g_cOption.SetOptionFile(L"ATData\\KoFilter.dat");

	OpenDenyAllow();
	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	OpenDenyAllow();
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.

		if (DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_OPTIONDIALOG), g_hSettingWnd, OptionDialogProc) == IDOK)
			ApplySetting();
		
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

	if (IsKorean(cszJapanese))
		return FALSE;
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
	int i = IsDenyAllow(cszInJapanese);
	if (i!=0)
	{
		if(i==1)
			return FALSE;
		else
			return TRUE;
	}
	else if (IsKorean(cszInJapanese))
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

void OpenDenyAllow()
{
	g_nAllow = 0;
	g_nDeny = 0;

	g_cOption.Clear();
	g_cOption.OpenOptionFile();

	g_Allow.clear();
	g_Deny.clear();

	//Deny
	for(int i=1;;i++)
	{
		if(g_cOption.IsInData(L"Deny",i))
		{
			g_Deny.insert(MakeStringHash(wtoa(g_cOption.GetOptionString(L"Deny",i)).c_str()));
			g_nDeny++;
		}
		else
			break;
	}

	for(int i=1;;i++)
	{
		if(g_cOption.IsInData(L"Allow",i))
		{
			g_Allow.insert(MakeStringHash(wtoa(g_cOption.GetOptionString(L"Allow",i)).c_str()));
			g_nAllow++;
		}
		else
			break;
	}
}

int IsDenyAllow(const char *pszTestString)
{
	UINT uHash = MakeStringHash(pszTestString);

	if(g_Deny.count(uHash)!=0)
	{
		return 1;
	}
	else if(g_Allow.count(uHash)!=0)
	{
		return 2;
	}
	else
		return 0;
}

BOOL IsKorean(const char *pszTestString, bool bCheckEncodeKor)
{
	volatile BYTE szBuf[3] = { '\0', };
	WORD wch;
	int nKoCount, nJpCount;

	int i, nLen;

	BOOL bIsKorean;

	nLen=lstrlenA(pszTestString);

	// �⺻ üũ ��ƾ
	nKoCount=nJpCount=0;
	for (i=0; i<nLen; i++)
	{
		bIsKorean=FALSE;	// ���⼭�� �� ������ �ѱ�����

		// üũ ����Ʈ
		szBuf[0]=(BYTE) pszTestString[i];
		szBuf[1]=(BYTE) pszTestString[i+1];

		wch=MAKEWORD(pszTestString[i+1], pszTestString[i]);

		if (szBuf[0] < 0x80)	// 1����Ʈ �ڵ�
			continue;	// ����

		if (bCheckEncodeKor && (					// EncodeKor �ڵ���� (CCharacterMapper::EncodeK2J from ATCode)
			( (0x889F <= wch) && (wch <= 0x9872) ) ||	// 0x889F - 0x9872,
			( (0x989F <= wch) && (wch <= 0x9FFC) ) ||	// 0x989F - 0x9FFC,
			( (0xE040 <= wch) && (wch <= 0xEAA4) ) ||	// 0xE040 - 0xEAA4,
			( (0xED40 <= wch) && (wch <= 0xEEEC) )		// 0xED40 - 0xEEEC
			))
		{
			// �ѱ��ΰ�? (EncodeKor)

			// Decode �غ���
			DecodeJ2K(pszTestString+i, (char *)szBuf);
		}

		// KSC-5601 : 0xA1A1 - 0xA1FE
		//   �ڵ�        -        -
		//   ����     0xFEA1 - 0xFEFE
		// ù��° ����Ʈ 
		// 0xA1 : ������, ���� ��ȣ
		// 0xA2 : ��ȣ (ȭ��ǥ, ��Ʈ ��)
		// 0xA3 : ���� 0-9A-Za-z��
		// 0xA4 : ����, ����, ���
		// 0xA5 : �θ�����, �θ�����
		// 0xA6 : ������ (ǥ ��)
		// 0xA7 : ������
		// 0xA8 : ������
		// 0xA9 : ��ȣ����
		// 0xAA : ���󰡳�
		// 0xAB : īŸī��
		// 0xAC : ���þƾ�
		// 0xAD-0xAF : ��Ÿ
		// 0xB0-0xC8 : �ѱ� (��-��)
		// 0xC9 : ��Ÿ
		// 0xCA-0xFD : ���� (ʡ-��)
		// 0xFE : ��Ÿ
		// �ι�° ����Ʈ
		// 0xA1-0xFE ���̸� ��ȿ

		// 1. �ѱ� üũ (0xB0-0xC8)
		if (((0xB0 <= szBuf[0])&&(szBuf[0] <= 0xC8)) && 
			((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xFE)))
			bIsKorean=TRUE;
		// 2. Ư������ - ������ (0xA1), ���� ����/������(0xA3)
		else if (((0xA1 == szBuf[0])||(0xA3 == szBuf[0])) && 
			((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xFE)))
			bIsKorean=TRUE;
		// 3. �θ�����
		else if (!g_bNoRoman && (0xA5 == szBuf[0]) &&
			( ((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xAA)) ||	// ��-��
			 ((0xB0 <= szBuf[1])&&(szBuf[1] <= 0xB9)) ) 	// ��-��
			)
			bIsKorean=TRUE;
		// 4. ������
		else if (!g_bNoRound && (0xA8 == szBuf[0]) &&
			((0xE7 <= szBuf[1])&&(szBuf[1] <= 0xF5))	// �� - ��
			)
			bIsKorean=TRUE;

		if (bIsKorean)
		{
			// �ѱ�
			nKoCount++;
			i++; // �̹� üũ�����Ƿ� ���� ���ڷ� �Ѿ	
		}
		else
		{
			// �Ͼ�
			nJpCount++;
			if (IsDBCSLeadByteEx(932, pszTestString[i]))
				i++;
		}
		// ��Ÿ ���ڴ� ����
	}

	// ���� ����
	bIsKorean=FALSE;	// ���⼭���ʹ� ��ü ������ �ѱ� ����
	switch(g_nLevel)
	{
		case 0: // �Ͼ 1�ڶ� ������ ����
			if (!nJpCount)
				bIsKorean=TRUE;
			break;

		case 5:	// �ѱ� > �Ͼ� �϶� ���� ����
			if (nKoCount > nJpCount)
				bIsKorean=TRUE;
			break;

		case 10:	// �ѱ� > �Ͼ�x2 �϶� ���� ����
			if (nKoCount > nJpCount*2)
				bIsKorean=TRUE;
			break;

		case 15:	// �ѱ� > �Ͼ�x3 �϶� ���� ����
			if (nKoCount > nJpCount*3)
				bIsKorean=TRUE;
			break;

		case 20:	// �ѱ��� 1�ڶ� ������ ���� ����
			if (nKoCount)
				bIsKorean=TRUE;
			break;

		default:
			if (nKoCount > nJpCount*2)
				bIsKorean=TRUE;

	}

	if ( (g_bLogKo && bIsKorean) || (g_bLogJp && !bIsKorean) )
	{
		FILE *fp=fopen("c:\\KoFilter.log", "a");
		
		int nSize = lstrlenA(pszTestString);
		
		if (bIsKorean)
			fprintf(fp, "Korean : %s\n", pszTestString);
		else
			fprintf(fp, "Japanese : %s\n", pszTestString);

		fprintf(fp, "Decode : ");
		
		for(int i=0; i<nSize; i++)
		{
			if ((BYTE)pszTestString[i] > 0x80)
			{
				DecodeJ2K(pszTestString+i, (char *)szBuf);
				fprintf(fp,"%s", szBuf);
				i++;
			}
			else
				fprintf(fp, "%c", pszTestString[i]);
		}
		fprintf(fp, "\nCount(Ko/Jp) = (%d/%d), Level = %d\n", nKoCount, nJpCount, g_nLevel);
		fclose(fp);
	}

	return bIsKorean;

}

BOOL DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode)
{
	BOOL bRetVal = FALSE;

	// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
	// 95~3074, 3119~4535, 16632~18622, 19089~19450
	if(cszJpnCode && szKorCode && 0x88 <= (BYTE)cszJpnCode[0] && (BYTE)cszJpnCode[0] <= 0xEE)
	{
		WORD dwKorTmp = (WORD)( (BYTE)cszJpnCode[0] - 0x88 ) * 189;
		dwKorTmp += (WORD)( (BYTE)cszJpnCode[1] - 0x40 );

		if(dwKorTmp > 19450) return FALSE;
		if(dwKorTmp > 18622) dwKorTmp -= 466;
		if(dwKorTmp > 4535) dwKorTmp -= 12096;
		if(dwKorTmp > 3074) dwKorTmp -= 44;
		dwKorTmp -= 95;

		szKorCode[0] = (char)( ((UINT_PTR)dwKorTmp/(UINT_PTR)0xA0) + (UINT_PTR)0xA0);
		szKorCode[1] = (char)( ((UINT_PTR)dwKorTmp%(UINT_PTR)0xA0) + (UINT_PTR)0xA0);

		//TRACE("[ aral1 ] DecodeJ2K : (%p) -> (%p)'%s' \n", (UINT_PTR)(MAKEWORD(cszJpnCode[1], cszJpnCode[0])), (UINT_PTR)(MAKEWORD(szKorCode[1], szKorCode[0])), szKorCode);

		if( 0xA0 <= (BYTE)szKorCode[0] && 0xA0 <= (BYTE)szKorCode[1] )
		{
			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA1 )
			{
				*((BYTE*)szKorCode) = 0xBE;
				*((BYTE*)szKorCode+1) = 0xC6;
			}

			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA2 )
			{
				*((BYTE*)szKorCode) = 0xB7;
				*((BYTE*)szKorCode+1) = 0xB8;
			}

			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA3 )
			{
				*((BYTE*)szKorCode) = 0xB8;
				*((BYTE*)szKorCode+1) = 0xD5;
			}

			bRetVal = TRUE;
		}
	}

	return bRetVal;
}

void ApplySetting()
{
	ATOPTION_ARRAY aOptions;
	ATOPTION_PAIR option;

	if (g_nLevel != LEVEL_DEFAULT)
	{
		TCHAR szLevel[10];
		option.strKey=_T("LEVEL");
		option.strValue=_itot(g_nLevel, szLevel, 10);
		aOptions.push_back(option);
	}
	if (g_bLogJp)
	{
		option.strKey=_T("LOG");
		option.strValue=_T("JP");
		aOptions.push_back(option);
	}
	if (g_bLogKo)
	{
		option.strKey=_T("LOG");
		option.strValue=_T("KO");
		aOptions.push_back(option);
	}
	if (g_bNoRoman)
	{
		option.strKey=_T("NOROMAN");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	if (g_bNoRound)
	{
		option.strKey=_T("NOROUND");
		option.strValue.erase();
		aOptions.push_back(option);
	}
	GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);
}