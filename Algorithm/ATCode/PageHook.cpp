// PageHook.cpp : implementation file
//

#pragma warning(disable:4996)
#pragma comment (lib, "psapi.lib")

#include "stdafx.h"
#include "ATCode.h"
#include "PageHook.h"
#include "OptionMgr.h"
#include "OptionDlg.h"
#include "MemoryDlg.h"
#include "ModuleInfoDlg.h"

// CPageHook dialog

IMPLEMENT_DYNAMIC(CPageHook, CDialog)

CPageHook::CPageHook(CWnd* pParent /*=NULL*/)
	: CDialog(CPageHook::IDD, pParent), m_pHookNode(NULL)
	, m_iTransMethod(0)
	, m_nHookType(0)
	, m_nOffset(-1)
{

}

CPageHook::~CPageHook()
{
}

void CPageHook::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_TRANS_ARGS, m_comboTransArgs);
	DDX_Radio(pDX, IDC_RADIO_HOOK1, m_nHookType);

	DDX_Control(pDX, IDC_CHK_SCRTRANS, m_chkScrTrans);
	DDX_Control(pDX, IDC_CHK_UNICODE, m_chkUnicode);
	DDX_Control(pDX, IDC_CHK_UNICODE8, m_chkUnicode8);
	DDX_Control(pDX, IDC_CHK_SAFE, m_chkSafe);
	DDX_Control(pDX, IDC_CHK_ALLSAMETEXT, m_chkAllSameText);
	DDX_Control(pDX, IDC_CHK_CLIP_JPN, m_chkClipJpn);
	DDX_Control(pDX, IDC_CHK_CLIP_KOR, m_chkClipKor);
	DDX_Control(pDX, IDC_CHK_REMOVE_SPACE, m_chkRemoveSpace);
	DDX_Control(pDX, IDC_CHK_TWOBYTE, m_chkTwoByte);
	DDX_Control(pDX, IDC_CHK_ONEBYTE, m_chkOneByte);
	DDX_Control(pDX, IDC_CHK_ADDNULL, m_chkAddNull);
	DDX_Control(pDX, IDC_CHK_KIRINAME, m_chkKiriName);

	DDX_Control(pDX, IDC_BTN_ARG_DEL, m_btnDelArg);
	DDX_Radio(pDX, IDC_RADIO_NOP, m_iTransMethod);
	DDX_Control(pDX, IDC_CHK_IGNORE, m_chkIgnore);
	DDX_Control(pDX, IDC_CHK_MATCH, m_chkMatch);
	DDX_Control(pDX, IDC_CHK_FORCEPTR, m_chkForcePtr);
	DDX_Control(pDX, IDC_CHK_USELEN, m_chkUseLen);
	DDX_Control(pDX, IDC_EDIT_LENPOS, m_editLenPos);
	DDX_Control(pDX, IDC_CHK_UNICODE2, m_chkNullchar);
	DDX_Control(pDX, IDC_CHK_UNICODE3, m_chkReverse);
	DDX_Control(pDX, IDC_CHK_YURIS, m_chkYuris);
	DDX_Control(pDX, IDC_CHK_ENDPOS, m_chkEndPos);
	DDX_Control(pDX, IDC_CHK_LENEND, m_chkLenEnd);
	DDX_Control(pDX, IDC_CHK_TJSSTR, m_chkTjsStr);
	DDX_Control(pDX, IDC_CHK_WILL, m_chkWill);
	DDX_Control(pDX, IDC_CHK_DUPLICATED, m_chkDuplicated);
	DDX_Control(pDX, IDC_CHK_PTRBACK, m_chkPtrBack);
	DDX_Control(pDX, IDC_RADIO_HOOK1, m_RdoHook1);
	DDX_Control(pDX, IDC_BTN_ARG_ADD, m_RdoHook2);
}


