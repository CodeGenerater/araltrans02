// AboutDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "DumpText.h"
#include "AboutDlg.h"
#include "afxdialogex.h"


// CAboutDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAboutDlg, CDialogEx)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAboutDlg::IDD, pParent)
{

}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAboutDlg �޽��� ó�����Դϴ�.
