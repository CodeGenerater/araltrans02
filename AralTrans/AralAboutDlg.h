#pragma once

#include "resource.h"

// CAralAboutDlg ��ȭ �����Դϴ�.

class CAralAboutDlg : public CDialog
{
	DECLARE_DYNAMIC(CAralAboutDlg)

public:
	CAralAboutDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CAralAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CString m_strInfo;
	HANDLE m_hMapFile;
	bool m_bIsSystemInfo;
private:
	BOOL Is64bitWindows();
	CString GetFilesVersion(LPCTSTR szFilesPath);
	BOOL TestUpdate();
	void GetVersionInfo(CString & strVersionInfo);
	void GetSystemInfo(CString & strSystemInfo);
public:
	afx_msg void OnBnClickedChangeInfoButton();
	afx_msg void OnBnClickedSaveButton();
};
