#pragma once
#include "afxwin.h"
#include "ATPlugin.h"


// CDlg ��ȭ �����Դϴ�.

class CDlg : public CDialog
{
	DECLARE_DYNAMIC(CDlg)

public:
	CDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	int UseCDic;
public:
	int UseGDic;
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnBnClickedButton2();
	int nNoMargin;
};
