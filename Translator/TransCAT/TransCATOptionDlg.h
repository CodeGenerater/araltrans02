#pragma once
#include "afxwin.h"


// CTransCATOptionDlg ��ȭ �����Դϴ�.

class CTransCATOptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransCATOptionDlg)

public:
	CTransCATOptionDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CTransCATOptionDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bRemoveTrace;
	BOOL m_bRemoveDupSpace;
};
