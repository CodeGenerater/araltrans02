// FilterDirectInputDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "resource.h"
#include "FilterDirectInputDlg.h"


// CFilterDirectInputDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CFilterDirectInputDlg, CDialog)

CFilterDirectInputDlg::CFilterDirectInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDirectInputDlg::IDD, pParent)
	, m_strFilters(_T(""))
{

}

CFilterDirectInputDlg::~CFilterDirectInputDlg()
{
}

void CFilterDirectInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILTERS, m_strFilters);
}


BEGIN_MESSAGE_MAP(CFilterDirectInputDlg, CDialog)
END_MESSAGE_MAP()


// CFilterDirectInputDlg �޽��� ó�����Դϴ�.
