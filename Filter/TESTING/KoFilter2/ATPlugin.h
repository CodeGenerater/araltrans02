// ATPlugin - �ƶ�Ʈ���� �÷����� �԰��� ������ �⺻ DLL ���
//

#ifndef __ATPLUGIN_H__
#define __ATPLUGIN_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"		// �� ��ȣ�Դϴ�.

#ifdef USE_MFC
#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif
#endif // USE_MFC

/////////////////////////////////////////////////////////////////////////////
// CATPluginApp
// Ŭ���� ��� ���� : CWinApp -> CATPluginFrameApp -> CATPluginApp
// �� Ŭ������ ������ ������ ATPlugin.cpp�� �����Ͻʽÿ�.
//

#include <vector>
#include <string>

using namespace std;

typedef vector<string> CStringArray;

#define ENCODE_MARKER_STRING "ENCODE2KOR"
#define ENCODE_MARKER_LENGTH 10


class CATPluginApp : public CATPluginFrameApp
{
	protected:
		CStringArray m_cStringArray;

	public:
		CATPluginApp();
	
	// �÷����� ����
	public:
		// ���� �޼ҵ�
		virtual BOOL OnPluginInit();
		virtual BOOL OnPluginOption();
		virtual BOOL OnPluginClose();

		// ��ŷ �÷����� ���� �⺻ �޼ҵ�
//		virtual BOOL OnPluginStart();
//		virtual BOOL OnPluginStop();

		// ���� �÷����� ���� �⺻ �޼ҵ�
//		virtual BOOL Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize);

		// ���� �÷����� ���� �⺻ �޼ҵ�
		virtual BOOL PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize);
		virtual BOOL PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize);


		// �ƶ�Ʈ���� 0.3 ����/���� �÷����ο� ��ü ���� �Լ� 
//		virtual BOOL OnObjectInit(TRANSLATION_OBJECT* pTransObj);	// ���� ��ü ���� �̺�Ʈ (�ƶ�Ʈ���� 0.3 ����)
//		virtual BOOL OnObjectClose(TRANSLATION_OBJECT* pTransObj);	// ���� ��ü ���� �̺�Ʈ	(�ƶ�Ʈ���� 0.3 ����)
//		virtual BOOL OnObjectMove(TRANSLATION_OBJECT* pTransObj);	// ���� ��ü �������� �̺�Ʈ? (�ƶ�Ʈ���� 0.3 ����)
//		virtual BOOL OnObjectOption(TRANSLATION_OBJECT* pTransObj);	// ���� ��ü �ɼ�â �̺�Ʈ? (�ƶ�Ʈ���� 0.3 ����)


// �������Դϴ�.
	// VC 6.0 ���� ȣȯ���� ���� �Ʒ� �ּ����� ������ ���� �ּ���.
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CATPluginApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CATPluginApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
#ifdef USE_MFC
	DECLARE_MESSAGE_MAP()
#endif // USE_MFC
};

extern CATPluginApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __ATPLUGIN_H__
