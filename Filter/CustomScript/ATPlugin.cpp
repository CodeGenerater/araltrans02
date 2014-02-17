// ATPlugin.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "tstring.h"
#include "CustomScript.h"
#include "CSDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Util ����Ϸ��� include
#include "Util.h"


/*
** �������� �����
*/
CCustomScript CustomScript;
CCSDlg CSDlg;

bool isWrite=false;
bool ScriptChecked=false;
bool Cached=false;
char TempJpn[4096]="";
bool AlwaysTrans=false;

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080704 },		// �÷����� ���۽� AT ����
	{ 0, 1, 20080810 },		// �÷����� ����
	"Hide_D��",	// ������
	"CustomScript",	// �÷����� �̸�
	"����� �뺻 �÷�����"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

// CATPluginApp

BEGIN_MESSAGE_MAP(CATPluginApp, CWinApp)
END_MESSAGE_MAP()

// CATPluginApp ����

CATPluginApp::CATPluginApp()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}


// ������ CATPluginApp ��ü�Դϴ�.

CATPluginApp theApp;

// Export Functions
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer)
{
	return theApp.OnPluginInit(hSettingWnd, cszOptionStringBuffer);
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	return theApp.OnPluginOption();
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	return theApp.OnPluginClose();
}

extern "C" __declspec(dllexport) BOOL __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	return theApp.PreTranslate(cszInJapanese, szOutJapanese, nBufSize);
}

extern "C" __declspec(dllexport) BOOL __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	return theApp.PostTranslate(cszInKorean, szOutKorean, nBufSize);
}

extern "C" __declspec(dllexport) const ATPLUGIN_VERSION * __stdcall OnPluginVersion()
{
	// �÷����� ���� ��ȯ

	return _OnPluginVersion();
}

const ATPLUGIN_VERSION * _OnPluginVersion()
{
	// �÷����� ���� ��ȯ

	return &g_pluginVer;
}




// CATPluginApp �ʱ�ȭ

BOOL CATPluginApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CATPluginApp::OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer)
{
	g_hSettingWnd = hSettingWnd;
	g_szOptionStringBuffer=cszOptionStringBuffer;

	ATOPTION_ARRAY aOptions;
	GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	for (ATOPTION_ARRAY::size_type i=0; i<aOptions.size(); i++)
	{
		if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("Write")) )
			isWrite=true;
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("Read")) )
			isWrite=false;
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("Pass")) )
			CustomScript.SetWritePassN(_ttoi(aOptions[i].strValue.c_str()));
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("Cache")) )
			Cached=true;
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), _T("Always")) )
			AlwaysTrans=true;
	}

	if(!isWrite)
	{
		CustomScript.InitR();
	}
	else
	{
		CustomScript.InitW();
	}

	return TRUE;
}

BOOL CATPluginApp::OnPluginClose()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: �÷����� ���� �غ�, �ڿ� ��ȯ ���� �Ѵ�.
	return TRUE;
}

BOOL CATPluginApp::OnPluginOption()
{
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�
		CustomScript.FileClose();

		CSDlg.SetisWrite(isWrite);
		CSDlg.SetPassN(CustomScript.GetWritePassN());
		CSDlg.SetListFilename(ListFileName);
		CSDlg.Cached=Cached;
		CSDlg.Always=AlwaysTrans;

		CSDlg.Init();

		if(CSDlg.DoModal()==IDOK)
		{
			isWrite=CSDlg.GetisWrite();
			CustomScript.SetWritePassN(CSDlg.GetPassN());
			Cached=CSDlg.Cached;
			AlwaysTrans = CSDlg.Always;
			
			if(!isWrite)
			{
				CustomScript.InitR();
				sprintf_s(g_szOptionStringBuffer,4096,"Read");

				if(AlwaysTrans)
				{
					strcat_s(g_szOptionStringBuffer,4096,",Always");
				}
			}



		}

		if(isWrite)
		{
				CustomScript.InitW();
				sprintf_s(g_szOptionStringBuffer,4096,"Write,Pass(%d)",CSDlg.GetPassN());
				if(Cached)
					strcat_s(g_szOptionStringBuffer,4096,",Cache");
		}
		
	}
	return TRUE;
}

BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	ScriptChecked=false;

	if((!isWrite || (isWrite&&Cached)) && CustomScript.CheckJpnLine(cszInJapanese)>=0)
	{
		ScriptChecked=true;
		szOutJapanese[0]='\0';
		if(AlwaysTrans)
			lstrcpyA(szOutJapanese, cszInJapanese);
	}
	else
	{
		lstrcpyA(szOutJapanese, cszInJapanese);
	}
	
	if(isWrite)
		strcpy_s(TempJpn,4096,cszInJapanese);
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL CATPluginApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	if(ScriptChecked)
	{
		CustomScript.GetKorLine(szOutKorean);
	}
	else
	{
		lstrcpyA(szOutKorean, cszInKorean);
	}

	if(isWrite)
			CustomScript.WriteScript(TempJpn,cszInKorean);
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}