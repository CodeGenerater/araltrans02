// DumpDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "DumpText.h"
#include "DumpDlg.h"


// CDumpDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDumpDlg, CDialog)

CDumpDlg::CDumpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDumpDlg::IDD, pParent)
{

}

CDumpDlg::~CDumpDlg()
{
}

BOOL CDumpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect WndRect;
	CDumpDlg::GetClientRect(WndRect);
	DumpEdit.MoveWindow(WndRect,TRUE);
	DumpEdit.SetLimitText(-1);

	g_csThread.Lock();
	g_bContinue = true;
	g_csThread.Unlock();

	m_PrintThread = AfxBeginThread(PrintThreadFunc,1324, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED,NULL);
	if(NULL == m_PrintThread)
	{
		//��������! ū����!
	}
}

void CDumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, DumpEdit);
}


BEGIN_MESSAGE_MAP(CDumpDlg, CDialog)
END_MESSAGE_MAP()


// CDumpDlg �޽��� ó�����Դϴ�.

BOOL CDumpDlg::PrintText(CString Text)
{
	if(m_PrintThread)
	{
		g_csThread.Lock();
		//�ؽ�Ʈ�� ����ֵ�������!
		m_PrintQueue.push(Text);

		g_csThread.Unlock();

		return TRUE;
	}
	else
	{
		//�����尡 �������� �ʾ����Ƿ�
		return FALSE;
	}
}

UINT CDumpDlg::PrintThreadFunc(LPVOID lpParam)
{
	CString Text;
	bool inText=false;
	while(true)
		{
		Sleep(1);
		inText=false;

		g_csThread.Lock();
		if(FALSE == g_bContinue)
		{
			g_csThread.Unlock();
			break;
		}
		else
		{
			if(!m_PrintQueue.empty())
			{
				Text = m_PrintQueue.front();
				m_PrintQueue.pop();
				inText=true;
			}
			g_csThread.Unlock();

			if(inText)
			{

			}
		}

	}
	return 0L;
}