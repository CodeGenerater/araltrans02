// OptionPage1.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Resource.h"
#include "OptionPage1.h"

// COptionPage1 ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(COptionPage1, CPropertyPage)

COptionPage1::COptionPage1()
	: CPropertyPage(COptionPage1::IDD)
	, m_nMaxChar(0)
	, m_nMaxLine(0)
	, m_bLimitLine(FALSE)
	, m_strRetChar(_T(""))
	, m_bOK(false)
	, m_bLoaded(false)
	, m_bUseRetChar(FALSE)
	, m_bUseRet(FALSE)
	, m_nLenRadio(0)
	, m_bForceLen(FALSE)
	, m_nMaxLen(0)
{
	m_psp.dwFlags &= ~PSP_HASHELP;
}

COptionPage1::~COptionPage1()
{
}

void COptionPage1::OnOK()
{
	m_nMaxChar=GetDlgItemInt(IDC_TAB1_EDIT2,0,1);
	m_nMaxLine=GetDlgItemInt(IDC_TAB1_EDIT5,0,1);
	UpdateData();
	if(m_strRetChar.GetLength()==0)
	{
		MessageBox(L"���� ��ȣ�� �������� �� �� �����ϴ�!",L"���",MB_ICONWARNING);
		m_strRetChar=L"\\n";
	}
	m_bOK=true;

	if(m_nLenRadio==0)
	{
		m_nMaxLen=-2;
		m_bForceLen=FALSE;
	}
	else if(m_nLenRadio==1)
	{
		m_nMaxLen=-1;
		m_bForceLen=TRUE;
	}
	else if(m_nLenRadio==2)
	{
		m_nMaxLen=GetDlgItemInt(IDC_TAB1_EDIT7);
	}

}

BOOL COptionPage1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_cToolTip.Create(this);
	m_cToolTip.SetMaxTipWidth(700);
	
	///////////////////////////////
	//���� ���� ����!!!
	///////////////////////////////

	//����
	//m_cToolTip.AddTool(&m_edit, L"�ؽ�Ʈ �ڽ��Դϴ�.");
	//m_cToolTip.AddTool(GetDlgItem(IDC_Check_Len_Min),L"�o��");

	//��� ���� ����
	m_cToolTip.AddTool(GetDlgItem(IDC_CHECK3),L"���๮�ڿ� ���� ���� ó���� ������� �ʽ��ϴ�.");

	////////////////���� ����////////////////////
	//���ٴ� �ִ� ����Ʈ
	m_cToolTip.AddTool(GetDlgItem(IDC_TAB1_EDIT2),L"�� �ٿ� ��� �� �� �ִ� �ִ� ���̸� �Է��մϴ�.\r\n���� ������ �����ذ��� �˸��� ���� ���մϴ�.");

	//�ִ� �ټ�
	m_cToolTip.AddTool(GetDlgItem(IDC_TAB1_EDIT5),L"�ִ� �� ���� ���մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_TAB1_CHECK1),L"���� �� ���� �ѱ��� �ʵ��� �մϴ�.");

	//���� ����
	m_cToolTip.AddTool(GetDlgItem(IDC_TAB1_EDIT1),L"������ ������ �Ǵ� ���ڸ� �Է��մϴ�.\r\n\\r\\n�̳� %0D%0A�� ���� �����ε� �Է� �� �� �ֽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_CHECK1),L"���� ���ڸ� �����մϴ�.\r\n���� ���ڰ� ������ �ڵ����� ����ó���� �ϴ� ��쿡 ����� �� �ֽ��ϴ�.");

	/////////////////��������///////////////////
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO8),L"���ڿ� ��ü ���� ������ ������� �ʽ��ϴ�.(�⺻��)");
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO9),L"���� ���� �״�� �����ؼ� ���� �� �ǵ����ݴϴ�.\r\n���� ���̸� �ʰ��ϴ� ��� ������ ���� ��� ����� �� �ֽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO10),L"������ ���̳��� �����ؼ� ���� �� �ǵ����ݴϴ�.\r\n�ַ� ��ȭâ �ִ���̸� �ʰ��Ҷ� ������ ���� ��� ����� �� �ֽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_TAB1_EDIT7),L"������ ���̳��� �����ؼ� ���� �� �ǵ����ݴϴ�.\r\n�ַ� ��ȭâ �ִ���̸� �ʰ��Ҷ� ������ ���� ��� ����� �� �ֽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_CHECK2),L"'�������̷�'�� ������\r\n'����'(������ ����ó�� O,X ����)�� �ƴ϶�\r\n'Byte ��'�� �ϵ��� �����մϴ�.");

	///////////////////////////////
	//���� ���� ��!!!
	///////////////////////////////

	m_ctrSpin1.SetRange(1,1024);
	m_ctrSpin2.SetRange(1,10);
	m_ctrSpin3.SetRange(1,1024);

	SetDlgItemInt(IDC_TAB1_EDIT2,m_nMaxChar);
	SetDlgItemInt(IDC_TAB1_EDIT5,m_nMaxLine);

	SetDisable(m_bUseRet);

	if(m_nMaxLen==-2)
	{
		OnBnClickedRadio8();
		m_nLenRadio=0;
		m_bForceLen=false;
	}
	else if(m_nMaxLen==-1)
	{
		OnBnClickedRadio9();
		m_nLenRadio=1;
		m_bForceLen=false;
	}
	else
	{
		OnBnClickedRadio10();
		SetDlgItemInt(IDC_TAB1_EDIT7,m_nMaxLen);
		m_nLenRadio=2;
	}


	UpdateData(0);
	m_bOK=false;

	m_bLoaded=true;

	return FALSE;
}

void COptionPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1_SPIN2, m_ctrSpin1);
	DDX_Control(pDX, IDC_TAB1_SPIN3, m_ctrSpin2);
	DDX_Control(pDX, IDC_TAB1_SPIN5, m_ctrSpin3);
	DDX_Check(pDX, IDC_TAB1_CHECK1, m_bLimitLine);
	DDX_Text(pDX, IDC_TAB1_EDIT1, m_strRetChar);
	DDX_Check(pDX, IDC_CHECK1, m_bUseRetChar);
	DDX_Check(pDX, IDC_CHECK3, m_bUseRet);
	DDX_Radio(pDX, IDC_RADIO8, m_nLenRadio);
	DDX_Check(pDX, IDC_CHECK2, m_bForceLen);
}


BEGIN_MESSAGE_MAP(COptionPage1, CPropertyPage)
	ON_BN_CLICKED(IDC_CHECK3, &COptionPage1::OnBnClickedCheck3)
	ON_EN_CHANGE(IDC_TAB1_EDIT7, &COptionPage1::OnEnChangeTab1Edit7)
	ON_BN_CLICKED(IDC_RADIO8, &COptionPage1::OnBnClickedRadio8)
	ON_BN_CLICKED(IDC_RADIO9, &COptionPage1::OnBnClickedRadio9)
	ON_BN_CLICKED(IDC_RADIO10, &COptionPage1::OnBnClickedRadio10)
END_MESSAGE_MAP()


// COptionPage1 �޽��� ó�����Դϴ�.

void COptionPage1::SetDisable(BOOL bDisable)
{
	if(bDisable)
		bDisable=false;
	else
		bDisable=true;
	GetDlgItem(IDC_TAB1_CHECK1)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_SPIN2)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_SPIN3)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_STATIC2)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_EDIT2)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_STATIC3)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_STATIC1)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_EDIT1)->EnableWindow(bDisable);
	GetDlgItem(IDC_TAB1_EDIT5)->EnableWindow(bDisable);
	GetDlgItem(IDC_CHECK1)->EnableWindow(bDisable);

	UpdateData(0);
}

void COptionPage1::OnBnClickedCheck3()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData();
	SetDisable(m_bUseRet);
}
void COptionPage1::OnEnChangeTab1Edit7()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CPropertyPage::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(m_bLoaded)
		m_nMaxLen=GetDlgItemInt(IDC_TAB1_EDIT7,0,1);
}

void COptionPage1::OnBnClickedRadio8()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	GetDlgItem(IDC_TAB1_EDIT7)->EnableWindow(FALSE);
	GetDlgItem(IDC_TAB1_SPIN5)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_Byte)->EnableWindow(FALSE);
}

void COptionPage1::OnBnClickedRadio9()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	GetDlgItem(IDC_TAB1_EDIT7)->EnableWindow(FALSE);
	GetDlgItem(IDC_TAB1_SPIN5)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_Byte)->EnableWindow(FALSE);
}

void COptionPage1::OnBnClickedRadio10()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	GetDlgItem(IDC_TAB1_EDIT7)->EnableWindow(TRUE);
	GetDlgItem(IDC_TAB1_SPIN5)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK2)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_Byte)->EnableWindow(TRUE);
}

BOOL COptionPage1::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	switch(pMsg->message) 
	{
    case WM_LBUTTONDOWN:            
    case WM_LBUTTONUP:              
    case WM_MOUSEMOVE:
        // ������ ������
        m_cToolTip.RelayEvent(pMsg);
    }
	return CPropertyPage::PreTranslateMessage(pMsg);
}
