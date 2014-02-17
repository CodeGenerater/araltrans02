#pragma once
#include "afxwin.h"
#include <queue>

using namespace std;

CCriticalSection g_csThread;
volatile bool g_bContinue;

// CDumpDlg ��ȭ �����Դϴ�.

class CDumpDlg : public CDialog
{
	DECLARE_DYNAMIC(CDumpDlg)

public:
	CDumpDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDumpDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DumpDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	CWinThread m_PrintThread;
	queue <CString> m_PrintQueue;
	UINT PrintThreadFunc(__in LPVOID lpParameter);

public:
	BOOL PrintText(CString Text);
public:
	CEdit DumpEdit;
};
