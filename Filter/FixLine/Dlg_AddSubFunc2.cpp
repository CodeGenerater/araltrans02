// Dlg_AddSubFunc2.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "FixLine.h"
#include "Dlg_AddSubFunc2.h"


// CDlg_AddSubFunc2 ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlg_AddSubFunc2, CDialog)

CDlg_AddSubFunc2::CDlg_AddSubFunc2(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_AddSubFunc2::IDD, pParent)
	, m_nSize(0)
	, m_bSizeAuto(false)
	, m_bSize(false)
	, m_nMin(0)
	, m_nMax(0)
	, m_bMin(false)
	, m_bMax(false)
	, nSize(0)
	, bTrans(false)
	, bDel(false)
	, bPass(false)
	, strText(_T(""))
	, nMin(0)
	, nMax(0)
	, bFunc(false)
{

}

CDlg_AddSubFunc2::~CDlg_AddSubFunc2()
{
}

void CDlg_AddSubFunc2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Spin_Size, m_ctrSpinSize);
	DDX_Control(pDX, IDC_Spin_Len_Min, m_ctrSpinMin);
	DDX_Control(pDX, IDC_Spin_Len_Max, m_ctrSpinMax);
}


BEGIN_MESSAGE_MAP(CDlg_AddSubFunc2, CDialog)
	ON_BN_CLICKED(IDC_RADIO_Func, &CDlg_AddSubFunc2::OnBnClickedRadioFunc)
	ON_BN_CLICKED(IDC_Radio_Text, &CDlg_AddSubFunc2::OnBnClickedRadioText)
	ON_BN_CLICKED(IDC_Radio_Size_Yes, &CDlg_AddSubFunc2::OnBnClickedRadioSizeYes)
	ON_BN_CLICKED(IDC_Radio_Size_No, &CDlg_AddSubFunc2::OnBnClickedRadioSizeNo)
	ON_BN_CLICKED(IDC_Radio_Size_Auto, &CDlg_AddSubFunc2::OnBnClickedRadioSizeAuto)
	ON_BN_CLICKED(IDC_Radio_Size_Custom, &CDlg_AddSubFunc2::OnBnClickedRadioSizeCustom)
	ON_BN_CLICKED(IDC_Check_Len_Min, &CDlg_AddSubFunc2::OnBnClickedCheckLenMin)
	ON_BN_CLICKED(IDC_Check_Len_Max, &CDlg_AddSubFunc2::OnBnClickedCheckLenMax)
	ON_BN_CLICKED(IDC_Check_Del, &CDlg_AddSubFunc2::OnBnClickedCheckDel)
	ON_BN_CLICKED(IDC_Check_Pass, &CDlg_AddSubFunc2::OnBnClickedCheckPass)
	ON_BN_CLICKED(IDC_Radio_Trans_On, &CDlg_AddSubFunc2::OnBnClickedRadioTransOn)
	ON_BN_CLICKED(IDC_Radio_Trans_Off, &CDlg_AddSubFunc2::OnBnClickedRadioTransOff)
	ON_BN_CLICKED(IDOK, &CDlg_AddSubFunc2::OnBnClickedOk)
	ON_BN_CLICKED(IDC_Text_Help, &CDlg_AddSubFunc2::OnBnClickedTextHelp)
END_MESSAGE_MAP()


// CDlg_AddSubFunc2 �޽��� ó�����Դϴ�.

