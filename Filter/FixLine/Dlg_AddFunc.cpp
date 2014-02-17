// Dlg_AddFunc.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "FixLine.h"
#include "Dlg_AddFunc.h"


// CDlg_AddFunc ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlg_AddFunc, CDialog)

CDlg_AddFunc::CDlg_AddFunc(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_AddFunc::IDD, pParent)
	, m_nSelect(0)
	, m_nItemN(0)
	, m_strText(_T(""))
	, m_nType(0)
	, m_bTrim(FALSE)
{

}

CDlg_AddFunc::~CDlg_AddFunc()
{
}

void CDlg_AddFunc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctrList);
	DDX_Text(pDX, IDC_EDIT1, m_strText);
	DDX_Radio(pDX, IDC_RADIO1, m_nType);
	DDX_Check(pDX, IDC_CHECK2, m_bTrim);
}

BOOL CDlg_AddFunc::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_cToolTip.Create(this);
	m_cToolTip.SetMaxTipWidth(700);
	
	///////////////////////////////
	//���� ���� ����!!!
	///////////////////////////////

	//����
	//m_cToolTip.AddTool(&m_edit, L"�ؽ�Ʈ �ڽ��Դϴ�.");
	//m_cToolTip.AddTool(GetDlgItem(IDC_Check_Len_Min),L"�o��");

	//����, ���� ����
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO1),L"�ش� ������ Ư���� �Ӽ��� ������ �ʵ��� �մϴ�.\r\n��κ��� ��� �� ���� �����մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO2),L"�ش� ������ �� ���ڿ��� ���� ������ �� ���(�;��� ���) ����մϴ�.");
	m_cToolTip.AddTool(GetDlgItem(IDC_RADIO3),L"�ش� ������ �� ���ڿ��� ������ ������ �� ���(�;��� ���) ����մϴ�.");

	///////////////////////////////
	//���� ���� ��!!!
	///////////////////////////////

	m_ctrList.ModifyStyle(LVS_TYPEMASK,LVS_REPORT|LVS_SHOWSELALWAYS|LVS_NOSORTHEADER);
	m_ctrList.SetExtendedStyle(m_ctrList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT|LVCF_TEXT|LVCF_WIDTH;
	lvColumn.fmt=LVCFMT_CENTER;
	lvColumn.cx=38;
	lvColumn.pszText = L"����";
	lvColumn.cchTextMax = 6;
	m_ctrList.InsertColumn(0,&lvColumn);
	m_ctrList.SetColumn(0,&lvColumn);
	m_ctrList.InsertColumn(1,L"���� ����",LVCFMT_LEFT,111);
	m_ctrList.InsertColumn(2,L"����",LVCFMT_CENTER,38);
	m_ctrList.InsertColumn(3,L"����",LVCFMT_CENTER,38);
	m_ctrList.InsertColumn(4,L"����",LVCFMT_CENTER,38);
	m_ctrList.InsertColumn(5,L"���",LVCFMT_CENTER,38);
	m_ctrList.SetColumnWidth(5,LVSCW_AUTOSIZE_USEHEADER);

	
	for(m_nItemN=0;m_nItemN<m_FuncPool.size();m_nItemN++)
	{
		CString strTemp;
		if(m_FuncPool[m_nItemN].bFunc)
		{
			m_ctrList.InsertItem(m_nItemN,L"����");
		}
		else
		{
			m_ctrList.InsertItem(m_nItemN,L"����");
		}
		
		m_ctrList.SetItemText(m_nItemN,1,m_FuncPool[m_nItemN].strText.c_str());


		if(m_FuncPool[m_nItemN].bTrans)
				strTemp=L"��";
			else
				strTemp=L"��";
		m_ctrList.SetItemText(m_nItemN,2,strTemp);

		switch(m_FuncPool[m_nItemN].nSize)
		{
		case -1:
			m_ctrList.SetItemText(m_nItemN,3,L"�ڵ�");
			break;
		case -2:
			m_ctrList.SetItemText(m_nItemN,3,L"��");
			break;
		default:
			strTemp.Format(L"%d",m_FuncPool[m_nItemN].nSize);
			m_ctrList.SetItemText(m_nItemN,3,strTemp);
		}

		if(m_FuncPool[m_nItemN].bDel)
				strTemp=L"��";
			else
				strTemp=L"��";
		m_ctrList.SetItemText(m_nItemN,4,strTemp);

		if(m_FuncPool[m_nItemN].bPass)
				strTemp=L"��";
			else
				strTemp=L"��";
		m_ctrList.SetItemText(m_nItemN,5,strTemp);
	}

	return FALSE;
}

