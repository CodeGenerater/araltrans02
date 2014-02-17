// ATPlugin.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "tstring.h"
#include "OutRuby.h"
#include "OptionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Util ����Ϸ��� include
//#include "Util.h"


/*
** �������� �����
*/

// �÷����� ����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080704 },		// �÷����� ���۽� AT ����
	{ 0, 1, 20080810 },		// �÷����� ����
	"������",	// ������
	"�÷����� �̸�",	// �÷����� �̸�
	"�ƶ�Ʈ���� �÷�����"	// �÷����� ����
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
COutRuby cOutRuby;
COptionDlg cOptionDlg;

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

	Used=false;

	int Len=(int)strlen(cszOptionStringBuffer);
	if(Len>=5)
	{
		cOutRuby.SetRubyString(cszOptionStringBuffer);
		Used=true;
	}

	
	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
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
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.

		cOptionDlg.SetRubyText(cOutRuby.GetRubyString());
		if(cOptionDlg.DoModal()==IDOK)
		{
			char szTemp[1024] = "\0";
			cOptionDlg.GetRubyText(szTemp);
			cOutRuby.SetRubyString(szTemp);

			lstrcpyA(g_szOptionStringBuffer,szTemp);

		}
	}
	return TRUE;
}

BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	if(Used)
	{
		cOutRuby.PreOutRuby(szOutJapanese, cszInJapanese);
	}
	else
	{
		lstrcpyA(szOutJapanese, cszInJapanese);
	}
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL CATPluginApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutKorean, cszInKorean);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}