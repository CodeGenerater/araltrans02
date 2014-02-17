#pragma once
#include "afxwin.h"
#include "DivideLine.h"
#include "atplugin.h"
#include "afxcmn.h"


// COptionDlg ��ȭ �����Դϴ�.

class COptionDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionDlg)

public:
	COptionDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~COptionDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	
	BOOL bRemoveSpace;
	BOOL bTwoByte;

private:
	CSpinButtonCtrl Spin1;
	CSpinButtonCtrl Spin2;
	CSpinButtonCtrl Spin3;
	CSpinButtonCtrl Spin4;

public:
	int GetReturnLen(void);
	void SetReturnLen(int nReturnLen);

	LPCWSTR GetReturnCharHex(void);
	void SetReturnCharHex(LPCWSTR szReturnCharHex);

	int GetIgnoreLine(void);
	void SetIgnoreLine(int nIgnoreLine);
	
	int GetMaxChar(void);
	void SetMaxChar(int nMaxChar);
	
	int GetMaxLine(void);
	void SetMaxLine(int nMaxLine);

public:
	afx_msg void OnBnClickedOk();
};