BEGIN_MESSAGE_MAP(CDlg_AddFunc, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlg_AddFunc::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlg_AddFunc::OnBnClickedButton3)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CDlg_AddFunc::OnNMClickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDlg_AddFunc::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlg_AddFunc::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CDlg_AddFunc::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlg_AddFunc �޽��� ó�����Դϴ�.

void CDlg_AddFunc::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	CDlg_AddSubFunc2 m_cAddSubFunc;
	//���� ����
	if(m_nSelect>=0&&m_nSelect<m_nItemN)
	{
		m_cAddSubFunc.nSize=m_FuncPool[m_nSelect].nSize;
		m_cAddSubFunc.bTrans=m_FuncPool[m_nSelect].bTrans;
		m_cAddSubFunc.bDel=m_FuncPool[m_nSelect].bDel;
		m_cAddSubFunc.bPass=m_FuncPool[m_nSelect].bPass;
		if(m_FuncPool[m_nSelect].bFunc) m_cAddSubFunc.strText=m_FuncPool[m_nSelect].strText.c_str();
		else							m_cAddSubFunc.strText=L"";
		
		m_cAddSubFunc.nMin=m_FuncPool[m_nSelect].nMin;
		m_cAddSubFunc.nMax=m_FuncPool[m_nSelect].nMax;
		m_cAddSubFunc.bFunc=m_FuncPool[m_nSelect].bFunc;
		
		if(m_cAddSubFunc.DoModal()==IDOK)
		{
			FLStringElement2 FLTemp;
			FLTemp.nSize=m_cAddSubFunc.nSize;
			FLTemp.bTrans=m_cAddSubFunc.bTrans;
			FLTemp.bDel=m_cAddSubFunc.bDel;
			FLTemp.bPass=m_cAddSubFunc.bPass;
			FLTemp.strText=m_cAddSubFunc.strText;
			FLTemp.nMin=m_cAddSubFunc.nMin;
			FLTemp.nMax=m_cAddSubFunc.nMax;
			FLTemp.bFunc=m_cAddSubFunc.bFunc;
			m_FuncPool[m_nSelect] = FLTemp;

			CString strTemp;
			if(m_FuncPool[m_nSelect].bFunc)
			{
				m_ctrList.SetItemText(m_nSelect,0,L"����");
			}
			else
			{
				m_ctrList.SetItemText(m_nSelect,0,L"����");
			}
			
			m_ctrList.SetItemText(m_nSelect,1,m_FuncPool[m_nSelect].strText.c_str());

			if(m_FuncPool[m_nSelect].bTrans)
				strTemp=L"��";
			else
				strTemp=L"��";
			m_ctrList.SetItemText(m_nSelect,2,strTemp);

			switch(m_FuncPool[m_nSelect].nSize)
			{
			case -1:
				m_ctrList.SetItemText(m_nSelect,3,L"�ڵ�");
				break;
			case -2:
				m_ctrList.SetItemText(m_nSelect,3,L"��");
				break;
			default:
				strTemp.Format(L"%d",m_FuncPool[m_nSelect].nSize);
				m_ctrList.SetItemText(m_nSelect,3,strTemp);
			}

			if(m_FuncPool[m_nSelect].bDel)
					strTemp=L"��";
				else
					strTemp=L"��";
			m_ctrList.SetItemText(m_nSelect,4,strTemp);

			if(m_FuncPool[m_nSelect].bPass)
					strTemp=L"��";
				else
					strTemp=L"��";
			m_ctrList.SetItemText(m_nSelect,5,strTemp);
		}
	}

	m_strText=L"";
	for(int i =0;i<m_nItemN;i++)
	{
		m_strText+=m_FuncPool[i].strText.c_str();
	}

	UpdateData(0);
}

