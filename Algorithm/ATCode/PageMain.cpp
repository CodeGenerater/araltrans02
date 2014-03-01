  // PageMain.cpp : implementation file
//

#pragma warning(disable:4996)
#pragma warning(disable:4101)

#include "stdafx.h"
#include "ATCode.h"
#include "PageMain.h"
#include "OptionMgr.h"
#include "OptionDlg.h"
#include "RegistryMgr/cRegistryMgr.h"

// CPageMain dialog

IMPLEMENT_DYNAMIC(CPageMain, CDialog)

LPCTSTR _FONT_LOAD_DESC[] = {
	_T("�ѱ� ��Ʈ�� �ε����� �ʽ��ϴ�."),
	_T("���� ��� �Լ��� �ѱ���Ʈ�� �����մϴ�."),
	_T("�ѱ���Ʈ ���� �� �������� �ʽ��ϴ�."),
	_T("��Ʈ �ε� �� �ѱ� ��Ʈ�� �ε����ݴϴ�."),
	_T("���α׷��� ��� ��Ʈ�� �ѱ۷� �ٲߴϴ�.")
};

LPCTSTR _UNIKO_LOAD_DESC[] = {
	_T("�����ڵ� KoFilter ������� �ʽ��ϴ�."),
	_T("�Ͼ 1�����̻��̸� ����"),
	_T("�ѱۡ�2 > �Ͼ��� ��� ���� ����"),
	_T("�ѱ� > �Ͼ��� ��� ���� ����"),
	_T("�ѱ� > �Ͼ2�� ��� ���� ����"),
	_T("�ѱ� > �Ͼ3�� ��� ���� ����"),
	_T("�ѱ��� 1���� �̻��̸� ���� ����")
};

LPCTSTR _M2W_LOAD_DESC[] = {
	_T("MultiByteToWideChar�� �ڵ��������� �������� �ʽ��ϴ�."),
	_T("�Ͼ 1���ڵ� ������ �ڵ������� ����"),
	_T("�ѱۡ�2 > �Ͼ��� ��� �ڵ������� ����"),
	_T("�ѱ� > �Ͼ��� ��� �ڵ������� ����"),
	_T("�ѱ� > �Ͼ2�� ��� �ڵ������� ����"),
	_T("�ѱ��� 1���� �̻��̸� �ڵ������� ����")
};


CPageMain::CPageMain(CWnd* pParent /*=NULL*/)
	: CDialog(CPageMain::IDD, pParent), m_pRootNode(NULL), m_nEncodeKor(0)
{

}

CPageMain::~CPageMain()
{
}

void CPageMain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_CHK_FORCE_FONT, m_chkForceFont);
	DDX_Control(pDX, IDC_SLIDER1, m_sliderFontLoad);
	DDX_Control(pDX, IDC_EDIT_FONTDESC, m_editFontDesc);
	DDX_Control(pDX, IDC_CHK_FIX_SIZE, m_chkFixSize);
	DDX_Control(pDX, IDC_CHK_ENCODEKOR, m_chkEncodeKor);
	DDX_Control(pDX, IDC_BTN_FONT, m_btnFont);
	DDX_Control(pDX, IDC_CHK_UITRANS, m_chkUITrans);
	DDX_Control(pDX, IDC_CHK_NOASLR, m_chkNoAslr);
	DDX_Control(pDX, IDC_SLIDER2, m_sliderUniKofilter);
	DDX_Control(pDX, IDC_SLIDER3, m_sliderM2W);
	DDX_Control(pDX, IDC_CHK_COMPAREJP, m_chkComJP);
}

BOOL CPageMain::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	m_sliderFontLoad.SetRange(0,4);
	m_sliderFontLoad.SetPos(0);
	m_sliderFontLoad.SetTicFreq(1);
	m_sliderUniKofilter.SetRange(0,6);
	m_sliderUniKofilter.SetPos(0);
	m_sliderUniKofilter.SetTicFreq(1);
	m_sliderM2W.SetRange(0,5);
	m_sliderM2W.SetPos(0);
	m_sliderM2W.SetTicFreq(1);

	return bRetVal;
}

