// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "resource.h"

#include "ATPlugin.h"
#include "OptionDialog.h"

#include "MultiPlugin.h"

#include "Util.h"

// �������� �����
ATPLUGIN_VERSION g_pluginVer = {
	{ 0, 2, 20080703 },		// �÷����� ���۽� AT ����
	{ 2, 0, 20080810 },		// �÷����� ����
	"whoami",	// ������
	"MultiPlugin",	// �÷����� �̸�
	"�����÷����� ���� �÷�����"	// �÷����� ����
};

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;
// �� �÷������� ��� �ڵ�
HMODULE g_hThisModule=NULL;
// �ɼ� ����
LPSTR g_szOptionStringBuffer=NULL;

const TCHAR g_szPluginName[]=_T("MultiPlugin");

// DLLMain
// ���� ��� �ڵ��� �˱� ���� ���⼭�� �� �ʿ���..
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hThisModule=(HMODULE)hModule;
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
	g_szOptionStringBuffer = cszOptionStringBuffer;

	ATPLUGIN_ARGUMENT_ARRAY args;

	GetATPluginArgsFromOptionString(cszOptionStringBuffer, args);
	
	if (!args.empty() && !g_cMultiPlugin.Add(args))
		return FALSE;

	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{
	if (g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		ATPLUGIN_ARGUMENT_ARRAY args;

		DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_OPTIONDIALOG), g_hSettingWnd, OptionDialogProc);

		g_cMultiPlugin.GetPluginArgs(args);
		GetOptionStringFromATPluginArgs(args, g_szOptionStringBuffer, 4096);
		
	}
	return TRUE;
}
BOOL  __stdcall OnPluginClose()
{
	g_cMultiPlugin.RemoveAll();

	return TRUE;
}
const ATPLUGIN_VERSION * __stdcall OnPluginVersion()
{
	// �÷����� ���� ��ȯ

	return &g_pluginVer;
}

// ��ŷ �÷����� ����
#ifdef ATPLUGIN_HOOKER
BOOL  __stdcall OnPluginStart()
{

	return g_cMultiPlugin.OnPluginStart();
}
BOOL  __stdcall OnPluginStop()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.

	return g_cMultiPlugin.OnPluginStop();
}
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
BOOL  __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
{
	return g_cMultiPlugin.Translate(cszJapanese, szKorean, nBufSize);
}
#endif
