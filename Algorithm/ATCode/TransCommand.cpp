#include "StdAfx.h"

#include "TransCommand.h"
#include "OptionMgr.h"
#include "CharacterMapper.h"
#include "TransScriptParser.h"
#include "ATCodeMgr.h"

#include "Debug.h"

#pragma warning(disable:4996)

//////////////////////////////////////////////////////////////////////////
//
// �⺻ Ŭ���� �޼ҵ�
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �ɼǿ� ���� �׿� �´� Ŭ���� �ν��Ͻ��� �����մϴ�.
//
CTransCommand *CTransCommand::CreateTransCommand(CString &strArgScript, COptionNode *pNode)
{
	CTransCommand * pCmd = NULL;

	// ���� ��� ����
	int cnt3 = pNode->GetChildCount();

	for(int k=1; k<cnt3; k++)
	{
		COptionNode* pNode3 = pNode->GetChild(k);
		CString strTransOption = pNode3->GetValue().MakeUpper();

		// �������
		if(strTransOption == _T("NOP"))
		{
			pCmd = new CTransCommandNOP();
			NOTIFY_DEBUG_MESSAGE(_T("CreateTransCommand: NOP created\n"));
			break;
		}
		else if(strTransOption == _T("PTRCHEAT"))
		{
			pCmd = new CTransCommandPTRCHEAT();

			// ����� �ɼ� �м�
			int cnt4 = pNode3->GetChildCount();
			for (int i=0; i<cnt4; i++)
			{
				COptionNode* pNode4 = pNode3->GetChild(i);
				pCmd->SetSpecialOption(pNode4->GetValue().MakeUpper(), pNode4);
			}
			NOTIFY_DEBUG_MESSAGE(_T("CreateTransCommand: PTRCHEAT created\n"));
			break;
		}
		else if(strTransOption == _T("OVERWRITE"))
		{
			pCmd = new CTransCommandOVERWRITE();
			NOTIFY_DEBUG_MESSAGE(_T("CreateTransCommand: OVERWRITE created\n"));

			// ����� �ɼ� �м�
			int cnt4 = pNode3->GetChildCount();
			for (int i=0; i<cnt4; i++)
			{
				COptionNode* pNode4 = pNode3->GetChild(i);
				pCmd->SetSpecialOption(pNode4->GetValue().MakeUpper(), pNode4);
			}
			break;
		}
		else if(strTransOption == _T("SOW"))
		{
			pCmd = new CTransCommandSOW();
			NOTIFY_DEBUG_MESSAGE(_T("CreateTransCommand: SOW created\n"));

			// ����� �ɼ� �м�
			int cnt4 = pNode3->GetChildCount();
			for (int i=0; i<cnt4; i++)
			{
				COptionNode* pNode4 = pNode3->GetChild(i);
				pCmd->SetSpecialOption(pNode4->GetValue().MakeUpper(), pNode4);
			}
			break;
		}
		else if(strTransOption == _T("SMSTR"))
		{
			pCmd = new CTransCommandSMSTR();
			NOTIFY_DEBUG_MESSAGE(_T("CreateTransCommand: SMSTR created\n"));

			// ����� �ɼ� �м�
			int cnt4 = pNode3->GetChildCount();
			for (int i=0; i<cnt4; i++)
			{
				COptionNode* pNode4 = pNode3->GetChild(i);
				pCmd->SetSpecialOption(pNode4->GetValue().MakeUpper(), pNode4);
			}
			break;
		}
		
	}

	if (!pCmd) 
	{
		pCmd = new CTransCommandNOP();
		NOTIFY_DEBUG_MESSAGE(_T("CreateTransCommand: (default) NOP created\n"));
	}

	pCmd->SetArgScript(strArgScript);

	// ���� �ɼǵ� ����
	for(int k=1; k<cnt3; k++)
	{
		COptionNode* pNode3 = pNode->GetChild(k);
		pCmd->SetTransOption(pNode3->GetValue().MakeUpper(), pNode3);
	}

	return pCmd;
}

//////////////////////////////////////////////////////////////////////////
// �ɼ��� �м�, �����մϴ�.
//
void CTransCommand::SetTransOption(CString strTransOption, COptionNode *pNode)
{
	// SCRTRANS �ɼ�
	if(strTransOption == _T("SCRTRANS"))
	{
		m_bTrans=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: SCRTRANS\n"));
	}

	// ��Ƽ����Ʈ / �����ڵ� ����
	else if(strTransOption == _T("ANSI"))
	{
		m_bUnicode=FALSE;
		m_bUnicode8=FALSE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: ANSI\n"));
	}
	else if(strTransOption == _T("UNICODE"))
	{
		m_bUnicode=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: UNICODE\n"));
	}
	else if(strTransOption == _T("UNICODE8"))
	{
		m_bUnicode=TRUE;
		m_bUnicode8=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: UNICODE8\n"));
	}

	// CLIPKOR �ɼ�
	else if(strTransOption == _T("CLIPKOR"))
	{
		m_bClipKor=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: CLIPKOR\n"));
	}

	// CLIPJPN �ɼ�
	else if(strTransOption == _T("CLIPJPN"))
	{
		m_bClipJpn=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: CLIPJPN\n"));
	}

	// REMOVESPACE �ɼ�
	else if(strTransOption == _T("REMOVESPACE"))
	{
		m_bRemoveSpace=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: REMOVESPACE\n"));
	}

	// TWOBYTE �ɼ�
	else if(strTransOption == _T("TWOBYTE"))
	{
		m_bTwoByte=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: TWOBYTE\n"));
	}

	// ONEBYTE �ɼ�
	else if(strTransOption == _T("ONEBYTE"))
	{
		m_bOneByte=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: ONEBYTE\n"));
	}

	// SAFE �ɼ�
	else if(strTransOption == _T("SAFE"))
	{
		m_bSafe=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: SAFE\n"));
	}

	// ADDNULL �ɼ�
	else if(strTransOption == _T("ADDNULL"))
	{
		m_bAddNull=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: ADDNULL\n"));
	}

	// LEN �ɼ�
	else if(strTransOption == _T("LEN"))
	{
		TCHAR *pszRegs[] = {_T("EAX"), _T("EBX"), _T("ECX"), _T("EDX"), _T("ESI"), _T("EDI"), _T("EBP"), _T("ESP"), NULL};
		CString strLenPos = pNode->GetChild(0)->GetValue().MakeUpper();

		int i=0;
		bool bIsExpression = false;
		while(pszRegs[i])
		{
			if (strLenPos.Find(pszRegs[i]) >= 0)
			{
				// �������� ������ ���� : ����
				bIsExpression = true;
				break;
			}
			i++;
		}

		if (bIsExpression)
		{
			m_strLenPos = pNode->GetChild(0)->GetValue();
			NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: LEN(%s)\n"), m_strLenPos);
		}
		else
		{
			// ������ �ƴ�
			bool bIsMinus=false;
			i=0;
/*
			if(strLenPos[0] == _T('-'))
			{
				bIsMinus = true;
				i++;
			}
*/			
			for(; i<strLenPos.GetLength(); i++)
			{
				// ���� ���������� ã�´�
				if ( strLenPos[i] != _T('+') && strLenPos[i] != _T('X') && strLenPos[i] != _T('0') )
					break;
			}
			
			_stscanf(strLenPos.Mid(i), _T("%x"), &m_nLenPos);

			NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: LEN(%d)\n"), m_nLenPos);
		}
		m_bAdjustLen = TRUE;
	}

	// LENEND �ɼ�
	else if(strTransOption == _T("LENEND"))
	{
		m_bLenEnd=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: LENEND\n"));
	}

	//KIRINAME �ɼ�
	else if(strTransOption == _T("KIRINAME"))
	{
		m_bKiriName=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: KIRINAME\n"));
	}

	//PTRBACKUP �ɼ�
	else if(strTransOption == _T("PTRBACKUP"))
	{
		m_bPtrBack=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: PTRBACKUP\n"));
	}

	//DUPLICATED �ɼ�
	else if(strTransOption == _T("DUPLICATED"))
	{
		m_bDuplicated=TRUE;
		NOTIFY_DEBUG_MESSAGE(_T("SetTransOption: DUPLICATED\n"));
	}
}

//////////////////////////////////////////////////////////////////////////
// ���� ������ ������ ��ġ�� ��ȯ�մϴ�.
//
void *CTransCommand::GetTextPoint(CTransScriptParser *pParser, void *pBackUp, BOOL bPtrBack, void *pBackUpLength)
{
	// ���� ������ ��ü�Ѱ� �����ϱ�
	if (bPtrBack)
	{
		// SOW �� ����
		RestoreBackup();

		if(BackUpTextPoint(pBackUp))
			// ���̰� ����
			if (m_bAdjustLen)
				BackUpLength(pBackUpLength);
	}

	if (pParser == NULL) return NULL;

	NOTIFY_DEBUG_MESSAGE(_T("GetTextPoint: called\n"));

	void * pArgText = NULL;

	// ArgScript ( [ESI] ����..) �� ����Ű�� ��ġ ���
	int nType;
	int* pRetVal = (int*)pParser->GetValue(GetArgScript(), &nType);
	if(pRetVal && 1 == nType)
	{
		pArgText = *(LPVOID *)pRetVal;
		delete pRetVal;
	}

	// ���� ��ġ�� ��ȿ�� �˻�
	if(pArgText == NULL || IsBadReadPtr(pArgText, sizeof(LPVOID))) 
	{
		return NULL;
	}

	if (m_bUnicode)
	{
		if (*(LPWSTR)pArgText == L'\0') 
		{
			return NULL;
		}

	}
	else // MBCS
	{
		if(*(LPSTR)pArgText == '\0') 
		{
			return NULL;
		}
	}

	m_pArgText=pArgText;
	NOTIFY_DEBUG_MESSAGE(_T("GetTextPoint: pArgText=%p\n"), pArgText);

	return pArgText;
}

//////////////////////////////////////////////////////////////////////////
// ���� ����� �����͸� �����մϴ�.
//
bool CTransCommand::BackUpTextPoint(void *pBackUp, int pMovSize)
{
	bool bTmp = false;
	if( *(&(*(DWORD*)pBackUp)+3) != 0 &&
		!IsBadReadPtr(((DWORD*)(*(&(*(DWORD*)pBackUp)))), pMovSize * 4) )
	{
		bTmp = true;
		if(*(&(*(DWORD*)pBackUp)+3) == 1)
		{
			if(*(&(*(DWORD*)(*(&(*(DWORD*)pBackUp))))+0) == *(&(*(DWORD*)pBackUp)+2))
				memmove((&(*(DWORD*)(*(&(*(DWORD*)pBackUp))))), (&(*(DWORD*)pBackUp)+4), pMovSize*4);
			else if(*(&(*(DWORD*)(*(&(*(DWORD*)pBackUp))))+0) == *(&(*(DWORD*)pBackUp)+1))
				memmove((&(*(DWORD*)(*(&(*(DWORD*)pBackUp))))), (&(*(DWORD*)pBackUp)+4), pMovSize*4);
		}
		else if(*(&(*(DWORD*)pBackUp)+3) == 2)
		{
			DWORD dwTest = 0, dwLen = *(&(*(DWORD*)pBackUp)+1) - 2;
			while(dwTest < dwLen){
				if(*(&(*(BYTE*)(*(&(*(DWORD*)pBackUp))))+dwTest)!=*(&(*(BYTE*)(*(&(*(DWORD*)pBackUp)+2)))+dwTest)){
					bTmp = false;
					break;
				}
				dwTest++;
			}
			if( bTmp )
				memmove((&(*(DWORD*)(*(&(*(DWORD*)pBackUp))))), (&(*(DWORD*)pBackUp)+4), *(&(*(DWORD*)pBackUp)+1));
		}
	}
	*(&(*(DWORD*)pBackUp)+1) = 0;
	*(&(*(DWORD*)pBackUp)+3) = 0;
	return bTmp;
}

//////////////////////////////////////////////////////////////////////////
// ����� �����͸� �޸𸮿� ����մϴ�.
//
void CTransCommand::SetUpTextPoint(void *pBackUp, void *pArgText, void *ppArgText, int pMovSize, int nType)
{
	if(pMovSize == 0)
	{
		*(&(*(DWORD*)pBackUp)+3) = 0;
		return;
	}
	if(nType == 1)
	{
		*(&(*(DWORD*)pBackUp)) = (DWORD)ppArgText;
		*(&(*(DWORD*)pBackUp)+1) = (DWORD)pArgText;
		*(&(*(DWORD*)pBackUp)+2) = (DWORD)m_pTransTextBuf;
		*(&(*(DWORD*)pBackUp)+3) = nType;
		memmove((&(*(DWORD*)pBackUp)+4), ppArgText, pMovSize*4);
	}
	else if(nType == 2)
	{
		*(&(*(DWORD*)pBackUp)) = (DWORD)ppArgText;
		*(&(*(DWORD*)pBackUp)+1) = pMovSize;
		*(&(*(DWORD*)pBackUp)+2) = (DWORD)m_pTransTextBuf;
		*(&(*(DWORD*)pBackUp)+3) = nType;
		memmove((&(*(DWORD*)pBackUp)+4), ppArgText, pMovSize);
	}
}

