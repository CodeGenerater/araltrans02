// WaitingDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "WaitingDlg.h"
#include <MMSystem.h>

#pragma comment( lib, "winmm.lib" )


// CWaitingDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CWaitingDlg, CDialog)

CWaitingDlg::CWaitingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWaitingDlg::IDD, pParent)
	, m_dwEndTime(0)
	, m_dwWaitingTime(0)
{

}


CWaitingDlg::~CWaitingDlg()
{
}


void CWaitingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TIME, m_lblWaitingTime);
	DDX_Control(pDX, IDC_STATIC_PIC, m_picWaiting);
}


BEGIN_MESSAGE_MAP(CWaitingDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CWaitingDlg �޽��� ó�����Դϴ�.
void CWaitingDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	DWORD dwCurTime = timeGetTime();
	if(m_dwEndTime > dwCurTime)
	{
		ShowRemainedTime(m_dwEndTime - dwCurTime);
	}
	else
	{
		ShowRemainedTime(0);
		KillTimer(11);
		PostMessage(WM_CLOSE);
	}
	
	CDialog::OnTimer(nIDEvent);
}


BOOL CWaitingDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();
	
	if(m_dwWaitingTime)
	{
		m_dwEndTime = timeGetTime() + m_dwWaitingTime;
		ShowRemainedTime(m_dwWaitingTime);
		SetTimer(11, 30, NULL);		
	}

	// �׸�
	//if (m_picWaiting.Load(_T("D:\\AralTrans02Master\\AralTrans\\res\\d0013354_481f257857fd1.gif")))
	if (m_picWaiting.Load(MAKEINTRESOURCE(IDR_GIF1), _T("GIFTYPE")))
	{
		m_picWaiting.Draw();
		//BOOL bIsGIF = m_picWaiting.IsGIF();
		//BOOL bIsAnimatedGIF = m_picWaiting.IsAnimatedGIF();
		//BOOL bIsPlaying = m_picWaiting.IsPlaying();
		//int  nFrameCount = m_picWaiting.GetFrameCount();
	}

	return bRetVal;
}


void CWaitingDlg::ShowRemainedTime( DWORD dwTime )
{
	float fRemain = (float)dwTime / 1000.0F;

	CString strText;
	if(fRemain > 10.0F) strText.Format(_T("%.0f�� ���ҽ��ϴ�."), fRemain);
	else strText.Format(_T("%.1f�� ���ҽ��ϴ�."), fRemain);
	m_lblWaitingTime.SetWindowText(strText);
}

void CWaitingDlg::OnNcDestroy()
{
	CDialog::OnNcDestroy();
	
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	m_picWaiting.UnLoad();
}
