#pragma once
#include "Resource.h"

// CDlg_AddName ��ȭ �����Դϴ�.

class CDlg_AddName : public CDialog
{
	DECLARE_DYNAMIC(CDlg_AddName)

public:
	CDlg_AddName(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDlg_AddName();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_AddName };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CString m_strText;
};
