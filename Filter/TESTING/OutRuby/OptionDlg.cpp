// OptionDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "OutRuby.h"
#include "OptionDlg.h"



// COptionDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(COptionDlg, CDialog)

COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent)
	, RubyText(_T(""))
{

}

COptionDlg::~COptionDlg()
{
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, RubyText);
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
END_MESSAGE_MAP()



// COptionDlg �޽��� ó�����Դϴ�.

void COptionDlg::SetRubyText(LPCSTR szRuby)
{
	RubyText=szRuby;
}

void COptionDlg::GetRubyText(LPSTR szDest)
{
	lstrcpyA(szDest,RubyText.GetString());
}