#pragma once


// CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAboutDlg)

public:
	CAboutDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};
