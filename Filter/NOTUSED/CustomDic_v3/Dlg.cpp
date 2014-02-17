// Dlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Dlg.h"
#include "Util.h"

// CDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlg, CDialog)

CDlg::CDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg::IDD, pParent)
	, UseCDic(false)
	, UseGDic(false)
	, nNoMargin(0)
{
}

CDlg::~CDlg()
{
}

void CDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX,IDC_CHECK1,UseCDic);
	DDX_Check(pDX,IDC_CHECK2,UseGDic);
	DDX_Check(pDX, IDC_CHECK3, nNoMargin);
}


BEGIN_MESSAGE_MAP(CDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CDlg �޽��� ó�����Դϴ�.

void CDlg::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	wstring Temp=GetGameDirectory();
	Temp+=L"\\ATData\\";
	MyCreateDirectory(Temp.c_str());
    ShellExecute(NULL, L"Open", L"explorer.exe",  Temp.c_str(), L"", SW_SHOWNORMAL);
}

void CDlg::OnBnClickedButton2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	wstring Temp=GetATDirectory();
	Temp+=L"\\";
	ShellExecute(NULL, L"Open", L"explorer.exe", Temp.c_str(), L"", SW_SHOWNORMAL);
}
