// ATPlugin.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "ATPlugin.h"

/*
** �������� �����
*/

bool gMode[5]={0,0,0,0,0};

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080712 },		// �÷����� ���۽� AT ����
	{ 0, 1, 20080801 },		// �÷����� ����
	"Hide_D��",	// ������
	"DumpText",	// �÷����� �̸�
	"����,������ ���"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �ؽ�Ʈâ ������ �ڵ�
HWND hDumpText=NULL;
HWND _hDumpText=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

LPSTR ConvertHex(LPCSTR source,LPSTR complete,int nCodePage);
void DumpText(LPCSTR source,int Type);
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

	ATOPTION_ARRAY aOptions;
	GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	for (unsigned int i=0; i<aOptions.size(); i++)
	{
		// SET(1100) ���� ��������
		if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("SET")) )
		{
			if(aOptions[i].strValue[0]==_T('1'))
				gMode[0]=true;
			else
				gMode[0]=false;

			if(aOptions[i].strValue[1]==_T('1'))
				gMode[1]=true;
			else
				gMode[1]=false;

			if(aOptions[i].strValue[2]==_T('1'))
				gMode[2]=true;
			else
				gMode[2]=false;

			if(aOptions[i].strValue[3]==_T('1'))
				gMode[3]=true;
			else
				gMode[3]=false;

			if(aOptions[i].strValue[4]==_T('1'))
				gMode[4]=true;
			else
				gMode[4]=false;
		}
	}

	//â ����
	hDumpText=CreateDialog((HINSTANCE)g_hThisModule,MAKEINTRESOURCE(IDD_Window),_hDumpText,DumpTextProc);
	ShowWindow(hDumpText,SW_SHOW);

	//Start���
	SendMessage(hDumpText, WM_USER+1, 0, 0);
	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.

		if(DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_Option), g_hSettingWnd, OptionDialogProc)==IDOK){
			ApplySetting();
		}
	}
	return TRUE;
}
BOOL  __stdcall OnPluginClose()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: �÷����� ���� �غ�, �ڿ� ��ȯ ���� �Ѵ�.

	// �������� �ݾƾ��� =��=
	DestroyWindow(hDumpText);
	hDumpText=NULL;
	return TRUE;
}
const ATPLUGIN_VERSION * __stdcall OnPluginVersion()
{
	// �÷����� ���� ��ȯ

	return &g_pluginVer;
}

BOOL  __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutJapanese, cszInJapanese);
	
	char temp[2048];

	if(cszInJapanese[0]!='\0')
	{
		if(gMode[0])
			DumpText(cszInJapanese,0);
		if(gMode[1])
			DumpText(ConvertHex(cszInJapanese,temp,932),1);
	}
	else
		szOutJapanese[0]='\0';

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutKorean, cszInKorean);
	
	char temp[2048];
	if(cszInKorean[0]!='\0')
	{
		if(gMode[2])
			DumpText(cszInKorean,2);
		if(gMode[3])
			DumpText(ConvertHex(cszInKorean,temp,949),3);
	}
	else
		szOutKorean[0]='\0';

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

LPSTR ConvertHex(LPCSTR source,LPSTR Temp, int nCodePage)
{
	size_t Len=strlen(source);

	BYTE szBuf[3] = { '\0', };

	int j=0;

	for(size_t i=0;i<Len;i++){
		szBuf[0]=(BYTE) source[i];
		szBuf[1]=(BYTE) source[i+1];
		if(!IsDBCSLeadByteEx(nCodePage,szBuf[0]))//1����Ʈ �ڵ���
		{
			j += sprintf_s(Temp+j,2048,"%02X ",szBuf[0]);
		}
		else{ //2����Ʈ���
			j += sprintf_s(Temp+j,2048,"%02X%02X ",szBuf[0],szBuf[1]);
			i++;
		}
	}
	return Temp;
}

void DumpText(LPCSTR source,int Type){
	wchar_t Temp[2048]=L"";
	int T=strlen(source);
	if(Type==0){
		MyMultiByteToWideChar(932,0,source,-1,Temp,2048);
	}
	else if(Type==2){
		MyMultiByteToWideChar(949,0,source,-1,Temp,2048);
	}
	else{
		for(int i=0;i<T;i++){
			Temp[i]=source[i];
		}
	}
	SendMessage(hDumpText, WM_USER, 0, (LPARAM)Temp);
}

void ApplySetting(){
	ATOPTION_ARRAY aOptions;
	ATOPTION_PAIR option;


	option.strKey=L"SET";

	if(gMode[0]==0)
		option.strValue.push_back(L'0');
	else
		option.strValue.push_back(L'1');

	if(gMode[1]==0)
		option.strValue.push_back(L'0');
	else
		option.strValue.push_back(L'1');

	if(gMode[2]==0)
		option.strValue.push_back(L'0');
	else
		option.strValue.push_back(L'1');

	if(gMode[3]==0)
		option.strValue.push_back(L'0');
	else
		option.strValue.push_back(L'1');

	if(gMode[4]==0)
		option.strValue.push_back(L'0');
	else
		option.strValue.push_back(L'1');
	aOptions.push_back(option);
	
	SendMessage(hDumpText, WM_USER+1, 0, (LPARAM)gMode[4]);

	GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);
}