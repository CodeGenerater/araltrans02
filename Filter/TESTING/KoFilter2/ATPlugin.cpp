// ATPlugin - �ƶ�Ʈ���� �÷����� �԰��� ������ �⺻ DLL ���
//

#include "stdafx.h"
#include "ATPlugin.h"

#include "ATPluginFrame/Debug.h"

// MBCS <-> UNICODE ��ȯ�� �ʿ��ϸ� include
//#include "ATPluginFrame/Libs/tstring.h"

// ATCApi ����Ϸ��� include
//#include "ATPluginFrame/Libs/ATCApi.h"

// ATOptionParser ����Ϸ��� include
//#include "ATPluginFrame/Libs/ATOptionParser.h"

// Util ����Ϸ��� include
//#include "ATPluginFrame/Libs/Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ���� ���� �����

// ������ CATPluginApp ��ü�Դϴ�.
CATPluginApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CATPluginApp

#ifdef USE_MFC
BEGIN_MESSAGE_MAP(CATPluginApp, CWinApp)
	//{{AFX_MSG_MAP(CATPluginApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// USE_MFC

/////////////////////////////////////////////////////////////////////////////
// CATPluginApp ����

CATPluginApp::CATPluginApp()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.

	// �����뿡�� PLUGIN_INFO ����ü�� ä���ִ´�.
	// ���� �⺻������ �����ӿ�ũ�� ä�������� �÷����� �̸��� �ٿ�ε� URL
	// ���� ���� �־��ִ� ���� ����.

	// PLUGIN_INFO ����ü�� ���
	//	int			cch;				// PLUGIN_INFO ����ü ������
	//	int			nIconID;			// Icon Resource ID
	//	wchar_t		wszPluginType[16];	// �÷����� Ÿ��
	//	wchar_t		wszPluginName[64];	// �÷����� �̸�
	//	wchar_t		wszDownloadUrl[256];// �÷����� �ٿ�ε� URL
	m_sPluginInfo.nIconID = IDI_PLUGINICON;
	lstrcpyn(m_sPluginInfo.wszPluginName, _T("KoFilter2"), 64);
	lstrcpyn(m_sPluginInfo.wszDownloadUrl, _T("http://"), 256);
}

/////////////////////////////////////////////////////////////////////////////
// CATPluginApp �ʱ�ȭ
BOOL CATPluginApp::InitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class

	return CATPluginFrameApp::InitInstance();
}

// ����

BOOL CATPluginApp::OnPluginInit()
{
	CATPluginFrameApp::OnPluginInit();

	// TODO : �÷������� �ʱ�ȭ, �ڿ� Ȯ��, �ɼ� �м����� �Ѵ�.

	// ATCApi ����
	// if ( !g_cATCApi.Init() ) return FALSE;

	// ATOptionParser �� �ɼ� �ļ� ��� ��
	// ATOPTION_ARRAY aOptions;
	// GetATOptionsFromOptionString(m_sOptionString, aOptions);

	// �ʱ�ȭ ������ TRUE, ���н� FALSE�� ��ȯ
	return TRUE;
}

BOOL CATPluginApp::OnPluginOption()
{
	if (m_hAralWnd && IsWindow(m_hAralWnd))
	{
		// TODO: �÷����� �ɼ�â ó�� �� �ɼ� ������ �Ѵ�.

		MessageBox(m_hAralWnd, _T("��밡���� �ɼ��� �����ϴ�"), m_sPluginInfo.wszPluginName, MB_OK);

		// ATOptionParser �� �ɼ� �ļ� ��� ��
		// ATOPTION_ARRAY aOptions;
		// ReadOptionsFromSomewhere(aOptions); // ��򰡿��� �ɼ��� �о�´�
		// GetOptionStringFromATOptions(aOptions, m_sOptionString, 4096);	// �ɼ��� ���ڿ��� �ٲ� �ƶ�Ʈ������ �ѱ��.

	}
	return TRUE;
}

BOOL CATPluginApp::OnPluginClose()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: �÷����� ���� �غ�, �ڿ� ��ȯ ���� �Ѵ�.

	return TRUE;
}

///////////////////////////////////////////////////////////////////////
// �÷����� ������ ���� Override �Ǵ� �÷����� ���� �Լ��� ��
// 

// ��ŷ �÷����� ����
/*
BOOL CATPluginApp::OnPluginStart()
{
	// TODO: ��ŷ ���� �۾�.

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
//*/

/*
BOOL CATPluginApp::OnPluginStop()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: ��ŷ ���� �۾�.

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
//*/

// ���� �÷����� ����
/*
BOOL CATPluginApp::Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
{
	// TODO: �����۾��� �Ѵ�.

	lstrcpyA(szKorean, cszJapanese);

	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
//*/

// ���� �÷����� ����
/*
BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutJapanese, cszInJapanese);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
//*/
/*
BOOL CATPluginApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	// TODO: ���� ��ó���� �Ѵ�.
	// Ư���� ó���� ���� �ʴ��� ���ڿ��� ������ �����־�� �Ѵ�.
	lstrcpyA(szOutKorean, cszInKorean);
	
	// �۾� ������ TRUE, ���н� FALSE ��ȯ 
	return TRUE;
}
//*/

/*
BOOL CATPluginApp::OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	if ( !CATPluginFrameApp::OnObjectInit(pTransObj) ) return FALSE;
	
	// TODO: �߰����� ���� ��ü �ʱ�ȭ �۾��� �Ѵ�.

	return TRUE;
}
//*/
/*
BOOL CATPluginApp::OnObjectClose(TRANSLATION_OBJECT* pTransObj)
{
	// TODO: �߰����� ���� ��ü ���� �۾��� �Ѵ�.

	if ( !CATPluginFrameApp::OnObjectClose(pTransObj) ) return FALSE;
	return TRUE;
}
//*/
/*
BOOL CATPluginApp::OnObjectMove(TRANSLATION_OBJECT* pTransObj)
{
	if ( !CATPluginFrameApp::OnObjectMove(pTransObj) ) return FALSE;
	
	// TODO: �߰����� ���� ��ü �������� �۾��� �Ѵ�.	
	
	return TRUE;
}
//*/
/*
BOOL CATPluginApp::OnObjectOption(TRANSLATION_OBJECT* pTransObj)
{
	// TODO: �߰����� ���� ��ü �ɼ�ó�� �۾��� �Ѵ�.


	if ( !CATPluginFrameApp::OnObjectOption(pTransObj) ) return FALSE;
	return TRUE;
}
//*/
