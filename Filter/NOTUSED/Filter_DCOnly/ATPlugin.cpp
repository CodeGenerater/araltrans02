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
//#include "Util.h"

/*
** �������� �����
*/

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080816 },		// �÷����� ���۽� AT ����
	{ 1, 0, 20080817 },		// �÷����� ����
	"Hide_D",	// ������
	"D.C.2 Only",	// �÷����� �̸�
	"��ī��2 ���� �����Դϴ�"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

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
	size_t Len=strlen(cszInJapanese);
	size_t InN=0;
	size_t OutN=0;


	bool Ignore=false;
	while(InN<Len)
	{
		if(IsDBCSLeadByteEx(932,cszInJapanese[InN]))
		{
			if((BYTE)cszInJapanese[InN]==0x81&&(BYTE)cszInJapanese[InN+1]==0x6F)
			{
				Ignore=true;
				InN+=2;
			}
			else if((BYTE)cszInJapanese[InN]==0x81&&(BYTE)cszInJapanese[InN+1]==0x5E)
			{
				Ignore=false;
				InN+=2;
			}
			else if((BYTE)cszInJapanese[InN]==0x81&&(BYTE)cszInJapanese[InN+1]==0x70)
			{
				//�� ����[..]
				InN+=2;
			}
			else if(!Ignore)
			{
				szOutJapanese[OutN++]=cszInJapanese[InN++];
				szOutJapanese[OutN++]=cszInJapanese[InN++];
			}
			else
			{
				InN+=2;
			}
		}
		else if(cszInJapanese[InN]!='\n' && !Ignore)
		{
			szOutJapanese[OutN++]=cszInJapanese[InN++];
		}
		else
		{
			InN++;
		}
	}
	szOutJapanese[OutN]='\0';
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.

	string Temp1=cszInKorean;
	string Temp2="";

	string::size_type Len1=Temp1.length();
	
	string::size_type SPoint1=0;
	int NLineChar=0;
	while(SPoint1<Len1){
		if(NLineChar>=22)
		{
			Temp2+='\n';
			NLineChar=0;
		}

		if(Temp1[SPoint1]==' '){
			//���� ���� =��=;;
			SPoint1++;
		}
		else if(IsDBCSLeadByteEx(949,Temp1[SPoint1]))
		{
			Temp2+=Temp1[SPoint1++];
			Temp2+=Temp1[SPoint1++];
			NLineChar++;
		}
		else
		{
			Temp2+=(char)0xA3;
			Temp2+=Temp1[SPoint1++]+(char)0x80;
			NLineChar++;
		}
	}

	lstrcpyA(szOutKorean, Temp2.c_str());
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
#endif