BOOL CDlg_AddSubFunc2::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_cToolTip.Create(this);
	m_cToolTip.SetMaxTipWidth(700);
	
	///////////////////////////////
	//���� ���� ����!!!
	///////////////////////////////

	//����
	//m_cToolTip.AddTool(&m_edit, L"�ؽ�Ʈ �ڽ��Դϴ�.");
	//m_cToolTip.AddTool(GetDlgItem(IDC_Check_Len_Min),L"�o��");

	//�Լ�, �ؽ�Ʈ ����
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO_Func),L"�ش� ������ �Ӽ��� Ư�� �ؽ�Ʈ�� �����մϴ�.\r\n�Ϲ����� ��� ���� �Լ��� ��������, ������ �Ǵ�\r\nƯ�����ڸ� �����ϱ� ���� ���˴ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Text),L"�ش� ������ �Ӽ��� ������ �ؽ�Ʈ�� �����մϴ�.\r\n��κ��� '��� ����'�� �����˴ϴ�.");

	//���� ����
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Size_Yes),L"���� ���� �� ���ڸ� ��꿡 �����ϵ��� �մϴ�.\r\n�Ϲ������� ������ ���� ��糪 Ư�� ������ �� �����մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Size_No),L"���� ���� �� ���ڸ� ��꿡 �������� �ʵ��� �մϴ�.\r\n���� ���Ǵ� ��� �ؽ�Ʈ�� �Լ����� �� �����մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Size_Auto),L"���̸� �ڵ����� ����մϴ�.\r\n�Ϲ������� ���ڿ��� ���̸�ŭ�Դϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Size_Custom),L"���̸� ���� �����մϴ�.\r\n������ Byte�̸� ���� 0�� ��� '����'�� ������ �Ͱ� �����մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Edit_Size),L"���̸� ���� �����մϴ�.\r\n������ Byte�̸� ���� 0�� ��� '����'�� ������ �Ͱ� �����մϴ�.");

	//���� ó��
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Trans_On),L"�� ���ڸ� �����մϴ�.\r\n�Ϲ����� ���� �� �ɼ��� �����մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Radio_Trans_Off),L"�� ���ڸ� �������� �ʽ��ϴ�.\r\n�Լ��γ� Ư���� ��޵Ǵ� �ؽ�Ʈ(�̸�, Ư������ ��)�� ��� �����մϴ�.");

	//���� ó��
	m_cToolTip.AddTool(GetDlgItem(IDC_Check_Len_Min),L"�ּ��� ������ �� ���̸� �����մϴ�.\r\n�� ���̸� ������Ű�� ������ ��� �������� �ν����� �ʽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Edit_Len_Min),L"�ּ��� ������ �� ���̸� �����մϴ�.\r\n�� ���̸� ������Ű�� ������ ��� �������� �ν����� �ʽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Check_Len_Max),L"������ �� ������ �ѵ��� �����մϴ�.\r\n�� ���̸� �ѱ��� �������� �ν����� �ʽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Edit_Len_Max),L"������ �� ������ �ѵ��� �����մϴ�.\r\n�� ���̸� �ѱ��� �������� �ν����� �ʽ��ϴ�.");

	//���ڿ� ����
	m_cToolTip.AddTool(GetDlgItem(IDC_Edit_Text),L"�ؽ�Ʈ�� ���մϴ�.\r\n�Ϲ����� ���� �̿ܿ��� Hex�ڵ�� ���� ��쵵 ���������մϴ�.\r\n�ڼ��� ������ ��ư�� ������ Ȯ���� �� �ֽ��ϴ�.");
	//m_cToolTip.AddTool(GetDlgItem(IDC_Text_Help),L"�o��");

	//��Ÿ ����
	m_cToolTip.AddTool(GetDlgItem(IDC_Check_Del),L"�� ���ڸ� �����մϴ�.\r\n���� �� ��ȯ�Ǵ� ���忡�� �� ���ڰ� ��µ��� �ʽ��ϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_Check_Pass),L"������ �������� ���� ��� '���� ��'���� ó���ϰ� ���� ���ڸ� Ȯ���ϵ��� �մϴ�.");

	///////////////////////////////
	//���� ���� ��!!!
	///////////////////////////////

	m_ctrSpinSize.SetRange(0,1024);
	m_ctrSpinMin.SetRange(0,1024);
	m_ctrSpinMax.SetRange(0,1024);

	//�ʱⰪ ���� (true,false);


	//���ڿ� ����
	SetDlgItemTextW(IDC_Edit_Text,strText.GetString());

	//��Ÿ ó��
	CheckDlgButton(IDC_Check_Del,bDel);
	CheckDlgButton(IDC_Check_Pass,bPass);

	//���� ó��
	CheckDlgButton(IDC_Radio_Trans_On,bTrans);
	CheckDlgButton(IDC_Radio_Trans_Off,!bTrans);

	//���� ����
	if(nSize>=0)
	{
		//���� ����
		m_bSize=true;
		m_bSizeAuto=false;
		SetDlgItemInt(IDC_Edit_Size,nSize);

		CheckDlgButton(IDC_Radio_Size_Yes,1);
		CheckDlgButton(IDC_Radio_Size_No,0);

		CheckDlgButton(IDC_Radio_Size_Auto,0);
		CheckDlgButton(IDC_Radio_Size_Custom,1);

		SetEnableSize(1);

		SetDlgItemInt(IDC_Edit_Size,nSize);
	}
	else if(nSize==-1)
	{
		//���� �ڵ�
		m_bSize=true;
		m_bSizeAuto=true;
		SetDlgItemInt(IDC_Edit_Size,0);

		CheckDlgButton(IDC_Radio_Size_Yes,1);
		CheckDlgButton(IDC_Radio_Size_No,0);

		CheckDlgButton(IDC_Radio_Size_Auto,1);
		CheckDlgButton(IDC_Radio_Size_Custom,0);

		SetEnableSize(1);
		
	}
	else
	{
		//����
		m_bSize=false;
		m_bSizeAuto=true;
		SetDlgItemInt(IDC_Edit_Size,0);

		CheckDlgButton(IDC_Radio_Size_Yes,0);
		CheckDlgButton(IDC_Radio_Size_No,1);

		CheckDlgButton(IDC_Radio_Size_Auto,1);
		CheckDlgButton(IDC_Radio_Size_Custom,0);

		SetEnableSize(0);
	}

	//���� ����
	CheckDlgButton(IDC_RADIO_Func,bFunc);
	CheckDlgButton(IDC_Radio_Text,!bFunc);
	
	//���� ����
	//�ּ�
	if(nMin>=0)
	{
		m_nMin=nMin;
		m_bMin=true;
		SetDlgItemInt(IDC_Edit_Len_Min,m_nMin);
		CheckDlgButton(IDC_Check_Len_Min,1);
	}
	else
	{
		m_nMin=0;
		m_bMin=false;
		SetDlgItemInt(IDC_Edit_Len_Min,m_nMin);
		CheckDlgButton(IDC_Check_Len_Min,0);
	}
	//�ִ�
	if(nMax>=0)
	{
		m_nMax=nMax;
		m_bMax=true;
		SetDlgItemInt(IDC_Edit_Len_Max,m_nMax);
		CheckDlgButton(IDC_Check_Len_Max,1);
	}
	else
	{
		m_nMax=0;
		m_bMax=false;
		SetDlgItemInt(IDC_Edit_Len_Max,m_nMax);
		CheckDlgButton(IDC_Check_Len_Max,0);
	}

	SetEnableMode(bFunc);

	return FALSE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CDlg_AddSubFunc2::OnOK()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CDialog::OnOK();

	
}