//////////////////////////////////////////////////////////////////////////
// �޾ƿ� ���ڿ��� ���� �� �ִ� ���ڿ����� Ȯ���մϴ�.
//
BOOL CTransCommand::IsValidTextPoint(void *pArgText, long nSize)
{
	BOOL bRet = TRUE;
	CCharacterMapper *pcCharMap = NULL;

	if (CATCodeMgr::GetInstance()->m_nEncodeKorean)
	{
		if (CATCodeMgr::GetInstance()->m_nEncodeKorean == 2)
			pcCharMap = new CCharacterMapper2;
		else
			pcCharMap = new CCharacterMapper;
	}

	NOTIFY_DEBUG_MESSAGE(_T("IsValidTextPoint: called, pArgText=%p, *pArgText=%08X\n"),pArgText, *(DWORD *) pArgText);

	if (m_bUnicode)
	{
		if( IsBadStringPtrW((LPCWSTR)pArgText, nSize) )
			bRet = FALSE;

	}
	else
	{
		if ( IsBadStringPtrA((LPCSTR)pArgText, nSize) )
			bRet = FALSE;

		// �߸��� �ؽ�Ʈ�ΰ�?
		if(m_bSafe && pcCharMap->IsShiftJISText((LPCSTR)pArgText) == FALSE)
		{
			delete pcCharMap;
			TRACE(_T("[aral1] ��Bad Text \n"));
			throw -4; //_T("�Ϻ��� �ؽ�Ʈ�� �ƴմϴ�.");
		}


	}
	
	NOTIFY_DEBUG_MESSAGE(_T("IsValidTextPoint: return %s\n"), (bRet?_T("TRUE"):_T("FALSE")) );
	if (pcCharMap) delete pcCharMap;
	return bRet;
}

//////////////////////////////////////////////////////////////////////////
// Ȥ�� ���� ���ڿ��� �ٽ� �����Ϸ����� Ȯ���մϴ�.
//
BOOL CTransCommand::IsDuplicateText(void *pArgText)
{
	NOTIFY_DEBUG_MESSAGE(_T("IsDuplicateText: called\n"));

	BOOL bRet=FALSE;
	if (m_bUnicode)
	{
		LPWSTR wszText = (LPWSTR) pArgText;

		// �ֱ� ������ �����ΰ�
		wchar_t wszPureText[MAX_TEXT_LENGTH];

		// UTF-8 -> UTF-16 ��ȯ
		if( m_bUnicode8 )
		{
			int len = MyMultiByteToWideChar(CP_UTF8, 0, (LPCSTR) pArgText, -1, NULL, 0);
			MyMultiByteToWideChar(CP_UTF8, 0, (LPCSTR) pArgText, -1, wszPureText, len);
		}
		else wcscpy(wszPureText, wszText);
		size_t wlen = wcslen(wszPureText);

		// ���� ���� ���� ����
		while(wlen>0 && wszPureText[wlen-1] == L' ')
		{
			wszPureText[wlen-1] = L'\0';
			wlen--;
		}

		// �ߺ� üũ
		if( wcslen((LPCWSTR)m_pTransTextBuf) > 0 && wcsstr((LPCWSTR)m_pTransTextBuf, wszPureText) )
		{
		//	throw -1; //_T("���� ��ġ�� �ߺ� �����Ϸ� �մϴ�.");
			bRet = TRUE;
		}
		// �ֱ� ó���� �Ϻ�� �ƴ� ���ο� �����̶��		
		else if( wcscmp(wszText, (LPCWSTR)m_pOrigTextBuf) )
			bRet = FALSE;	// ���� �ʿ�
		else
			bRet = TRUE;	// ���� ���ʿ�
	}
	else
	{
		LPSTR szText = (LPSTR) pArgText;

		// �ֱ� ������ �����ΰ�
		size_t nCmpCnt = strlen(szText);

		// ���� ���� ���� ����
		while(nCmpCnt > 0 && szText[nCmpCnt-1] == ' ') nCmpCnt--;

		// �ߺ� üũ
		if( strlen((LPCSTR)m_pTransTextBuf) > 0 && strncmp(szText, (LPCSTR)m_pTransTextBuf, nCmpCnt) == 0 )
		{
		//	throw -1; //_T("���� ��ġ�� �ߺ� �����Ϸ� �մϴ�.");
			bRet = TRUE;
		}
		// �ֱ� ó���� �Ϻ�� �ƴ� ���ο� �����̶��
		else if( strcmp(szText, (LPCSTR)m_pOrigTextBuf) )
			bRet = FALSE;	// ���� �ʿ�
		// ���� �Ϻ��� �����̶� �����Ͱ� ��ġ�� �ٸ��� (��: ���� �ܾ ������ �ٸ� ��ġ�� ���� ���)
		else if ( pArgText != m_pPrevArgText)
			bRet = FALSE;
		else
			bRet = TRUE;	// ���� ���ʿ�
	}

	NOTIFY_DEBUG_MESSAGE(_T("IsDuplicateText: return %s\n"), (bRet?_T("TRUE"):_T("FALSE")) );
	return bRet;
}

//////////////////////////////////////////////////////////////////////////
// ���ڿ� ���̸� ������ ���ڿ��� ���̷� �����մϴ�.
//
BOOL CTransCommand::ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp)
{
	if (!m_bAdjustLen) return FALSE;

	NOTIFY_DEBUG_MESSAGE(_T("ChangeLength: called\n"));

	// pNewBasePtr �� �����Ǹ� pOrigBasePtr �� ���� ������
	if (pNewBasePtr == NULL)
		pNewBasePtr = pOrigBasePtr;

	int *pnLen=NULL;

	// ���̰� ����� ��ġ�� ã�´�
	if (m_nLenPos)
	{
		// m_nLenPos �� �ִٸ� pOrigBasePtr���� �����ġ�� ���
		pnLen = (int *)((DWORD)pOrigBasePtr + m_nLenPos);
	}
	else if (!m_strLenPos.IsEmpty())
	{
		// m_strLenPos �� �ִٸ� �״�� �ļ��� �־ ���
		int nType;
		int *pRetVal = (int *)pParser->GetValue(m_strLenPos, &nType);

		if (pRetVal && 1 == nType)
		{
			pnLen = *(int **)pRetVal;
			delete pRetVal;

		}
		else
			return FALSE;
	}
	else
		return FALSE;

	if (IsBadReadPtr(pnLen, sizeof(int *)))
	{
		NOTIFY_DEBUG_MESSAGE(_T("ChangeLength: Cannot read address %p\n"), pnLen);
		return FALSE;
	}

	// ã�� ���̰� ��Ȯ���� Ȯ���ϰ� �� ����Ʈ¥�������� Ȯ���Ѵ�.
	int nOrigLen = (m_bUnicode ? lstrlenW((LPCWSTR)m_pOrigTextBuf) : lstrlenA((LPCSTR)m_pOrigTextBuf));
	int nTempLen = *pnLen;
	int nBytes;
	int nNullIncluded = -2;

	/*if (nOrigLen == nTempLen)
		nBytes=4;
	else if (nOrigLen == (nTempLen & 0x0FFFF))
		nBytes=2;
	else if (nOrigLen == (nTempLen & 0x0FF))
		nBytes=1;
	else
	{*/
	// NULL ���� ���Ե� �������� �ѹ� �� üũ
	for(nNullIncluded = -2; nNullIncluded <= 2; nNullIncluded++)
	{
		int nOffset = nOrigLen + nNullIncluded;
		if (nOffset == nTempLen)
		{
			nBytes=4;
			break;
		}
		else if (nOffset == (nTempLen & 0x0FFFF))
		{
			nBytes=2;
			break;
		}
		else if (nOffset == (nTempLen & 0x0FF))
		{
			nBytes=1;
			break;
		}
	}
	//}

	NOTIFY_DEBUG_MESSAGE(_T("ChangeLength: address(%p), bytes(%d), nOrigLen=%d, nTempLen=%d, nNullIncluded=%d\n"),
			pnLen, nBytes, nOrigLen, nTempLen, nNullIncluded);

	// ���� �������� ���̷� �ٲ�ִ´�.
	if (m_nLenPos)
		pnLen = (int *)((DWORD)pNewBasePtr + m_nLenPos + nNullIncluded);

	if (IsBadWritePtr(pnLen, sizeof(int)))
	{
		NOTIFY_DEBUG_MESSAGE(_T("ChangeLength: Cannot write address %p\n"), pnLen);
		return FALSE;
	}

	nTempLen = (m_bUnicode ? lstrlenW((LPCWSTR)m_pTransTextBuf) : lstrlenA((LPCSTR)m_pTransTextBuf));

	nTempLen += nNullIncluded;

	// ���̰� ���
	SetUpLength(pBackUp, pnLen, nBytes, nTempLen);

	NOTIFY_DEBUG_MESSAGE(_T("ChangeLength: success (address:%p, %d -> %d)\n"), pnLen, nOrigLen, nTempLen);

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// ���� ����� ���ڿ� ���̸� �����մϴ�.
//
void CTransCommand::BackUpLength(void *pBackUp)
{
	if( *(&(*(DWORD*)pBackUp)+3) != 0 &&
		!IsBadReadPtr(((DWORD*)(*(&(*(DWORD*)pBackUp)))), sizeof(DWORD *)) )
	{
		if(*(&(*(DWORD*)(*(&(*(DWORD*)pBackUp)))))==*(&(*(DWORD*)pBackUp)+2))
		{
			*(&(*(DWORD*)(*(&(*(DWORD*)pBackUp)))))=*(&(*(DWORD*)pBackUp)+1);
		}
	}
	*(&(*(DWORD*)pBackUp)+3) = 0;
}

//////////////////////////////////////////////////////////////////////////
// ���ڿ� ���̸� ������ ���� �޸𸮿� ����մϴ�.
//
void CTransCommand::SetUpLength(void *pBackUp, int *pnLen, int nBytes, int nTempLen)
{
	*(&(*(DWORD*)pBackUp)) = (DWORD)pnLen;
	*(&(*(DWORD*)pBackUp)+1) = *(&(*(DWORD*)pnLen));

	if (nBytes == 1)
		*(BYTE *)pnLen = nTempLen & 0x0FF;
	else if (nBytes == 2)
		*(WORD *)pnLen = nTempLen & 0x0FFFF;
	else
		*pnLen = nTempLen;

	*(&(*(DWORD*)pBackUp)+2) = *(&(*(DWORD*)pnLen));
	*(&(*(DWORD*)pBackUp)+3) = 1;
}

//////////////////////////////////////////////////////////////////////////
//
// PTRCHEAT ��� Ŭ���� �޼ҵ�
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ���ڿ� ���̸� ������ ���ڿ��� ���̷� �����մϴ�. (�������̵��)
//
BOOL CTransCommandPTRCHEAT::ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp)
{
	if (!m_bAdjustLen) return FALSE;

	NOTIFY_DEBUG_MESSAGE(_T("PTRCHEAT: ChangeLength: m_pTransTextBuf(%p)\n"), m_pTransTextBuf);
	return CTransCommand::ChangeLength(pOrigBasePtr, pParser, m_pTransTextBuf, pBackUp);

}


