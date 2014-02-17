#pragma once
#include "Resource.h"
#include "afxwin.h"

// CShortcutDlg ��ȭ �����Դϴ�.

class CShortcutDlg : public CDialog
{
	DECLARE_DYNAMIC(CShortcutDlg)

public:
	CShortcutDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CShortcutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SHORTCUT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CString m_strShortcutFilePath;
	CString m_strDelayTime;
	CString m_strLoaderPath;
	BOOL m_nShortcutType;
	BOOL m_nHideType;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	CButton m_chkHide;
	afx_msg void OnBnClickedCheckHide();
};
