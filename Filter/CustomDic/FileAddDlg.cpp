// FileAddDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "CustomDic.h"
#include "FileAddDlg.h"
#include "afxdialogex.h"
#include "SubFunc.h"
#include "resource.h"
#include "Util.h"

// CFileAddDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CFileAddDlg, CDialogEx)

CFileAddDlg::CFileAddDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileAddDlg::IDD, pParent)
	, m_nLoadType(0)
	, m_nDicType(1)
	, m_bDragFile(false)
	, m_strPath(_T(""))
{

}

CFileAddDlg::~CFileAddDlg()
{
}

void CFileAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_NEW, m_nLoadType);
	DDX_Radio(pDX, IDC_RADIO_PRE, m_nDicType);
	DDX_Control(pDX, IDC_FILE_PATH, m_ctrPath);
	DDX_Text(pDX, IDC_FILE_PATH, m_strPath);
	DDX_Control(pDX, IDC_BTN_FIND, m_ctrFind);
}


BEGIN_MESSAGE_MAP(CFileAddDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_OPEN, &CFileAddDlg::OnBnClickedRadioOpen)
	ON_BN_CLICKED(IDC_RADIO_NEW, &CFileAddDlg::OnBnClickedRadioNew)
	ON_BN_CLICKED(IDC_BTN_FIND, &CFileAddDlg::OnBnClickedBtnFind)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CFileAddDlg �޽��� ó�����Դϴ�.


void CFileAddDlg::OnBnClickedRadioOpen()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	
	OnBnClickedBtnFind();
}


void CFileAddDlg::OnBnClickedRadioNew()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	OnBnClickedBtnFind();
}


BOOL CFileAddDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_ctrPath.SetReadOnly(1);
	if(m_bDragFile){
		m_nLoadType=1;
		SetFileType(m_strPath);
	}

	m_ctrFind.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDI_ICON_FOLDER),IMAGE_ICON,16,16,LR_SHARED));

	UpdateData(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CFileAddDlg::SetFileType(CString strPath){
	//������ ù���� �о ���� Ÿ���� �о����

	//����
	//Header{DicType:Pre}
	//Header{DicType:Body}
	//Header{DicType:Post}
	//�������� ���� �� �ְڴ�!
	//�ٸ� ���� �󸶵��� ������ �ִٸ� �� ��쿡�� Body�� �ν��Ѵ�.

	map<wstring,wstring> header = parseHeader(m_strPath.GetString(),true);

	//Header�� �����ؼ� �⺻ ������ ������
	wstring& strDicType = header[L"DicType"];
	if(strDicType==L"Pre"){
		m_nDicType=0;
	}
	else if(strDicType==L"Post"){
		m_nDicType=2;
	}
	else{
		m_nDicType=1;
	}

	UpdateData(0);
}

void CFileAddDlg::OnBnClickedBtnFind()
{
	UpdateData();

	wchar_t szPathBuff[MAX_PATH];
	swprintf_s(szPathBuff,MAX_PATH,L"%s\\ATData",GetGameDirectory());
	MyCreateDirectory(szPathBuff);

	CFileDialog* pDlg=NULL;
	if(m_nLoadType==0){
		//������
		CFileDialog cDlg(FALSE,L"*.txt",L"CustomDic.txt",
			OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT|OFN_NONETWORKBUTTON|OFN_HIDEREADONLY,
			L"�ؽ�Ʈ ����(*.txt)|*.txt|��� ����(*.*)|*.*||");
		cDlg.m_ofn.Flags |= OFN_NOCHANGEDIR;
		cDlg.m_ofn.lpstrInitialDir=szPathBuff;
		if(cDlg.DoModal()==IDOK){
			m_strPath = cDlg.GetPathName();
		}
		else{
			return;
		}
	}
	else{
		//���� ����
		CFileDialog cDlg(TRUE,L"*.txt",L"CustomDic.txt",
			OFN_FILEMUSTEXIST|OFN_NONETWORKBUTTON|OFN_HIDEREADONLY,
			L"�ؽ�Ʈ ����(*.txt)|*.txt|��� ����(*.*)|*.*||");
		cDlg.m_ofn.Flags |= OFN_NOCHANGEDIR;
		cDlg.m_ofn.lpstrInitialDir=szPathBuff;
		if(cDlg.DoModal()==IDOK){
			m_strPath = cDlg.GetPathName();
		}
		else{
			return;
		}
	}

	if(m_nLoadType==1){
		SetFileType(m_strPath);
	}

	UpdateData(0);
}


void CFileAddDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DWORD dwBufSize;
	CString strFileName;

	dwBufSize = DragQueryFile(hDropInfo,0,NULL,0)+1;
	LPWSTR pszFileName = strFileName.GetBufferSetLength(dwBufSize);
	DragQueryFile(hDropInfo,0,pszFileName,dwBufSize);
	strFileName.ReleaseBuffer();

	m_ctrFind.EnableWindow(1);
	m_nLoadType=1;

	m_strPath = strFileName;

	SetFileType(m_strPath);

	UpdateData(0);
	CDialogEx::OnDropFiles(hDropInfo);
}