BEGIN_MESSAGE_MAP(CPageMain, CDialog)
	ON_BN_CLICKED(IDC_BTN_FONT, &CPageMain::OnBnClickedBtnFont)
	ON_BN_CLICKED(IDC_CHK_FIX_SIZE, &CPageMain::OnBnClickedChkFixSize)
	ON_BN_CLICKED(IDC_CHK_ENCODEKOR, &CPageMain::OnBnClickedChkEncodekor)	
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_ADD_HOOK, &CPageMain::OnBnClickedBtnAddHook)
	ON_BN_CLICKED(IDC_CHK_UITRANS, &CPageMain::OnBnClickedChkUitrans)
	ON_BN_CLICKED(IDC_CHK_NOASLR, &CPageMain::OnBnClickedChkNoAslr)
	ON_BN_CLICKED(IDC_CHK_COMPAREJP, &CPageMain::OnBnClickedChkComparejp)
	ON_BN_CLICKED(IDC_BTN_HELP, &CPageMain::OnBnClickedBtnHelp)
	ON_BN_CLICKED(IDC_RADIO_ENCODEKOR1, &CPageMain::OnBnClickedRadioEncodekor1)
	ON_BN_CLICKED(IDC_RADIO_ENCODEKOR2, &CPageMain::OnBnClickedRadioEncodekor2)
END_MESSAGE_MAP()


// CPageMain message handlers

void CPageMain::OnBnClickedBtnFont()
{
	// TODO: Add your control notification handler code here
	
	LOGFONT logfont;
	ZeroMemory(&logfont, sizeof(LOGFONT));
	
	// ���� ��Ʈ ��带 ������ LOGFONT ����ü �� ����
	COptionNode* pFontNode = m_pRootNode->GetChild(_T("FONT"));
	if(pFontNode)
	{
		COptionNode* pFontFaceNode = pFontNode->GetChild(0);
		if(pFontFaceNode) _tcscpy(logfont.lfFaceName, pFontFaceNode->GetValue());
		COptionNode* pFontSizeNode = pFontNode->GetChild(1);
		if(pFontSizeNode) logfont.lfHeight = (LONG) _ttoi(pFontSizeNode->GetValue());
	}
	

	// ��Ʈ ���̾�α�
	CFontDialog fd(&logfont, CF_EFFECTS | CF_SCREENFONTS | CF_NOVERTFONTS);
	if( fd.DoModal() == IDOK )
	{
		// LOGFONT ����ü�� ���� ��Ʈ ��带 ����
		if(NULL==pFontNode)
		{
			pFontNode = m_pRootNode->CreateChild();
			pFontNode->SetValue(_T("FONT"));
		}

		// ��Ʈ ���̽���
		COptionNode* pFontFaceNode = pFontNode->GetChild(0);
		if(NULL==pFontFaceNode)
		{
			pFontFaceNode = pFontNode->CreateChild();
		}
		pFontFaceNode->SetValue(logfont.lfFaceName);
		if( pFontFaceNode->GetValue().IsEmpty() ) pFontFaceNode->SetValue(_T("�ü�"));

		// ��Ʈ ������
		COptionNode* pFontSizeNode = pFontNode->GetChild(1);
		if(NULL==pFontSizeNode)
		{
			pFontSizeNode = pFontNode->CreateChild();
		}
		CString strSize;
		strSize.Format(_T("%d"), logfont.lfHeight);
		pFontSizeNode->SetValue(strSize);

		// ���õ� ��Ʈ ǥ��
		CString strFontDesc;
		strFontDesc.Format(_T("%s,%d"), logfont.lfFaceName, logfont.lfHeight);
		m_editFontDesc.SetWindowText(strFontDesc);

		COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);

	}
}

