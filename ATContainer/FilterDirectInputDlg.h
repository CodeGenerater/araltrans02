#pragma once


// CFilterDirectInputDlg ��ȭ �����Դϴ�.

class CFilterDirectInputDlg : public CDialog
{
	DECLARE_DYNAMIC(CFilterDirectInputDlg)

public:
	CFilterDirectInputDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CFilterDirectInputDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIRECT_OPTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	// ���� ����Ʈ�ڽ� DDE ��Ʈ��
	CString m_strFilters;
};
