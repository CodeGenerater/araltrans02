// ATPlugin.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "tstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Util ����Ϸ��� include
#include "Util.h"

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



// CATPluginApp �ʱ�ȭ

BOOL CATPluginApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	
	return TRUE;
}

BOOL CATPluginApp::OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer)
{
	g_hSettingWnd = hSettingWnd;
	g_szOptionStringBuffer=cszOptionStringBuffer;

	m_bUseFixLine=true;
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
	bool bSetup=false;
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.
		COptionSheet m_cOption(L"FixLine Option");
		if(m_cOption.DoModal()==IDOK)
		{
			bSetup=true;
		}
	}
	if(bSetup)
			m_cFixLine.Initialize();
	return TRUE;
}

BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	m_bCanUse=true;
	if(cszInJapanese[0]=='\0')
	{
		m_bCanUse=false;
	}
	if(m_bUseFixLine && m_bCanUse)
	{
		m_cFixLine.PreFixLine(szOutJapanese,cszInJapanese);
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
	if(m_bUseFixLine && m_bCanUse)
	{
		m_cFixLine.PostFixLine(szOutKorean,cszInKorean);
	}
	else
	{
		lstrcpyA(szOutKorean, cszInKorean);
	}
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}