void CDlg_AddFunc::OnBnClickedButton3()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	//���� �߰�
	CDlg_AddSubFunc2 m_cAddSubFunc;
	m_cAddSubFunc.bFunc=true;
	m_cAddSubFunc.nSize=-1;
	m_cAddSubFunc.bTrans=false;
	m_cAddSubFunc.bDel=false;
	m_cAddSubFunc.bPass=false;
	m_cAddSubFunc.strText=L"";
	m_cAddSubFunc.nMin=-1;
	m_cAddSubFunc.nMax=-1;
	if(m_cAddSubFunc.DoModal()==IDOK)
	{
		FLStringElement2 FLTemp;
		FLTemp.nSize=m_cAddSubFunc.nSize;
		FLTemp.bTrans=m_cAddSubFunc.bTrans;
		FLTemp.bDel=m_cAddSubFunc.bDel;
		FLTemp.bPass=m_cAddSubFunc.bPass;
		FLTemp.strText=m_cAddSubFunc.strText;
		FLTemp.nMin=m_cAddSubFunc.nMin;
		FLTemp.nMax=m_cAddSubFunc.nMax;
		FLTemp.bFunc=m_cAddSubFunc.bFunc;
		m_FuncPool.push_back(FLTemp);

		CString strTemp;
		if(m_FuncPool[m_nItemN].bFunc)
		{
			m_ctrList.InsertItem(m_nItemN,L"����");
		}
		else
		{
			m_ctrList.InsertItem(m_nItemN,L"����");
		}
		
		m_ctrList.SetItemText(m_nItemN,1,m_FuncPool[m_nItemN].strText.c_str());

		if(m_FuncPool[m_nItemN].bTrans)
			strTemp=L"��";
		else
			strTemp=L"��";
		m_ctrList.SetItemText(m_nItemN,2,strTemp);


		switch(m_FuncPool[m_nItemN].nSize)
		{
		case -1:
			m_ctrList.SetItemText(m_nItemN,3,L"�ڵ�");
			break;
		case -2:
			m_ctrList.SetItemText(m_nItemN,3,L"��");
			break;
		default:
			strTemp.Format(L"%d",m_FuncPool[m_nItemN].nSize);
			m_ctrList.SetItemText(m_nItemN,3,strTemp);
		}

		if(m_FuncPool[m_nItemN].bDel)
				strTemp=L"��";
			else
				strTemp=L"��";
		m_ctrList.SetItemText(m_nItemN,4,strTemp);

		if(m_FuncPool[m_nItemN].bPass)
				strTemp=L"��";
			else
				strTemp=L"��";
		m_ctrList.SetItemText(m_nItemN,5,strTemp);
		m_nItemN++;
	}
	m_strText=L"";
	for(int i =0;i<m_nItemN;i++)
	{
		m_strText.Append(m_FuncPool[i].strText.c_str());
	}
	
	UpdateData(0);
}

void CDlg_AddFunc::OnBnClickedButton2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	//���� ����
	if(m_nSelect>=0&&m_nSelect<m_nItemN)
	{
		m_ctrList.DeleteItem(m_nSelect);
		m_FuncPool.erase(m_FuncPool.begin()+m_nSelect);
		
		m_nItemN--;
		m_nSelect--;
	}

	m_strText=L"";
	for(int i =0;i<m_nItemN;i++)
	{
		m_strText+=m_FuncPool[i].strText.c_str();
	}
	UpdateData(0);
}

void CDlg_AddFunc::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	*pResult = 0;
	NM_LISTVIEW * pNMListView = (NM_LISTVIEW*) pNMHDR;

	m_nSelect=pNMListView->iItem;
}

void CDlg_AddFunc::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	*pResult = 0;
	NM_LISTVIEW * pNMListView = (NM_LISTVIEW*) pNMHDR;

	m_nSelect=pNMListView->iItem;

	OnBnClickedButton1();
}



void CDlg_AddFunc::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(m_nItemN>0)
		OnOK();
	else
		MessageBox(L"���ڰ� �ϳ��� �����ϴ�",L"����",MB_ICONWARNING);
}

BOOL CDlg_AddFunc::PreTranslateMessage(MSG* pMsg)
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
