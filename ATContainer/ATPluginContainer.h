#ifndef __ATPLUGINCONTAINER_H__
#define __ATPLUGINCONTAINER_H__

/*
// ���� �Ծ� ���� - ������ ����.

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
*/
#include <tchar.h>

// ���� ������ Ÿ�� ����
typedef BOOL (__stdcall * PROC_OnPluginDefault) ();
typedef BOOL (__stdcall * PROC_OnPluginInit) (HWND, LPSTR);
//typedef const ATPLUGIN_VERSION * (__stdcall * PROC_OnPluginVersion)();
typedef BOOL (__stdcall * PROC_Translate) (LPCSTR, LPSTR, int);

class CATPluginContainer {
	protected:
		HMODULE m_hModule;
		char *m_pszOption;

		PROC_OnPluginInit m_pfnOnPluginInit;
		PROC_OnPluginDefault m_pfnOnPluginOption;
		PROC_OnPluginDefault m_pfnOnPluginClose;

// �Ⱦ���
//		PROC_OnPluginVersion m_pfnOnPluginVersion;

/*
// �˰��� �÷����ο� - ������������ ����� ���̹Ƿ� �Ⱦ���
		PROC_OnPluginDefault m_pfnOnPluginStart;
		PROC_OnPluginDefault m_pfnOnPluginStop;
*/

/*
// ���� �÷����ο� - ������������ ����� ���̹Ƿ� �Ⱦ���
		PROC_Translate m_pfnTranslate;
*/
		PROC_Translate m_pfnPreTranslate;
		PROC_Translate m_pfnPostTranslate;

	public:
		CATPluginContainer() : m_hModule(NULL), m_pszOption(NULL),
			m_pfnOnPluginInit(NULL), m_pfnOnPluginOption(NULL), m_pfnOnPluginClose(NULL), // m_pfnOnPluginVersion(NULL),
//			m_pfnOnPluginStart(NULL), m_pfnOnPluginStop(NULL), m_pfnTranslate(NULL),
			m_pfnPreTranslate(NULL), m_pfnPostTranslate(NULL) {}
		~CATPluginContainer() { Unload(); }

		const char *GetOptionString() const { return (m_pszOption)?m_pszOption:""; }

		virtual BOOL Load(LPCTSTR pszModulePath);
		virtual void Unload();

		virtual BOOL OnPluginInit(HWND hSettingWnd, LPCSTR cszOptionStringBuffer);
		virtual BOOL OnPluginOption()
			{ return (m_pfnOnPluginOption)?m_pfnOnPluginOption():FALSE; }
		virtual BOOL OnPluginClose()
			{ return (m_pfnOnPluginClose)?m_pfnOnPluginClose():FALSE; }
/*
		virtual const ATPLUGIN_VERSION *OnPluginVersion()
			{ return (m_pfnOnPluginVersion)?m_pfnOnPluginVersion():NULL; }
		
		virtual BOOL OnPluginStart()
			{ return (m_pfnOnPluginStart)?m_pfnOnPluginStart():FALSE; }
		virtual BOOL OnPluginStop()
			{ return (m_pfnOnPluginStop)?m_pfnOnPluginStop():FALSE; }
		
		virtual BOOL Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
			{ return (m_pfnTranslate)?m_pfnTranslate(cszJapanese, szKorean, nBufSize):FALSE; }
*/
		virtual BOOL PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
			{ return (m_pfnPreTranslate)?m_pfnPreTranslate(cszInJapanese, szOutJapanese, nBufSize):FALSE; }
		virtual BOOL PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
			{ return (m_pfnPostTranslate)?m_pfnPostTranslate(cszInKorean, szOutKorean, nBufSize):FALSE; }


};

#endif // __ATPLUGINCONTAINER_H__