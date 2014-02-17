#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "PictureEx/PictureEx.h"


// CWaitingDlg ��ȭ �����Դϴ�.

class CWaitingDlg : public CDialog
{
	DECLARE_DYNAMIC(CWaitingDlg)

public:
	CWaitingDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CWaitingDlg();
	DWORD	m_dwEndTime;
	void	ShowRemainedTime(DWORD dwTime);

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_WAITING_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CStatic		m_lblWaitingTime;
	CPictureEx	m_picWaiting;
	DWORD		m_dwWaitingTime;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNcDestroy();
};
