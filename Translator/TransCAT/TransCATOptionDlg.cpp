// TransCATOptionDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "TransCAT.h"
#include "TransCATOptionDlg.h"


// CTransCATOptionDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CTransCATOptionDlg, CDialog)

CTransCATOptionDlg::CTransCATOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransCATOptionDlg::IDD, pParent)
	, m_bRemoveTrace(FALSE)
	, m_bRemoveDupSpace(FALSE)
{

}

CTransCATOptionDlg::~CTransCATOptionDlg()
{
}

void CTransCATOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_ALLOW_TRACE, m_bRemoveTrace);
	DDX_Check(pDX, IDC_CHK_ALLOW_DUPSPACE, m_bRemoveDupSpace);
}


BEGIN_MESSAGE_MAP(CTransCATOptionDlg, CDialog)
END_MESSAGE_MAP()


// CTransCATOptionDlg �޽��� ó�����Դϴ�.