BOOL CPageMain::InitFromRootNode( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;

	try
	{
		if(NULL==pRootNode) throw -1;

		// ��Ʈ�ѵ� �⺻ ���·� ����
		ClearCtrlValues();

		// ��� ��� ��ȸ
		int cnt = pRootNode->GetChildCount();
		for(int i=0; i<cnt; i++)
		{
			COptionNode* pNode = pRootNode->GetChild(i);
			CString strValue = pNode->GetValue().MakeUpper();

			// FORCEFONT �ɼ�
			if(strValue == _T("FORCEFONT"))
			{
				COptionNode* pLevelNode = pNode->GetChild(0);
				
				// ���� ���İ� ȣȯ�� ����
				if(NULL==pLevelNode)
				{
					pLevelNode = pNode->CreateChild();
					pLevelNode->SetValue(_T("10"));
				}
				
				int nLevel = _ttoi(pLevelNode->GetValue().Trim());
				nLevel /= 5;
				this->GetDlgItem(IDC_FONT_DESC)->SetWindowText(_FONT_LOAD_DESC[nLevel]);
				m_sliderFontLoad.SetPos(nLevel);

				m_chkFixSize.EnableWindow(TRUE);
				m_btnFont.EnableWindow(TRUE);
			}

			// UNIKOFILTER �ɼ�
			if(strValue == _T("UNIKOFILTER"))
			{
				COptionNode* pLevelNode = pNode->GetChild(0);

				int nLevel = _ttoi(pLevelNode->GetValue().Trim());
				nLevel /= 5;
				this->GetDlgItem(IDC_UNIKO_DESC)->SetWindowText(_UNIKO_LOAD_DESC[nLevel]);
				m_sliderUniKofilter.SetPos(nLevel);
			}

			// MULTTOWIDE �ɼ�
			if(strValue == _T("MULTTOWIDE"))
			{
				COptionNode* pLevelNode = pNode->GetChild(0);

				int nLevel = _ttoi(pLevelNode->GetValue().Trim());
				nLevel /= 5;
				this->GetDlgItem(IDC_M2W_DESC)->SetWindowText(_M2W_LOAD_DESC[nLevel]);
				m_sliderM2W.SetPos(nLevel);
			}

			// FIXFONTSIZE �ɼ�
			else if(strValue == _T("FIXFONTSIZE"))
			{
				m_chkFixSize.SetCheck(1);
			}

			// FONT �ɼ�
			else if(strValue == _T("FONT"))
			{
				CString strFontDesc = pNode->GetChild(0)->GetValue();
				if(pNode->GetChild(1))
				{
					strFontDesc += _T(',');
					strFontDesc += pNode->GetChild(1)->GetValue();
				}
				m_editFontDesc.SetWindowText( strFontDesc );
			}

			// ENCODEKOR �ɼ�
			else if(strValue == _T("ENCODEKOR"))
			{
				m_chkEncodeKor.SetCheck(1);

				if (pNode->GetChildCount())
				{
					COptionNode *pEncodeOptionNode = pNode->GetChild(0);
					m_nEncodeKor = _ttoi(pEncodeOptionNode->GetValue().Trim());
				}
				else
					m_nEncodeKor = 1;

				CButton * pButton = (CButton *)(this->GetDlgItem(IDC_RADIO_ENCODEKOR2));
				pButton->EnableWindow(TRUE);

				if (m_nEncodeKor == 2)
					pButton->SetCheck(1);

				pButton = (CButton *)(this->GetDlgItem(IDC_RADIO_ENCODEKOR1));
				pButton->EnableWindow(TRUE);

				if (m_nEncodeKor != 2)
					pButton->SetCheck(1);

			}

			// UITRANS �ɼ�
			else if(strValue == _T("UITRANS"))
			{
				m_chkUITrans.SetCheck(1);
			}

			// NOASLR �ɼ�
			else if(strValue == _T("NOASLR"))
			{
				m_chkNoAslr.SetCheck(0);
			}

			// COMPAREJP �ɼ�
			else if(strValue == _T("COMPAREJP"))
			{
				m_chkComJP.SetCheck(1);
			}
		}

		bRetVal = TRUE;

		m_pRootNode = pRootNode;


	}
	catch (int nErr)
	{
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// ��� UI ��Ʈ�� �ʱ�ȭ
//
//////////////////////////////////////////////////////////////////////////
void CPageMain::ClearCtrlValues()
{
	// ��Ʈ���� ���� UI
	m_chkFixSize.SetCheck(0);
	m_chkFixSize.EnableWindow(FALSE);
	m_editFontDesc.Clear();
	m_editFontDesc.EnableWindow(FALSE);
	m_btnFont.EnableWindow(FALSE);
	m_chkEncodeKor.SetCheck(0);
	m_chkUITrans.SetCheck(0);
	m_chkNoAslr.SetCheck(1);
	m_chkComJP.SetCheck(0);
}



//////////////////////////////////////////////////////////////////////////
//
// ���ο� ��ŷ�ڵ� �߰�
//
//////////////////////////////////////////////////////////////////////////
void CPageMain::SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox, BOOL reverse)
{
	if(NULL==pParentNode || NULL==cszChildName || _T('\0')==cszChildName[0]) return;

	COptionNode* pNode = pParentNode->GetChild(cszChildName);

	// ��üũ�� ���
	if(reverse)	reverse=!checkbox.GetCheck();
	else reverse=checkbox.GetCheck();
	// üũ�� ���
	if(reverse)
	{
		if(NULL==pNode)
		{
			pNode = pParentNode->CreateChild();
			pNode->SetValue(cszChildName);
		}
	}
	// üũ ���� �� ���
	else
	{
		if(pNode)
		{
			pParentNode->DeleteChild(pNode);
		}
	}

	
	if( COptionDlg::_Inst && ::IsWindow(COptionDlg::_Inst->m_btnApply.m_hWnd))
	{
		COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);
	}
}



