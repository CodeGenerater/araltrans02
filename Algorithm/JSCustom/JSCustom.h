// JSCustom.h : JSCustom DLL�� �⺻ ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


extern "C" BOOL PASCAL EXPORT OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
extern "C" BOOL PASCAL EXPORT OnPluginOption();
extern "C" BOOL PASCAL EXPORT OnPluginStart();
extern "C" BOOL PASCAL EXPORT OnPluginStop();
extern "C" BOOL PASCAL EXPORT OnPluginClose();

// CJSCustomApp
// �� Ŭ������ ������ ������ JSCustom.cpp�� �����Ͻʽÿ�.
//

class CJSCustomApp : public CWinApp
{
public:
	BOOL OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer);
	BOOL OnPluginOption();
	BOOL OnPluginClose();
	BOOL OnPluginStart();
	BOOL OnPluginStop();

	CJSCustomApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
