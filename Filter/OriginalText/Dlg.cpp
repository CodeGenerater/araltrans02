// Dlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "ATPlugin.h"
#include "Dlg.h"


// CDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlg, CDialog)

CDlg::CDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg::IDD, pParent)
	, Mode(0)
	, bNonTrans(FALSE)
{

}

CDlg::~CDlg()
{
}

void CDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, Mode);
	DDV_MinMaxInt(pDX, Mode, 0, 2);
	DDX_Check(pDX, IDC_CHECK1, bNonTrans);
}


BEGIN_MESSAGE_MAP(CDlg, CDialog)
END_MESSAGE_MAP()


// CDlg �޽��� ó�����Դϴ�.
