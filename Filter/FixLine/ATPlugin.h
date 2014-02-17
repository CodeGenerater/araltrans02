// ATPlugin.h : ATPlugin DLL�� �⺻ ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.
#include "FixLine.h"
#include "OptionSheet.h"

// Export Functions
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption();
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose();
extern "C" __declspec(dllexport) BOOL __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize);
extern "C" __declspec(dllexport) BOOL __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize);

// ����â ������ �ڵ�
extern HWND g_hSettingWnd;

// �� �÷������� �ɼ�
extern LPSTR g_szOptionStringBuffer;

// CATPluginApp
// �� Ŭ������ ������ ������ ATPlugin.cpp�� �����Ͻʽÿ�.
//

class CATPluginApp : public CWinApp
{
public:
	CATPluginApp();

	BOOL OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
	BOOL OnPluginOption();
	BOOL OnPluginClose();
	BOOL PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize);
	BOOL PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize);

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

private:
	CFixLine m_cFixLine;
	bool m_bUseFixLine;
	bool m_bCanUse;
};
