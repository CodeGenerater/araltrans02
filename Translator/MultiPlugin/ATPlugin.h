#ifndef __ATPLUGIN_H__
#define __ATPLUGIN_H__

/*
** �÷����� �⺻ ����
*/

// ��ŷ �÷������� ��� define
//#define ATPLUGIN_HOOKER

// ���� �÷������� ��� define
#define ATPLUGIN_TRANSLATOR

/*
** �÷����� �⺻ ���� ��
*/

// ATPLUGIN_VERSION
// ���� �÷����� ���� ����� ȣȯ�� ���� �ذ��� ���� ���� ����.
// 0.2.20080518 ���� �÷����� �԰��� �ƴ����� ������ ���� �־��.
typedef struct _tagATPLUGIN_VERSION {
	struct {
		WORD Major;	// ������ ����. ���� 0
		WORD Minor;	// ���̳� ����. ���� 2
		DWORD BuildDate;	// ���� ����. ���� 20080518
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

/*
** ���� ����
*/

// �÷����� ���� ����
extern ATPLUGIN_VERSION g_pluginVer;
// ����â ������ �ڵ�
extern HWND g_hSettingWnd;

extern HMODULE g_hThisModule;

extern const TCHAR g_szPluginName[];

extern LPSTR g_szOptionStringBuffer;

// �÷����� export �Լ� ����
#ifdef __cplusplus
extern "C"
{
#endif

// ����
BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
BOOL __stdcall OnPluginOption();
BOOL __stdcall OnPluginClose();
const ATPLUGIN_VERSION * __stdcall OnPluginVersion();

// ��ŷ �÷����� ����
#ifdef ATPLUGIN_HOOKER
BOOL __stdcall OnPluginStart();
BOOL __stdcall OnPluginStop();
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
BOOL __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize);
#endif

// export �� ���� ��Ŀ �ɼ�
// ����
#pragma comment(linker, "/EXPORT:OnPluginInit=_OnPluginInit@8")
#pragma comment(linker, "/EXPORT:OnPluginOption=_OnPluginOption@0")
#pragma comment(linker, "/EXPORT:OnPluginClose=_OnPluginClose@0")
#pragma comment(linker, "/EXPORT:OnPluginVersion=_OnPluginVersion@0")

// ��ŷ �÷����� ����
#ifdef ATPLUGIN_HOOKER
#pragma comment(linker, "/EXPORT:OnPluginStart=_OnPluginStart@0")
#pragma comment(linker, "/EXPORT:OnPluginStop=_OnPluginStop@0")
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
#pragma comment(linker, "/EXPORT:Translate=_Translate@12")
#endif

#ifdef __cplusplus
}
#endif

#endif //__ATPLUGIN_H__