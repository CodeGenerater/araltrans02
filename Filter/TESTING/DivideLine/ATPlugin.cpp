// ATPlugin.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "tstring.h"

#include "DivideLine.h"
#include "OptionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Util ����Ϸ��� include
#include "Util.h"


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
COptionDlg g_cOptionDlg;

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

	// �ɼ� ���
	// Base(IgnoreLine,MaxChar,MaxLine,ReturnLen,ReturnChar,LimitLine),Add(OptLen,OnlyText,OptString)
	for (size_t i=0; i<aOptions.size(); i++)
	{

		if ( !lstrcmpi(aOptions[i].strKey.c_str(), L"Base"))
		{
			wstring::size_type SPoint=0;
			wstring::size_type Divider=0;
			wstring Temp1=aOptions[i].strValue;
			wstring Temp2=L"";
			wstring Temp3=L"";

			int nReturnLen;
			int nMaxChar;
			int nMaxLine;
			int nIgnoreLine;

			string strReturnChar;

			for(int Flag=0;Flag<5;Flag++)
			{
				if(Flag<4)
				{
					Divider=Temp1.find(L',',SPoint);
				}
				else
				{
					Divider=Temp1.length()-1;
				}

				Temp2=Temp1.substr(SPoint,Divider-SPoint+1);
				SPoint=Divider+1;

				switch(Flag)
				{
				case 0://IgnoreLine
					nIgnoreLine=_wtoi(Temp2.c_str());
					break;

				case 1://MaxChar
					nMaxChar=_wtoi(Temp2.c_str());
					break;

				case 2://MaxLine
					nMaxLine=_wtoi(Temp2.c_str());
					break;

				case 3://ReturnLen
					nReturnLen=_wtoi(Temp2.c_str());
					break;

				case 4://ReturnChar
					strReturnChar=HexToString(Temp2);
					break;

				default:;
				}
			}
			g_cDivideLine.SetBase(nIgnoreLine,nMaxChar,nMaxLine,nReturnLen,strReturnChar);
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), L"RemoveSpace"))
		{
			g_cDivideLine.SetRemoveSpace(true);
		}
		else if ( !lstrcmpi(aOptions[i].strKey.c_str(), L"TwoByte"))
		{
			g_cDivideLine.SetTwoByte(true);
		}
	}

	ApplySetting();

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

		if(g_cOptionDlg.DoModal()==IDOK)
		{
			ApplySetting();
		}
	}
	return TRUE;
}

BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	g_cDivideLine.PreDivideLine(szOutJapanese, cszInJapanese);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

BOOL CATPluginApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	g_cDivideLine.PostDivideLine(szOutKorean, cszInKorean);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}

void CATPluginApp::ApplySetting()
{
	ATOPTION_ARRAY aOptions;
	ATOPTION_PAIR option;
	
	//Base
	if(1)
	{
		TCHAR szbuf[10];
		option.strKey=_T("BASE");
		// Base(IgnoreLine,MaxChar,MaxLine,ReturnLen,ReturnChar)
		_itot_s(g_cDivideLine.GetIgnoreLine(),szbuf,10,10);
		option.strValue =szbuf;
		option.strValue+=_T(',');
		_itot_s(g_cDivideLine.GetMaxChar(),szbuf,10,10);
		option.strValue+=szbuf;
		option.strValue+=_T(',');
		_itot_s(g_cDivideLine.GetMaxLine(),szbuf,10,10);
		option.strValue+=szbuf;
		option.strValue+=_T(',');
		_itot_s(g_cDivideLine.GetReturnLen(),szbuf,10,10);
		option.strValue+=szbuf;
		option.strValue+=_T(',');
		option.strValue+=StringToHex(g_cDivideLine.GetReturnChar());
		aOptions.push_back(option);
	}


	if (g_cDivideLine.GetRemoveSpace()!=FALSE)
	{
		option.strKey=_T("RemoveSpace");
		option.strValue.clear();
		aOptions.push_back(option);
	}
	if (g_cDivideLine.GetTwoByte()!=FALSE)
	{
		option.strKey=_T("TwoByte");
		option.strValue.clear();
		aOptions.push_back(option);
	}

	GetOptionStringFromATOptions(aOptions, g_szOptionStringBuffer, 4096);
}