// ATPlugin.h : ATPlugin DLL�� �⺻ ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.

// ATPLUGIN_VERSION
// ���� �÷����� ���� ����� ȣȯ�� ���� �ذ��� ���� ���� ����.
// 0.2.20080518 ���� �÷����� �԰��� �ƴ����� ������ ���� �־��.
typedef struct _tagATPLUGIN_VERSION {
	struct {
		WORD Major;	// ������ ����. ���� 0
		WORD Minor;	// ���̳� ����. ���� 2
		DWORD BuildDate;	// ���� ����. ���� 20080704
	} ATVersion;	// �÷����� ���� ����� AralTrans ���� ����

	struct {
		WORD Major;	// ������ ����
		WORD Minor;	// ���̳� ����
		DWORD BuildDate;	// ���� ����
	} PluginVersion;	// �÷����� ��ü ���� ����
	
	char szAuthor [16];	// ������
	char szPluginName [16];	// �÷����� �̸�
	char szDescription [32];	// �÷����� ����

} ATPLUGIN_VERSION;

const ATPLUGIN_VERSION * _OnPluginVersion();

// Export Functions
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption();
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose();
extern "C" __declspec(dllexport) BOOL __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize);
extern "C" __declspec(dllexport) BOOL __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize);
extern "C" __declspec(dllexport) const ATPLUGIN_VERSION * __stdcall OnPluginVersion();

// �÷����� ���� ����
extern ATPLUGIN_VERSION g_pluginVer;


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
};
