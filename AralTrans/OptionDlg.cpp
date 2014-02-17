// OptionDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "OptionDlg.h"

// COptionDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(COptionDlg, CDialog)

COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent)
	, m_nAutoUpdate(0)
	, m_bUseATDataZip(FALSE)
	, m_bDeleteLocalFiles(FALSE)
	, m_bAppLocAutoConfirm(FALSE)
{

}

COptionDlg::~COptionDlg()
{
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_AUTOUPDATE0, m_nAutoUpdate);
	DDX_Check(pDX, IDC_CHECK_USEATDATA, m_bUseATDataZip);
	DDX_Check(pDX, IDC_CHECK_DELETELOCALFILE, m_bDeleteLocalFiles);
	DDX_Check(pDX, IDC_CHECK_APPLOCAUTOCONFIRM, m_bAppLocAutoConfirm);
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_USEATDATA, &COptionDlg::OnBnClickedCheckUseATData)
END_MESSAGE_MAP()


// COptionDlg �޽��� ó�����Դϴ�.

BOOL COptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	if (m_bUseATDataZip)
		GetDlgItem(IDC_CHECK_DELETELOCALFILE)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_CHECK_DELETELOCALFILE)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}
void COptionDlg::OnBnClickedCheckUseATData()
{
	UpdateData(TRUE);

	if (m_bUseATDataZip)
		GetDlgItem(IDC_CHECK_DELETELOCALFILE)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_CHECK_DELETELOCALFILE)->EnableWindow(FALSE);
}
