#pragma once

#include "resource.h"
// COptionDialog ��ȭ �����Դϴ�.

class COptionDialog : public CDialog
{
	DECLARE_DYNAMIC(COptionDialog)

public:
	COptionDialog(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~COptionDialog();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_OPTIONDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bMatchLen;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMatchlencheck();
};