void CDlg_AddSubFunc2::SetEnableMode(bool bFunc)
{
	bool a,b;
	if(bFunc)
	{
		//�̰��� �Լ�
		SetEnableMin(-1);
		SetEnableMax(-1);

		a=true;
		b=false;
	}
	else
	{
		//�̰��� �ؽ�Ʈ
		if(m_bMin)	SetEnableMin(1);
		else		SetEnableMin(0);

		if(m_bMax)	SetEnableMax(1);
		else		SetEnableMax(0);

		a=false;
		b=true;
	}

	//true�� Enable�� -_-;

	GetDlgItem(IDC_Edit_Text)->EnableWindow(a);
	GetDlgItem(IDC_Text_Help)->EnableWindow(a);
}

void CDlg_AddSubFunc2::SetEnableSize(int nMode)
{
	bool a,b;
	if(nMode==0)
	{
		a=false;
		b=false;
		SetEnableSizeAuto(-1);
	}
	else
	{
		a=true;
		b=true;
		if(m_bSizeAuto)
		{
			SetEnableSizeAuto(1);
		}
		else
		{
			SetEnableSizeAuto(0);
		}
		
	}

	GetDlgItem(IDC_Radio_Size_Auto)->EnableWindow(a);
	GetDlgItem(IDC_Radio_Size_Custom)->EnableWindow(a);
	
}

