#pragma once
#include "afxeditbrowsectrl.h"
#include "afxwin.h"


// CFileAddDlg ��ȭ �����Դϴ�.

class CFileAddDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileAddDlg)

public:
	CFileAddDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CFileAddDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_File };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()

	void SetFileType(CString strPath);
public:
	bool m_bDragFile;
	int m_nLoadType;
	int m_nDicType;


	afx_msg void OnBnClickedRadioOpen();
	afx_msg void OnBnClickedRadioNew();

	virtual BOOL OnInitDialog();
	CEdit m_ctrPath;
	CString m_strPath;
	CButton m_ctrFind;
	afx_msg void OnBnClickedBtnFind();
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
