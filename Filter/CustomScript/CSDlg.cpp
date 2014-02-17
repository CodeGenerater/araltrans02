// CSDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "resource.h"
#include "CSDlg.h"
#include "Util.h"
#include <string>
#include "hash.hpp"

using namespace std;

// CCSDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CCSDlg, CDialog)

CCSDlg::CCSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCSDlg::IDD, pParent)
	, Mode(0)
	, ItemN(0)
	, SelectN(-1)
	, ListFilename(_T(""))
	, PassN(0)
	, Radio2(0)
	, Cached(false)
	, Always(false)
{
}

CCSDlg::~CCSDlg()
{
}

void CCSDlg::Init()
{
	FILE * Src=NULL;
	_wfopen_s(&Src,ListFilename.GetString(),L"rt,ccs=UTF-8");
	FileList.clear();
	CSFList.DeleteAllItems();
	ItemN=0;
	if(Src)
	{
		wchar_t TempString[4096]=L"";
		UINT TempHash=0;

		while(fgetws(TempString,4096,Src))
		{
			size_t Len=wcslen(TempString);
			if(TempString[Len-1]==L'\n')
			{
				TempString[--Len]=L'\0';
			}
			if(Len>0)
				TempHash=MakeFilenameHash(TempString);
			if(Len>0&&FileList.count(TempHash)==0)
			{
				FileDetail TempFile=SetFileDetail(MakeFullPathMFC(TempString));
				CSFList.InsertItem(ItemN,L"");
				CSFList.SetItemText(ItemN,0,TempFile.Subject);
				CSFList.SetItemText(ItemN,1,TempFile.Description);
				CSFList.SetItemText(ItemN,2,TempFile.FileName);
				ItemN++;
				FileList.insert(TempHash);
			}
		}
		fclose(Src);
	}
}

BOOL CCSDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	CSFList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CSFList.InsertColumn(0,L"����",0,80,0);
	CSFList.InsertColumn(1,L"����",0,200,1);
	CSFList.InsertColumn(2,L"��ġ",0,400,2);
	
	SetDlgItemTextW(IDC_EDIT1,L"\\ATData\\CustomScript_Auto.txt");
	Spin1.SetRange(0,1000);

	Init();

	VisibleSet();


	if(Radio2==2)
	{
		SetDlgItemInt(IDC_EDIT2,PassN,1);
		GetDlgItem(IDC_EDIT2)->EnableWindow();
		GetDlgItem(IDC_SPIN1)->EnableWindow();
	}
	else
	{
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN1)->EnableWindow(FALSE);
	}
	return FALSE;
}

void CCSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, Mode);
	DDX_Control(pDX, IDC_LIST1, CSFList);
	DDX_Control(pDX, IDC_SPIN1, Spin1);
	DDX_Radio(pDX, IDC_RADIO3, Radio2);
	DDX_Check(pDX, IDC_CHECK1, Cached);
	DDX_Check(pDX, IDC_CHECK2, Always);
}


