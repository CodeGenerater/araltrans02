// Dlg_AddName.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "FixLine.h"
#include "Dlg_AddName.h"


// CDlg_AddName ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlg_AddName, CDialog)

CDlg_AddName::CDlg_AddName(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_AddName::IDD, pParent)
	, m_strText(_T(""))
{

}

CDlg_AddName::~CDlg_AddName()
{
}

void CDlg_AddName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_strText);
}


BEGIN_MESSAGE_MAP(CDlg_AddName, CDialog)
END_MESSAGE_MAP()


// CDlg_AddName �޽��� ó�����Դϴ�.