void CDlg_AddSubFunc2::SetEnableSizeAuto(int nMode)
{
	bool a,b;
	if(nMode==-1)
	{
		a=false;
		b=false;
	}
	else if(nMode==0)
	{
		a=true;
		b=true;
	}
	else
	{
		a=true;
		b=false;
	}
	GetDlgItem(IDC_Edit_Size)->EnableWindow(b);
	GetDlgItem(IDC_Spin_Size)->EnableWindow(b);
	GetDlgItem(IDC_STATIC_Byte2)->EnableWindow(b);
}

void CDlg_AddSubFunc2::SetEnableMin(int nMode)
{
	bool a,b;
	if(nMode==-1)
	{
		a=false;
		b=false;
	}
	else if(nMode==0)
	{
		a=true;
		b=false;
	}
	else
	{
		a=true;
		b=true;
	}

	GetDlgItem(IDC_Check_Len_Min)->EnableWindow(a);
	GetDlgItem(IDC_Edit_Len_Min)->EnableWindow(b);
	GetDlgItem(IDC_Spin_Len_Min)->EnableWindow(b);
	GetDlgItem(IDC_Spin_Len_Min)->EnableWindow(b);
	GetDlgItem(IDC_STATIC_Len_Min)->EnableWindow(b);

	//�߰� �ؽ�Ʈ�� ���⼭ ó���Ѵ�.
	GetDlgItem(IDC_STATIC_Len1)->EnableWindow(a);
	GetDlgItem(IDC_STATIC_Len2)->EnableWindow(a);
}

void CDlg_AddSubFunc2::SetEnableMax(int nMode)
{
	bool a,b;
	if(nMode==-1)
	{
		a=false;
		b=false;
	}
	else if(nMode==0)
	{
		a=true;
		b=false;
	}
	else
	{
		a=true;
		b=true;
	}

	GetDlgItem(IDC_Check_Len_Max)->EnableWindow(a);
	GetDlgItem(IDC_Edit_Len_Max)->EnableWindow(b);
	GetDlgItem(IDC_Spin_Len_Max)->EnableWindow(b);
	GetDlgItem(IDC_STATIC_Len_Max)->EnableWindow(b);
}

void CDlg_AddSubFunc2::OnBnClickedRadioFunc()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	bFunc=true;
	SetEnableMode(bFunc);
}

void CDlg_AddSubFunc2::OnBnClickedRadioText()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	bFunc=false;
	SetEnableMode(bFunc);
}

void CDlg_AddSubFunc2::OnBnClickedRadioSizeYes()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_bSize=true;
	SetEnableSize(m_bSize);
}

void CDlg_AddSubFunc2::OnBnClickedRadioSizeNo()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_bSize=false;
	SetEnableSize(m_bSize);
}


void CDlg_AddSubFunc2::OnBnClickedRadioSizeAuto()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_bSizeAuto=true;
	SetEnableSizeAuto(m_bSizeAuto);
}

void CDlg_AddSubFunc2::OnBnClickedRadioSizeCustom()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_bSizeAuto=false;
	SetEnableSizeAuto(m_bSizeAuto);
}

void CDlg_AddSubFunc2::OnBnClickedCheckLenMin()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(IsDlgButtonChecked(IDC_Check_Len_Min))
	{
		m_bMin=true;
	}
	else
	{
		m_bMin=false;
	}
	SetEnableMin(m_bMin);
}

void CDlg_AddSubFunc2::OnBnClickedCheckLenMax()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(IsDlgButtonChecked(IDC_Check_Len_Max))
	{
		m_bMax=true;
	}
	else
	{
		m_bMax=false;
	}
	SetEnableMax(m_bMax);
}

void CDlg_AddSubFunc2::OnBnClickedCheckDel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(IsDlgButtonChecked(IDC_Check_Del))
	{
		bDel=true;
	}
	else
	{
		bDel=false;
	}
}

void CDlg_AddSubFunc2::OnBnClickedCheckPass()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(IsDlgButtonChecked(IDC_Check_Pass))
	{
		bPass=true;
	}
	else
	{
		bPass=false;
	}
}