BEGIN_MESSAGE_MAP(CPageHook, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANS_ARGS, &CPageHook::OnCbnSelchangeComboTransArgs)
	ON_BN_CLICKED(IDC_BTN_ARG_ADD, &CPageHook::OnBnClickedBtnArgAdd)
	ON_BN_CLICKED(IDC_BTN_ARG_DEL, &CPageHook::OnBnClickedBtnArgDel)
	
	ON_BN_CLICKED(IDC_RADIO_HOOK1, &CPageHook::OnBnClickedRadioHook)
	ON_BN_CLICKED(IDC_RADIO_HOOK2, &CPageHook::OnBnClickedRadioHook)

	ON_BN_CLICKED(IDC_BTN_DEL_HOOK, &CPageHook::OnBnClickedBtnDelHook)

	ON_BN_CLICKED(IDC_RADIO_NOP, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_PTRCHANGE, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_OVERWRITE, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_SOW, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_SMSTR, &CPageHook::OnBnClickedRadio)

	ON_BN_CLICKED(IDC_CHK_UNICODE, &CPageHook::OnBnClickedChkUnicode)
	ON_BN_CLICKED(IDC_CHK_ALLSAMETEXT, &CPageHook::OnBnClickedChkAllsametext)
	ON_BN_CLICKED(IDC_CHK_CLIP_JPN, &CPageHook::OnBnClickedChkClipJpn)
	ON_BN_CLICKED(IDC_CHK_CLIP_KOR, &CPageHook::OnBnClickedChkClipKor)
	ON_BN_CLICKED(IDC_CHK_REMOVE_SPACE, &CPageHook::OnBnClickedChkRemoveSpace)
	ON_BN_CLICKED(IDC_CHK_TWOBYTE, &CPageHook::OnBnClickedChkTwobyte)
	ON_BN_CLICKED(IDC_CHK_SAFE, &CPageHook::OnBnClickedChkSafe)
	ON_BN_CLICKED(IDC_CHK_ADDNULL, &CPageHook::OnBnClickedChkAddnull)
	ON_BN_CLICKED(IDC_CHK_KIRINAME, &CPageHook::OnBnClickedChkKiriname)
	ON_BN_CLICKED(IDC_CHK_IGNORE, &CPageHook::OnBnClickedChkIgnore)
	ON_BN_CLICKED(IDC_CHK_MATCH, &CPageHook::OnBnClickedChkMatch)
	ON_BN_CLICKED(IDC_CHK_USELEN, &CPageHook::OnBnClickedChkUselen)
	ON_EN_CHANGE(IDC_EDIT_LENPOS, &CPageHook::OnEnChangeEditLenpos)
	ON_BN_CLICKED(IDC_CHK_UNICODE2, &CPageHook::OnBnClickedChkNullchar)
	ON_BN_CLICKED(IDC_CHK_UNICODE3, &CPageHook::OnBnClickedChkReverse)
	ON_BN_CLICKED(IDC_CHK_YURIS, &CPageHook::OnBnClickedChkYuris)
	ON_BN_CLICKED(IDC_CHK_FORCEPTR, &CPageHook::OnBnClickedChkForcePtr)
	ON_BN_CLICKED(IDC_CHK_ENDPOS, &CPageHook::OnBnClickedChkEndpos)
	ON_BN_CLICKED(IDC_CHK_ONEBYTE, &CPageHook::OnBnClickedChkOnebyte)
	ON_BN_CLICKED(IDC_CHK_LENEND, &CPageHook::OnBnClickedChkLenend)
	ON_BN_CLICKED(IDC_CHK_TJSSTR, &CPageHook::OnBnClickedChkTjsstr)
	ON_BN_CLICKED(IDC_CHK_SCRTRANS, &CPageHook::OnBnClickedChkScrtrans)
	ON_BN_CLICKED(IDC_CHK_WILL, &CPageHook::OnBnClickedChkWill)
	ON_BN_CLICKED(IDC_CHK_PTRBACK, &CPageHook::OnBnClickedChkPtrback)
	ON_BN_CLICKED(IDC_CHK_DUPLICATED, &CPageHook::OnBnClickedChkDuplicated)
	ON_BN_CLICKED(IDC_CHK_UNICODE8, &CPageHook::OnBnClickedChkUnicode8)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
