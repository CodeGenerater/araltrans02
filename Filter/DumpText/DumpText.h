// DumpText.h : DumpText DLL�� �⺻ ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CDumpTextApp
// �� Ŭ������ ������ ������ DumpText.cpp�� �����Ͻʽÿ�.
//

class CDumpTextApp : public CWinApp
{
public:
	CDumpTextApp();

	BOOL OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer);
	BOOL OnPluginOption();
	BOOL OnPluginClose();
	BOOL PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize);
	BOOL PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize);

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

public:
	HWND m_hSettingWnd;
	HINSTANCE m_hThisModule;
	LPSTR m_szOptionStringBuffer;
	UINT_PTR m_pTmpTimer;
	CWinThread * m_pThread;
};