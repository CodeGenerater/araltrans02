#pragma once
#include "afxcmn.h"


// CSubOptDlg ��ȭ �����Դϴ�.

class CSubOptDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSubOptDlg)

public:
	CSubOptDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CSubOptDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_Sub };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	int m_nAutoInterval;
	BOOL m_bAutoLoad;
	BOOL m_bTrim;
	CSpinButtonCtrl m_ctrSpin;
	BOOL m_bSaveDefaultOption;
	virtual BOOL OnInitDialog();
};
