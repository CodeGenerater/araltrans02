// ATPluginFrame - �ƶ�Ʈ���� �÷����� �����ӿ�ũ
//

#ifndef __ATPLUGINFRAME_H__
#define __ATPLUGINFRAME_H__

#include "ATPluginInterface.h"

/////////////////////////////////////////////////////////////////////////////
// CATPluginFrameApp
// �ƶ�Ʈ���� �÷����ΰ��� ������ ���� ���� Ŭ����
class CATPluginFrameApp : public CWinApp
{
	public:
		// ���� �������� �ƶ�Ʈ���� ����
		DWORD m_dwAralVersion;

		// �÷����� ����
		PLUGIN_INFO m_sPluginInfo;

		// �ƶ�Ʈ���� (����)â ������ �ڵ�
		HWND m_hAralWnd;
		
		// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
		// ���� - �ɼǱ��̴� �ƶ�Ʈ���� 0.2�� ��� �ְ� 4096 ����Ʈ, 0.3�� ��� 10240 (MAX_OPTION_LEN)

		struct OPTION_STRING {
			union {
				LPSTR pszAnsi;	// �ƶ�Ʈ���� 0.2 �� ��� ANSI
				LPWSTR pwszUnicode;	// �ƶ�Ʈ���� 0.3 �� ��� UNICODE
			};
			BOOL bIsUnicode;
		} m_sOptionString;

	public:
		CATPluginFrameApp();
		BOOL _PrePluginInit(HWND hAralWnd, LPVOID pPluginOption);	// �ɼ� ��Ʈ���� �ƶ� ������ �ڵ� ����
		
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �÷����� �̺�Ʈ ����
	public:
		// ����
		virtual BOOL OnPluginInit() { return TRUE; } // �÷����� �ʱ�ȭ �̺�Ʈ
		virtual BOOL OnPluginOption() { return TRUE; }	// �÷����� �ɼ�â �̺�Ʈ
		virtual BOOL OnPluginClose() { return TRUE; }	// �÷����� ���� �̺�Ʈ
		virtual BOOL GetPluginInfo(PLUGIN_INFO * pPluginInfo);	// �÷����� ���� (�ƶ�Ʈ���� 0.3 ����)

		// ��ŷ �÷����� ���� (�ƶ�Ʈ���� 0.2)
		virtual BOOL OnPluginStart() { return TRUE; }	// �÷����� ���� �̺�Ʈ (�ƶ�Ʈ���� 0.2 ����)
		virtual BOOL OnPluginStop() { return TRUE; }	// �÷����� ���� �̺�Ʈ (�ƶ�Ʈ���� 0.2 ����)
		