//
// ��ŷ ���� UI ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CPageHook::InitFromHookNode( COptionNode* pHookNode )
{
	BOOL bRetVal = FALSE;

	m_pHookNode = NULL;

	m_comboTransArgs.Clear();
	m_comboTransArgs.ResetContent();
	m_comboTransArgs.EnableWindow(FALSE);

	this->GetDlgItem(IDC_RADIO_NOP)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_PTRCHANGE)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_OVERWRITE)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_SOW)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_SMSTR)->EnableWindow(FALSE);

	this->GetDlgItem(IDC_RADIO_HOOK1)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_HOOK2)->EnableWindow(FALSE);

	m_chkScrTrans.EnableWindow(FALSE);
	m_chkUnicode.EnableWindow(FALSE);
	m_chkUnicode8.EnableWindow(FALSE);
	m_chkNullchar.EnableWindow(FALSE);
	m_chkReverse.EnableWindow(FALSE);
	m_chkAllSameText.EnableWindow(FALSE);
	m_chkClipKor.EnableWindow(FALSE);
	m_chkClipJpn.EnableWindow(FALSE);
	m_chkRemoveSpace.EnableWindow(FALSE);
	m_chkSafe.EnableWindow(FALSE);
	m_chkTwoByte.EnableWindow(FALSE);
	m_chkOneByte.EnableWindow(FALSE);
	m_chkAddNull.EnableWindow(FALSE);
	m_chkKiriName.EnableWindow(FALSE);
	m_chkIgnore.EnableWindow(FALSE);
	m_chkMatch.EnableWindow(FALSE);
	m_chkForcePtr.EnableWindow(FALSE);
	m_chkYuris.EnableWindow(FALSE);
	m_chkWill.EnableWindow(FALSE);
	m_chkEndPos.EnableWindow(FALSE);
	m_chkTjsStr.EnableWindow(FALSE);
	m_chkUseLen.EnableWindow(FALSE);
	m_editLenPos.EnableWindow(FALSE);
	m_chkLenEnd.EnableWindow(FALSE);
	m_chkPtrBack.EnableWindow(FALSE);
	m_chkDuplicated.EnableWindow(FALSE);


	m_btnDelArg.EnableWindow(FALSE);

	ClearCtrlValues();

	try
	{
		if(NULL==pHookNode || pHookNode->GetValue().CompareNoCase(_T("HOOK"))) throw -1;

		// �� �ּҰ� ���� ����� ����
		COptionNode* pAddrNode = pHookNode->GetChild(0);
		if(pAddrNode==NULL) throw -2;

		CString strAddr = pAddrNode->GetValue();

		m_nOffset = strAddr.Find('~');

		// �� �ּҿ� ���� ��ŷ ��ɵ� ����
		int cnt2 = pHookNode->GetChildCount();
		for(int j=1; j<cnt2; j++)
		{
			COptionNode* pCmdNode = pHookNode->GetChild(j);
			CString strCmdValue = pCmdNode->GetValue();

			// ���� ���
			if(strCmdValue.CompareNoCase(_T("TRANS"))==0)
			{
				// ���� �Ÿ�
				COptionNode* pDistNode = pCmdNode->GetChild(0);
				if(pDistNode==NULL) throw -4;

				CString strStorage = pDistNode->GetValue().MakeUpper();
				int nComboIdx = m_comboTransArgs.AddString(strStorage);
				m_comboTransArgs.SetItemData(nComboIdx, (DWORD_PTR)pCmdNode);

			}
			else if(strCmdValue.CompareNoCase(_T("RETNPOS"))==0)
			{
				// ���� �Ÿ�
				COptionNode* pDistNode = pCmdNode->GetChild(0);
				if(pDistNode==NULL) throw -4;

				CString strStorage = pDistNode->GetValue().MakeUpper();
				if(strStorage == _T("SOURCE"))
				{
					m_nHookType = 1;
				}
				else if(strStorage == _T("COPY"))
				{
					m_nHookType = 0;
				}
			}
		}

		// �޺��ڽ��� �ϳ��� ������ Ȱ��ȭ��Ŵ
		if( m_comboTransArgs.GetCount() > 0 )
		{
			m_comboTransArgs.SetCurSel(0);
			m_comboTransArgs.EnableWindow(TRUE);

			if( m_nOffset < 0 )
			{
				this->GetDlgItem(IDC_RADIO_HOOK1)->EnableWindow(TRUE);
				this->GetDlgItem(IDC_RADIO_HOOK2)->EnableWindow(TRUE);
			}

			this->GetDlgItem(IDC_RADIO_NOP)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_PTRCHANGE)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_OVERWRITE)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_SOW)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_SMSTR)->EnableWindow(TRUE);

			m_chkScrTrans.EnableWindow(TRUE);
			m_chkUnicode.EnableWindow(TRUE);
			m_chkUnicode8.EnableWindow(TRUE);
			m_chkAllSameText.EnableWindow(TRUE);
			m_chkClipKor.EnableWindow(TRUE);
			m_chkClipJpn.EnableWindow(TRUE);
			m_chkRemoveSpace.EnableWindow(TRUE);
			m_chkSafe.EnableWindow(TRUE);
			m_chkTwoByte.EnableWindow(TRUE);
			m_chkOneByte.EnableWindow(TRUE);
			m_chkAddNull.EnableWindow(TRUE);
			m_chkKiriName.EnableWindow(TRUE);
			m_chkUseLen.EnableWindow(TRUE);
			m_chkDuplicated.EnableWindow(TRUE);

			m_btnDelArg.EnableWindow(TRUE);
			OnCbnSelchangeComboTransArgs();
		}

		UpdateData(FALSE);
		m_pHookNode = pHookNode;
		bRetVal = TRUE;
	}
	catch (int nErr)
	{
		nErr = nErr;
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// UI �ʱ�ȭ
//
//////////////////////////////////////////////////////////////////////////
void CPageHook::ClearCtrlValues()
{
	m_iTransMethod = 0;

	m_chkScrTrans.SetCheck(0);
	m_chkUnicode.SetCheck(0);
	m_chkNullchar.SetCheck(0);
	m_chkReverse.SetCheck(0);
	m_chkAllSameText.SetCheck(0);
	m_chkClipJpn.SetCheck(0);
	m_chkClipKor.SetCheck(0);
	m_chkSafe.SetCheck(0);
	m_chkRemoveSpace.SetCheck(0);
	m_chkTwoByte.SetCheck(0);
	m_chkOneByte.SetCheck(0);
	m_chkAddNull.SetCheck(0);
	m_chkKiriName.SetCheck(0);
	m_chkIgnore.SetCheck(0);
	m_chkMatch.SetCheck(0);
	m_chkForcePtr.SetCheck(0);
	m_chkYuris.SetCheck(0);
	m_chkWill.SetCheck(0);
	m_chkEndPos.SetCheck(0);
	m_chkTjsStr.SetCheck(0);
	m_chkUseLen.SetCheck(0);
	m_editLenPos.SetWindowText(_T(""));
	m_chkLenEnd.SetCheck(0);
	m_chkPtrBack.SetCheck(0);
	m_chkDuplicated.SetCheck(0);
	m_chkIgnore.EnableWindow(FALSE);
	m_chkMatch.EnableWindow(FALSE);
	m_chkForcePtr.EnableWindow(FALSE);
	m_chkNullchar.EnableWindow(FALSE);
	m_chkReverse.EnableWindow(FALSE);
	m_chkYuris.EnableWindow(FALSE);
	m_chkWill.EnableWindow(FALSE);
	m_chkEndPos.EnableWindow(FALSE);
	m_chkTjsStr.EnableWindow(FALSE);
	m_chkPtrBack.EnableWindow(FALSE);

	UpdateData(FALSE);

}
// CPageHook message handlers



//////////////////////////////////////////////////////////////////////////
//
// ���� ���ڰ� �ٲ���� ��
//
//////////////////////////////////////////////////////////////////////////
void CPageHook::OnCbnSelchangeComboTransArgs()
{
	ClearCtrlValues();

	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		// ���� �ɼǵ� ����
		int cnt3 = pTransCmdNode->GetChildCount();
		for(int k=1; k<cnt3; k++)
		{
			COptionNode* pNode3 = pTransCmdNode->GetChild(k);
			CString strTransOption = pNode3->GetValue().MakeUpper();

			if(strTransOption.IsEmpty())
			{
				continue;
			}

			// NOP �ɼ� (�ƹ��۾� ����)
			else if(strTransOption == _T("NOP"))
			{
				m_chkPtrBack.EnableWindow(FALSE);
				m_iTransMethod = 0;
				UpdateData(FALSE);
			}
			// PTRCHEAT �ɼ� (������ �ٲ�ġ��)
			else if(strTransOption == _T("PTRCHEAT"))
			{
				if( m_nOffset<0 )
					m_chkPtrBack.EnableWindow(TRUE);
				else
					m_chkPtrBack.EnableWindow(FALSE);

				m_iTransMethod = 1;
				m_chkIgnore.SetCheck(0);
				m_chkIgnore.EnableWindow(FALSE);
				UpdateData(FALSE);
			}
			// OVERWRITE �ɼ� (�޸� �����)
			else if(strTransOption == _T("OVERWRITE"))
			{
				if( m_nOffset<0 )
					m_chkPtrBack.EnableWindow(TRUE);
				else
					m_chkPtrBack.EnableWindow(FALSE);

				m_iTransMethod = 2;
				m_chkIgnore.SetCheck(0);
				m_chkIgnore.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("IGNORE")) != NULL)
				{
					m_chkIgnore.SetCheck(1);
				}
				UpdateData(FALSE);
			}
			// Script OverWrite �ɼ� (��ũ��Ʈ �����)
			else if(strTransOption == _T("SOW"))
			{
				m_iTransMethod = 4;
				m_chkPtrBack.EnableWindow(FALSE);
				UpdateData(FALSE);
			}
			// SMart STRing �ɼ� (����Ʈ ���ڿ�)
			else if(strTransOption == _T("SMSTR"))
			{
				if( m_nOffset<0 )
					m_chkPtrBack.EnableWindow(TRUE);
				else
					m_chkPtrBack.EnableWindow(FALSE);

				m_iTransMethod = 3;
				m_chkIgnore.SetCheck(0);
				m_chkIgnore.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("IGNORE")) != NULL)
				{
					m_chkIgnore.SetCheck(1);
				}
				m_chkMatch.SetCheck(0);
				m_chkMatch.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("MATCH")) != NULL)
				{
					m_chkMatch.SetCheck(1);
				}
				m_chkForcePtr.SetCheck(0);
				m_chkForcePtr.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("FORCEPTR")) != NULL)
				{
					m_chkForcePtr.SetCheck(1);
				}
				m_chkNullchar.SetCheck(0);
				m_chkNullchar.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("NULLCHAR")) != NULL)
				{
					m_chkNullchar.SetCheck(1);
				}
				m_chkReverse.SetCheck(0);
				m_chkReverse.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("REVERSE")) != NULL)
				{
					m_chkReverse.SetCheck(1);
				}
				m_chkYuris.SetCheck(0);
				m_chkYuris.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("YURIS")) != NULL)
				{
					m_chkYuris.SetCheck(1);
				}
				m_chkWill.SetCheck(0);
				m_chkWill.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("WILL")) != NULL)
				{
					m_chkWill.SetCheck(1);
				}
				m_chkEndPos.SetCheck(0);
				m_chkEndPos.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("ENDPOS")) != NULL)
				{
					m_chkEndPos.SetCheck(1);
				}
				m_chkTjsStr.SetCheck(0);
				m_chkTjsStr.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("TJSSTR")) != NULL)
				{
					m_chkTjsStr.SetCheck(1);
				}
				UpdateData(FALSE);
			}

			// ���� ���ҹ��� �������
			else if(strTransOption == _T("SCRTRANS"))
			{
				m_chkScrTrans.SetCheck(1);
			}

			// ��Ƽ����Ʈ / �����ڵ� ����
			else if(strTransOption == _T("ANSI"))
			{
				m_chkUnicode.SetCheck(0);
				m_chkUnicode8.SetCheck(0);
			}
			else if(strTransOption == _T("UNICODE"))
			{
				m_chkUnicode.SetCheck(1);
				m_chkUnicode8.SetCheck(0);
			}
			else if(strTransOption == _T("UNICODE8"))
			{
				m_chkUnicode8.SetCheck(1);
				m_chkUnicode.SetCheck(0);
			}

			// ��� ��ġ�ϴ� �ؽ�Ʈ ����
			else if(strTransOption == _T("ALLSAMETEXT"))
			{
				m_chkAllSameText.SetCheck(1);
			}

			// ������ �ؽ�Ʈ�� Ŭ�������
			else if(strTransOption == _T("CLIPKOR"))
			{
				m_chkClipKor.SetCheck(1);
			}

			// ���� �ؽ�Ʈ�� Ŭ�������
			else if(strTransOption == _T("CLIPJPN"))
			{
				m_chkClipJpn.SetCheck(1);
			}

			// ���� �ؽ�Ʈ�� �� ��� ���� ����
			else if(strTransOption == _T("REMOVESPACE"))
			{
				m_chkRemoveSpace.SetCheck(1);
			}

			// �Ϻ��� ��ȿ�� �˻�
			else if(strTransOption == _T("SAFE"))
			{
				m_chkSafe.SetCheck(1);
			}

			// �ݰ� ���ڸ� ���� ���ڷ�
			else if(strTransOption == _T("TWOBYTE"))
			{
				m_chkTwoByte.SetCheck(1);
			}

			// ���� ���ڸ� �ݰ� ���ڷ�
			else if(strTransOption == _T("ONEBYTE"))
			{
				m_chkOneByte.SetCheck(1);
			}

			// ���ڻ��� ���鹮�� ����
			else if(strTransOption == _T("ADDNULL"))
			{
				m_chkAddNull.SetCheck(1);
			}

			else if(strTransOption == _T("LEN"))
			{
				COptionNode *pNode4 = pNode3->GetChild(0);
				CString strLenPos = pNode4->GetValue().MakeUpper();

				if (!strLenPos.IsEmpty())
				{
					m_chkUseLen.SetCheck(1);
					m_editLenPos.EnableWindow(TRUE);
					m_editLenPos.SetWindowText(strLenPos);
					m_chkLenEnd.EnableWindow(TRUE);
				}
			}

			// ���Ṯ�� ��������
			else if(strTransOption == _T("LENEND"))
			{
				m_chkLenEnd.SetCheck(1);
			}

			// �⸮�⸮ �̸� ó��
			else if(strTransOption == _T("KIRINAME"))
			{
				m_chkKiriName.SetCheck(1);
			}

			// ������ �������
			else if(strTransOption == _T("PTRBACKUP"))
			{
				m_chkPtrBack.SetCheck(1);
			}

			// �ߺ����� üũ
			else if(strTransOption == _T("DUPLICATED"))
			{
				m_chkDuplicated.SetCheck(1);
			}
		}
	}
	UpdateData(FALSE);
}