//////////////////////////////////////////////////////////////////////////
//
// ��� UI ��Ʈ�� �̺�Ʈ �ڵ鷯
//
//////////////////////////////////////////////////////////////////////////
/*
void CPageMain::OnBnClickedChkForceFont()
{
	SetChildNodeFromCheckbox(m_pRootNode, _T("FORCEFONT"), m_chkForceFont);
	
	BOOL bEnable = (BOOL)m_chkForceFont.GetCheck();
	
	m_chkFixSize.EnableWindow(bEnable);
	m_editFontDesc.EnableWindow(bEnable);
	m_btnFont.EnableWindow(bEnable);

}
*/

void CPageMain::OnBnClickedChkFixSize()
{
	SetChildNodeFromCheckbox(m_pRootNode, _T("FIXFONTSIZE"), m_chkFixSize);
}


void CPageMain::OnBnClickedChkEncodekor()
{
	SetChildNodeFromCheckbox(m_pRootNode, _T("ENCODEKOR"), m_chkEncodeKor);

	if (m_chkEncodeKor.GetCheck())
	{
		GetDlgItem(IDC_RADIO_ENCODEKOR1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ENCODEKOR2)->EnableWindow(TRUE);

		TCHAR szOption[2]={NULL, };
		szOption[0] = _T('0') + m_nEncodeKor;

		COptionNode *pNode = m_pRootNode->GetChild(_T("ENCODEKOR"));
		pNode = pNode->CreateChild();
		pNode->SetValue(szOption);
	}
	else
	{
		GetDlgItem(IDC_RADIO_ENCODEKOR1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ENCODEKOR2)->EnableWindow(FALSE);
	}
}


void CPageMain::OnBnClickedChkUitrans()
{
	SetChildNodeFromCheckbox(m_pRootNode, _T("UITRANS"), m_chkUITrans);
}

void CPageMain::OnBnClickedChkNoAslr()
{
	SetChildNodeFromCheckbox(m_pRootNode, _T("NOASLR"), m_chkNoAslr, TRUE);
}

void CPageMain::OnBnClickedBtnAddHook()
{
	// TODO: Add your control notification handler code here
	COptionDlg::_Inst->OnBnClickedBtnAddHook();
}


