#pragma once

#include "resource.h"

// COptionDlg ��ȭ �����Դϴ�.

class COptionDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionDlg)

public:
	COptionDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~COptionDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_OPTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	int m_nAutoUpdate;
	BOOL m_bUseATDataZip;
	afx_msg void OnBnClickedCheckUseATData();
	virtual BOOL OnInitDialog();
	BOOL m_bDeleteLocalFiles;
	BOOL m_bAppLocAutoConfirm;
};