void CPageHook::OnBnClickedRadio()
{	
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		// ���� ���� ��� ����
		pTransCmdNode->DeleteChild(_T("NOP"));
		pTransCmdNode->DeleteChild(_T("PTRCHEAT"));
		pTransCmdNode->DeleteChild(_T("OVERWRITE"));
		pTransCmdNode->DeleteChild(_T("SMSTR"));
		pTransCmdNode->DeleteChild(_T("SOW"));

		// �� ���� ��� ����
		UpdateData(TRUE);
		LPCTSTR TRANS_METHOS[] = {
			_T("NOP"),
			_T("PTRCHEAT"),
			_T("OVERWRITE"),
			_T("SMSTR"),
			_T("SOW")
		};
		
		COptionNode* pNode = pTransCmdNode->CreateChild();
		pNode->SetValue(TRANS_METHOS[m_iTransMethod]);
		
		if(2 == m_iTransMethod)	// 2 (OVERWRITE)
		{
			m_chkIgnore.EnableWindow(TRUE);
			m_chkMatch.EnableWindow(FALSE);
			m_chkForcePtr.EnableWindow(FALSE);
			m_chkNullchar.EnableWindow(FALSE);
			m_chkReverse.EnableWindow(FALSE);
			m_chkYuris.EnableWindow(FALSE);
			m_chkWill.EnableWindow(FALSE);
			m_chkEndPos.EnableWindow(FALSE);
			m_chkTjsStr.EnableWindow(FALSE);
			OnBnClickedChkIgnore();
			if( m_nOffset>0 ) m_chkPtrBack.EnableWindow(FALSE);
			else m_chkPtrBack.EnableWindow(TRUE);
		}
		else if(3 == m_iTransMethod) // 3(SMSTR)
		{
			m_chkIgnore.EnableWindow(TRUE);
			m_chkMatch.EnableWindow(TRUE);
			m_chkForcePtr.EnableWindow(TRUE);
			m_chkNullchar.EnableWindow(TRUE);
			m_chkReverse.EnableWindow(TRUE);
			m_chkYuris.EnableWindow(TRUE);
			m_chkWill.EnableWindow(TRUE);
			m_chkEndPos.EnableWindow(TRUE);
			m_chkTjsStr.EnableWindow(TRUE);
			if( m_nOffset>0 ) m_chkPtrBack.EnableWindow(FALSE);
			else m_chkPtrBack.EnableWindow(TRUE);
			OnBnClickedChkIgnore();
			OnBnClickedChkMatch();
			OnBnClickedChkForcePtr();
			OnBnClickedChkNullchar();
			OnBnClickedChkReverse();
			OnBnClickedChkYuris();
			OnBnClickedChkEndpos();
			OnBnClickedChkTjsstr();
			OnBnClickedChkWill();
		}
		else if(1 == m_iTransMethod) //  1(PTRCHEAT)
		{
			m_chkIgnore.EnableWindow(FALSE);
			m_chkMatch.EnableWindow(FALSE);
			m_chkForcePtr.EnableWindow(FALSE);
			m_chkNullchar.EnableWindow(FALSE);
			m_chkReverse.EnableWindow(FALSE);
			m_chkYuris.EnableWindow(FALSE);
			m_chkWill.EnableWindow(FALSE);
			m_chkEndPos.EnableWindow(FALSE);
			m_chkTjsStr.EnableWindow(FALSE);
			if( m_nOffset>0 ) m_chkPtrBack.EnableWindow(FALSE);
			else m_chkPtrBack.EnableWindow(TRUE);
		}
		else // 0 (NOP), 4 (SOW)
		{
			m_chkIgnore.EnableWindow(FALSE);
			m_chkMatch.EnableWindow(FALSE);
			m_chkForcePtr.EnableWindow(FALSE);
			m_chkNullchar.EnableWindow(FALSE);
			m_chkReverse.EnableWindow(FALSE);
			m_chkYuris.EnableWindow(FALSE);
			m_chkWill.EnableWindow(FALSE);
			m_chkEndPos.EnableWindow(FALSE);
			m_chkTjsStr.EnableWindow(FALSE);
			m_chkPtrBack.EnableWindow(FALSE);
		}
	}
	UpdateData(FALSE);
}