//////////////////////////////////////////////////////////////////////////
// ������ ���ڿ��� �� ���α׷��� �����մϴ�. (�������̵��)
//
BOOL CTransCommandPTRCHEAT::ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp)
{
	if (bIsDuplicated)
	{
		if (pArgText != m_pPrevArgText)
			return FALSE;

		if (pArgText == m_pTransTextBuf)
			return FALSE;
	}

	CString strScript = m_strArgScript;
	int nLen = strScript.GetLength();

	NOTIFY_DEBUG_MESSAGE(_T("PTRCHEAT: ApplyTranslatedText: called\n"));

#if defined(_DEBUG) || defined(FORCE_DEBUG_LOG)

	if (m_bUnicode)
	{
		NOTIFY_DEBUG_MESSAGE(_T("PTRCHEAT::ApplyTranslatedText: szSrc =%s\n"), (LPCWSTR)m_pTransTextBuf);
		NOTIFY_DEBUG_MESSAGE(_T("PTRCHEAT::ApplyTranslatedText: szDest =%s\n"), (LPCWSTR)m_pOrigTextBuf);
	}
	else
	{
		WCHAR wszTemp[MAX_TEXT_LENGTH];
		MyMultiByteToWideChar(949, 0, (LPCSTR)m_pTransTextBuf, -1, wszTemp, MAX_TEXT_LENGTH);
		NOTIFY_DEBUG_MESSAGE(_T("PTRCHEAT::ApplyTranslatedText: szSrc =%s\n"), wszTemp);
		MyMultiByteToWideChar(932, 0, (LPCSTR)m_pOrigTextBuf, -1, wszTemp, MAX_TEXT_LENGTH);
		NOTIFY_DEBUG_MESSAGE(_T("PTRCHEAT::ApplyTranslatedText: szDest=%s\n"), wszTemp);
	}
#endif

	if(strScript.CompareNoCase(_T("EAX")) == 0) pRegisters->_EAX = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("EBX")) == 0) pRegisters->_EBX = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("ECX")) == 0) pRegisters->_ECX = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("EDX")) == 0) pRegisters->_EDX = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("ESI")) == 0) pRegisters->_ESI = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("EDI")) == 0) pRegisters->_EDI = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("EBP")) == 0) pRegisters->_EBP = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(strScript.CompareNoCase(_T("ESP")) == 0) pRegisters->_ESP = (DWORD)(UINT_PTR)m_pTransTextBuf;
	else if(nLen >= 5 && _T('[') == strScript[0] && _T(']') == strScript[nLen-1])
	{
		int nType;
		int* pRetVal = (int*)pParser->GetValue(strScript.Mid(1, nLen-2), &nType);
		if(pRetVal && 1 == nType)
		{
			LPBYTE* ppArgText = *(LPBYTE**)pRetVal;
			delete pRetVal;

			if(::IsBadWritePtr(ppArgText, sizeof(LPBYTE)) == FALSE)
			{
				// �����͸� ��ü�ϱ��� �̸� ����صд�
				SetUpTextPoint(pBackUp, pArgText, ppArgText, 1);

				*ppArgText = m_pTransTextBuf;
			}

		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// OVERWRITE ��� Ŭ���� �޼ҵ�
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �ɼ��� �м�, �����մϴ�.
//
void CTransCommandOVERWRITE::SetTransOption(CString strTransOption, COptionNode *pNode)
{
	if (strTransOption == _T("ALLSAMETEXT"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("OVERWRITE: SetTransOption: ALLSAMETEXT\n"));
		m_bAllSameText=TRUE;
	}
	else
		CTransCommand::SetTransOption(strTransOption, pNode);	// global options
}

//////////////////////////////////////////////////////////////////////////
// Ư�� �ɼ��� �м�, �����մϴ�.
//
void CTransCommandOVERWRITE::SetSpecialOption(CString strSpecialOption, COptionNode *pNode)
{
	if(strSpecialOption == _T("IGNORE"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("OVERWRITE: SetSpecialOption: IGNORE\n"));
		m_bIgnoreBufLen=TRUE;
	}
	else
		CTransCommand::SetSpecialOption(strSpecialOption, pNode);
}

//////////////////////////////////////////////////////////////////////////
// �������� �� ���α׷��� �����ϴ�.
//
BOOL CTransCommandOVERWRITE::OverwriteTextBytes( LPVOID pDest, LPVOID pSrc, void *pBackUp )
{
	NOTIFY_DEBUG_MESSAGE(_T("OVERWRITE: OverwriteTextBytes: called\n"));

	if(m_bUnicode)
	{
		LPWSTR wszSrc = (LPWSTR)pSrc;
		UINT_PTR nSrcLen = wcslen(wszSrc);
		LPWSTR wszDest = (LPWSTR)pDest;
		UINT_PTR nDestLen = ( m_bIgnoreBufLen ? nSrcLen : wcslen(wszDest) );

		// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
		if( IsBadWritePtr(wszDest, nDestLen) || IsBadStringPtrW(wszDest, 1024*1024*1024) ) return FALSE;
		
		// ��� ����
		size_t len = min(nDestLen, nSrcLen);

		// �޸� ���
		SetUpTextPoint(pBackUp, pDest, wszDest, (nDestLen+1)*sizeof(wchar_t), 2);

		// �ؽ�Ʈ ����
		while(len<nDestLen)
		{
			wszSrc[len] = L' ';
			len++;
		}
		wszSrc[len+1] = 0;
		memcpy(wszDest, wszSrc, (nDestLen+1)*sizeof(wchar_t));
	}
	else
	{

		LPSTR szSrc = (LPSTR)pSrc;
		UINT_PTR nSrcLen = strlen(szSrc);
		LPSTR szDest = (LPSTR)pDest;
		UINT_PTR nDestLen = ( m_bIgnoreBufLen ? nSrcLen : strlen(szDest) );


#if defined(_DEBUG) || defined(FORCE_DEBUG_LOG)
		WCHAR wszTemp[MAX_TEXT_LENGTH];
		MyMultiByteToWideChar(949, 0, szSrc, -1, wszTemp, MAX_TEXT_LENGTH);
		NOTIFY_DEBUG_MESSAGE(_T("OVERWRITE: OverwriteTextBytes: szSrc =%s\n"), wszTemp);
		MyMultiByteToWideChar(932, 0, szDest, -1, wszTemp, MAX_TEXT_LENGTH);
		NOTIFY_DEBUG_MESSAGE(_T("OVERWRITE: OverwriteTextBytes: szDest=%s\n"), wszTemp);
#endif


		// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
		if( IsBadWritePtr(szDest, nDestLen) || IsBadStringPtrA(szDest, 1024*1024*1024) ) return FALSE;

		// ��� ����
		size_t len = 0;
		while(len<nDestLen && len<nSrcLen)
		{
			size_t addval = 1;
			if( (BYTE)0x80 <= (BYTE)szSrc[len] ) addval = 2;

			if( len + addval > nDestLen ) break;

			len += addval;
		}

		// �޸� ���
		SetUpTextPoint(pBackUp, pDest, szDest, nDestLen+1, 2);

		// �ؽ�Ʈ ����
		while(len<nDestLen)
		{
			szSrc[len] = ' ';
			len++;
		}
		memset(szDest, 0, (nDestLen+1));
		memcpy(szDest, szSrc, (nDestLen));
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ���� ���� ��� ������ ������ ã�� ���������� �����ϴ�.
// (ANSI ����)
//
BOOL CTransCommandOVERWRITE::SearchTextA(UINT_PTR ptrBegin, LPCSTR cszText, list<LPVOID>* pTextList)
{
	BOOL bRetVal = FALSE;

	size_t dist = 0;
	size_t nOrigLen = strlen(cszText);

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);

		// ��ġ�Ѵٸ�
		if( IsBadStringPtrA(*ppText, 1024*1024*1024)==FALSE 
			&& strlen(*ppText) == nOrigLen
			&& strcmp(*ppText, cszText) == 0 )
		{
			// ��Ͽ� �߰�
			pTextList->push_back(*ppText);
			bRetVal = TRUE;
		}

		dist += sizeof(void*);
	}

	TRACE(_T(" [ aral1 ] SearchTextA ã���Ÿ�:0x%p~0x%p (%d bytes) \n"), ptrBegin, ptrBegin+dist, dist);

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// ���� ���� ��� ������ ������ ã�� ���������� �����ϴ�.
// (UNICODE ����)
//
BOOL CTransCommandOVERWRITE::SearchTextW(UINT_PTR ptrBegin, LPCWSTR cwszText, list<LPVOID>* pTextList)
{
	BOOL bRetVal = FALSE;

	size_t dist = 0;
	size_t nOrigLen = wcslen(cwszText);

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);

		// ��ġ�Ѵٸ�
		if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE 
			&& wcslen(*ppText) == nOrigLen
			&& wcscmp(*ppText, cwszText) == 0 )
		{
			// ��Ͽ� �߰�
			pTextList->push_back(*ppText);
			bRetVal = TRUE;
		}

		dist += sizeof(void*);

	}	

	TRACE(_T(" [ aral1 ] SearchTextW ã���Ÿ�:0x%p~0x%p (%d bytes) \n"), ptrBegin, ptrBegin+dist, dist);

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
//  ������ ���ڿ��� �� ���α׷��� �����մϴ�. (�������̵��)
//
BOOL CTransCommandOVERWRITE::ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp)
{
	if (bIsDuplicated)
	{
		if (m_bUnicode || m_bUnicode8)
		{
			if (*(WCHAR *)m_pTransTextBuf == L'\0')
				return FALSE;
		}
		else
		{
			if (*(char *)m_pTransTextBuf == '\0')
				return FALSE;
		}
		// ���� ���ۿ� ������ ������ �״�� ����Ѵ�.
	}

	NOTIFY_DEBUG_MESSAGE(_T("OVERWRITE: ApplyTranslatedText: called\n"));

	list<LPVOID> listTexts;
	listTexts.push_back(pArgText);
	

	// �����ؾ��� ��ġ�� ����
	if(m_bAllSameText)
	{
		if(m_bUnicode)
		{
			SearchTextW((UINT_PTR)pRegisters->_EBP, (LPCWSTR)pArgText, &listTexts);
		}
		else
		{
			SearchTextA((UINT_PTR)pRegisters->_EBP, (LPCSTR)pArgText, &listTexts);
		}
	}


	// ��ġ�鿡 ������ �ؽ�Ʈ Overwrite
	for(list<LPVOID>::iterator iter = listTexts.begin();
		iter != listTexts.end();
		iter++)
	{
		LPVOID pDest = (*iter);
		OverwriteTextBytes(pDest, m_pTransTextBuf, pBackUp);
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// SOW ��� Ŭ���� �޼ҵ�
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// SOW ����� �޸𸮸� ����ϰ� �������� �����ϴ�
//
void CTransCommandSOW::DoBackupAndOverwrite(const PBYTE pBackupPoint, UINT nBackupSize)
{
	// ��� ������ �Ǿ����� ������ ����
	if (m_pBackupPoint)
		RestoreBackup();

	NOTIFY_DEBUG_MESSAGE(_T("SOW: DoBackupAndOverwrite: called\n"));

	m_pBackupBuffer = new BYTE [nBackupSize];
	m_nBackupSize = nBackupSize;
	m_pBackupPoint = pBackupPoint;

	// ���
	CopyMemory(m_pBackupBuffer, m_pBackupPoint, m_nBackupSize);

	if (m_nLenBytes)
	{
		int nLen = (m_bUnicode ? lstrlenW((LPCWSTR)m_pTransTextBuf) : lstrlenA((LPCSTR)m_pTransTextBuf));
		if (m_bIsNullIncluded)
			nLen++;

		try
		{
		// ���� ��ġ�� �����ġ�� ��ġ�� ���ϰ�
		if (!m_pnBackupLenPoint)
			m_pnBackupLenPoint = (int *)(pBackupPoint + m_nLenPos);

		NOTIFY_DEBUG_MESSAGE(_T("SOW: DoBackupAndOverwrite: m_pnBackupLenPoint=%p\n"), m_pnBackupLenPoint);

		if (IsBadReadPtr(m_pnBackupLenPoint, sizeof(int *)))
		{
			NOTIFY_DEBUG_MESSAGE(_T("SOW: DoBackupAndOverwrite: BadReadPtr m_pnBackupLenPoint\n"));
			throw;
		}

		if (IsBadWritePtr(m_pnBackupLenPoint, sizeof(int)))
		{
			NOTIFY_DEBUG_MESSAGE(_T("SOW: DoBackupAndOverwrite: BadWritePtr m_pnBackupLenPoint\n"));

		}

		// ���̰� �� ��ġ�� ������ ���
		m_nBackupLenPoint = *m_pnBackupLenPoint;

		// ���� �����
		if (m_nLenBytes == 1)
			*(BYTE *)m_pnBackupLenPoint = nLen & 0x0FF;
		else if (m_nLenBytes == 2)
			*(WORD *)m_pnBackupLenPoint = nLen & 0x0FFFF;
		else
			*m_pnBackupLenPoint = nLen;

		NOTIFY_DEBUG_MESSAGE(_T("SOW: DoBackupAndOverwrite: Len changed to %d\n"), nLen);

		// �۾� �ϷḦ ǥ���ϱ� ���� m_nLenBytes �� 0 ���� ����
		m_nLenBytes = 0;

		}
		catch (...)
		{
			m_pnBackupLenPoint = NULL;
			m_nLenBytes = 0;
			m_nBackupLenPoint = 0;
		}

	}

	// �����
	CopyMemory(m_pBackupPoint, m_pTransTextBuf, m_nBackupSize);

}

//////////////////////////////////////////////////////////////////////////
// SOW ����� ��� ����Ÿ�� �����մϴ�
//
void CTransCommandSOW::RestoreBackup()
{
	NOTIFY_DEBUG_MESSAGE(_T("SOW: RestoreBackup: called\n"));

	if (m_pBackupPoint)
	{
		// �� �޸𸮰� ���Ⱑ���� ���� ��� ����
		if (!IsBadWritePtr(m_pBackupPoint, m_nBackupSize))
		{
			bool bIsMismatch = false;
			UINT i;

			// �� �޸𸮰� ��� ���� �ٲ������ üũ
			for(i=0; i<m_nBackupSize; i++)
			{
				if (*(m_pBackupPoint+i) != *(m_pTransTextBuf+i))
				{
					bIsMismatch=true;
					break;
				}
			}

			// �ٲ��� �ʾ����� ����
			if (!bIsMismatch)
			{
				CopyMemory(m_pBackupPoint, m_pBackupBuffer, m_nBackupSize);

				// m_nLenBytes == 0 �ε� m_pnBackupLenPoint �� �����ϸ� ���ڱ��� ����
				if (m_nLenBytes == 0 && m_pnBackupLenPoint)
				{
					*m_pnBackupLenPoint = m_nBackupLenPoint;

					m_pnBackupLenPoint = NULL;
					m_nBackupLenPoint = 0;
					NOTIFY_DEBUG_MESSAGE(_T("SOW: RestoreBackup: Len restored\n"));
				}

			}
		}
		delete[] m_pBackupBuffer;
		m_pBackupPoint=NULL;

	}

}

//////////////////////////////////////////////////////////////////////////
// ���ڿ� ���̸� ������ ���ڿ��� ���̷� �����մϴ�. (�������̵��)
//
BOOL CTransCommandSOW::ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp)
{
	// SOW ��Ŀ����� ���� �����Ͱ� ����Ǵ� ����̱� ������ ���⼭ ������ ���� �ʰ�
	// ���ڿ� ���� Ȯ�θ� �� �� �� ���� ������ DoBackupAndOverwrite ���� �����Ѵ�.

	if (!m_bAdjustLen) return FALSE;

	NOTIFY_DEBUG_MESSAGE(_T("SOW: ChangeLength: called\n"));

	int *pnLen=NULL;

	// ���̰� ����� ��ġ�� ã�´�
	if (m_nLenPos)
	{
		// m_nLenPos �� �ִٸ� pOrigBasePtr���� �����ġ�� ���
		pnLen = (int *)((DWORD)pOrigBasePtr + m_nLenPos);
	}
	else if (!m_strLenPos.IsEmpty())
	{
		// m_strLenPos �� �ִٸ� �״�� �ļ��� �־ ���
		int nType;
		int *pRetVal = (int *)pParser->GetValue(m_strLenPos, &nType);

		if (pRetVal && 1 == nType)
		{
			pnLen = *(int **)pRetVal;
			delete pRetVal;

		}
		else
		{
			m_nLenBytes=0;
			return FALSE;
		}
	}
	else
	{
		m_nLenBytes=0;
		return FALSE;
	}

	if (IsBadReadPtr(pnLen, sizeof(int *)))
	{
		NOTIFY_DEBUG_MESSAGE(_T("ChangeLength: Cannot read address %p\n"), pnLen);
		m_nLenBytes=0;
		return FALSE;
	}

	// ã�� ���̰� ��Ȯ���� Ȯ���ϰ� �� ����Ʈ¥�������� Ȯ���Ѵ�.
	int nOrigLen = (m_bUnicode ? lstrlenW((LPCWSTR)m_pOrigTextBuf) : lstrlenA((LPCSTR)m_pOrigTextBuf));
	int nTempLen = *pnLen;
	int nBytes;
	bool bIsNullIncluded=false;

	if (nOrigLen == nTempLen)
		nBytes=4;
	else if (nOrigLen == (nTempLen & 0x0FFFF))
		nBytes=2;
	else if (nOrigLen == (nTempLen & 0x0FF))
		nBytes=1;
	else
	{
		// NULL ���� ���Ե� �������� �ѹ� �� üũ
		nOrigLen++;
		if (nOrigLen == nTempLen)
		{
			bIsNullIncluded=true;
			nBytes=4;
		}
		else if (nOrigLen == (nTempLen & 0x0FFFF))
		{
			bIsNullIncluded=true;
			nBytes=2;
		}
		else if (nOrigLen == (nTempLen & 0x0FF))
		{
			bIsNullIncluded=true;
			nBytes=1;
		}
		else
		{
			m_nLenBytes=0;
			return FALSE;
		}
	}

	// �ʿ��� ����Ÿ�� nBytes, bIsNullIncluded.
	m_nLenBytes = nBytes;
	m_bIsNullIncluded = bIsNullIncluded;

	// ���� LEN(����) �� ��� ���ڿ� ���� ��ġ�� �ʿ�.
	if (!m_strLenPos.IsEmpty())
		m_pnBackupLenPoint = pnLen;

	NOTIFY_DEBUG_MESSAGE(_T("SOW: ChangeLength: address(%p), bytes(%d), nOrigLen=%d, nTempLen=%d, bIsNullIncluded=%s\n"),
			pnLen, nBytes, nOrigLen, nTempLen, (bIsNullIncluded?_T("TRUE"):_T("FALSE")));

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//  ������ ���ڿ��� �� ���α׷��� �����մϴ�. (�������̵��)
//
BOOL CTransCommandSOW::ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp)
{
	if (bIsDuplicated)
	{
		if (m_bUnicode || m_bUnicode8)
		{
			if (*(WCHAR *)m_pTransTextBuf == L'\0')
				return FALSE;
		}
		else
		{
			if (*(char *)m_pTransTextBuf == '\0')
				return FALSE;
		}
		// ���� ���ۿ� ������ ������ �״�� ����Ѵ�.
	}

	NOTIFY_DEBUG_MESSAGE(_T("SOW: ApplyTranslatedText: called\n"));

	int nOrigSize=0, nTransSize=0, nDelta=0;

	CString strScript = m_strArgScript;
	int nLen = strScript.GetLength();

	LPBYTE *ppArgText=NULL;

#if defined(_DEBUG) || defined(FORCE_DEBUG_LOG)

	if (m_bUnicode)
	{
		NOTIFY_DEBUG_MESSAGE(_T("SOW::ApplyTranslatedText: szSrc =%s\n"), (LPCWSTR)m_pTransTextBuf);
		NOTIFY_DEBUG_MESSAGE(_T("SOW::ApplyTranslatedText: szDest =%s\n"), (LPCWSTR)m_pOrigTextBuf);
	}
	else
	{
		WCHAR wszTemp[MAX_TEXT_LENGTH];
		MyMultiByteToWideChar(949, 0, (LPCSTR)m_pTransTextBuf, -1, wszTemp, MAX_TEXT_LENGTH);
		NOTIFY_DEBUG_MESSAGE(_T("SOW::ApplyTranslatedText: szSrc =%s\n"), wszTemp);
		MyMultiByteToWideChar(932, 0, (LPCSTR)m_pOrigTextBuf, -1, wszTemp, MAX_TEXT_LENGTH);
		NOTIFY_DEBUG_MESSAGE(_T("SOW::ApplyTranslatedText: szDest=%s\n"), wszTemp);
	}
#endif

	// ���̸� ���ϰ�
	if (m_bUnicode)
	{
		nOrigSize=wcslen((LPWSTR) m_pOrigTextBuf) *2;
		nTransSize=wcslen((LPWSTR) m_pTransTextBuf) *2;
	}
	else
	{
		nOrigSize=strlen((LPSTR) m_pOrigTextBuf);
		nTransSize=strlen((LPSTR) m_pTransTextBuf);
	}

	// ��� �� �����
	nDelta = nOrigSize - nTransSize;

	NOTIFY_DEBUG_MESSAGE(_T("pArgText=%p, BackupPoint=%p\n"), pArgText, (LPBYTE)pArgText + nDelta);
	DoBackupAndOverwrite((LPBYTE)pArgText + nDelta, nTransSize);

	// ������ ��ġ ����
	if(strScript.Left(3).CompareNoCase(_T("EAX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EAX);
	else if(strScript.Left(3).CompareNoCase(_T("EBX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EBX);
	else if(strScript.Left(3).CompareNoCase(_T("ECX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_ECX);
	else if(strScript.Left(3).CompareNoCase(_T("EDX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EDX);
	else if(strScript.Left(3).CompareNoCase(_T("ESI")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_ESI);
	else if(strScript.Left(3).CompareNoCase(_T("EDI")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EDI);
	else if(strScript.Left(3).CompareNoCase(_T("EBP")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EBP);
	else if(strScript.Left(3).CompareNoCase(_T("ESP")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_ESP);
	else if(nLen >= 5 && _T('[') == strScript[0])
	{
		int nType;
		int nBracketCount=0;
		int i;
		int *pRetVal;

		for(i=0; i < nLen; i++)
		{
			if (strScript[i] == _T('['))
				nBracketCount++;
			else if (strScript[i] == _T(']'))
				nBracketCount--;

			if (nBracketCount == 0)
			{
				i++;
				break;
			}
		}

		pRetVal = (int*)pParser->GetValue(strScript.Mid(1, i-2), &nType);
		if(pRetVal && 1 == nType)
		{
			ppArgText = *(LPBYTE**)pRetVal;
			delete pRetVal;
		}
	}

	if (ppArgText && !IsBadWritePtr(ppArgText, sizeof(LPBYTE)))
	{
		SetUpTextPoint(pBackUp, (LPBYTE)pArgText + nDelta, ppArgText, 1);
		*ppArgText += nDelta;
	}
	else
		return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// SMSTR ��� Ŭ���� �޼ҵ�
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ư�� �ɼ��� �м�, �����մϴ�.
//
void CTransCommandSMSTR::SetSpecialOption(CString strSpecialOption, COptionNode *pNode)
{
	if(strSpecialOption == _T("IGNORE"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: IGNORE\n"));
		m_bIgnoreBufLen=TRUE;
	}
	else if(strSpecialOption == _T("MATCH"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: MATCH\n"));
		m_bMatchSize=TRUE;
	}
	else if(strSpecialOption == _T("FORCEPTR"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: FORCEPTR\n"));
		m_bForcePtr=TRUE;
	}
	else if(strSpecialOption == _T("NULLCHAR"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: NULLCHAR\n"));
		m_bNullchar=TRUE;
	}
	else if(strSpecialOption == _T("REVERSE"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: REVERSE\n"));
		m_bReverse=TRUE;
	}
	else if(strSpecialOption == _T("YURIS"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: YURIS\n"));
		m_YurisOffset = 0;
		m_bYuris=TRUE;
		m_bYurisP=TRUE;
	}
	else if(strSpecialOption == _T("WILL"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: WILL\n"));
		m_bWill=TRUE;
	}
	else if(strSpecialOption == _T("ENDPOS"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: ENDPOS\n"));
		m_bEndPos=TRUE;
	}
	else if(strSpecialOption == _T("TJSSTR"))
	{
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: SetSpecialOption: TJSSTR\n"));
		m_bTjsStr=TRUE;
	}
	else
		CTransCommand::SetSpecialOption(strSpecialOption, pNode);
}

//////////////////////////////////////////////////////////////////////////
// ���� ������ ������ ��ġ�� ��ȯ�մϴ�. (�������̵��)
//
void *CTransCommandSMSTR::GetTextPoint(CTransScriptParser *pParser, void *pBackUp, BOOL bPtrBack, void *pBackUpLength)
{
	// ���� ������ ��ü�Ѱ� �����ϱ�
	if (bPtrBack)
	{
		if(BackUpTextPoint(pBackUp, m_nBackUpSize))
			// ���̰� ����
			if (m_bAdjustLen)
				BackUpLength(pBackUpLength);
	}

	if (pParser == NULL) return NULL;

	NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: called\n"));

	void * pArgText = NULL;

	// ArgScript ( [ESI] ����..) �� ����Ű�� ��ġ ���
	int nType;
	int* pRetVal = (int*)pParser->GetValue(GetArgScript(), &nType);
	if(pRetVal && 1 == nType)
	{
		pArgText = *(LPVOID *)pRetVal;
		delete pRetVal;
	}

	// ���� ��ġ�� ��ȿ�� �˻� (SMSTR�� ����Ÿ ũ��� 6 DWORD (24 ����Ʈ))
	if(pArgText == NULL || IsBadReadPtr(pArgText, sizeof(DWORD) * 6)) return NULL;

	m_pArgText=pArgText;

	NOTIFY_DEBUG_MESSAGE(_T("  m_pArgText=%p\n "), m_pArgText);
	NOTIFY_DEBUG_MESSAGE(_T("  %08X %08X %08X %08X %08X %08X\n"), 
		*(&(*(DWORD*)pArgText)+0), *(&(*(DWORD*)pArgText)+1), *(&(*(DWORD*)pArgText)+2),
		*(&(*(DWORD*)pArgText)+3), *(&(*(DWORD*)pArgText)+4), *(&(*(DWORD*)pArgText)+5));

	if (m_bWill)
	{
		// ������� ����
		DWORD dwLen = *(&(*(DWORD*)pArgText));

		if(dwLen == 0)
		{
			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
			return NULL;
		}

		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: Will Plus\n"));

		// ���ڼ��� ���������̸� ����
		if(dwLen > 4096) return NULL;

		// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
		if ( IsBadReadPtr(pArgText, sizeof(CHAR) * 0x100) )
			return NULL;

		
		m_nBackUpSize = 0;

		DWORD dwTest=0;
		DWORD dwDest=0;
		BOOL bwOne=0;

		if (m_bUnicode)
		{
			LPWSTR wszText = NULL;
			int wPos = 0;

			wszText = (LPWSTR)pArgText + 4;
			dwTest = *(&(*(BYTE*)pArgText)+4);

			// ���� ������ �����ڵ� ���·� ������ ������
			while(dwLen > 1)
			{
				int i = 0;
				dwDest = *(&(*(BYTE*)pArgText)+8 + dwTest);
				if((dwTest == *(&(*(BYTE*)pArgText)+4)) && bwOne == 0)
				{
					if( ((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x69)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x6b)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x6d)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x6f)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x71)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x73)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x75)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x77)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x79)) )
					{
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4)) = 0x9F;
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 1) = 0;
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 2) = 0xFF;
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 3) = 0;
						wPos += 2;
					}
					else
					{
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4)) = 0x0A;
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 1) = 0;
					}
					wPos += 2;
					bwOne = 1;
				}
				while(dwDest > (DWORD)i){
					*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos) = *(&(*(BYTE*)pArgText)+8 + dwTest + 4 + i);
					i ++;
					wPos ++;
				}
				*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos) = 0x0A;
				*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos + 1) = 0;
				wPos += 2;
				dwTest += dwDest + 4;
				dwLen --;
			}
			wPos -= 2;
			*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos) = 0;
			*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos + 1) = 0;
			return wszText;
		}
		else
		{
			LPSTR szText = NULL;
			int wPos = 0;

			szText = (LPSTR)pArgText + 8;
			dwTest = *(&(*(BYTE*)pArgText)+4);

			// ���� ������ �����ڵ� ���·� ������ ������
			while(dwLen > 1)
			{
				int i = 0;
				dwDest = *(&(*(BYTE*)pArgText)+8 + dwTest);
				if((dwTest == *(&(*(BYTE*)pArgText)+4)) && bwOne == 0)
				{
					if( ((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x69)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x6b)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x6d)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x6f)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x71)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x73)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x75)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x77)) ||
						((*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 4 ) == 0x81)  &&
						 (*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 5 ) == 0x79)) )
					{
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4)) = 0x9F;
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + 1) = 0xFF;
						wPos ++;
					}
					else
					{
						*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4)) = 0x0A;
					}
					wPos ++;
					bwOne = 1;
				}
				while(dwDest > (DWORD)i){
					*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos) = *(&(*(BYTE*)pArgText)+8 + dwTest + 4 + i);
					i ++;
					wPos ++;
				}
				*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos) = 0x0A;
				wPos ++;
				dwTest += dwDest + 4;
				dwLen --;
			}
			wPos --;
			*(&(*(BYTE*)pArgText)+8 + *(&(*(BYTE*)pArgText)+4) + wPos) = 0;
			return szText;
		}
	}

	if (m_bEndPos)
	{
		// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
		if (*(DWORD *)pArgText == NULL) return NULL;
		if (*(&(*(DWORD*)pArgText)) == NULL) return NULL;
		if (*(&(*(DWORD*)pArgText)+1) == NULL) return NULL;

		DWORD dwLen = *(&(*(DWORD*)pArgText)+1) - *(&(*(DWORD*)pArgText));

		if(dwLen == 0)
		{
			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
			return NULL;
		}

		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: EndPos\n"));

		// ���ڼ��� ���������̸� ����
		if(dwLen > 4096) return NULL;

		// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
		if ( IsBadReadPtr((WCHAR *)(*(&(*(DWORD*)pArgText))), sizeof(WCHAR) * (dwLen + 1)) )
			return NULL;

		DWORD dwTest=0;

		if (m_bUnicode)
		{
			// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
			while(dwTest < dwLen){
				if(*(&(*(WCHAR*)(*(&(*(DWORD*)pArgText))))+dwTest)==L'\0'){
					return NULL;
				}
				dwTest++;
			}
		}
		else
		{
			// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
			while(dwTest < dwLen){
				if(*(&(*(CHAR*)(*(&(*(DWORD*)pArgText))))+dwTest)=='\0'){
					return NULL;
				}
				dwTest++;
			}
		}
		m_nBackUpSize = 2;
		return pArgText;
	}

	if (m_bTjsStr)
	{
		// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
		char chChk = *(&(*(BYTE*)pArgText));
		if(!(
			/*
			chChk == 0x0 || // ��ɾ�
			chChk == 0x1 || // ���� �̸�, ��ɾ�
			chChk == 0x3 || // �ε��ɾ�
			*/
			chChk == 0x2 || // ��罺ũ��, �ε�Ÿ�ӽ�ũ��
			chChk == 0x4 || // ��ε彺ũ��, ���Ǳ׽�ũ��, �ε彺ũ��
			chChk == 0x5 || // ���̺��ɾ�, ���Ǳ�Ÿ���ν�ũ��, ��ư ���� ��ũ��, ��α� �̸�
			chChk == 0x6 || // ���Ǳ� ���� ��ũ��
			chChk == 0x7 || // ?? ��ɾ� �̸�
			chChk == 0x8 || // ���â��ɾ�, ����̸�
			chChk == 0x9 || // ��������� �Ʒ� ��ũ��
			chChk == 0xA || // Ư������, ������
			chChk == 0xB || // �׸��� �̸�
			chChk == 0xC || // ��罺ũ��2
			chChk == 0xD || // �����ܾ�
			chChk == 0x13|| // �����ܾ�
			chChk == 0x16|| // �����ܾ�
			chChk == 0x17|| // �����ܾ�
			chChk == 0x18|| // �����ܾ�
			chChk == 0x19|| // �����ܾ�
			chChk == 0x1D|| // �����ܾ�
			chChk == 0x1E|| // �����ܾ�
			chChk == 0x23)) // ������ũ��
			return NULL;

		DWORD dwLen = *(&(*(DWORD*)pArgText)+13);

		if(dwLen == 0)
		{
			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
			return NULL;
		}

		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: KiriKiri TJSString\n"));

		// ���ڼ��� ���������̸� ����
		if(dwLen > 4096) return NULL;

		// 5 ���� �ѱ����ϰ�� ����
		if(chChk == 0x5 && dwLen <= 2) return NULL;

		// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
		if ( IsBadReadPtr(pArgText, sizeof(WCHAR) * 13) )
			return NULL;

		DWORD dwTest=0;

		if (m_bUnicode)
		{
			LPWSTR wszText = NULL;

			if (*(&(*(DWORD*)pArgText)+1) == NULL)
				wszText = (LPWSTR)pArgText + 4;
			else
				wszText = (LPWSTR)*(&(*(DWORD*)pArgText)+1);

			// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
			while(dwTest < dwLen){
				if(*(&(*(WCHAR*)wszText)+dwTest)==L'\0'){
					return NULL;
				}
				dwTest++;
			}
			m_nBackUpSize = 13;
			return wszText;
		}
		else
		{
			LPSTR szText = NULL;

			if (*(&(*(DWORD*)pArgText)+1) == NULL)
				szText = (LPSTR)pArgText + 8;
			else
				szText = (LPSTR)*(&(*(DWORD*)pArgText)+1);

			// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
			while(dwTest < dwLen){
				if(*(&(*(CHAR*)szText)+dwTest)=='\0'){
					return NULL;
				}
				dwTest++;
			}
			m_nBackUpSize = 13;
			return szText;
		}
	}

	if (m_bYuris)
	{
		if (m_bUnicode)
		{
			LPWSTR wszText = NULL;
			DWORD dwLen = 0;
			int nCount = 2;

			//������ ���� ��ġ Offset�� ã��
			if(m_YurisOffset==0/*&&m_bYurisP==TRUE*/)
			{
				while(nCount<20){
					if(*(&(*(DWORD*)pArgText)+nCount) == 0 && *(&(*(DWORD*)pArgText)+nCount+1) == 0)
					{
						m_YurisOffset = nCount - 2;
						dwLen = *(&(*(DWORD*)pArgText)+m_YurisOffset);
						m_bYurisP=TRUE;
						break;
					}
					nCount++;
				}
				if(nCount>=20) 
				{
					m_bYurisP=FALSE;
					m_YurisOffset=0;
					return NULL;
				}
			}
			/*else if(m_bYurisP==TRUE)
			{
				dwLen = *(&(*(DWORD*)pArgText)+m_YurisOffset);
			}*/
			else dwLen = *(&(*(DWORD*)pArgText)+m_YurisOffset); //return NULL;

			if(dwLen == 0)
			{
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
				return NULL;
			}

			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: Unicode Yuris\n"));
			// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
			if (*(DWORD *)pArgText == NULL) return NULL;

			// ���ڼ��� ���������̸� ����
			if(dwLen > 4096) return NULL;

			// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
			if ( IsBadReadPtr((WCHAR *)(*(&(*(DWORD*)pArgText))), sizeof(WCHAR) * (dwLen + 1)) )
				return NULL;

			DWORD dwTest=0;

			// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
			while(dwTest < dwLen){
				if(*(&(*(WCHAR*)(*(&(*(DWORD*)pArgText))))+dwTest)==L'\0'){
					return NULL;
				}
				dwTest++;
			}

			// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
			if(*(&(*(WCHAR*)(*(&(*(DWORD*)pArgText))))+dwLen)!=L'\0'){
				return NULL;
			}
		
			// ���� �ؽ�Ʈ ��ġ�� pArgText �� ����Ű�� �޸�
			wszText = (LPWSTR)*(&(*(DWORD*)pArgText));

			m_nBackUpSize = m_YurisOffset + 2;
			return wszText;
		}
		else // MBCS
		{
			LPSTR szText = NULL;
			DWORD dwLen = 0;
			int nCount = 2;

			//������ ���� ��ġ Offset�� ã��
			if(m_YurisOffset==0/*&&m_bYurisP==TRUE*/)
			{
				while(nCount<20){
					if(*(&(*(DWORD*)pArgText)+nCount) == 0 && *(&(*(DWORD*)pArgText)+nCount+1) == 0)
					{
						m_YurisOffset = nCount - 2;
						dwLen = *(&(*(DWORD*)pArgText)+m_YurisOffset);
						m_bYurisP=TRUE;
						break;
					}
					nCount++;
				}
				if(nCount>=20) 
				{
					m_bYurisP=FALSE;
					m_YurisOffset=0;
					return NULL;
				}
			}
			/*else if(m_bYurisP==TRUE)
			{
				dwLen = *(&(*(DWORD*)pArgText)+m_YurisOffset);
			}*/
			else dwLen = *(&(*(DWORD*)pArgText)+m_YurisOffset); //return NULL;

			if(dwLen == 0)
			{
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
				return NULL;
			}

			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: ANSI Yuris\n"));
			// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
			if (*(DWORD *)pArgText == NULL) return NULL;

			// ���ڼ��� ���������̸� ����
			if(dwLen > 4096) return FALSE;

			// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
			if ( IsBadReadPtr((CHAR *)(*(&(*(DWORD*)pArgText))), sizeof(CHAR) * (dwLen + 1)) )
				return NULL;

			DWORD dwTest=0;
		
			// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
			while(dwTest < dwLen){
				if(*(&(*(CHAR*)(*(&(*(DWORD*)pArgText))))+dwTest)=='\0'){
					return NULL;
				}
				dwTest++;
			}

			// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
			if(*(&(*(CHAR*)(*(&(*(DWORD*)pArgText))))+dwLen)!='\0'){
				return NULL;
			}
		
			// ���� �ؽ�Ʈ ��ġ�� pArgText �� ����Ű�� �޸�
			szText = (LPSTR)*(&(*(DWORD*)pArgText));

			m_nBackUpSize = m_YurisOffset + 2;
			return szText;
		}
	}
	else
	{
		m_nBackUpSize = 6;

		if (m_bUnicode)
		{
			LPWSTR wszText = NULL;
			DWORD dwLen = *(&(*(DWORD*)pArgText)+4);

			if(dwLen == 0)
			{
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
				return NULL;
			}
			else if (m_bNullchar)
			{
				if((dwLen >= 0x04) || m_bForcePtr)	// ���ڼ� 4�� (16����Ʈ) �̻��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: UnicodeX2 Long\n"));
					// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
					if (*(DWORD *)pArgText == NULL) return NULL;

					// ���ڼ��� ���������̸� ����
					if(dwLen > 4096) return NULL;

					// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
					if ( IsBadReadPtr((WCHAR *)(*(&(*(DWORD*)pArgText))), sizeof(DWORD) * (dwLen + 1)) )
						return NULL;

					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if(*(&(*(DWORD*)(*(&(*(DWORD*)pArgText))))+dwTest)==NULL){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(DWORD*)(*(&(*(DWORD*)pArgText))))+dwLen)!=NULL){
						return NULL;
					}

					dwTest=0;

					// ���� ������ �����ڵ� ���·� ������ ������
					while(dwTest <= dwLen){
						*(&(*(WORD*)(*(&(*(DWORD*)pArgText))))+dwTest) = (WORD)*(&(*(DWORD*)(*(&(*(DWORD*)pArgText))))+dwTest);
						dwTest++;
					}
				
					// ���� �ؽ�Ʈ ��ġ�� pArgText �� ����Ű�� �޸�
					wszText = (LPWSTR)*(&(*(DWORD*)pArgText));		
				}
				else	// ���ڼ��� 4�� (16����Ʈ) �̸��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: Unicode Short\n"));
					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if((*(&(*(DWORD*)pArgText)+dwTest))==NULL){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(DWORD*)pArgText)+dwLen)!=NULL){
						return NULL;
					}

					dwTest=0;

					// ���� ������ �����ڵ� ���·� ������ ������
					while(dwTest <= dwLen){
						*(&(*(WORD*)pArgText)+dwTest) = (WORD)*(&(*(DWORD*)pArgText)+dwTest);
						dwTest++;
					}
				
					// ���� ���ڴ� pArgText���� ����ִ�
					wszText = (LPWSTR)pArgText;
				}
			}
			else
			{
				if((dwLen >= 0x08) || m_bForcePtr)	// ���ڼ� 8�� (16����Ʈ) �̻��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: Unicode Long\n"));
					// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
					if (*(DWORD *)pArgText == NULL) return NULL;

					// ���ڼ��� ���������̸� ����
					if(dwLen > 4096) return NULL;

					// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
					if ( IsBadReadPtr((WCHAR *)(*(&(*(DWORD*)pArgText))), sizeof(WCHAR) * (dwLen + 1)) )
						return NULL;

					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if(*(&(*(WCHAR*)(*(&(*(DWORD*)pArgText))))+dwTest)==L'\0'){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(WCHAR*)(*(&(*(DWORD*)pArgText))))+dwLen)!=L'\0'){
						return NULL;
					}
				
					// ���� �ؽ�Ʈ ��ġ�� pArgText �� ����Ű�� �޸�
					wszText = (LPWSTR)*(&(*(DWORD*)pArgText));		
				}
				else	// ���ڼ��� 8�� (16����Ʈ) �̸��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: Unicode Short\n"));
					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if((*(&(*(WCHAR*)pArgText)+dwTest))==L'\0'){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(WCHAR*)pArgText)+dwLen)!=L'\0'){
						return NULL;
					}
				
					// ���� ���ڴ� pArgText���� ����ִ�
					wszText = (LPWSTR)pArgText;
				}
			}

			// �����ڵ� ������ (ex. 0xXX 0xYY) ��ġ�� ���� (ex. 0xYY 0xXX)
			if(m_bReverse)
			{
				BYTE dwTest=0;
				BYTE wszBuf;
				while(dwTest < dwLen){
					wszBuf = *(&(*(BYTE*)wszText)+dwTest);
					*(&(*(BYTE*)wszText)+dwTest) = *(&(*(BYTE*)wszText)+dwTest+1);
					*(&(*(BYTE*)wszText)+dwTest+1) = wszBuf;
					dwTest+= 2;
				}
			}

			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: %p\n"), wszText);
			return wszText;

			/*&if (pCmd->GetSmbuf() &&  pCmd->GetTransMethod() == TRANSCOMMAND_OVERWRITE ){
				if(*(&(*(DWORD*)pArgText)+4) >= 0x08) continue;
			}*/

		}
		else // MBCS
		{
			LPSTR szText = NULL;
			DWORD dwLen = *(&(*(DWORD*)pArgText)+4);

			if(dwLen == 0)
			{
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: dwLen=0\n"));
				return NULL;
			}
			else if (m_bNullchar)
			{
				if((dwLen >= 0x4) || m_bForcePtr)	// ���ڼ� 4�� (16����Ʈ) �̻��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: ANSI Long\n"));
					// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
					if (*(DWORD *)pArgText == NULL) return NULL;

					// ���ڼ��� ���������̸� ����
					if(dwLen > 4096) return FALSE;

					// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
					if ( IsBadReadPtr((CHAR *)(*(&(*(DWORD*)pArgText))), sizeof(DWORD) * (dwLen + 1)) )
						return NULL;

					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if(*(&(*(DWORD*)(*(&(*(DWORD*)pArgText))))+dwTest)==NULL){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(DWORD*)(*(&(*(DWORD*)pArgText))))+dwLen)!=NULL){
						return NULL;
					}

					dwTest=0;

					// ���� ������ �ƽ�Ű ���·� ������ ������
					while(dwTest <= dwLen){
						*(&(*(WORD*)(*(&(*(DWORD*)pArgText))))+dwTest) = (WORD)*(&(*(DWORD*)(*(&(*(DWORD*)pArgText))))+dwTest);
						dwTest++;
					}
				
					// ���� �ؽ�Ʈ ��ġ�� pArgText �� ����Ű�� �޸�
					szText = (LPSTR)*(&(*(DWORD*)pArgText));
				}
				else 	// ���ڼ��� 4�� (16����Ʈ) �̸��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: ANSI Short\n"));
					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if((*(&(*(DWORD*)pArgText)+dwTest))==NULL){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(DWORD*)pArgText)+dwLen)!=NULL){
						return NULL;
					}

					dwTest=0;

					// ���� ������ �ƽ�Ű ���·� ������ ������
					while(dwTest <= dwLen){
						*(&(*(WORD*)pArgText)+dwTest) = (WORD)*(&(*(DWORD*)pArgText)+dwTest);
						dwTest++;
					}
				
					// ���� ���ڴ� pArgText���� ����ִ�
					szText = (LPSTR)pArgText;
				}
			}
			else
			{
				if((dwLen >= 0x10) || m_bForcePtr)	// ���ڼ� 16�� (16����Ʈ) �̻��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: ANSI Long\n"));
					// pArgText �� ����Ű�� �����Ͱ� �������̸� ����
					if (*(DWORD *)pArgText == NULL) return NULL;

					// ���ڼ��� ���������̸� ����
					if(dwLen > 4096) return FALSE;

					// pArgText �� ����Ű�� ���۰� �б�Ұ��� ����
					if ( IsBadReadPtr((CHAR *)(*(&(*(DWORD*)pArgText))), sizeof(CHAR) * (dwLen + 1)) )
						return NULL;

					DWORD dwTest=0;
				
					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if(*(&(*(CHAR*)(*(&(*(DWORD*)pArgText))))+dwTest)=='\0'){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(CHAR*)(*(&(*(DWORD*)pArgText))))+dwLen)!='\0'){
						return NULL;
					}
				
					// ���� �ؽ�Ʈ ��ġ�� pArgText �� ����Ű�� �޸�
					szText = (LPSTR)*(&(*(DWORD*)pArgText));
				}
				else 	// ���ڼ��� 8�� (16����Ʈ) �̸��϶�
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: ANSI Short\n"));
					DWORD dwTest=0;

					// ���� ���ڼ��� ����� ���ڼ��� ��Ȯ���� üũ
					while(dwTest < dwLen){
						if((*(&(*(CHAR*)pArgText)+dwTest))=='\0'){
							return NULL;
						}
						dwTest++;
					}

					// ���ڼ� �̻����� �ٸ� ����Ÿ�� ���ִ��� üũ
					if(*(&(*(CHAR*)pArgText)+dwLen)!='\0'){
						return NULL;
					}
				
					// ���� ���ڴ� pArgText���� ����ִ�
					szText = (LPSTR)pArgText;
				}
			}

			/*if (pCmd->GetSmbuf() &&  pCmd->GetTransMethod() == TRANSCOMMAND_OVERWRITE ){
				if(*(&(*(DWORD*)pArgText)+4) >= 0x10) continue;
			}*/

			// �ƽ�Ű ������ (ex. 0xXX 0xYY) ��ġ�� ���� (ex. 0xYY 0xXX)
			// �ƽ�Ű�϶��� 1����Ʈ ���ھ���, ������ ��ü���� ���ſ�� (1����Ʈ ����)
			if(m_bReverse)
			{
				BYTE dsTest=0;
				BYTE szBuf;
				BYTE dwSub=0;

				while(dsTest < dwLen){
					szBuf = *(&(*(BYTE*)szText)+(dsTest * 2));
					if(*(&(*(BYTE*)szText)+(dsTest * 2)+1) == 0x00)
						dwSub++;
					else
						*(&(*(BYTE*)szText)+(dsTest * 2)-dwSub) = *(&(*(BYTE*)szText)+(dsTest * 2)+1);
					*(&(*(BYTE*)szText)+(dsTest * 2)+1-dwSub) = szBuf;
					
					dsTest++;
				}
				*(&(*(BYTE*)szText)+(dsTest * 2)-dwSub) = 0x00;
			}

			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: GetTextPoint: %p\n"), szText);
			return szText;

		}
	}
}
//////////////////////////////////////////////////////////////////////////
// �������� ���̿� ���� ����Ʈ ���ڿ��� �����ŵ�ϴ�.
//
BOOL CTransCommandSMSTR::ApplySmartString( LPVOID pDest, LPVOID pSrc, void *pBackUp )
{
	NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: called\n"));

	void *pMain = m_pArgText;

	DWORD sLen;
	BOOL bWasLongMode;

	if (m_bWill)
	{
		// �����͸� ��ü�ϱ��� �̸� ����صд�
		SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);

		if(m_bUnicode)
		{
			LPWSTR wszSrc = (LPWSTR)pSrc;
			UINT_PTR nSrcLen =  wcslen(wszSrc);
			LPWSTR wszDest = (LPWSTR)pDest;
			UINT_PTR nDestLen = wcslen(wszDest);

			BYTE dwTest=0;
			WORD wszBuf;
			BYTE dwSub=0;
			BYTE dwLen=1;

			memmove(wszSrc, wszSrc+2, nSrcLen);
			*(&(*(DWORD*)wszSrc)) = 0;
			nSrcLen += 2;
			
			while(dwTest < nSrcLen)
			{
				wszBuf = *(&(*(WORD*)wszSrc)+dwTest);
				if(wszBuf == 0x0A)
				{
					memmove(wszSrc+dwTest+1, wszSrc+dwTest, nSrcLen-dwTest);
					*(&(*(WORD*)wszSrc)+dwSub) = dwTest - 2 - dwSub;
					*(&(*(WORD*)wszSrc)+dwSub+1) = 0;
					*(&(*(WORD*)wszSrc)+dwTest) = 0;
					*(&(*(WORD*)wszSrc)+dwTest+1) = 0;
					dwSub = dwTest;
					nSrcLen += 2;
					dwTest ++;
					dwLen ++;
				}
				else if((*(&(*(WORD*)wszSrc)+dwTest+1) == 0xFF) && wszBuf == 0x9F)
				{
					memmove(wszSrc+dwTest, wszSrc+dwTest+1, nSrcLen-dwTest);
					*(&(*(WORD*)wszSrc)+dwSub) = dwTest - 2 - dwSub;
					*(&(*(WORD*)wszSrc)+dwSub+1) = 0;
					*(&(*(WORD*)wszSrc)+dwTest) = 0;
					*(&(*(WORD*)wszSrc)+dwTest+1) = 0;
					dwSub = dwTest;
					nSrcLen ++;
					dwTest ++;
					dwLen ++;
				}
				dwTest ++;
			}
			*(&(*(WORD*)wszSrc)+dwSub) = dwTest - 2 - dwSub;
			*(&(*(WORD*)wszSrc)+dwSub+1) = 0;
			*(&(*(WORD*)wszSrc)+nSrcLen) = 0;

			*(&(*(DWORD*)pMain)) = dwLen;

			// �޸� ������ ������ ���� ����
			*(&(*(DWORD*)pBackUp)+3) = 0;

			memcpy(*(DWORD**)pMain+1, wszSrc, nSrcLen*2);
		}
		else	// MBCS
		{
			LPSTR szSrc = (LPSTR)pSrc;
			UINT_PTR nSrcLen =  strlen(szSrc);
			LPSTR szDest = (LPSTR)pDest;
			UINT_PTR nDestLen = strlen(szDest);

			BYTE dwTest=0;
			BYTE szBuf;
			BYTE dwSub=0;
			BYTE dwLen=1;
 
			memmove(szSrc+4, szSrc, nSrcLen);
			*(&(*(DWORD*)szSrc)) = 0;
			nSrcLen += 4;
			
			while(dwTest < nSrcLen)
			{
				szBuf = *(&(*(BYTE*)szSrc)+dwTest);
				if(szBuf >= 0x80 && !((*(&(*(BYTE*)szSrc)+dwTest+1) == 0xFF) && szBuf == 0x9F))
					dwTest ++;
				else if(szBuf == 0x0A)
				{
					memmove(szSrc+dwTest+3, szSrc+dwTest, nSrcLen-dwTest);
					*(&(*(BYTE*)szSrc)+dwSub) = dwTest - 4 - dwSub;
					*(&(*(BYTE*)szSrc)+dwSub+1) = 0;
					*(&(*(BYTE*)szSrc)+dwSub+2) = 0;
					*(&(*(BYTE*)szSrc)+dwSub+3) = 0;
					*(&(*(BYTE*)szSrc)+dwTest) = 0;
					*(&(*(BYTE*)szSrc)+dwTest+1) = 0;
					*(&(*(BYTE*)szSrc)+dwTest+2) = 0;
					*(&(*(BYTE*)szSrc)+dwTest+3) = 0;
					dwSub = dwTest;
					nSrcLen += 3;
					dwTest += 4;
					dwLen ++;
				}
				else if((*(&(*(BYTE*)szSrc)+dwTest+1) == 0xFF) && szBuf == 0x9F)
				{
					memmove(szSrc+dwTest+2, szSrc+dwTest, nSrcLen-dwTest);
					*(&(*(BYTE*)szSrc)+dwSub) = dwTest - 4 - dwSub;
					*(&(*(BYTE*)szSrc)+dwSub+1) = 0;
					*(&(*(BYTE*)szSrc)+dwSub+2) = 0;
					*(&(*(BYTE*)szSrc)+dwSub+3) = 0;
					*(&(*(BYTE*)szSrc)+dwTest) = 0;
					*(&(*(BYTE*)szSrc)+dwTest+1) = 0;
					*(&(*(BYTE*)szSrc)+dwTest+2) = 0;
					*(&(*(BYTE*)szSrc)+dwTest+3) = 0;
					dwSub = dwTest;
					nSrcLen += 2;
					dwTest += 3;
					dwLen ++;
				}
				dwTest ++;
			}
			*(&(*(BYTE*)szSrc)+dwSub) = dwTest - 4 - dwSub;
			*(&(*(BYTE*)szSrc)+dwSub+1) = 0;
			*(&(*(BYTE*)szSrc)+dwSub+2) = 0;
			*(&(*(BYTE*)szSrc)+dwSub+3) = 0;
			*(&(*(BYTE*)szSrc)+nSrcLen) = 0;

			*(&(*(DWORD*)pMain)) = dwLen;

			// �޸� ������ ������ ���� ����
			*(&(*(DWORD*)pBackUp)+3) = 0;

			memcpy((DWORD*)pMain+1, szSrc, nSrcLen+1);
		}
	}
	else if (m_bEndPos)
	{
		// �����͸� ��ü�ϱ��� �̸� ����صд�
		SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);

		if(m_bUnicode)
		{
			LPWSTR wszSrc = (LPWSTR)pSrc;
			sLen = wcslen(wszSrc);
			size_t wszJpnLen = 0;
			
			if (m_bIgnoreBufLen)
			{
				// �޸� ������ ������ ���� ����
				*(&(*(DWORD*)pBackUp)+3) = 0;

				if(*(&(*(DWORD*)pMain)+1) - *(&(*(DWORD*)pMain)) == sLen)
					memcpy(*(WCHAR **)pMain, wszSrc, sLen);
				else
				{
					wszJpnLen = wcslen((wchar_t*)*(&(*(DWORD*)pMain)+1));
					memmove(*(WCHAR **)pMain+sLen, (wchar_t*)*(&(*(DWORD*)pMain)+1), wszJpnLen);
					*(&(*(WORD*)(*(&(*(DWORD*)pMain))))+wszJpnLen+sLen) = 0x00;
					memcpy(*(WCHAR **)pMain, wszSrc, sLen);
					*(&(*(DWORD*)pMain)+1) = (DWORD)(*(WCHAR **)pMain+sLen);
				}
			}
			else
			{
				*(WCHAR **)pMain = wszSrc;
				*(&(*(DWORD*)pMain)+1) = (DWORD)(wszSrc+sLen);
			}
		}
		else	// MBCS
		{
			LPSTR szSrc = (LPSTR)pSrc;
			sLen = strlen(szSrc);
			size_t szJpnLen = 0;

			if (m_bIgnoreBufLen)
			{
				// �޸� ������ ������ ���� ����
				*(&(*(DWORD*)pBackUp)+3) = 0;

				if(*(&(*(DWORD*)pMain)+1) - *(&(*(DWORD*)pMain)) == sLen)
					memcpy(*(CHAR **)pMain, szSrc, sLen);
				else
				{
					szJpnLen = strlen((char*)*(&(*(DWORD*)pMain)+1));
					memmove(*(CHAR **)pMain+sLen, (char*)*(&(*(DWORD*)pMain)+1), szJpnLen);
					*(&(*(BYTE*)(*(&(*(DWORD*)pMain))))+szJpnLen+sLen) = 0x00;
					memcpy(*(CHAR **)pMain, szSrc, sLen);
					*(&(*(DWORD*)pMain)+1) = (DWORD)(*(CHAR **)pMain+sLen);
				}
			}
			else
			{
				*(CHAR **)pMain = szSrc;
				*(&(*(DWORD*)pMain)+1) = (DWORD)(szSrc+sLen);
			}
		}
	}

	else if (m_bTjsStr)
	{
		if(*(&(*(int*)pMain)) == 0x2) m_nBackUpSize = 0;

		// �����͸� ��ü�ϱ��� �̸� ����صд�
		SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain)+1)))), (&(*(DWORD*)pMain)+1), m_nBackUpSize);

		if(m_bUnicode)
		{
			LPWSTR wszSrc = (LPWSTR)pSrc;
			sLen =  wcslen(wszSrc);
			if(sLen >= 10)
				*(&(*(DWORD*)pMain)+1) = (DWORD)pSrc;
			else
			{
				*(&(*(DWORD*)pMain)+1) = 0;
				memcpy((DWORD*)pMain+2, wszSrc, (sLen+1)*2);
			}
		}
		else
			*(&(*(DWORD*)pMain)+1) = (DWORD)pSrc;
	}

	else if(m_bNullchar)	// Nullchar
	{
		LPWSTR wszSrc = (LPWSTR)pSrc;
		UINT_PTR nSrcLen =  wcslen(wszSrc);
		LPWSTR wszDest = (LPWSTR)pDest;
		// MATCH �� OVERWRITE�� IGNORE �� �ݴ밳���̹Ƿ� ������ �ݴ�
		UINT_PTR nDestLen = (m_bMatchSize ? wcslen(wszDest) : nSrcLen);	

		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: wszSrc =%s\n"), wszSrc);
		NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: wszDest=%s\n"), wszDest);

		// ���� �� ���ڿ��̾����� üũ
		if ((*(&(*(DWORD*)pMain)+4) >= 0x4) || m_bForcePtr)
			bWasLongMode = TRUE;
		else
			bWasLongMode = FALSE;

		// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
		if( IsBadWritePtr(wszDest, nDestLen) || IsBadStringPtrW(wszDest, 1024*1024*1024) ) return FALSE;

		// �����͸� ��ü�ϱ��� �̸� ����صд�
		SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);

		if(m_bReverse)
		{
			if(m_bUnicode)	// UnicodeRev
			{
				BYTE dwTest=0;
				BYTE wszBuf;
				while(dwTest < nDestLen){
					wszBuf = *(&(*(BYTE*)wszSrc)+dwTest);
					*(&(*(BYTE*)wszSrc)+dwTest) = *(&(*(BYTE*)wszSrc)+dwTest+1);
					*(&(*(BYTE*)wszSrc)+dwTest+1) = wszBuf;
					dwTest+= 2;
				}
			}
			else
			{
				BYTE dwTest=0;
				BYTE wszBuf;
				BYTE dwSub=0;
				while(dwTest < nDestLen){
					wszBuf = *(&(*(BYTE*)wszSrc)+dwTest);
					if((*(&(*(BYTE*)wszSrc)+dwTest+1) == 0x0a) && wszBuf == 0x0d)
					{
						*(&(*(BYTE*)wszSrc)+dwTest) = *(&(*(BYTE*)wszSrc)+dwTest+1);
						*(&(*(BYTE*)wszSrc)+dwTest+1) = 0x00;
					}
					else if(wszBuf < 0x80 && wszBuf != 0x0d)
					{
						dwSub = nDestLen - dwTest;
						*(&(*(BYTE*)wszSrc)+nDestLen) = 0x00;
						*(&(*(BYTE*)wszSrc)+nDestLen+1) = 0x00;
						while(dwSub > 0)
						{
							*(&(*(BYTE*)wszSrc)+dwTest+dwSub) = *(&(*(BYTE*)wszSrc)+dwTest+dwSub-1);
							dwSub--;
						}
						*(&(*(BYTE*)wszSrc)+dwTest+dwSub+1) = 0x00;
						nDestLen++;
					}
					else 
					{
						*(&(*(BYTE*)wszSrc)+dwTest) = *(&(*(BYTE*)wszSrc)+dwTest+1);
						*(&(*(BYTE*)wszSrc)+dwTest+1) = wszBuf;
					}
					dwTest+=2;
				}
				*(&(*(BYTE*)wszSrc)+dwTest) = 0x00;
			}
		}

		// dwLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
		*(&(*(DWORD*)pMain)+4) = nDestLen;

		// i �� ����
		int i=1;
		sLen = (DWORD)nDestLen;

		// �ٸ� ���
		i=(sLen/0x4+1) * 0x4 - 1;

		// ����� i ���� �ְ�
		*(&(*(DWORD*)pMain)+5) = i;

		sLen = (DWORD)nDestLen;
		if((sLen >= 0x4) || m_bForcePtr){
			// �� ���ڿ�

			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: Unicode long\n"));

			// ���� ���� ����
			// ZeroMemory(pMain, sizeof(DWORD) * 4);

			// ������ ����
			if (m_bMatchSize)	// ���� ����
			{
				// �޸� ������ ������ ���� ����
				*(&(*(DWORD*)pBackUp)+3) = 0;

				size_t len = min(nDestLen, nSrcLen);
				memcpy(wszDest, wszSrc, (len+1)*sizeof(wchar_t));
				while(len<nDestLen)
				{
					wszDest[len] = L' ';
					len++;
				}
			}
			else if (m_bIgnoreBufLen)	// ���۱��� ����
			{
				if (bWasLongMode)	// ���� ����ϴ� ���۰� �ִ�
				{
					// �޸� ������ ������ ���� ����
					*(&(*(DWORD*)pBackUp)+3) = 0;

					lstrcpyW(wszDest, wszSrc);	// ���ۿ� ����
				}
				else	// ���� ª�� ���ڿ��� (���� ����)
				{
					// ���� ����
					*(WCHAR **)pMain = wszSrc;
				}
			}
			else	// �⺻ ����
			{
				// ���� ����
				*(WCHAR **)pMain = wszSrc;
			}

			// �������� ��� ���¿� �°� �ø���
			*(&(*(DWORD*)(*(&(*(DWORD*)pMain))))+nDestLen) = NULL;
			while(nDestLen > 0){
				nDestLen--;
				*(&(*(DWORD*)(*(&(*(DWORD*)pMain))))+nDestLen) = *(&(*(WORD*)(*(&(*(DWORD*)pMain))))+nDestLen);
			}
		}
		else
		{
			// ª�� ���ڿ�

			NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: Unicode short\n"));

			// ���� ���� ����
			ZeroMemory(pMain, sizeof(DWORD) * 4);

			// ������ ����
			if (m_bMatchSize)	// ���� ����
			{
				size_t len = min(nDestLen, nSrcLen);
				memcpy(wszDest, wszSrc, (len+1)*sizeof(wchar_t));
				while(len<nDestLen)
				{
					wszDest[len] = L' ';
					len++;
				}
			}
			else	// ���۱��� ���� & �⺻ ���� ����
			{
				// ���ڿ� ����
				lstrcpyW((WCHAR *)pMain, wszSrc);
			}

			// �������� ��� ���¿� �°� �ø���
			*(&(*(DWORD*)pMain)+nDestLen) = NULL;
			while(nDestLen > 0){
				nDestLen--;
				*(&(*(DWORD*)pMain)+nDestLen) = *(&(*(WORD*)pMain)+nDestLen);
			}
		}
	}
	else
	{
		if(m_bYuris)
		{
			if(m_bUnicode)
			{
				LPWSTR wszSrc = (LPWSTR)pSrc;
				LPWSTR wszDest = (LPWSTR)pDest;
				UINT_PTR nSrcLen;
				// ���������� ���� ũ�Ⱑ �����ɶ� �� ���̺��� ������ �ȵȴ�
				//  ������ < ����
				if( wcslen(wszSrc) < wcslen(wszDest) )
				{
					nSrcLen = wcslen(wszDest);
					sLen = wcslen(wszSrc);
			
					while( sLen < (DWORD)nSrcLen )
					{
						*(&(*(WORD*)wszSrc)+sLen/2) = 0x20;
						sLen+=2;
					}
					*(&(*(WORD*)wszSrc)+sLen/2) = 0x00;
				}
				else nSrcLen =  wcslen(wszSrc);
				UINT_PTR nDestLen = nSrcLen;

				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: wszSrc =%s\n"), wszSrc);
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: wszDest=%s\n"), wszDest);
				// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
				if( IsBadWritePtr(wszDest, nDestLen) || IsBadStringPtrW(wszDest, 1024*1024*1024) ) return FALSE;

				// �����͸� ��ü�ϱ��� �̸� ����صд�
				SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);
				
				// dwLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
				if(m_bYurisP == TRUE)
					*(&(*(DWORD*)pMain)+m_YurisOffset) = (DWORD)nDestLen;
				else return FALSE;

				// i �� ����
				int i=1;
				sLen = 0;

				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: Unicode Yuris\n"));

				if (m_bIgnoreBufLen)	// ���۱��� ����
				{
					// �޸� ������ ������ ���� ����
					*(&(*(DWORD*)pBackUp)+3) = 0;

					lstrcpyW(wszDest, wszSrc);	// ���ۿ� ����
				}
				else	// �⺻ ����
				{
					// ���� ����
					*(WCHAR **)pMain = wszSrc;
				}

				while((sLen * 2) < (DWORD)nDestLen )
				{
					*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen) = 0x32;
					sLen++;
				}
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+1) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+2) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+3) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+4) =0x00;

				// sLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
				*(&(*(DWORD*)pMain)+m_YurisOffset+1) = sLen;
			}
			else	// MBCS
			{
				LPSTR szSrc = (LPSTR)pSrc;
				LPSTR szDest = (LPSTR)pDest;
				UINT_PTR nSrcLen;
				// ���������� ���� ũ�Ⱑ �����ɶ� �� ���̺��� ������ �ȵȴ�
				//  ������ < ����
				if( strlen(szSrc) < strlen(szDest) )
				{
					nSrcLen = strlen(szDest);
					sLen = strlen(szSrc);
			
					while( sLen < (DWORD)nSrcLen )
					{
						*(&(*(BYTE*)szSrc)+sLen) = 0x20;
						sLen++;
					}
					*(&(*(BYTE*)szSrc)+sLen) = 0x00;
				}
				else nSrcLen =  strlen(szSrc);
				UINT_PTR nDestLen = nSrcLen;

				#if defined(_DEBUG) || defined(FORCE_DEBUG_LOG)
				WCHAR wszTemp[MAX_TEXT_LENGTH];

				MyMultiByteToWideChar(949, 0, szSrc, -1, wszTemp, MAX_TEXT_LENGTH);
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: szSrc =%s\n"), wszTemp);
				MyMultiByteToWideChar(932, 0, szDest, -1, wszTemp, MAX_TEXT_LENGTH);
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: szDest=%s\n"), wszTemp);
				#endif

				// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
				if( IsBadWritePtr(szDest, nDestLen) || IsBadStringPtrA(szDest, 1024*1024*1024) ) return FALSE;

				// �����͸� ��ü�ϱ��� �̸� ����صд�
				SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);

				// dwLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
				if(m_bYurisP == TRUE)
					*(&(*(DWORD*)pMain)+m_YurisOffset) = (DWORD)nDestLen;
				else return FALSE;

				// i �� ����
				int i=1;
				sLen = 0;

				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: ANSI Yuris\n"));
				
				if (m_bIgnoreBufLen)	// ���۱��� ����
				{
					// �޸� ������ ������ ���� ����
					*(&(*(DWORD*)pBackUp)+3) = 0;

					lstrcpyA(szDest, szSrc);
				}
				else	// �⺻ ����
				{
					// ���� ����
					*(CHAR **)pMain = szSrc;
				}

				DWORD dwTest=0;

				while(dwTest < (DWORD)nDestLen )
				{
					if( *(&(*(BYTE*)(*(&(*(DWORD*)pMain))))+dwTest) < 0x80 ||
						(*(&(*(BYTE*)(*(&(*(DWORD*)pMain))))+dwTest) > 0xA0 && *(&(*(BYTE*)(*(&(*(DWORD*)pMain))))+dwTest) < 0xE0) )
						*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen) = 0x31;
					else 
					{
						*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen) = 0x32;
						dwTest++;
					}
					dwTest++;
					sLen++;
				}
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+1) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+2) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+3) = 0x00;
				*(&(*(BYTE*)(*(&(*(DWORD*)pMain)+1)))+sLen+4) =0x00;

				// sLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
				*(&(*(DWORD*)pMain)+m_YurisOffset+1) = sLen;
			}
		}
		else
		{
			if(m_bUnicode)	// Unicode
			{
				LPWSTR wszSrc = (LPWSTR)pSrc;
				UINT_PTR nSrcLen =  wcslen(wszSrc);
				LPWSTR wszDest = (LPWSTR)pDest;
				// MATCH �� OVERWRITE�� IGNORE �� �ݴ밳���̹Ƿ� ������ �ݴ�
				UINT_PTR nDestLen = (m_bMatchSize ? wcslen(wszDest) : nSrcLen);	

				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: wszSrc =%s\n"), wszSrc);
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: wszDest=%s\n"), wszDest);

				// ���� �� ���ڿ��̾����� üũ
				if ((*(&(*(DWORD*)pMain)+4) >= 0x8) || m_bForcePtr)
					bWasLongMode = TRUE;
				else
					bWasLongMode = FALSE;

				// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
				if( IsBadWritePtr(wszDest, nDestLen) || IsBadStringPtrW(wszDest, 1024*1024*1024) ) return FALSE;

				// �����͸� ��ü�ϱ��� �̸� ����صд�
				SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);

				// dwLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
				*(&(*(DWORD*)pMain)+4) = nDestLen;

				// i �� ����
				int i=1;
				sLen = (DWORD)nDestLen;

		/*		// ���� ���
				while(sLen>=0x8){
					i++;
					sLen-=0x8;
				}
				i=(i*0x8)-0x1;
		*/
				// �ٸ� ���
				i=(sLen/0x8+1) * 0x8 - 1;

				// ����� i ���� �ְ�
				*(&(*(DWORD*)pMain)+5) = i;

				sLen = (DWORD)nDestLen;
				if((sLen >= 0x8) || m_bForcePtr){
					// �� ���ڿ�

					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: Unicode long\n"));

					// ���� ���� ����
					// ZeroMemory(pMain, sizeof(DWORD) * 4);

					// ������ ����
					if (m_bMatchSize)	// ���� ����
					{
						// �޸� ������ ������ ���� ����
						*(&(*(DWORD*)pBackUp)+3) = 0;

						size_t len = min(nDestLen, nSrcLen);
						memcpy(wszDest, wszSrc, (len+1)*sizeof(wchar_t));
						while(len<nDestLen)
						{
							wszDest[len] = L' ';
							len++;
						}
					}
					else if (m_bIgnoreBufLen)	// ���۱��� ����
					{
						if (bWasLongMode)	// ���� ����ϴ� ���۰� �ִ�
						{
							// �޸� ������ ������ ���� ����
							*(&(*(DWORD*)pBackUp)+3) = 0;

							lstrcpyW(wszDest, wszSrc);	// ���ۿ� ����
						}
						else	// ���� ª�� ���ڿ��� (���� ����)
						{
							// ���� ����
							*(WCHAR **)pMain = wszSrc;
						}
					}
					else	// �⺻ ����
					{
						// ���� ����
						*(WCHAR **)pMain = wszSrc;
					}
				}
				else
				{
					// ª�� ���ڿ�

					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: Unicode short\n"));

					// ���� ���� ����
					ZeroMemory(pMain, sizeof(DWORD) * 4);

					// ������ ����
					if (m_bMatchSize)	// ���� ����
					{
						size_t len = min(nDestLen, nSrcLen);
						memcpy(wszDest, wszSrc, (len+1)*sizeof(wchar_t));
						while(len<nDestLen)
						{
							wszDest[len] = L' ';
							len++;
						}
					}
					else	// ���۱��� ���� & �⺻ ���� ����
					{
						// ���ڿ� ����
						lstrcpyW((WCHAR *)pMain, wszSrc);
					}
				}

		/*		// �̰� ����?
				nDestLen=nSrcLen;
				if(nDestLen==0){
					return TRUE;
				}
		*/
			}
			else	// MBCS
			{

				// MATCH �� OVERWRITE�� IGNORE �� �ݴ밳���̹Ƿ� ������ �ݴ�
				LPSTR szSrc = (LPSTR)pSrc;
				UINT_PTR nSrcLen =  strlen(szSrc);
				LPSTR szDest = (LPSTR)pDest;
				UINT_PTR nDestLen = (m_bMatchSize ? strlen(szDest) : nSrcLen);

				#if defined(_DEBUG) || defined(FORCE_DEBUG_LOG)
				WCHAR wszTemp[MAX_TEXT_LENGTH];

				MyMultiByteToWideChar(949, 0, szSrc, -1, wszTemp, MAX_TEXT_LENGTH);
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: szSrc =%s\n"), wszTemp);
				MyMultiByteToWideChar(932, 0, szDest, -1, wszTemp, MAX_TEXT_LENGTH);
				NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: szDest=%s\n"), wszTemp);
				#endif

				// ���� �� ���ڿ��̾����� üũ
				if ((*(&(*(DWORD*)pMain)+4) >= 0x10) || m_bForcePtr)
					bWasLongMode = TRUE;
				else
					bWasLongMode = FALSE;
		
				// ���ڿ� �����Ͱ� �߸��Ǿ��ٸ� ����
				if( IsBadWritePtr(szDest, nDestLen) || IsBadStringPtrA(szDest, 1024*1024*1024) ) return FALSE;

				// �����͸� ��ü�ϱ��� �̸� ����صд�
				SetUpTextPoint(pBackUp, (&(*(DWORD*)(*(&(*(DWORD*)pMain))))), (&(*(DWORD*)pMain)), m_nBackUpSize);

				// dwLen �ڸ��� �ٲ� ���ڱ��̸� �ְ�
				*(&(*(DWORD*)pMain)+4) = nDestLen;

				// i �� ����
				int i=1;
				sLen = (DWORD)nDestLen;

		/*		// ���� ���
				while(sLen>=0x10){
					i++;
					sLen-=0x10;
				}
				i=(i*0x10)-0x1;
		*/
				// �ٸ� ���
				i=(sLen/0x10+1) * 0x10 - 1;

				// ����� i ���� �ְ�
				*(&(*(DWORD*)pMain)+5) = i;
			
				sLen = (DWORD)nDestLen;
				if((sLen >= 0x10) || m_bForcePtr){
					// �� ���ڿ�

					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: ANSI long\n"));

					// ���� ���� ����
					// ZeroMemory(pMain, sizeof(DWORD) * 4);

					// ������ ����
					if (m_bMatchSize)	// ���� ����
					{
						// �޸� ������ ������ ���� ����
						*(&(*(DWORD*)pBackUp)+3) = 0;

						size_t len = 0;
						while(len<nDestLen && len<nSrcLen)
						{
							size_t addval = 1;
							if( (BYTE)0x80 <= (BYTE)szSrc[len] ) addval = 2;

							if( len + addval > nDestLen ) break;

							len += addval;
						}

						memcpy(szDest, szSrc, (len+1));
						while(len<nDestLen)
						{
							szDest[len] = ' ';
							len++;
						}				

					}
					else if (m_bIgnoreBufLen)	// ���۱��� ����
					{
						if (bWasLongMode)	// ���� ����ϴ� ���۰� �ִ�
						{
							// �޸� ������ ������ ���� ����
							*(&(*(DWORD*)pBackUp)+3) = 0;

							lstrcpyA(szDest, szSrc);
						}
						else	// ���� ª�� ���ڿ��� (���� ����)
						{
							// ���� ����
							*(CHAR **)pMain = szSrc;
						}
					}
					else	// �⺻ ����
					{
						if(nSrcLen >= 0x10)
						{
							//*(&(*(DWORD*)szSrc)-1) = *(&(*(DWORD*)(*(&(*(DWORD*)pMain))))-1);
							//*(&(*(DWORD*)szSrc)-2) = *(&(*(DWORD*)(*(&(*(DWORD*)pMain))))-2);
						}
						// ���� ����
						*(CHAR **)pMain = szSrc;
					}
				}
				else
				{
					NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: ANSI short\n"));
					// ª�� ���ڿ�

					// ���� ���� ����
					ZeroMemory(pMain, sizeof(DWORD) * 4);

					// ������ ����
					if (m_bMatchSize)	// ���� ����
					{
						size_t len = 0;
						while(len<nDestLen && len<nSrcLen)
						{
							size_t addval = 1;
							if( (BYTE)0x80 <= (BYTE)szSrc[len] ) addval = 2;

							if( len + addval > nDestLen ) break;

							len += addval;
						}

						memcpy(szDest, szSrc, (len+1));
						while(len<nDestLen)
						{
							szDest[len] = ' ';
							len++;
						}				

					}
					else	// ���۱��� ���� & �⺻ ���� ����
					{
						// ���ڿ� ����
						lstrcpyA((CHAR *)pMain, szSrc);
					}
				}
			}
		}
	}
	NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplySmartString: success\n"));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ���ڿ� ���̸� ������ ���ڿ��� ���̷� �����մϴ�. (�������̵��)
//
BOOL CTransCommandSMSTR::ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp)
{
	return CTransCommand::ChangeLength(m_pArgText, pParser, pNewBasePtr, pBackUp);
}

//////////////////////////////////////////////////////////////////////////
//  ������ ���ڿ��� �� ���α׷��� �����մϴ�. (�������̵��)
//
BOOL CTransCommandSMSTR::ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp)
{
	if (bIsDuplicated)
	{
		if (m_bUnicode || m_bUnicode8)
		{
			if (*(WCHAR *)m_pTransTextBuf == L'\0')
				return FALSE;
		}
		else
		{
			if (*(char *)m_pTransTextBuf == '\0')
				return FALSE;
		}
		// ���� ���ۿ� ������ ������ �״�� ����Ѵ�.
	}

	NOTIFY_DEBUG_MESSAGE(_T("SMSTR: ApplyTranslatedText: called\n"));

	return ApplySmartString(pArgText, m_pTransTextBuf, pBackUp);
	
}