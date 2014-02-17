// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"

// ATCApi ����Ϸ��� include
//#include "ATCApi.h"

// Util ����Ϸ��� include
//#include "Util.h"

#include "tstring.h"

#pragma warning(disable:4305 4309)

/*
** �������� �����
*/

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080704 },		// �÷����� ���۽� AT ����
	{ 0, 1, 20080726 },		// �÷����� ����
	"whoami",	// ������
	"preHK2FK",	// �÷����� �̸�
	"Shift-JIS �ݰ�-���� ��ȯ ����"	// �÷����� ����
//	.12345678901234567890123456789012
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;


// HK2FK
void HK2FK(const char *pszIn, char *pszOut);

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
	// ATOPTION_ARRAY aOptions;
	// GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.

		char szTemp[1024];
		wsprintfA(szTemp, "%s : %s\r\n\r\n������ %s, �÷����� ���� %d.%d.%d\r\n\r\n�ʿ� AT ���� %d.%d.%d\r\n",
			g_pluginVer.szPluginName, g_pluginVer.szDescription,
			g_pluginVer.szAuthor, 
			g_pluginVer.PluginVersion.Major, g_pluginVer.PluginVersion.Minor, g_pluginVer.PluginVersion.BuildDate,
			g_pluginVer.ATVersion.Major, g_pluginVer.ATVersion.Minor, g_pluginVer.ATVersion.BuildDate);

#ifdef _UNICODE
		WCHAR wszTemp[1024], wszName[16];
		Kor2Wide(szTemp, wszTemp, 1024);
		Kor2Wide(g_pluginVer.szPluginName, wszName, 16);
		MessageBox(g_hSettingWnd, wszTemp, wszName, MB_OK);
#else
		MessageBox(g_hSettingWnd, szTemp, g_pluginVer.szPluginName, MB_OK);
#endif

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

	HK2FK(cszJapanese, szKorean);

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
	HK2FK(cszInJapanese, szOutJapanese);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	wsprintfA(szOutKorean, "%s", cszInKorean);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
#endif


void HK2FK(const char *pszIn, char *pszOut)
{
	const char szHK2FKTable[][2] = {
		{0x81, 0x40}, {0x81, 0x42}, {0x81, 0x75}, {0x81, 0x76}, 	 // 0xA0... 0xA3
		{0x81, 0x41}, {0x81, 0x45}, {0x83, 0x92}, {0x83, 0x40}, 	 // 0xA4... 0xA7
		{0x83, 0x42}, {0x83, 0x44}, {0x83, 0x46}, {0x83, 0x48}, 	 // 0xA8... 0xAB
		{0x83, 0x83}, {0x83, 0x85}, {0x83, 0x87}, {0x83, 0x62}, 	 // 0xAC... 0xAF
		{0x81, 0x5B}, {0x83, 0x41}, {0x83, 0x43}, {0x83, 0x45}, 	 // 0xB0... 0xB3
		{0x83, 0x47}, {0x83, 0x49}, {0x83, 0x4A}, {0x83, 0x4C}, 	 // 0xB4... 0xB7
		{0x83, 0x4E}, {0x83, 0x50}, {0x83, 0x52}, {0x83, 0x54}, 	 // 0xB8... 0xBB
		{0x83, 0x56}, {0x83, 0x58}, {0x83, 0x5A}, {0x83, 0x5C}, 	 // 0xBC... 0xBF
		{0x83, 0x5E}, {0x83, 0x60}, {0x83, 0x63}, {0x83, 0x65}, 	 // 0xC0... 0xC3
		{0x83, 0x67}, {0x83, 0x69}, {0x83, 0x6A}, {0x83, 0x6B}, 	 // 0xC4... 0xC7
		{0x83, 0x6C}, {0x83, 0x6D}, {0x83, 0x6E}, {0x83, 0x71}, 	 // 0xC8... 0xCB
		{0x83, 0x74}, {0x83, 0x77}, {0x83, 0x7A}, {0x83, 0x7D}, 	 // 0xCC... 0xCF
		{0x83, 0x7E}, {0x83, 0x80}, {0x83, 0x81}, {0x83, 0x82}, 	 // 0xD0... 0xD3
		{0x83, 0x84}, {0x83, 0x86}, {0x83, 0x88}, {0x83, 0x89}, 	 // 0xD4... 0xD7
		{0x83, 0x8A}, {0x83, 0x8B}, {0x83, 0x8C}, {0x83, 0x8D}, 	 // 0xD8... 0xDB
		{0x83, 0x8F}, {0x83, 0x93}, {0x81, 0x4A}, {0x81, 0x4B}  	 // 0xDC... 0xDF
	};
	
	
	int idxIn, idxOut;
	int nInLen;
	BYTE chNow;
	nInLen=lstrlenA(pszIn);
	
	idxIn=idxOut=0;
	
	while(idxIn < nInLen)
	{
		// ���� �޾ƿ�
		chNow=(BYTE)pszIn[idxIn];
		
		// ���� = LeadByte?
		if (IsDBCSLeadByteEx(932, chNow))
//		if ( ((0x81 <= chNow) && (chNow < 0xA0)) || ((0xE0 <=chNow) && (chNow < 0xFF)) )
		{
			// LeadByte �̸� 2�� ����
			pszOut[idxOut]=pszIn[idxIn];
			pszOut[idxOut+1]=pszIn[idxIn+1];
			
			// index ����
			idxIn+=2;
			idxOut+=2;
		}
		else if( (0xA0 <=chNow) && (chNow < 0xE0) )
		{
			// ���� = �ݰ� ī��
			chNow-=0xA0;
			pszOut[idxOut]=szHK2FKTable[chNow][0];
			pszOut[idxOut+1]=szHK2FKTable[chNow][1];

			// Ź��, ��Ź�� ó��
			chNow=(BYTE)pszIn[idxIn+1];
			if ( chNow == 0xDE && (BYTE)pszIn[idxIn] != 0xB3)
			{
				// Ź��
				pszOut[idxOut+1]++;
				idxIn+=2;
			}
			else if ( chNow == 0xDF)
			{
				// ��Ź��
				pszOut[idxOut+1]+=2;
				idxIn+=2;
			}
			else if ( chNow == 0xDE && (BYTE)pszIn[idxIn] == 0xB3)
			{
				// ���� Ź��
				pszOut[idxOut+1]+=79;
				idxIn+=2;
			}
			else
			{
				// �Ϲ�
				idxIn++;
			}
			idxOut+=2;
		}
		else
		{
			// �Ϲ� ASCII
			pszOut[idxOut]=pszIn[idxIn];
			idxIn++;
			idxOut++;
		}
		
	}	// while(...)
	pszOut[idxOut]='\0';
}