void CPageHook::OnBnClickedRadioHook()
{
	UpdateData(TRUE);
	COptionNode* pRootNode = m_pHookNode;
	int cnt = pRootNode->GetChildCount();
	BOOL bRetnPos = 0;
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();
		if (strValue == _T("RETNPOS"))
		{
			COptionNode * pNode2 = pNode->GetChild(0);
			if (m_nHookType)
			{
				pNode2->SetValue(_T("SOURCE"));
				pNode2->DeleteChild(_T("COPY"));
			}
			else
			{
				pNode2->SetValue(_T("COPY"));
				pNode2->DeleteChild(_T("SOURCE"));
			}
			bRetnPos = 1;
		}
	}

	if(bRetnPos == 0)
	{
		COptionNode * pNode = pRootNode->CreateChild();
		pNode->SetValue(_T("RETNPOS"));
		COptionNode * pNode2 = pNode->CreateChild();
		if (m_nHookType)
		{
			pNode2->SetValue(_T("SOURCE"));
		}
		else
		{
			pNode2->SetValue(_T("COPY"));
		}
	}
}

void CPageHook::OnBnClickedChkScrtrans()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("SCRTRANS"), m_chkScrTrans);
	}
}

void CPageHook::OnBnClickedChkUnicode()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("UNICODE"), m_chkUnicode);
		if(m_chkUnicode.GetCheck() == 1)
		{
			m_chkUnicode8.SetCheck(0);
			SetChildNodeFromCheckbox(pTransCmdNode, _T("UNICODE8"), m_chkUnicode8);
		}
	}
}