void CDlg_AddSubFunc2::OnBnClickedRadioTransOn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	bTrans=true;
}

void CDlg_AddSubFunc2::OnBnClickedRadioTransOff()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	bTrans=false;
}
void CDlg_AddSubFunc2::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	bool bOK=true;
	//nSize
	if(m_bSize)
	{
		if(m_bSizeAuto)
		{
			nSize=-1;
		}
		else
		{
			nSize=GetDlgItemInt(IDC_Edit_Size);
		}
	}
	else
	{
		nSize=-2;
	}
	
	//bTrans OK
	//bPass OK
	//bDell OK

	
	if(!bFunc)
	{
		//nMin
		if(m_bMin)
		{
			nMin=GetDlgItemInt(IDC_Edit_Len_Min);
		}
		else
		{
			nMin=-1;
		}
		//nMax
		if(m_bMax)
		{
			nMax=GetDlgItemInt(IDC_Edit_Len_Max);
		}
		else
		{
			nMax=-1;
		}
	}
	else
	{
		nMin=-1;
		nMax=-1;
	}

	//bFunc OK

	//strText
	if(bFunc)
	{
		GetDlgItemTextW(IDC_Edit_Text,strText.GetBuffer(2048),2048);
		if(strText==L"")
		{
			bOK=false;
			MessageBox(L"������ �Է����ּ���.",L"����",MB_ICONWARNING);
		}
	}
	else
	{
		strText=L"{T";

		CString strTmp2;
		if(nMin>=0&&nMax>=0)	strTmp2.Format(L",%d~%d",nMin,nMax);
		else if(nMin>=0)		strTmp2.Format(L",%d~",nMin);
		else if(nMax>=0)		strTmp2.Format(L",~%d",nMax);
		strText += strTmp2;

		strText += L"}";
	}

	if(nMin!=-1&&nMax!=-1)
	{
		if(nMin>nMax)
		{
			bOK=false;
			MessageBox(L"���� ������ �߸��Ǿ����ϴ�.",L"����",MB_ICONWARNING);
		}
	}

	if(bOK)
		OnOK();
}

BOOL CDlg_AddSubFunc2::PreTranslateMessage(MSG* pMsg)
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
	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_AddSubFunc2::OnBnClickedTextHelp()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	//���� ���ô�~
	CString strHelp;
	strHelp+=L"���ڿ� ������ �Ϲ����� ���� �Ӹ� �ƴ϶�\r\n";
	strHelp+=L"�߰��� ���ϴ� ���ڸ� ���� �� �� �ֽ��ϴ�.\r\n";
	strHelp+=L"\r\n";
	strHelp+=L"\\t��: ��(\\t, 0x09)\r\n";
	strHelp+=L"\\r��: ���ǵ� ��ȣ(\\r, 0x0D)\r\n";
	strHelp+=L"\\n��: ĳ���� ���� ��ȣ (\\n, 0x0A)\r\n";
	strHelp+=L"\\\\ : \\��� ���˴ϴ�.\r\n";
	strHelp+=L"\r\n";
	strHelp+=L"%FF : Hex�ڵ带 �Է��մϴ�. 1����Ʈ�� �ѹ��� �� �־�� �մϴ�. \r\n";
	strHelp+=L"������(ex:0xA0 0x22 -> %A0%22)\r\n";
	strHelp+=L"%S��: ������ 1����Ʈ ���� �ϳ��� �޽��ϴ�.\r\n";
	strHelp+=L"������������ '�ݰ�����'(����, ���ĺ� ��)�� ��쿡�� �ش�˴ϴ�.\r\n";
	strHelp+=L"������(ex:125 -> %S%S%S)\r\n";
	strHelp+=L"%T��: ������ 2����Ʈ ���� �ϳ��� �޽��ϴ�.\r\n";
	strHelp+=L"������������ '��������'(�Ϻ��� ��)�� ��쿡�� �ش�˴ϴ�.\r\n";
	strHelp+=L"������(ex:���Ϫ誦 -> %T%T%T%T)\r\n";
	strHelp+=L"%%��: %��� ���˴ϴ�.";
	MessageBox(strHelp,L"���� ����");
}
