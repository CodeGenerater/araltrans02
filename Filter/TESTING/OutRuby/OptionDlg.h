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
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
private:
	CString RubyText;

public:
	void SetRubyText(LPCSTR szRuby);
	void GetRubyText(LPSTR szDest);
};