void CPageHook::OnBnClickedChkUnicode8()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("UNICODE8"), m_chkUnicode8);
		if(m_chkUnicode8.GetCheck() == 1)
		{
			m_chkUnicode.SetCheck(0);
			SetChildNodeFromCheckbox(pTransCmdNode, _T("UNICODE"), m_chkUnicode);
		}
	}
}


void CPageHook::OnBnClickedChkAllsametext()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("ALLSAMETEXT"), m_chkAllSameText);
	}
}


void CPageHook::OnBnClickedChkClipJpn()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("CLIPJPN"), m_chkClipJpn);
	}
}

void CPageHook::OnBnClickedChkClipKor()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("CLIPKOR"), m_chkClipKor);
	}
}

void CPageHook::OnBnClickedChkRemoveSpace()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("REMOVESPACE"), m_chkRemoveSpace);
	}
}

void CPageHook::OnBnClickedChkTwobyte()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("TWOBYTE"), m_chkTwoByte);
	}
}

void CPageHook::OnBnClickedChkOnebyte()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("ONEBYTE"), m_chkOneByte);
	}
}

void CPageHook::OnBnClickedChkSafe()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("SAFE"), m_chkSafe);
	}
}

void CPageHook::OnBnClickedChkAddnull()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("ADDNULL"), m_chkAddNull);
	}
}


