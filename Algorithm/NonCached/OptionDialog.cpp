// OptionDialog.cpp : ���� �����Դϴ�.
//

#include "NonCached.h"
#include "OptionDialog.h"


// COptionDialog ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(COptionDialog, CDialog)

COptionDialog::COptionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDialog::IDD, pParent)
	, m_bMatchLen(FALSE)
{

}

COptionDialog::~COptionDialog()
{
}

void COptionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_MATCHLENCHECK, m_bMatchLen);
}


BEGIN_MESSAGE_MAP(COptionDialog, CDialog)
	ON_BN_CLICKED(IDC_MATCHLENCHECK, &COptionDialog::OnBnClickedMatchlencheck)
END_MESSAGE_MAP()


// COptionDialog �޽��� ó�����Դϴ�.

BOOL COptionDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_bMatchLen = g_bMatchLen;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void COptionDialog::OnBnClickedMatchlencheck()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData();
}
