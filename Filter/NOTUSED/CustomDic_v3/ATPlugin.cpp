// ATPlugin.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "tstring.h"
#include "CustomDic2.h"
#include "Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Util ����Ϸ��� include
#include "Util.h"


/*
** �������� �����
*/

int UseCDic=false;
int UseGDic=false;

bool bInit=false;

bool g_bNoMargin=false;

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
void Init();

// ������ CATPluginApp ��ü�Դϴ�.

CATPluginApp theApp;
CCustomDic2	* g_pCustomDic=NULL;
CDlg *g_pDlg=NULL;

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

	// TODO : �÷������� �ʱ�ȭ, �ڿ� Ȯ��, �ɼ� �м����� �Ѵ�.

	// ATCApi ����
	// if ( !g_cATCApi.IsValid() ) return FALSE;

	// Util �� �ɼ� �ļ� ��� ��
	ATOPTION_ARRAY aOptions;
	GetATOptionsFromOptionString(cszOptionStringBuffer, aOptions);

	for (size_t i=0; i<aOptions.size(); i++)
	{
		if ( !lstrcmpi(aOptions[i].strKey.c_str(), L"CDic"))
		{
			UseCDic=true;
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), L"GDic"))
		{
			UseGDic=true;
		}
		else if(!lstrcmpi(aOptions[i].strKey.c_str(), L"NoMargin"))
		{
			g_bNoMargin=true;
		}
	}
	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL CATPluginApp::OnPluginClose()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: �÷����� ���� �غ�, �ڿ� ��ȯ ���� �Ѵ�.
	if(bInit)
	{
		if(g_pCustomDic)
		{
			g_pCustomDic->End();
			delete g_pCustomDic;
		}
		if(g_pDlg)
		{
			delete g_pDlg;
		}
	}
	return TRUE;
}

BOOL CATPluginApp::OnPluginOption()
{
	if(!bInit)
	{
		bInit=true;
		Init();
	}
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.
		g_pDlg->UseCDic=UseCDic;
		g_pDlg->UseGDic=UseGDic;
		g_pDlg->nNoMargin=g_bNoMargin;

		if(g_pDlg->DoModal()==IDOK)
		{
			bool Reset=false;

			if(UseCDic!=g_pDlg->UseCDic||UseGDic!=g_pDlg->UseGDic)
				Reset=true;

			UseCDic=g_pDlg->UseCDic;
			UseGDic=g_pDlg->UseGDic;
			g_bNoMargin=g_pDlg->nNoMargin==0?false:true;

			if(Reset)
			{
				Reset=false;
				g_pCustomDic->FileClear();
				if(UseGDic)g_pCustomDic->AddDic(1,L"");
				if(UseCDic)g_pCustomDic->AddDic(2,L"");
				g_pCustomDic->SetMarginMode(g_bNoMargin);
			}


			ATOPTION_ARRAY aOptions;
			ATOPTION_PAIR option;

			if(UseCDic)
			{
				option.strKey=L"CDic";
				option.strValue=L"";
				aOptions.push_back(option);
			}

			if(UseGDic)
			{
				option.strKey=L"GDic";
				option.strValue=L"";
				aOptions.push_back(option);
			}

			if(g_bNoMargin)
			{
				option.strKey=L"NoMargin";
				option.strValue=L"";
				aOptions.push_back(option);
			}
		
			GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);
		}

	}
	return TRUE;
}

BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	if(!bInit)
	{
		bInit=true;
		Init();
		Sleep(300);
	}
	if(UseCDic||UseGDic)
		g_pCustomDic->PreCustomDic(szOutJapanese,cszInJapanese);
	else
		lstrcpyA(szOutJapanese, cszInJapanese);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL CATPluginApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	if(UseCDic||UseGDic)
		g_pCustomDic->PostCustomDic(szOutKorean,cszInKorean);
	else
		lstrcpyA(szOutKorean, cszInKorean);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

void Init()
{
	g_pCustomDic=new CCustomDic2();
	g_pCustomDic->SetMarginMode(g_bNoMargin);
	g_pDlg=new CDlg();

	g_pCustomDic->FileClear();
	if(UseGDic)g_pCustomDic->AddDic(1,L"");
	if(UseCDic)g_pCustomDic->AddDic(2,L"");

	g_pCustomDic->Init();
	
}