void CPageHook::OnBnClickedChkKiriname()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("KIRINAME"), m_chkKiriName);
	}
}


void CPageHook::OnBnClickedChkPtrback()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("PTRBACKUP"), m_chkPtrBack);
	}
}

void CPageHook::OnBnClickedChkDuplicated()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("DUPLICATED"), m_chkDuplicated);
	}
}



void CPageHook::SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox)
{
	COptionNode* pNode = pParentNode->GetChild(cszChildName);

	// üũ�� ���
	if(checkbox.GetCheck())
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

void CPageHook::OnBnClickedBtnArgAdd()
{
	CMemoryDlg memdlg;

	if( memdlg.DoModal() == IDOK)
	{
		try
		{
			 if( NULL == m_pHookNode ) throw -1;
			
			CString strMem = _T("");

			if(memdlg.m_bCustom)
			{
				strMem = memdlg.m_strCustomMem;
			}
			else
			{
				strMem = memdlg.m_strSelectedArg;
			}
			strMem.Remove(_T(' '));

			// ���� ��ȿ�� �˻�
			if(strMem.IsEmpty())
			{
				this->MessageBox(_T("������ �޸𸮸� ������ �ֽʽÿ�."), _T("���"));
				throw -4;
			}

			// TRANS ��� ����
			COptionNode* pNode = m_pHookNode->CreateChild();
			if(NULL == pNode) throw -2;
			pNode->SetValue(_T("TRANS"));

			// �޸� ����
			COptionNode* pMemNode = pNode->CreateChild();
			if(NULL == pMemNode) throw -3;
			pMemNode->SetValue(strMem);

			// UI ����
			if( InitFromHookNode(m_pHookNode) )
			{
				int nComboIdx = m_comboTransArgs.FindString(-1, strMem);
				if(nComboIdx != CB_ERR)
				{
					m_comboTransArgs.SetCurSel(nComboIdx);
					OnCbnSelchangeComboTransArgs();
				}

				COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);				
			}
			else
			{
				m_pHookNode->DeleteChild(pNode);
				InitFromHookNode(m_pHookNode);
			}

		}
		catch (int nErrCode)
		{
			nErrCode = nErrCode;
		}
	}
}

void CPageHook::OnBnClickedBtnArgDel()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	
	if(nSelIdx >= 0)
	{
		m_pHookNode->DeleteChild( (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx) );
		InitFromHookNode(m_pHookNode);
	}

}


void CPageHook::OnBnClickedBtnDelHook()
{
	
	if( COptionDlg::_Inst && ::IsWindow(COptionDlg::_Inst->m_btnApply.m_hWnd))
	{
		CString msg = m_pHookNode->GetChild(0)->GetValue() + _T("�� �����Ͻðڽ��ϱ�?");
		if(MessageBox(msg, _T("Delete"), MB_YESNO) == IDYES)
		{
			COptionDlg::_Inst->PostMessage(WM_DELETE_HOOK, (WPARAM)m_pHookNode, 0);
		}
		
	}
}

void CPageHook::OnBnClickedChkIgnore()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("OVERWRITE"));
		
		if(!pTransMethod)
			pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));
		
		if(pTransMethod)
		{
			if (m_chkMatch.GetCheck() == 1)
			{
				m_chkMatch.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("MATCH"), m_chkMatch);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("IGNORE"), m_chkIgnore);
		}
		
	}
}

void CPageHook::OnBnClickedChkMatch()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));
		
		if(pTransMethod)
		{
			if (m_chkIgnore.GetCheck() == 1)
			{
				m_chkIgnore.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("IGNORE"), m_chkIgnore);
			}
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("MATCH"), m_chkMatch);
		}

	}
}

void CPageHook::OnBnClickedChkForcePtr()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));

		if(pTransMethod)
		{
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("FORCEPTR"), m_chkForcePtr);
		}
	}
}

void CPageHook::OnBnClickedChkNullchar()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));

		if(pTransMethod)
		{
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("NULLCHAR"), m_chkNullchar);
		}
	}
}

void CPageHook::OnBnClickedChkReverse()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));

		if(pTransMethod)
		{
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("REVERSE"), m_chkReverse);
		}
	}
}

void CPageHook::OnBnClickedChkYuris()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));
		
		if(pTransMethod)
		{
			if (m_chkMatch.GetCheck() == 1)
			{
				m_chkMatch.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("MATCH"), m_chkMatch);
			}
			if (m_chkForcePtr.GetCheck() == 1)
			{
				m_chkForcePtr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("FORCEPTR"), m_chkForcePtr);
			}
			if (m_chkNullchar.GetCheck() == 1)
			{
				m_chkNullchar.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("NULLCHAR"), m_chkNullchar);
			}
			if (m_chkReverse.GetCheck() == 1)
			{
				m_chkReverse.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("REVERSE"), m_chkReverse);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
		}
		
	}
}

