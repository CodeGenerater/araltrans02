// KirikiriOptionDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Kirikiri.h"
#include "KirikiriOptionDlg.h"

extern CKirikiriApp theApp;

// CKirikiriOptionDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CKirikiriOptionDlg, CDialog)

CKirikiriOptionDlg::CKirikiriOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKirikiriOptionDlg::IDD, pParent)
	, m_nCacheMode(0)
	, m_strFont(_T(""))
	, m_bAlsoSrc(FALSE)
	, m_bUseCP2(FALSE)
	, m_nCP2Type(0)
{

}

CKirikiriOptionDlg::~CKirikiriOptionDlg()
{
}

void CKirikiriOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_nCacheMode);
	DDX_Text(pDX, IDC_EDIT1, m_strFont);
	DDX_Check(pDX, IDC_CHECK_ALSO_SRC, m_bAlsoSrc);
	DDX_Check(pDX, IDC_CHECK_USE_CODEPOINT2, m_bUseCP2);
	DDX_Control(pDX, IDC_COMBO_CODEPOINTTYPE, m_comboCP2Type);
}


BEGIN_MESSAGE_MAP(CKirikiriOptionDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_FONT, &CKirikiriOptionDlg::OnBnClickedBtnFont)
	ON_BN_CLICKED(IDC_BTN_CLEAR_CACHE, &CKirikiriOptionDlg::OnBnClickedBtnClearCache)
	ON_BN_CLICKED(IDC_CHECK_USE_CODEPOINT2, &CKirikiriOptionDlg::OnBnClickedCheckUseCodepoint2)
	ON_CBN_SELCHANGE(IDC_COMBO_CODEPOINTTYPE, &CKirikiriOptionDlg::OnCbnSelchangeComboCodepointtype)
END_MESSAGE_MAP()


// CKirikiriOptionDlg �޽��� ó�����Դϴ�.

void CKirikiriOptionDlg::OnBnClickedBtnFont()
{
	LOGFONT logfont;
	ZeroMemory(&logfont, sizeof(LOGFONT));

	_tcscpy_s(logfont.lfFaceName, 32, m_strFont);
	logfont.lfHeight = -18;

	// ��Ʈ ���̾�α�
	CFontDialog fd(&logfont);
	if( fd.DoModal() == IDOK )
	{
		//m_strFont.Format(_T("%s,%d"), logfont.lfFaceName, logfont.lfHeight);
		m_strFont = logfont.lfFaceName;
		UpdateData(FALSE);
	}
}


void CKirikiriOptionDlg::OnBnClickedBtnClearCache()
{
	int nMsgRes = this->MessageBox(_T("���ݱ��� ������ ��ũ��Ʈ��(ZIP ����, �ؽ�Ʈ ����)�� \r\n��� ����� �ʱ�ȭ �Ͻðڽ��ϱ�?"), _T("Warning!"), MB_YESNO | MB_ICONWARNING);

	if(IDYES == nMsgRes)
	{
		if(theApp.ClearCache()) this->MessageBox(_T("ĳ�� ����Ұ� �ʱ�ȭ�Ǿ����ϴ�."), _T("Aral Trans"));
		else  this->MessageBox(_T("ĳ�� �ʱ�ȭ�� �����߽��ϴ�."), _T("Aral Trans"));
	}
}

void CKirikiriOptionDlg::OnBnClickedCheckUseCodepoint2()
{
	UpdateData();

	if (m_bUseCP2)
	{
		m_comboCP2Type.EnableWindow(TRUE);
	}
	else
	{
		m_nCP2Type=0;
		m_comboCP2Type.SetCurSel(0);
		m_comboCP2Type.EnableWindow(FALSE);
	}
}

BOOL CKirikiriOptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_comboCP2Type.InsertString(0, _T("��������"));
	m_comboCP2Type.InsertString(1, _T("�Ϲ�"));
	m_comboCP2Type.InsertString(2, _T("SilverHawk"));

	m_comboCP2Type.SetCurSel(m_nCP2Type);

	if (m_bUseCP2)
		m_comboCP2Type.EnableWindow(TRUE);
	else
		m_comboCP2Type.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CKirikiriOptionDlg::OnCbnSelchangeComboCodepointtype()
{
	UpdateData();
	m_nCP2Type = m_comboCP2Type.GetCurSel();
}