void CPageMain::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	// � �����̴����� �˻�
	if(pScrollBar == (CScrollBar*)&m_sliderFontLoad)
	{
		// ���� ��Ʈ �ε� ���� ����
		COptionNode* pNode = m_pRootNode->GetChild(_T("FORCEFONT"));
		if(pNode)
		{
			m_pRootNode->DeleteChild(pNode);
		}

		// �����̴��κ��� ���� ����
		int pos = m_sliderFontLoad.GetPos();

		// ����
		CString strValue = _T("");
		this->GetDlgItem(IDC_FONT_DESC)->SetWindowText(_FONT_LOAD_DESC[pos]);
		switch(pos)
		{
		case 1:
			strValue = _T("5");
			break;
		case 2:
			strValue = _T("10");
			break;
		case 3:
			strValue = _T("15");
			break;
		case 4:
			strValue = _T("20");
			break;
		default:
			pNode = NULL;
		}

		BOOL bEnable = FALSE;
		if(!strValue.IsEmpty())
		{
			bEnable = TRUE;
			pNode = m_pRootNode->CreateChild();
			pNode->SetValue(_T("FORCEFONT"));
			pNode = pNode->CreateChild();
			pNode->SetValue(strValue);
		}

		m_chkFixSize.EnableWindow(bEnable);
		m_editFontDesc.EnableWindow(bEnable);
		m_btnFont.EnableWindow(bEnable);

	}

	// � �����̴����� �˻�
	else if(pScrollBar == (CScrollBar*)&m_sliderUniKofilter)
	{
		// ���� ��Ʈ �ε� ���� ����
		COptionNode* pNode = m_pRootNode->GetChild(_T("UNIKOFILTER"));
		if(pNode)
		{
			m_pRootNode->DeleteChild(pNode);
		}

		// �����̴��κ��� ���� ����
		int pos = m_sliderUniKofilter.GetPos();

		// ����
		CString strValue = _T("");
		this->GetDlgItem(IDC_UNIKO_DESC)->SetWindowText(_UNIKO_LOAD_DESC[pos]);
		switch(pos)
		{
		case 1:
			strValue = _T("5");
			break;
		case 2:
			strValue = _T("10");
			break;
		case 3:
			strValue = _T("15");
			break;
		case 4:
			strValue = _T("20");
			break;
		case 5:
			strValue = _T("25");
			break;
		case 6:
			strValue = _T("30");
			break;
		default:
			pNode = NULL;
		}

		BOOL bEnable = FALSE;
		if(!strValue.IsEmpty())
		{
			bEnable = TRUE;
			pNode = m_pRootNode->CreateChild();
			pNode->SetValue(_T("UNIKOFILTER"));
			pNode = pNode->CreateChild();
			pNode->SetValue(strValue);
		}
	}

	// � �����̴����� �˻�
	else if(pScrollBar == (CScrollBar*)&m_sliderM2W)
	{
		// ���� ��Ʈ �ε� ���� ����
		COptionNode* pNode = m_pRootNode->GetChild(_T("MULTTOWIDE"));
		if(pNode)
		{
			m_pRootNode->DeleteChild(pNode);
		}

		// �����̴��κ��� ���� ����
		int pos = m_sliderM2W.GetPos();

		// ����
		CString strValue = _T("");
		this->GetDlgItem(IDC_M2W_DESC)->SetWindowText(_M2W_LOAD_DESC[pos]);
		switch(pos)
		{
		case 1:
			strValue = _T("5");
			break;
		case 2:
			strValue = _T("10");
			break;
		case 3:
			strValue = _T("15");
			break;
		case 4:
			strValue = _T("20");
			break;
		case 5:
			strValue = _T("25");
			break;
		default:
			pNode = NULL;
		}

		BOOL bEnable = FALSE;
		if(!strValue.IsEmpty())
		{
			bEnable = TRUE;
			pNode = m_pRootNode->CreateChild();
			pNode->SetValue(_T("MULTTOWIDE"));
			pNode = pNode->CreateChild();
			pNode->SetValue(strValue);
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
void CPageMain::OnBnClickedChkComparejp()
{
	SetChildNodeFromCheckbox(m_pRootNode, _T("COMPAREJP"), m_chkComJP);
}

void CPageMain::OnBnClickedBtnHelp()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CString path = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AralTransHomeDir")) + _T("\\help.chm");
	::ShellExecute(::GetDesktopWindow(), L"open", path , 0, 0, SW_SHOWDEFAULT);
}

void CPageMain::OnBnClickedRadioEncodekor1()
{
	m_nEncodeKor = 1;

	COptionNode *pNode = m_pRootNode->GetChild(_T("ENCODEKOR"));

	if (!pNode->GetChildCount())
		pNode = pNode->CreateChild();
	else
		pNode = pNode->GetChild(0);

	pNode->SetValue(_T("1"));

}

void CPageMain::OnBnClickedRadioEncodekor2()
{
	m_nEncodeKor = 2;

	COptionNode *pNode = m_pRootNode->GetChild(_T("ENCODEKOR"));

	if (!pNode->GetChildCount())
		pNode = pNode->CreateChild();
	else
		pNode = pNode->GetChild(0);

	pNode->SetValue(_T("2"));

}