BEGIN_MESSAGE_MAP(CCSDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CCSDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCSDlg::OnBnClickedButton2)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CCSDlg::OnNMClickList1)
	ON_BN_CLICKED(IDOK, &CCSDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &CCSDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CCSDlg::OnBnClickedRadio2)
	ON_EN_CHANGE(IDC_EDIT2, &CCSDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_RADIO5, &CCSDlg::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO4, &CCSDlg::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO3, &CCSDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_BUTTON3, &CCSDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CCSDlg �޽��� ó�����Դϴ�.
#define MAX_USER_BUFFER            6000


void CCSDlg::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	//Knowleadge Base 01
	CString strWorkDir; 
	::GetCurrentDirectory(MAX_PATH, strWorkDir.GetBufferSetLength(MAX_PATH));
	strWorkDir.ReleaseBuffer();
	//Knowleadge Base 01

	TCHAR FileFilter[]=_T("�뺻 ����(*.txt)|*.txt|��� ����(*.*)|*.*||");
	CFileDialog FileDlg(TRUE,L"txt",L"*.txt",OFN_ALLOWMULTISELECT,FileFilter, NULL);
	FileDlg.m_ofn.nFilterIndex = 0;
	FileDlg.m_ofn.nMaxFile = MAX_USER_BUFFER;
	wchar_t *p_file_name_buffer = new wchar_t[MAX_USER_BUFFER];
	wcscpy_s(p_file_name_buffer,MAX_USER_BUFFER, L"*.txt");
	FileDlg.m_ofn.lpstrFile = p_file_name_buffer;
	if(FileDlg.DoModal()==IDOK)
	{
		POSITION pos = FileDlg.GetStartPosition();
		while(pos != NULL){
			CString TFile=FileDlg.GetNextPathName(pos);
			UINT TempHash=MakeFilenameHash(TFile.GetString());
			if(FileList.count(TempHash)==0)
			{
				FileDetail TempFile=SetFileDetail(TFile);
				CSFList.InsertItem(ItemN,L"");
				CSFList.SetItemText(ItemN,0,TempFile.Subject);
				CSFList.SetItemText(ItemN,1,TempFile.Description);
				CSFList.SetItemText(ItemN,2,TempFile.FileName);
				ItemN++;

				FileList.insert(TempHash);
			}
		}
	}
	delete[] p_file_name_buffer;

	//Knowleadge Base 01
	::SetCurrentDirectory(strWorkDir);
}

FileDetail CCSDlg::SetFileDetail(CString FilePath)
{
	FileDetail	Temp;
	Temp.FileName=MakeShortPathMFC(FilePath);
	FILE * Src=NULL;
	_wfopen_s(&Src,FilePath.GetString(),L"rt,ccs=UTF-8");
	if(Src)
	{
		wchar_t TempString[4096]=L"";

		if(fgetws(TempString,4096,Src))
		{
			size_t Len=wcslen(TempString);
			if(TempString[Len-1]==L'\n')
			{
				TempString[--Len]=L'\0';
			}
			Temp.Subject=TempString;
			int Tab=Temp.Subject.Find(L"\t");
			int Tab2=Temp.Subject.Find(L"\t",Tab+1);
			if(Tab2<0)
			{
				Tab2=Temp.Subject.GetLength();
			}
			if(Temp.Subject[Tab+1]==L'\"')
			{
				Temp.Subject=Temp.Subject.Mid(Tab+2,Tab2-Tab-3);
			}
			else
			{
				Temp.Subject=Temp.Subject.Mid(Tab+1,Tab2-Tab-1);
			}
		}

		if(fgetws(TempString,4096,Src))
		{
			size_t Len=wcslen(TempString);
			if(TempString[Len-1]==L'\n')
			{
				TempString[--Len]=L'\0';
			}
			Temp.Description=TempString;
			int Tab=Temp.Description.Find(L"\t");
			int Tab2=Temp.Description.Find(L"\t",Tab+1);
			if(Tab2<0)
			{
				Tab2=Temp.Description.GetLength();
			}
			if(Temp.Description[Tab+1]==L'\"')
			{
				Temp.Description=Temp.Description.Mid(Tab+2,Tab2-Tab-3);
			}
			else
			{
				Temp.Description=Temp.Description.Mid(Tab+1,Tab2-Tab-1);
			}
		}
		fclose(Src);
	}
	else
	{
		CString strTemp = Temp.FileName + CString(L"\r\n������ ������ �ʽ��ϴ�.\r\n������ ���ų� �ٸ� ���α׷��� ��� ���� �� �ֽ��ϴ�.");
		::MessageBox(NULL, strTemp.GetString(), L"����", MB_OK|MB_ICONWARNING);
		Temp.Subject = L"���� ����";
		Temp.Description = L"������ �ʽ��ϴ�.";
	}

	return Temp;
}

void CCSDlg::OnBnClickedButton2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(SelectN>=0&&SelectN<ItemN)
	{
		FileList.erase(FileList.find(MakeFilenameHash(CSFList.GetItemText(SelectN,2).GetString())));
		CSFList.DeleteItem(SelectN);
		ItemN--;
		SelectN=-1;
	}
}

void CCSDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	NM_LISTVIEW * pNMListView = (NM_LISTVIEW*) pNMHDR;

	SelectN=pNMListView->iItem;

}
void CCSDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(Mode==0)
	{
		FILE * Src=NULL;
		_wfopen_s(&Src,ListFilename.GetString(),L"wt,ccs=UTF-8");
		if(Src)
		{
			for(int i=0;i<ItemN;i++)
			{
				fwprintf(Src,L"%s\n",CSFList.GetItemText(i,2).GetString());
			}
		fclose(Src);
		}
	}
	OnOK();
}

void CCSDlg::OnBnClickedRadio1()
{
	VisibleSet();
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}

void CCSDlg::OnBnClickedRadio2()
{
	VisibleSet();
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}

void CCSDlg::OnEnChangeEdit2()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialog::OnInitDialog() �Լ��� �������ϰ�  ����ũ�� OR �����Ͽ� ������
	// ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ���ؾ߸�
	// �ش� �˸� �޽����� �����ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	PassN=GetDlgItemInt(IDC_EDIT2,0,1);
}

void CCSDlg::OnBnClickedRadio5()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	PassN=GetDlgItemInt(IDC_EDIT2,0,1);
	GetDlgItem(IDC_EDIT2)->EnableWindow();
	GetDlgItem(IDC_SPIN1)->EnableWindow();
}

void CCSDlg::OnBnClickedRadio4()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	PassN=-1;
	GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN1)->EnableWindow(FALSE);
}

void CCSDlg::OnBnClickedRadio3()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	PassN=0;
	GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN1)->EnableWindow(FALSE);
}

void CCSDlg::SetPassN(int _PassN)
{
	PassN=_PassN;
	if(PassN==0)
	{
		Radio2=0;
	}
	else if(PassN==-1)
	{
		Radio2=1;
	}
	else
	{
		Radio2=2;
	}
}

int CCSDlg::GetPassN()
{
	switch (Radio2)
	{
	case 0:
		return 0;
		break;
	case 1:
		return -1;
		break;
	default:
		return PassN;
		break;
	}
}
void CCSDlg::OnBnClickedButton3()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(::MessageBox(NULL,L"��� ������ ����ðڽ��ϱ�?",L"Ȯ��",MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
		FileList.clear();
		while (ItemN>0)
		{
			CSFList.DeleteItem(0);
			ItemN--;
		}
	}
}

void CCSDlg::VisibleSet()
{
	UpdateData(TRUE);
	//�����ϰ�

	int A=SW_HIDE;
	int B=SW_SHOWNORMAL;
	if(Mode==1)
	{
		A=SW_HIDE;
		B=SW_SHOWNORMAL;
	}
	else
	{
		A=SW_SHOWNORMAL;
		B=SW_HIDE;
	}
	GetDlgItem(IDC_BUTTON1)->ShowWindow(A);
	GetDlgItem(IDC_BUTTON2)->ShowWindow(A);
	GetDlgItem(IDC_BUTTON3)->ShowWindow(A);
	GetDlgItem(IDC_LIST1)->ShowWindow(A);
	GetDlgItem(IDC_STATIC2)->ShowWindow(A);
	GetDlgItem(IDC_CHECK2)->ShowWindow(A);
	
	GetDlgItem(IDC_STATIC3)->ShowWindow(B);
	GetDlgItem(IDC_EDIT1)->ShowWindow(B);
	GetDlgItem(IDC_STATIC4)->ShowWindow(B);
	GetDlgItem(IDC_EDIT2)->ShowWindow(B);
	GetDlgItem(IDC_SPIN1)->ShowWindow(B);
	GetDlgItem(IDC_RADIO3)->ShowWindow(B);
	GetDlgItem(IDC_RADIO4)->ShowWindow(B);
	GetDlgItem(IDC_RADIO5)->ShowWindow(B);
	GetDlgItem(IDC_CHECK1)->ShowWindow(B);
}