void CPageHook::OnBnClickedChkWill()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));
		
		if(pTransMethod)
		{
			if (m_chkIgnore.GetCheck() == 1)
			{
				m_chkIgnore.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("IGNORE"), m_chkIgnore);
			}
			if (m_chkMatch.GetCheck() == 1)
			{
				m_chkMatch.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("MATCH"), m_chkMatch);
			}
			if (m_chkForcePtr.GetCheck() == 1)
			{
				m_chkForcePtr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("FORCEPTR"), m_chkForcePtr);
			}
			if (m_chkNullchar.GetCheck() == 1)
			{
				m_chkNullchar.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("NULLCHAR"), m_chkNullchar);
			}
			if (m_chkReverse.GetCheck() == 1)
			{
				m_chkReverse.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("REVERSE"), m_chkReverse);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
		}
		
	}
}

void CPageHook::OnBnClickedChkEndpos()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));
		
		if(pTransMethod)
		{
			if (m_chkMatch.GetCheck() == 1)
			{
				m_chkMatch.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("MATCH"), m_chkMatch);
			}
			if (m_chkForcePtr.GetCheck() == 1)
			{
				m_chkForcePtr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("FORCEPTR"), m_chkForcePtr);
			}
			if (m_chkNullchar.GetCheck() == 1)
			{
				m_chkNullchar.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("NULLCHAR"), m_chkNullchar);
			}
			if (m_chkReverse.GetCheck() == 1)
			{
				m_chkReverse.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("REVERSE"), m_chkReverse);
			}
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			if (m_chkTjsStr.GetCheck() == 1)
			{
				m_chkTjsStr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
		}
		
	}
}

void CPageHook::OnBnClickedChkTjsstr()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("SMSTR"));
		
		if(pTransMethod)
		{
			if (m_chkIgnore.GetCheck() == 1)
			{
				m_chkIgnore.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("IGNORE"), m_chkIgnore);
			}
			if (m_chkMatch.GetCheck() == 1)
			{
				m_chkMatch.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("MATCH"), m_chkMatch);
			}
			if (m_chkForcePtr.GetCheck() == 1)
			{
				m_chkForcePtr.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("FORCEPTR"), m_chkForcePtr);
			}
			if (m_chkNullchar.GetCheck() == 1)
			{
				m_chkNullchar.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("NULLCHAR"), m_chkNullchar);
			}
			if (m_chkReverse.GetCheck() == 1)
			{
				m_chkReverse.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("REVERSE"), m_chkReverse);
			}
			if (m_chkYuris.GetCheck() == 1)
			{
				m_chkYuris.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("YURIS"), m_chkYuris);
			}
			if (m_chkEndPos.GetCheck() == 1)
			{
				m_chkEndPos.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("ENDPOS"), m_chkEndPos);
			}
			if (m_chkWill.GetCheck() == 1)
			{
				m_chkWill.SetCheck(0);
				SetChildNodeFromCheckbox(pTransMethod, _T("WILL"), m_chkWill);
			}
			SetChildNodeFromCheckbox(pTransMethod, _T("TJSSTR"), m_chkTjsStr);
		}
		
	}
}

void CPageHook::OnBnClickedChkUselen()
{
	int nUseLen=m_chkUseLen.GetCheck();

	if (nUseLen)
	{
		m_editLenPos.EnableWindow(TRUE);
		m_chkLenEnd.EnableWindow(TRUE);
	}
	else
	{
		m_editLenPos.SetWindowText(_T(""));
		m_editLenPos.EnableWindow(FALSE);
		m_chkLenEnd.EnableWindow(FALSE);
		
		int nSelIdx = m_comboTransArgs.GetCurSel();
		COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

		if (pTransCmdNode)
		{
			SetChildNodeFromCheckbox(pTransCmdNode, _T("LEN"), m_chkUseLen);
			if (m_chkLenEnd.GetCheck() == 1)
			{
				m_chkLenEnd.SetCheck(0);
				SetChildNodeFromCheckbox(pTransCmdNode, _T("LENEND"), m_chkLenEnd);
			}
		}
	}
}


void CPageHook::OnBnClickedChkLenend()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("LENEND"), m_chkLenEnd);
	}
}


void CPageHook::OnEnChangeEditLenpos()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialog::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	if (m_chkUseLen.GetCheck() == 0)
		return;

	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode *pLenNode = pTransCmdNode->GetChild(_T("LEN"));

		CString strLenPos;

		m_editLenPos.GetWindowText(strLenPos);

		strLenPos.Trim();

		if (!strLenPos.IsEmpty())
		{
			// LEN(pos) ����
			if (pLenNode == NULL)
			{
				pLenNode = pTransCmdNode->CreateChild();
				pLenNode->SetValue(_T("LEN"));
			}

			COptionNode *pLenPosNode = pLenNode->GetChild(0);

			if (pLenPosNode == NULL)
			{
				pLenPosNode = pLenNode->CreateChild();
			}

			pLenPosNode->SetValue(strLenPos);
		}
		else
		{
			if (pLenNode)
				pTransCmdNode->DeleteChild(pLenNode);
		}
		
		if( COptionDlg::_Inst && ::IsWindow(COptionDlg::_Inst->m_btnApply.m_hWnd))
		{
			COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);
		}
	}
}

