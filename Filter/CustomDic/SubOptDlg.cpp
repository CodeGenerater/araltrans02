// SubOptDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "CustomDic.h"
#include "SubOptDlg.h"
#include "afxdialogex.h"


// CSubOptDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CSubOptDlg, CDialogEx)

CSubOptDlg::CSubOptDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSubOptDlg::IDD, pParent)
	, m_nAutoInterval(0)
	, m_bAutoLoad(FALSE)
	, m_bTrim(FALSE)
	, m_bSaveDefaultOption(FALSE)
{

}

CSubOptDlg::~CSubOptDlg()
{
}

void CSubOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_nAutoInterval);
	DDX_Check(pDX, IDC_CHECK1, m_bAutoLoad);
	DDX_Check(pDX, IDC_CHECK2, m_bTrim);
	DDX_Control(pDX, IDC_SPIN1, m_ctrSpin);
	DDX_Check(pDX, IDC_CHECK3, m_bSaveDefaultOption);
}


BEGIN_MESSAGE_MAP(CSubOptDlg, CDialogEx)
END_MESSAGE_MAP()


// CSubOptDlg �޽��� ó�����Դϴ�.


BOOL CSubOptDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	m_ctrSpin.SetRange(1,300);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}