		// ���� �÷����� ���� (�ƶ�Ʈ���� 0.2 Ȥ�� �ƶ�Ʈ���� 0.3 �����ӿ�ũ���� �⺻ ���)
		virtual BOOL Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)	// ���� �̺�Ʈ
		{ lstrcpyA(szKorean, cszJapanese); return TRUE; }
		
		// ���� �÷����� ���� (�ƶ�Ʈ���� 0.2 Ȥ�� �ƶ�Ʈ���� 0.3 �����ӿ�ũ���� �⺻ ���)
		virtual BOOL PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)	// ���� ��ó�� �̺�Ʈ
		{ lstrcpyA(szOutJapanese, cszInJapanese); return TRUE; }
		virtual BOOL PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)	// ���� ��ó�� �̺�Ʈ
		{ lstrcpyA(szOutKorean, cszInKorean); return TRUE; }


		// ���� ��ü ���� �̺�Ʈ (�ƶ�Ʈ���� 0.3 ����/���� �÷����� ����)
	public:
		virtual BOOL OnObjectInit(TRANSLATION_OBJECT* pTransObj);	// ���� ��ü ���� �̺�Ʈ (�ƶ�Ʈ���� 0.3 ����)
		virtual BOOL OnObjectClose(TRANSLATION_OBJECT* pTransObj);	// ���� ��ü ���� �̺�Ʈ	(�ƶ�Ʈ���� 0.3 ����)
		virtual BOOL OnObjectMove(TRANSLATION_OBJECT* pTransObj) { return TRUE; }	// ���� ��ü �������� �̺�Ʈ? (�ƶ�Ʈ���� 0.3 ����)
		virtual BOOL OnObjectOption(TRANSLATION_OBJECT* pTransObj) { return TRUE; }	// ���� ��ü �ɼ�â �̺�Ʈ? (�ƶ�Ʈ���� 0.3 ����)


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MainTranslateProcedure �Ļ� ���� (����/���� �÷����� ���� ��� �޼ҵ�)
	//
	// �ƶ�Ʈ���� 0.3�� �����ӿ�ũ �⺻ ���� ���ν��� (MainTranslateProcedure)�� ������ �Ļ� �޼ҵ带 �ҷ��ش�.
	// �ƶ�Ʈ���� 0.3�� �߰����� ��� (�����ڵ� ���� Ȥ�� ���� ������ü�� ���� �ѱ��� �ʱ� ��) �� ������� �ʴ´ٸ�
	// 0.2 �� PreTranslate/PostTranslate/Translate �� �׳� ����ص� �����ϴ�.
	// (�Ļ� �޼ҵ��� �⺻ ���ÿ��� �ڵ����� �ҷ���)
	//
	public:
		virtual BOOL MainTranslateProcedure(TRANSLATION_OBJECT* pTransObj);	// ���� �̺�Ʈ (�ƶ�Ʈ���� 0.3 ����)

		// �����
		enum TRESULT {
			TR_STOPALL			= 0,	// 00000000 (2), �����Ұ� (FALSE) ������ ������.
			TR_POSTONLY			= 1,	// 00000001 (2), ��ó���� �Ѵ�.
			TR_TRANSONLY		= 2,	// 00000010 (2), (���� �� �������� ��) ��ü ������ �Ѵ�.
			TR_TRANS_POST		= 3,	// 00000011 (2), (���� �� �������� ��) ��ü ���� �� ��ó���� �Ѵ�.
			TR_NEXTONLY			= 4,	// 00000100 (2), ���� ���� ��ü�� ���� ���ν����� �����Ѵ�.
			TR_NEXT_POST		= 5,	// 00000101 (2), (���� �� �������� ��) ���� ���� ��ü�� ���� ���ν����� ��ó���� �Ѵ�.
			TR_NEXT_TRANS		= 6,	// 00000110 (2), (���� �� �������� ��) ��ü ���� �� ���� ���� ��ü�� ���� ���ν����� �Ѵ�.
			TR_TRANS_NEXT_POST	= 7,	// 00000111 (2), ��ü ����, ���� ������ü, ��ó���� ��� �Ѵ�.
			TR_DOALL			= 7,	// = TR_TRANS_NEXT_POST
			TR_STOP_OK			= 8,	// 00001000 (2), �� �̻��� ó���� ���������� �����Ϸ� (TRUE) ������ ������.
		};

		virtual TRESULT PreTranslateEx(TRANSLATION_OBJECT *pTransObj);	// ���� ��ó�� �۾�
		virtual TRESULT TranslateEx(TRANSLATION_OBJECT *pTransObj);		// ��ü ���� �۾�
		virtual TRESULT NextTranslateEx(TRANSLATION_OBJECT *pTransObj);	// ���� ��ü�� ���� �۾�
		virtual TRESULT PostTranslateEx(TRANSLATION_OBJECT *pTransObj);	// ���� ��ó�� �۾�



	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ��ƿ��Ƽ �޼ҵ�
	//
		TRANSLATION_OBJECT *GetFirstObject(TRANSLATION_OBJECT *pTransObj);	// ���� ��ü ü���� ���� ó�� ��ü�� ã�´�.
		TRANSLATION_OBJECT *GetLastObject(TRANSLATION_OBJECT *pTransObj);	// ���� ��ü ü���� ���� ������ ��ü�� ã�´�.

};



#endif //__ATPLUGINFRAME_H__
