
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATCodeMgr.h"
#include "HookPoint.h"
#include "TransCommand.h"
#include "CharacterMapper.h"
#include "OptionMgr.h"

#include "Debug.h"

int CHookPoint::m_nKernelVer = 0;

bool CHookPoint::IsWinVistaAndAbove()
{
	/*
		OS Version
		Windows 2000	5.0
		Windows XP		5.1
		Windows 2003	5.2

		Windows Vista	6.0	: ASLR (Address Space Layout Randomization) ����
		Windows 2008	6.0
		Windows	2008 R2	6.1
		Windows 7		6.1
	*/
	if(CATCodeMgr::GetInstance()->m_bNoAslr)
	{
		return false;
	}

	if (m_nKernelVer) return (m_nKernelVer > 5);

	OSVERSIONINFO osVer = {0, };

	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osVer))
	{
		NOTIFY_DEBUG_MESSAGE(_T("GetVersionEx error: %d\n"), GetLastError());
		return false;
	}

	NOTIFY_DEBUG_MESSAGE(_T("IsWinVistaAndAbove: WinVer %d.%d.%d, Platform(%d) %s\n"),
		osVer.dwMajorVersion, osVer.dwMinorVersion, osVer.dwBuildNumber, osVer.dwPlatformId, osVer.szCSDVersion);

	CHookPoint::m_nKernelVer = osVer.dwMajorVersion;
	return (m_nKernelVer > 5);
}

CHookPoint::CHookPoint(void)
: m_hModule(NULL), m_pCodePoint(0), m_pCodeOffset(0), m_nCodeSubCount(0)
{
}

CHookPoint::~CHookPoint(void)
{
	// ���� ��ɵ� ��� ����
	DeleteAllTransCmd();
	
	// Unhook �Լ� �غ�
	PROC_UnhookCodePoint UnhookCodePoint = CATCodeMgr::GetInstance()->m_sContainerFunc.pfnUnhookCodePoint;
	if(UnhookCodePoint)
	{
		// Unhook
		if( m_pCodeOffset != NULL )
			UnhookCodePoint((LPVOID)(GetHookAddress() + m_pCodeOffset));
		UnhookCodePoint((LPVOID)GetHookAddress());
	}
}

CHookPoint* CHookPoint::CreateInstance(CString strAddr)
{
	CHookPoint* pInst = new CHookPoint();

	try
	{
		int nIdx = strAddr.ReverseFind(_T('!'));
		int nOffx = strAddr.Find(_T('~'));
		int nSubx = strAddr.Find(_T('['));

		if(nSubx>0)
		{
			int nEndx = strAddr.Find(_T(']'));
			int nMidx = strAddr.Find(_T('-'));
			if( nMidx == -1 ) nMidx = nEndx;

			int nCount = 0;
			pInst->m_pCodeSub[nCount] = 0;
			while(nEndx>0)
			{
				
				int nOptStrLen = 0;
				DWORD dwPatchAdr;

				CString m_patchAdr = strAddr.Mid(nSubx+1, nMidx-nSubx-1);
				m_patchAdr.Replace(_T(' '),_T(''));
				m_patchAdr.Replace(_T(','),_T('-'));
				m_patchAdr.Replace(_T('x'),_T(''));
				dwPatchAdr = (DWORD)wcstol(m_patchAdr, NULL, 16);

				pInst->m_pCodeSub[nCount] = dwPatchAdr;

				nCount ++;
				if( nMidx == nEndx ) break;

				nSubx = nMidx;
				nMidx = strAddr.Find(_T('-'),nMidx+1);

				if( nMidx == -1 ) nMidx = nEndx;
			}
			pInst->m_nCodeSubCount = nCount;
			pInst->m_pCodeSub[nCount] = 0;
			strAddr = strAddr.Left(strAddr.Find(_T('[')));
		}

		if(nOffx>0)
		{
			_stscanf(strAddr.Mid(nOffx + 1), _T("%x"), &pInst->m_pCodeOffset);
			strAddr = strAddr.Left(nOffx);
		}
		
		if(nIdx>=0)
		{
			pInst->m_strModuleName = strAddr.Left(nIdx);

			pInst->m_hModule = GetModuleHandle(pInst->m_strModuleName);
			if(NULL == pInst->m_hModule) throw -4;
			
			_stscanf(strAddr.Mid(nIdx+1), _T("%x"), &pInst->m_pCodePoint);
		}
		else
		{
			if (CHookPoint::IsWinVistaAndAbove())
			{
				TCHAR szExePath[1024];

				GetModuleFileName(NULL, szExePath, 1024);

				for (int i=lstrlen(szExePath); i>=0; i--)
				{
					if (szExePath[i] == _T('\\'))
					{
						pInst->m_strModuleName = szExePath+i+1;
						break;
					}
				}

				if (pInst->m_strModuleName.IsEmpty())
				{
					pInst->m_hModule = NULL;
					_stscanf(strAddr.Mid(nIdx+1), _T("%x"), &pInst->m_pCodePoint);
				}
				else
				{
					UINT_PTR pCodePoint; 
					_stscanf(strAddr.Mid(nIdx+1), _T("%x"), &pCodePoint);

					pInst->m_hModule = GetModuleHandle(NULL);
					pInst->m_pCodePoint = pCodePoint - 0x400000;
				}
				NOTIFY_DEBUG_MESSAGE(_T("CreateInstance: module %s, hModule %p, Codepoint %p\n"),
					pInst->m_strModuleName, pInst->m_hModule, pInst->m_pCodePoint);
			}
			else
			{
				pInst->m_hModule = NULL;
				_stscanf(strAddr.Mid(nIdx+1), _T("%x"), &pInst->m_pCodePoint);
			}
		}

		if(NULL == pInst->m_pCodePoint) throw -1;

		// ��ŷ �Լ� �غ�
		PROC_HookCodePointEx HookCodePointEx = CATCodeMgr::GetInstance()->m_sContainerFunc.pfnHookCodePointEx;
		if(HookCodePointEx == NULL) throw -2;

		if(NULL != pInst->m_pCodeOffset)
		{
			HookCodePointEx((LPVOID)(pInst->GetHookAddress() + pInst->m_pCodeOffset), PointCallback, 1);
			BOOL bHookRes = HookCodePointEx((LPVOID)pInst->GetHookAddress(), PointCallback, 1);
			if(bHookRes == FALSE)
			{
				//MessageBox(NULL, L"��ŷ����", NULL,0);
				throw -3;
			}
		}
		else
		{
			// ��ŷ
			BOOL bHookRes = HookCodePointEx((LPVOID)pInst->GetHookAddress(), PointCallback, CATCodeMgr::GetInstance()->m_nHookType);
			if(bHookRes == FALSE)
			{
				//MessageBox(NULL, L"��ŷ����", NULL,0);
				throw -3;
			}
		}
		
		TRACE("[aral1] �� %p Hooked \n", pInst->GetHookAddress());
	}
	catch(int nErrCode)
	{
		nErrCode = nErrCode;
		delete pInst;
		pInst = NULL;
	}

	return pInst;
}

//////////////////////////////////////////////////////////////////////////
//
// �� �ν��Ͻ��� ��ŷ �ּҸ� ��ȯ�մϴ�.
//
//////////////////////////////////////////////////////////////////////////
UINT_PTR CHookPoint::GetHookAddress()
{
	return ( (UINT_PTR)m_hModule + m_pCodePoint );
}

//////////////////////////////////////////////////////////////////////////
//
// �� �ν��Ͻ��� ��ŷ �ּҸ� ����ȭ�� ���ڿ� ���·� ��ȯ�մϴ�.
//
//////////////////////////////////////////////////////////////////////////
CString CHookPoint::GetHookAddressString()
{
	CString strRetVal;

	if(NULL == m_hModule)
	{
		if(NULL == m_pCodeOffset)
			strRetVal.Format(_T("0x%p[%d]"), m_pCodePoint, m_nCodeSubCount);
		else
			strRetVal.Format(_T("0x%p~0x%p[%d]"), m_pCodePoint, m_pCodeOffset, m_nCodeSubCount);
	}
	else
	{
		if(NULL == m_pCodeOffset)
			strRetVal.Format(_T("%s!0x%p[%d]"), (LPCTSTR)m_strModuleName, m_pCodePoint, m_nCodeSubCount);
		else
			strRetVal.Format(_T("%s!0x%p~0x%p[%d]"), (LPCTSTR)m_strModuleName, m_pCodePoint, m_pCodeOffset, m_nCodeSubCount);
	}
	
	return strRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ���ο� ���� ����� �����մϴ�.
//
//////////////////////////////////////////////////////////////////////////
CTransCommand* CHookPoint::AddTransCmd(COptionNode *pNode)
{

	// ���� �Ÿ�
	COptionNode* pDistNode = pNode->GetChild(0);

	CString strArgScript = pDistNode->GetValue();
	strArgScript.Remove(_T(' '));
	CTransCommand* pCmd = FindTransCmd(strArgScript);
	
	if(pCmd == NULL)
	{
		pCmd = CTransCommand::CreateTransCommand(strArgScript, pNode);

		m_vectorTransCmd.push_back(pCmd);
	}

	return pCmd;
}

//////////////////////////////////////////////////////////////////////////
//
// ���� ����� ã���ϴ�.
//
//////////////////////////////////////////////////////////////////////////
CTransCommand* CHookPoint::FindTransCmd(CString strArgScript)
{
	CTransCommand* pCmd = NULL;

	strArgScript.Remove(_T(' '));
	for(vector<CTransCommand*>::iterator iter = m_vectorTransCmd.begin();
		iter != m_vectorTransCmd.end();
		iter++)
	{
		if((*iter)->GetArgScript().CompareNoCase(strArgScript) == 0)
		{
			pCmd = (*iter);
			break;
		}
	}

	return pCmd;
}


//////////////////////////////////////////////////////////////////////////
//
// ���� ����� �� ������ �����մϴ�.
//
//////////////////////////////////////////////////////////////////////////
int CHookPoint::GetTransCmdCount()
{
	return (int)m_vectorTransCmd.size();
}


//////////////////////////////////////////////////////////////////////////
//
// Ư����°�� ���� ����� �����մϴ�.
//
//////////////////////////////////////////////////////////////////////////
CTransCommand* CHookPoint::GetTransCmd(int nIdx)
{
	CTransCommand* pCmd = NULL;
	
	if((size_t)nIdx < m_vectorTransCmd.size())
	{
		pCmd = m_vectorTransCmd[nIdx];
	}

	return pCmd;
}


//////////////////////////////////////////////////////////////////////////
//
// ���� ����� �����մϴ�.
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::DeleteTransCmd(int nIdx)
{
	if((size_t)nIdx < m_vectorTransCmd.size())
	{
		CTransCommand* pCmd = m_vectorTransCmd[nIdx];
		delete pCmd;
		m_vectorTransCmd.erase(m_vectorTransCmd.begin()+nIdx);
	}	
}

//////////////////////////////////////////////////////////////////////////
//
// ��� ���� ����� �����մϴ�.
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::DeleteAllTransCmd()
{
	for(vector<CTransCommand*>::iterator iter = m_vectorTransCmd.begin();
		iter != m_vectorTransCmd.end();
		iter++)
	{
		CTransCommand* pCmd = (*iter);
		delete pCmd;
	}

	m_vectorTransCmd.clear();
}



//////////////////////////////////////////////////////////////////////////
//
// Ư������ ��ŷ�� �ݹ� �Լ� (static)
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::PointCallback(LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters)
{
	
	CATCodeMgr* pATCodeMgr = CATCodeMgr::GetInstance();
	if(NULL == pATCodeMgr) return;

	bool bRestore=false;
	list<CHookPoint*>::iterator iter;
	CHookPoint* pPoint;

	// �Ÿʿ��� ��ŷ���� ���ϱ�
	for(iter = pATCodeMgr->m_listHookPoint.begin(); iter != pATCodeMgr->m_listHookPoint.end(); iter++)
	{
		pPoint = (*iter);

		if(pPoint->GetHookAddress() == (UINT_PTR)pHookedPoint)
		{
			if (pPoint->GetTransCmdCount() == 0)
			{
				bRestore=true;
				break;
			}

			DWORD pRetValue = pRegisters->_ESP;
			for(int i = 0; i < pPoint->m_nCodeSubCount; i++)
			{
				if( pPoint->m_pCodeSub[i] == *(&(*(DWORD*)pRetValue)) - 0x5 ||
					pPoint->m_pCodeSub[i] == *(&(*(DWORD*)pRetValue)) - 0x6 ||
					pPoint->m_pCodeSub[i] == *(&(*(DWORD*)pRetValue)) )
				{
					bRestore=true;
					break;
				}
			}

			// ���� ��� ����
			pPoint->ExecuteTransCmds(pRegisters, bRestore);
			break;
		}
		else if(pPoint->m_pCodeOffset != NULL && pPoint->GetHookAddress() + pPoint->m_pCodeOffset == (UINT_PTR)pHookedPoint)
		{
			bRestore=true;

			// ������ ���� ����
			pPoint->ExecuteTransCmds(pRegisters, bRestore);
			break;
		}
	}

	if (bRestore) // ���� ����� ���� ��ŷ�ڵ�
	{
		// ��� ��� �޸� ��������
		for(iter = pATCodeMgr->m_listHookPoint.begin(); iter != pATCodeMgr->m_listHookPoint.end(); iter++)
		{
			pPoint = (*iter);

			int cnt = pPoint->GetTransCmdCount();
			for(int i=0; i<cnt; i++)
			{
				CTransCommand *pCmd=pPoint->GetTransCmd(i);
				pCmd->RestoreBackup();
			}

		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// ���� �۾� ���� (��Ƽ����Ʈ / �����ڵ� ����)
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::ExecuteTransCmds(PREGISTER_ENTRY pRegisters, bool bPtrBack)
{
	NOTIFY_DEBUG_MESSAGE(_T("ExecuteTransCmds: called, CodePoint=%p\n"), (UINT_PTR)m_hModule + m_pCodePoint );

	int cnt = GetTransCmdCount();

	m_parser.SetRegisterValues(pRegisters);

	for(int i=0; i<cnt; i++)
	{
		CTransCommand* pCmd = GetTransCmd(i);		
		LPVOID pArgText = NULL;

		NOTIFY_DEBUG_MESSAGE(_T("ExecuteTransCmds: ArgScript=%s\n"),pCmd->GetArgScript());
		
		try
		{
			BOOL bTrans = FALSE;
			CString strClipboardText = _T("");

			// ������ ���� ��� ����
			/*if (!pCmd->GetPtrBack() && !bPtrBack)
			{
				*(&(*(DWORD*)pCmd->m_pBackUpBuf)+3) = 0;
				*(&(*(DWORD*)pCmd->m_pBackUpLength)+3) = 0;
			}*/

			// �ؽ�Ʈ�� �ִ� ��ġ�� ������
			pArgText=pCmd->GetTextPoint(&m_parser, pCmd->m_pBackUpBuf, pCmd->GetPtrBack() || bPtrBack, pCmd->m_pBackUpLength);

			// ���� �ڵ��̹Ƿ� ����
			if (bPtrBack) continue;

			// ���Ṯ�� ������ ����
			if (pCmd->GetLenEnd())
				EndCharAdd(pCmd, pArgText, &m_parser);

			if (pArgText == NULL) continue;	// �ؽ�Ʈ ��ġ�� �߸��Ǿ��ų� �����ؾ� �� ����Ÿ��


			// �������� �ؽ�Ʈ���� Ȯ��
			if (!pCmd->IsValidTextPoint(pArgText)) throw -3; // _T("�߸��� ���ڿ� �������Դϴ�.");

			// �ߺ� ���� ���� Ȯ��
			BOOL bIsDuplicated = pCmd->IsDuplicateText(pArgText);
			if ((!pCmd->GetDuplicated())||(!bIsDuplicated&&pCmd->GetDuplicated())) 
			{
				// �����ڵ� �����̶��
				if( pCmd->GetUnicode() || pCmd->GetUnicode8() )
				{
					LPWSTR wszText;
					LPWSTR wszOriText[2048];

					// �ֱ� �Ϻ���� ����
					ZeroMemory(pCmd->m_pOrigTextBuf, sizeof(pCmd->m_pOrigTextBuf));

					// ��Ʈ�� ����
					if( pCmd->GetEndPos() )
					{
						size_t wsLen = *(&(*(DWORD*)pArgText)+1) - *(&(*(DWORD*)pArgText));
						wmemcpy((LPWSTR) wszOriText, (WCHAR *)(*(&(*(DWORD*)pArgText))), wsLen);
						wszText = (LPWSTR) wszOriText;
					}
					else wszText = (LPWSTR) pArgText;

					// UTF-8 -> UTF-16 ��ȯ
					if( pCmd->GetUnicode8() )
					{
						int len = MyMultiByteToWideChar(CP_UTF8, 0, (LPCSTR) pArgText, -1, NULL, 0);
						MyMultiByteToWideChar(CP_UTF8, 0, (LPCSTR) pArgText, -1, (LPWSTR)pCmd->m_pOrigTextBuf, len);
					}
					else
						wcscpy((LPWSTR)pCmd->m_pOrigTextBuf, wszText);

					// ��Ƽ����Ʈ�� ��ȯ
					char szAnsiJpn[MAX_TEXT_LENGTH*2];
					char szAnsiKor[MAX_TEXT_LENGTH*2];
					wchar_t wszAnsiKor[MAX_TEXT_LENGTH];

					// �����ڵ�� KoFilter
					if( AnsiError((LPWSTR)pCmd->m_pOrigTextBuf) )
					{
						continue;
					}

					// �Ϻ��� ��� Ŭ������ ���� �˻�
					if( pCmd->GetClipJpn() )
					{
#ifdef UNICODE
						strClipboardText += wszText;
#else
						strClipboardText += szAnsiJpn;
#endif
					}
	
					// �����⿡ ����־��
					PROC_TranslateText pfnTransJ2K = CATCodeMgr::GetInstance()->m_sContainerFunc.pfnTranslateText;
					if( pfnTransJ2K == NULL ) throw -2; //_T("AT �����̳��� �����Լ��� ã�� �� �����ϴ�.");
					
					// ������� ����
					if( pCmd->GetTrans() )
					{
						bTrans = TranslateScript((LPWSTR)pCmd->m_pOrigTextBuf, wszAnsiKor);
						MyWideCharToMultiByte(949, 0, wszAnsiKor, -1, szAnsiKor, MAX_TEXT_LENGTH*2, NULL, NULL);
					}
					else
					{
						MyWideCharToMultiByte(932, 0, (LPWSTR)pCmd->m_pOrigTextBuf, -1, szAnsiJpn, MAX_TEXT_LENGTH*2, NULL, NULL);
						bTrans = pfnTransJ2K(szAnsiJpn, szAnsiKor, MAX_TEXT_LENGTH*2);			
					}

					if( bTrans )
					{

						// �������Ÿ�� �ִ� ��� ����
						pCmd->RestoreBackup();

#ifdef UNICODE
						wchar_t wszTmpKor[MAX_TEXT_LENGTH];
						MyMultiByteToWideChar(949, 0, szAnsiKor, -1, wszTmpKor, MAX_TEXT_LENGTH);
						CString strKorean = wszTmpKor;
#else
						CString strKorean = szAnsiKor;
#endif

						// �ѱ��� ��� Ŭ������ ���� �˻�
						if( pCmd->GetClipKor() )
						{
							strClipboardText += strKorean;
						}

						// �ߺ� ���� ����
						strKorean.Replace(_T("  "), _T(" "));

						// ���� ����
						//if(CATCodeMgr::GetInstance()->m_bPtrCheat == FALSE && CATCodeMgr::GetInstance()->m_bRemoveSpace)
						if( pCmd->GetRemoveSpace() )
						{
							strKorean.Replace(_T(" "), _T(""));
						}

						// �ֱ� �ѱ���� ����
						/*if( pCmd->m_pTransTextBuf == NULL )
							pCmd->m_pTransTextBuf = new BYTE[MAX_TEXT_LENGTH*2];*/
						ZeroMemory(pCmd->m_pTransTextBuf, sizeof(pCmd->m_pTransTextBuf));

#ifdef UNICODE
						wcscpy((LPWSTR)pCmd->m_pTransTextBuf, (LPCWSTR)strKorean);
#else
						MyMultiByteToWideChar(949, 0, (LPCSTR)strKorean, -1, (LPWSTR)pCmd->m_pTransTextBuf, MAX_TEXT_LENGTH);
#endif
						// 1����Ʈ ��ȯ���� ���ڸ� �ݰ�����
						if( pCmd->GetOneByte() )
						{
							EnforceOneBtye(0, (LPWSTR)pCmd->m_pTransTextBuf);
						}

						// ���ڻ��� ���鹮�� ����
						if( pCmd->GetAddNull() )
						{
							EnforceAddNull(0, (LPWSTR)pCmd->m_pTransTextBuf, pCmd);
						}

						// 2����Ʈ�� �籸��
						if( pCmd->GetTwoByte() )
						{
							EnforceTwoBtye(0, (LPWSTR)pCmd->m_pTransTextBuf, pCmd->GetAddNull());
						}

						// �⸮�⸮ �̸� ����
						if( pCmd->GetKiriName() )
						{
							KiriName((LPWSTR)pCmd->m_pTransTextBuf, (LPWSTR)pCmd->m_pOrigTextBuf);
						}
					}
					else // bTrans == FALSE
					{
						NOTIFY_DEBUG_MESSAGE(_T("ExecuteTransCmds: Translate failed\n"));
						continue;
					}
				}		
				// ��Ƽ����Ʈ �����̶��
				else
				{
					LPSTR szText;
					LPSTR szOriText[2048];

					// �ֱ� �Ϻ���� ����
					ZeroMemory(pCmd->m_pOrigTextBuf, sizeof(pCmd->m_pOrigTextBuf));

					// ��Ʈ�� ����
					if( pCmd->GetEndPos() )
					{
						size_t sLen = *(&(*(DWORD*)pArgText)+1) - *(&(*(DWORD*)pArgText));
						memcpy((LPSTR) szOriText, (CHAR *)(*(&(*(DWORD*)pArgText))), sLen);
						szText = (LPSTR) szOriText;
					}
					else szText = (LPSTR) pArgText;

					strcpy((LPSTR)pCmd->m_pOrigTextBuf, szText);

					// �����ڵ�� ��ȯ
					wchar_t wszAnsiJpn[MAX_TEXT_LENGTH];
					wchar_t wszAnsiKor[MAX_TEXT_LENGTH];
					char szAnsiKor[MAX_TEXT_LENGTH*2];

					// �Ϻ��� ��� Ŭ������ ���� �˻�
					if( pCmd->GetClipJpn() )
					{
#ifdef UNICODE
						wchar_t wszTmpJpn[MAX_TEXT_LENGTH];
						MyMultiByteToWideChar(932, 0, szText, -1, wszTmpJpn, MAX_TEXT_LENGTH);
						strClipboardText += wszTmpJpn;
#else
						strClipboardText += szText;
#endif
					}

					// �����⿡ ����־��
					PROC_TranslateText pfnTransJ2K = CATCodeMgr::GetInstance()->m_sContainerFunc.pfnTranslateText;
					if( pfnTransJ2K == NULL ) throw -2; //"AT �����̳��� �����Լ��� ã�� �� �����ϴ�.";
					
					// ������� ����
					if( pCmd->GetTrans() )
					{
						MyMultiByteToWideChar(932, 0, szText, -1, wszAnsiJpn, MAX_TEXT_LENGTH);
						bTrans = TranslateScript(wszAnsiJpn, wszAnsiKor);
						MyWideCharToMultiByte(949, 0, wszAnsiKor, -1, szAnsiKor, MAX_TEXT_LENGTH*2, NULL, NULL);
					}
					else
						bTrans = pfnTransJ2K(szText, szAnsiKor /*(char*)pCmd->m_pTransTextBuf*/, MAX_TEXT_LENGTH*2);

					if( bTrans )
					{
						// �������Ÿ�� �ִ� ��� ����
						pCmd->RestoreBackup();

						/*if( pCmd->m_pTransTextBuf == NULL )
							pCmd->m_pTransTextBuf = new BYTE[MAX_TEXT_LENGTH*2];*/
						ZeroMemory(pCmd->m_pTransTextBuf, sizeof(pCmd->m_pTransTextBuf));

						strcpy((char*)pCmd->m_pTransTextBuf, szAnsiKor);

#ifdef UNICODE
						wchar_t wszTmpKor[MAX_TEXT_LENGTH];
						MyMultiByteToWideChar(949, 0, (LPCSTR)pCmd->m_pTransTextBuf, -1, wszTmpKor, MAX_TEXT_LENGTH);
						CString strKorean = wszTmpKor;
#else
						CString strKorean = (LPCSTR)pCmd->m_pTransTextBuf;
#endif

						// �ѱ��� ��� Ŭ������ ���� �˻�
						if( pCmd->GetClipKor() )
						{
							strClipboardText += strKorean;
						}

						// 1����Ʈ ��ȯ���� ���ڸ� �ݰ�����
						if( pCmd->GetOneByte() )
						{
							EnforceOneBtye((LPSTR)pCmd->m_pTransTextBuf, 0);
						}

						// �ߺ� ���� ����
						strKorean.Replace(_T("  "), _T(" "));

						// ���� ����
						if( pCmd->GetRemoveSpace() )
						{
							strKorean.Replace(_T(" "), _T(""));
							MyWideCharToMultiByte(949, 0, strKorean, -1, (LPSTR)pCmd->m_pTransTextBuf, MAX_TEXT_LENGTH*2, NULL, NULL);
							//ReduceSpaceA((LPSTR)pCmd->m_pTransTextBuf, (int)strlen(szText));
							// 1����Ʈ ��ȯ���� ���ڸ� �ݰ�����
							if( pCmd->GetOneByte() )	EnforceOneBtye((LPSTR)pCmd->m_pTransTextBuf, 0);
						}

						// ���ڻ��� ���鹮�� ����
						if( pCmd->GetAddNull() )
						{
							EnforceAddNull((LPSTR)pCmd->m_pTransTextBuf, 0, NULL);
						}

						// 2����Ʈ�� �籸��
						if( pCmd->GetTwoByte() )
						{
							EnforceTwoBtye((LPSTR)pCmd->m_pTransTextBuf, 0, pCmd->GetAddNull());
						}

						// �ѱ� ���ڵ� (��Ƽ����Ʈ�� ��츸)
						if( CATCodeMgr::GetInstance()->m_bEncodeKorean )
						{
							EncodeMultiBytes((LPSTR)pCmd->m_pTransTextBuf);
						}
					}
					else // bTrans == FALSE
					{
						NOTIFY_DEBUG_MESSAGE(_T("ExecuteTransCmds: Translate failed\n"));
						continue;
					}
				} // if(pCmd->GetUnicode()) ...
			} // if (!pCmd->IsDuplicateText(pArgText)) ...

			// NOTE
			// �Ϻ��� �и���Ű���� pCmd->m_pOrigTextBuf �� ���� ����Ÿ�� �ִ� ������
			// pCmd->m_pTransTextBuf �� ���� �����͸� �ִ� ���� (Duplicate �� ��� pass�ϴ� ���� ����)
			// ���� CTransCommand �� ���� ���������� ������ ����� �Ѿ.
			// ���� Ŭ���� �и��� ������ �� ��ĸ����� ó���� �и����� ���ο� ����� �߰� �� ����������
			// ���� �ϱ� ���� ���ε� �ܼ��� ���ڿ� ���縦 ���� �޼ҵ� �����鼭 �߰��� �ʿ䰡
			// ���� ���� �� ��.

			// ���� pCmd->m_pOrigTextBuf ���� ����, pCmd->m_pTransTextBuf ���� �������� �� ����

			// UTF-16 -> UTF-8 ��ȯ
			if( pCmd->GetUnicode8() )
			{
				int len = MyWideCharToMultiByte(CP_UTF8, 0, (LPWSTR) pCmd->m_pTransTextBuf, -1, NULL, 0, 0, 0);
				MyWideCharToMultiByte(CP_UTF8, 0, (LPWSTR) pCmd->m_pTransTextBuf, -1, (LPSTR)pCmd->m_pTransTextBuf, len, 0, 0);
			}

			// ���� ����
			if (pCmd->m_bAdjustLen && (!pCmd->ChangeLength(pArgText, &m_parser, 0, pCmd->m_pBackUpLength)))
				throw -5;

			// ������ ���ڿ��� ������ ���α׷��� �ݿ�
			if (!pCmd->ApplyTranslatedText(pArgText, pRegisters, &m_parser, bIsDuplicated&&pCmd->GetDuplicated(), pCmd->m_pBackUpBuf))
				throw -5;

			pCmd->m_pPrevArgText = pArgText;

			// Ŭ������� ����
			if(!strClipboardText.IsEmpty())
			{
				CATCodeMgr::GetInstance()->SetClipboardText(strClipboardText);
			}

		}
		catch (int nErrCode)
		{
			nErrCode = nErrCode;
			TRACE(_T("ExecuteTransCmds exception code : %d \n"), nErrCode);
			NOTIFY_DEBUG_MESSAGE(_T("ExecuteTransCmds exception code : %d \n"), nErrCode);
		}

	} // end of for

}

BOOL CHookPoint::EnforceTwoBtye(LPSTR szText, LPWSTR wszText, BOOL b_nNull)
{
	char pszTestString[MAX_TEXT_LENGTH*2];
	BOOL b_uniCode=FALSE;

	if( 0 == szText )
	{
		MyWideCharToMultiByte(949, 0, wszText, -1, pszTestString, MAX_TEXT_LENGTH*2, NULL, NULL);
		szText = pszTestString;
		b_uniCode=TRUE;
	}

	if(NULL == szText) return FALSE;

	char szKorean[MAX_TEXT_LENGTH*2];
	strcpy(szKorean, szText);

	int len = (int)strlen(szKorean);
	int i = 0;
	int j = 0;
	while( i<len )
	{
		if( (0x80 <= (BYTE)szKorean[i])&&
			!(0xA1 == (BYTE)szKorean[i] && 0xA1 == (BYTE)szKorean[i+1]) )
		{
			szText[j] = szKorean[i];
			szText[j+1] = szKorean[i+1];

			i+=2;
		}
		else if(0xA1 == (BYTE)szKorean[i] && 0xA1 == (BYTE)szKorean[i+1])
		{
			if( (0xA3 == (BYTE)szText[j-2]) ||
				(0xA1 == (BYTE)szText[j-2] && 0xAD == (BYTE)szText[j-1]) ||
				(0xA1 == (BYTE)szText[j-2] && 0xA1 == (BYTE)szText[j-1]) ||
				(0x80 > (BYTE)szText[i-2] && 0x80 > (BYTE)szText[i-1]) ||
				(0x80 > (BYTE)szText[i-2] && ' ' == szText[i-1]))
			{
				if(!b_nNull)
					j-=2;
				else
				{
					szText[j] = szKorean[i];
					szText[j+1] = szKorean[i+1];
				}
			}
			else
			{
				szText[j] = szKorean[i];
				szText[j+1] = szKorean[i+1];
			}
			i+=2;
		}
		else if(' ' == szKorean[i])
		{
			if( (0xA3 == (BYTE)szText[j-2]) ||
				(0xA1 == (BYTE)szText[j-2] && 0xAD == (BYTE)szText[j-1]) ||
				(0xA1 == (BYTE)szText[j-2] && 0xA1 == (BYTE)szText[j-1]) ||
				(0x80 > (BYTE)szText[i-2] && 0x80 > (BYTE)szText[i-1]) ||
				(0x80 > (BYTE)szText[i-2] && ' ' == szText[i-1]))
			{
				if(!b_nNull)
					j-=2;
				else
				{
					szText[j] = (BYTE)0xA1;
					szText[j+1] = (BYTE)0xA1;
				}
			}
			else
			{
				szText[j] = (BYTE)0xA1;
				szText[j+1] = (BYTE)0xA1;
			}
			i++;
		}
		else
		{
			
			//WORD wChar = CCharacterMapper::Ascii2ShiftJIS(szKorean[i]);
			WORD wChar = CCharacterMapper::Ascii2KS5601(szKorean[i]);
			//if(' ' == szKorean[i]) wChar = 0x8140;
			
			if(wChar)
			{
				szText[j] = (char)HIBYTE(wChar);
				szText[j+1] = (char)LOBYTE(wChar);
			}
			else
			{
				szText[j] = szKorean[i];
				if( 0x80 <= (BYTE)szKorean[i+1] )
				{
					szText[j+1] = ' ';
				}
				else
				{
					i++;
					szText[j+1] = szKorean[i];
				}
			}
			i++;
		}
		j+=2;
	}

	szText[j] = '\0';

	if( b_uniCode == TRUE )
	{
		MyMultiByteToWideChar(949, 0, szText, -1, wszText, MAX_TEXT_LENGTH*2);
	}

	return TRUE;
}

BOOL CHookPoint::EnforceOneBtye(LPSTR szText, LPWSTR wszText)
{
	char pszTestString[MAX_TEXT_LENGTH*2];
	BOOL b_uniCode=FALSE;

	if( 0 == szText )
	{
		MyWideCharToMultiByte(949, 0, wszText, -1, pszTestString, MAX_TEXT_LENGTH*2, NULL, NULL);
		szText = pszTestString;
		b_uniCode=TRUE;
	}

	if(NULL == szText) return FALSE;

	char szKorean[MAX_TEXT_LENGTH*2];
	strcpy(szKorean, szText);

	int len = (int)strlen(szKorean);
	int i = 0;
	int j = 0;
	while( i<len )
	{
		if( 0x80 <= (BYTE)szKorean[i] )
		{
			if(		0xA3 == (BYTE)szKorean[i] )
				szText[j] = (BYTE)szKorean[i+1] - 0x80;
			else if(	0xA1 == (BYTE)szKorean[i] 
					&&	0xAD == (BYTE)szKorean[i+1] )
				szText[j] = (BYTE)szKorean[i+1] - 0x2F;
			else if(	0xA1 == (BYTE)szKorean[i] 
					&&	0xA1 == (BYTE)szKorean[i+1] )
				szText[j] = 0x20;
			else
			{
				szText[j] = szKorean[i];
				szText[j+1] = szKorean[i+1];
				j++;
			}
			i++;
		}
		else
			szText[j] = szKorean[i];
		i++;
		j++;
	}

	szText[j] = '\0';

	if( b_uniCode == TRUE )
	{
		MyMultiByteToWideChar(949, 0, szText, -1, wszText, MAX_TEXT_LENGTH*2);
	}

	return TRUE;
}

BOOL CHookPoint::EncodeMultiBytes( LPSTR szNewText )
{

	// ��Ƽ����Ʈ ���ڵ�
	int len = (int)strlen(szNewText);
	int i = 0;
	while( i<len )
	{
		if( 0x80 <= (BYTE)szNewText[i] )
		{
			char tmpbuf[3];
			tmpbuf[0] = szNewText[i];
			tmpbuf[1] = szNewText[i+1];
			tmpbuf[2] = '\0';

			CCharacterMapper::EncodeK2J(tmpbuf, &szNewText[i]);

			i++;
		}

		i++;
	}
	
	szNewText[i] = '\0';

	return TRUE;
}

BOOL CHookPoint::ReduceSpaceA( LPSTR szText, int nSizeTo )
{
	if(NULL == szText) return FALSE;

	char szKorean[MAX_TEXT_LENGTH*2];
	strcpy(szKorean, szText);

	int len = (int)strlen(szKorean);
	int nRemoveCnt = len - nSizeTo;
	int i = 0;
	int j = 0;
	while( i<len )
	{
		if( 0x80 <= (BYTE)szKorean[i])
		{
			szText[j++] = szKorean[i++];
			szText[j++] = szKorean[i++];
		}
		else
		{
			if(' ' != szKorean[i]  || nRemoveCnt <= 0)
			{
				szText[j++] = szKorean[i];
			}
			else
			{
				nRemoveCnt--;
			}

			i++;
		}
	}

	szText[j] = '\0';

	return TRUE;

}


BOOL CHookPoint::EnforceAddNull(LPSTR szText, LPWSTR wszText, CTransCommand* pCmd)
{
	if( 0 == szText )
	{
		if(NULL == wszText) return FALSE;
		
		wchar_t wszKorean[MAX_TEXT_LENGTH*2];
		wcscpy(wszKorean, wszText);

		int len = (int)wcslen(wszKorean);
		int i = 0;
		int j = 0;
		while( i<len )
		{
				if( (0x000D == wszKorean[i]) &&
					(0x000A == wszKorean[i+1]) ||
					(0x009F == wszKorean[i]) &&
					(0x00FF == wszKorean[i+1]))
				{
					wszText[j] = wszKorean[i];
					wszText[j+1] = wszKorean[i+1];
					i+=2;
					j+=2;
				}
				else
				{
					wszText[j] = wszKorean[i];
					if( wszText[j] == 0x0020 ) j--;
					if( pCmd->GetTwoByte() ) wszText[j+1] = 0x3000;
					else wszText[j+1] = 0x0020;
					i++;
					j+=2;
				}
		}

		wszText[j] = '\0';

		return TRUE;
	}
	else
	{
		if(NULL == szText) return FALSE;

		char szKorean[MAX_TEXT_LENGTH*2];
		strcpy(szKorean, szText);

		int len = (int)strlen(szKorean);
		int i = 0;
		int j = 0;
		while( i<len )
		{
			if( 0x80 <= (BYTE)szKorean[i] )
			{
				szText[j] = szKorean[i];
				szText[j+1] = szKorean[i+1];
				szText[j+2] = 0x20;
				i+=2;
				j+=3;
			}
			else if((0x0D == (BYTE)szKorean[i]) &&
					(0x0A == (BYTE)szKorean[i+1]) ||
					(0x9F == (BYTE)szKorean[i]) &&
					(0xFF == (BYTE)szKorean[i+1]))
			{
				szText[j] = szKorean[i];
				szText[j+1] = szKorean[i+1];
				i+=2;
				j+=2;
			}
			else
			{
				szText[j] = szKorean[i];
				if( 0x20 == (BYTE)szKorean[i] ) j--;
				if( 0x20 == (BYTE)szKorean[i+1] ) i++;
				szText[j+1] = 0x20;
				i++;
				j+=2;
			}
		}

		szText[j] = '\0';

		return TRUE;
	}
}

BOOL CHookPoint::AnsiError(LPWSTR wszText)
{
	char pszTestString[MAX_TEXT_LENGTH*2];
	MyWideCharToMultiByte(949, 0, wszText, -1, pszTestString, MAX_TEXT_LENGTH*2, NULL, NULL);

	volatile BYTE szBuf[3] = { '\0', };
	WORD wch;
	int nKoCount, nJpCount;

	int i, nLen;

	BOOL bIsKorean, bIsnKorean;

	nLen=lstrlenA(pszTestString);

	// �⺻ üũ ��ƾ
	nKoCount=0;
	for (i=0; i<nLen; i++)
	{
		bIsKorean=FALSE;	// ���⼭�� �� ������ �ѱ�����
		bIsnKorean=FALSE;

		// üũ ����Ʈ
		szBuf[0]=(BYTE) pszTestString[i];
		szBuf[1]=(BYTE) pszTestString[i+1];

		wch=MAKEWORD(pszTestString[i+1], pszTestString[i]);

		if (szBuf[0] < 0x80)	// 1����Ʈ �ڵ�
			continue;	// ����
/*
		if ((	// EncodeKor �ڵ���� (CCharacterMapper::EncodeK2J from ATCode)
			( (0x889F <= wch) && (wch <= 0x9872) ) ||	// 0x889F - 0x9872,
			( (0x989F <= wch) && (wch <= 0x9FFC) ) ||	// 0x989F - 0x9FFC,
			( (0xE040 <= wch) && (wch <= 0xEAA4) ) ||	// 0xE040 - 0xEAA4,
			( (0xED40 <= wch) && (wch <= 0xEEEC) )		// 0xED40 - 0xEEEC
			))
		{
			// �ѱ��ΰ�? (EncodeKor)

			// Decode �غ���
			DecodeJ2K(pszTestString+i, (char *)szBuf);
		}
*/
		// KSC-5601 : 0xA1A1 - 0xA1FE
		//   �ڵ�        -        -
		//   ����     0xFEA1 - 0xFEFE
		// ù��° ����Ʈ 
		// 0xA1 : ������, ���� ��ȣ
		// 0xA2 : ��ȣ (ȭ��ǥ, ��Ʈ ��)
		// 0xA3 : ���� 0-9A-Za-z��
		// 0xA4 : ����, ����, ���
		// 0xA5 : �θ�����, �θ�����
		// 0xA6 : ������ (ǥ ��)
		// 0xA7 : ������
		// 0xA8 : ������
		// 0xA9 : ��ȣ����
		// 0xAA : ���󰡳�
		// 0xAB : īŸī��
		// 0xAC : ���þƾ�
		// 0xAD-0xAF : ��Ÿ
		// 0xB0-0xC8 : �ѱ� (��-��)
		// 0xC9 : ��Ÿ
		// 0xCA-0xFD : ���� (ʡ-��)
		// 0xFE : ��Ÿ
		// �ι�° ����Ʈ
		// 0xA1-0xFE ���̸� ��ȿ

		// 1. �ѱ� üũ (0xB0-0xC8)
		if (((0xB0 <= szBuf[0])&&(szBuf[0] <= 0xC8)) && 
			((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xFE)))
			bIsKorean=TRUE;
		// 2. Ư������ - ������ (0xA1), ���� ����/������(0xA3)
		else if (((0xA1 == szBuf[0])||(0xA3 == szBuf[0])) && 
			((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xFE)))
			bIsnKorean=TRUE;
		// 3. �θ�����
		else if ((0xA5 == szBuf[0]) &&
			( ((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xAA)) ||	// ��-��
			 ((0xB0 <= szBuf[1])&&(szBuf[1] <= 0xB9)) ) 	// ��-��
			)
			bIsnKorean=TRUE;
		// 4. ������
		else if ((0xA8 == szBuf[0]) &&
			((0xE7 <= szBuf[1])&&(szBuf[1] <= 0xF5))	// �� - ��
			)
			bIsnKorean=TRUE;

		// �ѱ�
		if (bIsKorean)
			// if (IsDBCSLeadByteEx(949, pszTestString[i]))
			nKoCount++;
		i++; // �̹� üũ�����Ƿ� ���� ���ڷ� �Ѿ
	}

	MyWideCharToMultiByte(932, 0, wszText, -1, pszTestString, MAX_TEXT_LENGTH*2, NULL, NULL);

	nLen=lstrlenA(pszTestString);

	// �⺻ üũ ��ƾ
	nJpCount=0;
	for (i=0; i<nLen; i++)
	{
		bIsKorean=FALSE;	// ���⼭�� �� ������ �ѱ�����
		bIsnKorean=FALSE;

		// üũ ����Ʈ
		szBuf[0]=(BYTE) pszTestString[i];
		szBuf[1]=(BYTE) pszTestString[i+1];

		wch=MAKEWORD(pszTestString[i+1], pszTestString[i]);

		if (szBuf[0] < 0x80)	// 1����Ʈ �ڵ�
			continue;	// ����

		/*
		if ((	// EncodeKor �ڵ���� (CCharacterMapper::EncodeK2J from ATCode)
			( (0x889F <= wch) && (wch <= 0x9872) ) ||	// 0x889F - 0x9872,
			( (0x989F <= wch) && (wch <= 0x9FFC) ) ||	// 0x989F - 0x9FFC,
			( (0xE040 <= wch) && (wch <= 0xEAA4) ) ||	// 0xE040 - 0xEAA4,
			( (0xED40 <= wch) && (wch <= 0xEEEC) )		// 0xED40 - 0xEEEC
			))
		{
			// �ѱ��ΰ�? (EncodeKor)

			// Decode �غ���
			DecodeJ2K(pszTestString+i, (char *)szBuf);
		}
		*/

		// KSC-5601 : 0xA1A1 - 0xA1FE
		//   �ڵ�        -        -
		//   ����     0xFEA1 - 0xFEFE
		// ù��° ����Ʈ 
		// 0xA1 : ������, ���� ��ȣ
		// 0xA2 : ��ȣ (ȭ��ǥ, ��Ʈ ��)
		// 0xA3 : ���� 0-9A-Za-z��
		// 0xA4 : ����, ����, ���
		// 0xA5 : �θ�����, �θ�����
		// 0xA6 : ������ (ǥ ��)
		// 0xA7 : ������
		// 0xA8 : ������
		// 0xA9 : ��ȣ����
		// 0xAA : ���󰡳�
		// 0xAB : īŸī��
		// 0xAC : ���þƾ�
		// 0xAD-0xAF : ��Ÿ
		// 0xB0-0xC8 : �ѱ� (��-��)
		// 0xC9 : ��Ÿ
		// 0xCA-0xFD : ���� (ʡ-��)
		// 0xFE : ��Ÿ
		// �ι�° ����Ʈ
		// 0xA1-0xFE ���̸� ��ȿ

		// 1. �ѱ� üũ (0xB0-0xC8)
		if (((0xB0 <= szBuf[0])&&(szBuf[0] <= 0xC8)) && 
			((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xFE)))
			bIsKorean=TRUE;
		// 2. Ư������ - ������ (0xA1), ���� ����/������(0xA3)
		else if (((0xA1 == szBuf[0])||(0xA3 == szBuf[0])) && 
			((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xFE)))
			bIsnKorean=TRUE;
		// 3. �θ�����
		else if ((0xA5 == szBuf[0]) &&
			( ((0xA1 <= szBuf[1])&&(szBuf[1] <= 0xAA)) ||	// ��-��
			 ((0xB0 <= szBuf[1])&&(szBuf[1] <= 0xB9)) ) 	// ��-��
			)
			bIsnKorean=TRUE;
		// 4. ������
		else if ((0xA8 == szBuf[0]) &&
			((0xE7 <= szBuf[1])&&(szBuf[1] <= 0xF5))	// �� - ��
			)
			bIsnKorean=TRUE;

		// �ݰ��̶�� ����
		if( (0xA0 <= szBuf[0]) && (szBuf[0] < 0xE0) )
		{
			bIsKorean=FALSE;
			bIsnKorean=FALSE;
		}


		// �ѱ��� �ø��� ����
		if (bIsKorean || bIsnKorean)
			i++; // �̹� üũ�����Ƿ� ���� ���ڷ� �Ѿ	
		else
		{
			// ���� ����, ��ҹ��� ����� ����
			if  ((0x82 == szBuf[0])&&(
				((0x4F <= szBuf[1])&&(szBuf[1] <= 0x58))||
				((0x60 <= szBuf[1])&&(szBuf[1] <= 0x79))||
				((0x81 <= szBuf[1])&&(szBuf[1] <= 0x9A))))
				i++;
			// �ѱ� ��ȯ�� ����� �� �ִ� Ư���� �α׸� ����
			else if  ((0x81 == szBuf[0])&&(
				((0x40 <= szBuf[1])&&(szBuf[1] <= 0x51))||
				((0x5A <= szBuf[1] && 0x5B != szBuf[1])&&(szBuf[1] <= 0xFF))))
				i++;
			// �Ͼ�
			else
			{
				if (IsDBCSLeadByteEx(932, (BYTE)pszTestString[i]))
				{
					WORD Char=(BYTE)pszTestString[i]*256+(BYTE)pszTestString[i+1];
					if(((0x8141 == Char) ||							//,
						(0x8142 == Char) ||							//.
						(0x8163 == Char) ||							//��
						((0x8175 <= Char) && (Char <= 0x82FA)) ||	//����ȣ
						(0x8158 == Char) ||							//���ȣ
						((0x829F <= Char) && (Char <= 0x82FA)) ||	//���󰡳�
						((0x8340 <= Char) && (Char <= 0x8396)) ||	//��Ÿī��
						((0x889F <= Char) && (Char <= 0xEAA4)) ||	//���� ��1~2����
						((0xED40 <= Char) && (Char <= 0xEEEC))		//���� ��3����
						))
						nJpCount++;
				}
				else if( (0xA0 <= szBuf[0]) && (szBuf[0] < 0xE0) )
					nJpCount++;
				i++;
			}
		}
		// ��Ÿ ���ڴ� ����
	}

	// ���� ����
	bIsKorean=FALSE;
	switch(CATCodeMgr::GetInstance()->m_nUniKofilterLevel)
	{
		case 0: // ������� ����
			break;

		case 5: // �Ͼ 1�ڶ� ������ ����
			if (!nJpCount)
				bIsKorean=TRUE;
			break;

		case 10:	// �ѱ�x2 > �Ͼ� �϶� ���� ����
			if (nKoCount*2 > nJpCount)
				bIsKorean=TRUE;
			break;

		case 15:	// �ѱ� > �Ͼ� �϶� ���� ����
			if (nKoCount > nJpCount)
				bIsKorean=TRUE;
			break;

		case 20:	// �ѱ� > �Ͼ�x2 �϶� ���� ����
			if (nKoCount > nJpCount*2)
				bIsKorean=TRUE;
			break;

		case 25:	//  �ѱ� > �Ͼ�x3 �϶� ���� ����
			if (nKoCount > nJpCount*3)
				bIsKorean=TRUE;
			break;

		case 30:	// �ѱ��� 1�ڶ� ������ ���� ����
		if (nKoCount)
			bIsKorean=TRUE;
		break;

		default:
			break;
	}
	return bIsKorean;
}
/*
BOOL CHookPoint::DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode)
{
	BOOL bRetVal = FALSE;

	// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
	// 95~3074, 3119~4535, 16632~18622, 19089~19450
	if(cszJpnCode && szKorCode && 0x88 <= (BYTE)cszJpnCode[0] && (BYTE)cszJpnCode[0] <= 0xEE)
	{
		WORD dwKorTmp = (WORD)( (BYTE)cszJpnCode[0] - 0x88 ) * 189;
		dwKorTmp += (WORD)( (BYTE)cszJpnCode[1] - 0x40 );

		if(dwKorTmp > 19450) return FALSE;
		if(dwKorTmp > 18622) dwKorTmp -= 466;
		if(dwKorTmp > 4535) dwKorTmp -= 12096;
		if(dwKorTmp > 3074) dwKorTmp -= 44;
		dwKorTmp -= 95;

		szKorCode[0] = (char)( ((UINT_PTR)dwKorTmp/(UINT_PTR)0xA0) + (UINT_PTR)0xA0);
		szKorCode[1] = (char)( ((UINT_PTR)dwKorTmp%(UINT_PTR)0xA0) + (UINT_PTR)0xA0);

		//TRACE("[ aral1 ] DecodeJ2K : (%p) -> (%p)'%s' \n", (UINT_PTR)(MAKEWORD(cszJpnCode[1], cszJpnCode[0])), (UINT_PTR)(MAKEWORD(szKorCode[1], szKorCode[0])), szKorCode);

		if( 0xA0 <= (BYTE)szKorCode[0] && 0xA0 <= (BYTE)szKorCode[1] )
		{
			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA1 )
			{
				*((BYTE*)szKorCode) = 0xBE;
				*((BYTE*)szKorCode+1) = 0xC6;
			}

			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA2 )
			{
				*((BYTE*)szKorCode) = 0xB7;
				*((BYTE*)szKorCode+1) = 0xB8;
			}

			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA3 )
			{
				*((BYTE*)szKorCode) = 0xB8;
				*((BYTE*)szKorCode+1) = 0xD5;
			}

			bRetVal = TRUE;
		}
	}

	return bRetVal;
}
*/
BOOL CHookPoint::EndCharAdd(CTransCommand* pCmd, LPVOID pOrigBasePtr, CTransScriptParser *pParser)
{
	int *pnLen = NULL;
	// ���̰� ����� ��ġ�� ã�´�
	if (pCmd->m_nLenPos)
	{
		// m_nLenPos �� �ִٸ� pOrigBasePtr���� �����ġ�� ���
		pnLen = (int *)((DWORD)pOrigBasePtr + pCmd->m_nLenPos);
	}
	else if (!pCmd->m_strLenPos.IsEmpty())
	{
		// m_strLenPos �� �ִٸ� �״�� �ļ��� �־ ���
		int nType;
		int *pRetVal = (int *)pParser->GetValue(pCmd->m_strLenPos, &nType);

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

	int nTempLen = *pnLen;

	if ( pCmd->GetUnicode() ) // �����ڵ���
		*(&(*(WORD*)pOrigBasePtr)+nTempLen) = 0x0000;
	else
		*(&(*(BYTE*)pOrigBasePtr)+nTempLen) = 0x00;

	return TRUE;
}

BOOL CHookPoint::TranslateScript(LPCWSTR cwszSrc, LPWSTR wszTar)
{
	BOOL bRetVal = FALSE;

	LPWSTR wszSrc = (LPWSTR)cwszSrc;
	int nTotalLines = GetTotalLines(cwszSrc);
	int nCurLine = 0;
	int nNotTra = 0;
	int nCrLf;

	CString strLine = L"";

	try
	{
		WCHAR wch, wszKorText[1024];
		while ( nCrLf = GetNextLine(wszSrc, strLine) )
		{
			wch = strLine[0];
			size_t nLen;
			if (wch != L''/* && 
				strLine.Find(L'{') == -1 &&
				strLine.Find(L'}') == -1 &&
				nNotTra == 0 &&
				(strLine[0] != L'/' && strLine[1] != L'/')*/)
			{
				nNotTra = 0;
				ZeroMemory(wszKorText, 1024 * sizeof(WCHAR));
				TranslateUnicodeText(strLine.GetString(), wszKorText);
				nLen = wcslen(wszKorText);
				wsprintfW(wszTar, L"%s", wszKorText);
				wszTar += nLen;
			}
			//else if (nNotTra > 0
			//	|| strLine.Find(L'{') != -1 
			//	|| strLine.Find(L'}') != -1)
			//{
			//	// ��ɾ�
			//	int i = 0;
			//	while( i < strLine.GetLength() )
			//	{
			//		if( strLine[i] == L'{' ) nNotTra ++;
			//		else if( strLine[i] == L'}' ) nNotTra --;
			//		i++;
			//	}
			//	wsprintfW(wszTar, L"%s", strLine.GetString());
			//	wszTar += strLine.GetLength();
			//}
			else
			{
				// ��ũ��Ʈ ����
				nNotTra = 0;
				wsprintfW(wszTar, L"%s", strLine.GetString());
				wszTar += strLine.GetLength();
			}
			if( nCrLf == 1 || nCrLf == 3 )
			{
				*wszTar = L'\r';
				wszTar++;
			}
			if( nCrLf == 2 || nCrLf == 3 )
			{
				*wszTar = L'\n';
				wszTar++;
			}
			*wszTar = L'\0';

			nCurLine++;

		}	// end of while (Line)

		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
		bRetVal = FALSE;
	}
	return bRetVal;
}

BOOL CHookPoint::TranslateUnicodeText( LPCWSTR cwszSrc, LPWSTR wszTar )
{
	BOOL bRetVal = FALSE;

	if(NULL == cwszSrc || NULL == wszTar || wcslen(cwszSrc) > 1024) return FALSE;
	
	//��� Ascii�θ� �Ǿ� �ִ� ��� ���� ���� ����
	bool bIsAscii=true;
	for(LPCWSTR pwch = cwszSrc;*pwch!=L'\0';pwch++){
		if(*pwch>=0x80){
			bIsAscii=false;
			break;
		}
	}

	if(bIsAscii){
		wcscpy(wszTar,cwszSrc);
		return TRUE;
	}

	char tmpJpn[2048];
	char tmpKor[2048];
	ZeroMemory(tmpJpn, 2048);
	ZeroMemory(tmpKor, 2048);

	MyWideCharToMultiByte(932, 0, cwszSrc, -1, tmpJpn, 2048, NULL, NULL);
	PROC_TranslateText pfnTransJ2K = CATCodeMgr::GetInstance()->m_sContainerFunc.pfnTranslateText;

	if(pfnTransJ2K)
	{
		bRetVal = pfnTransJ2K(tmpJpn, tmpKor, 2048);

		if(TRUE == bRetVal)
		{
			MyMultiByteToWideChar(949, 0, tmpKor, -1, wszTar, 1024);
		}
	}
	
	if (!bRetVal)
	{
		int nSize = lstrlenW(cwszSrc) + 1;
		CopyMemory(wszTar, cwszSrc, nSize * 2);
	}

	return bRetVal;
}

int CHookPoint::GetTotalLines( LPCWSTR cwszText )
{
	int nRetVal = 0;
	
	if(cwszText && cwszText[0])
	{
		nRetVal = 1;

		while(cwszText[0])
		{
			if( L'\n' == cwszText[0] )
			{
				nRetVal++;
			}
			else if ( L'\r' == cwszText[0])
			{
				if ( L'\n' == cwszText[1])
					cwszText++;
				nRetVal++;
			}
			cwszText++;
		}
	}
	return nRetVal;
}

int CHookPoint::GetNextLine(LPWSTR& wszBuf, CString& strLine)
{
	int bRetVal = 0;
	
	strLine = _T("");
	
	if(wszBuf && wszBuf[0])
	{
		bRetVal = 4;
		while(wszBuf[0])
		{
			if( L'\n' == wszBuf[0] )
			{
				wszBuf ++;
				bRetVal = 2;
				break;
			}
			else if ( L'\r' == wszBuf[0])
			{
				if ( L'\n' == wszBuf[1])
				{
					wszBuf+=2;
					bRetVal = 3;
					break;
				}
				else
				{
					// ���� �幰�� \r ������ �ణ�� ������ ���� ��쵵 ����.. oTL
					wszBuf++;
					bRetVal = 1;
					break;
				}
			}
			else
			{
				strLine += wszBuf[0];
			}

			wszBuf++;
		}
	}
	return bRetVal;
}

BOOL CHookPoint::KiriName(LPWSTR wszTransTextBuf, LPWSTR wszOrigTextBuf)
{
	wchar_t tmpKorText[1024];
	ZeroMemory(tmpKorText, 1024*sizeof(wchar_t));

	CString strTextJpn = wszOrigTextBuf;
	int nJpnLen = strTextJpn.GetLength();

	int nJpnNameStart = -1;
	int nJpnNameEnd	  = -1;

	if(0x3010 == strTextJpn[0])
		nJpnNameStart = 0;

	if(nJpnNameStart != -1)
		nJpnNameEnd = strTextJpn.Find(0x3011,1);

	// �̸� ����/���� ã�� �Ϻ���

	if( nJpnLen > 2
		&&	nJpnNameStart != -1
		&&  nJpnNameEnd != -1)
	{
		CString strTextKor = wszTransTextBuf;

		int nKorNameStart = strTextKor.Find(0x3010);
		int nKorNameEnd = strTextKor.Find(0x3011,nKorNameStart+1);
		// �̸� ����,���� ã�� �ѱ���

		int nJpnSlash = CString(strTextJpn.Left(nJpnNameEnd)).Find(_T('/'),1);
		int nKorSlash = CString(strTextKor.Left(nKorNameEnd)).Find(_T('/'),1);
		
		// �����ð� �̹� ���Ե� ���
		if(nJpnSlash > 0 && nKorSlash > 0)
		{
			strTextKor = strTextJpn.Left(nJpnSlash) + strTextKor.Mid(nKorSlash);
		}
		// �����ð� ������ �̸��� �籸���Ѵ�.
		else
		{
			strTextKor = strTextJpn.Left(nJpnNameEnd) + L"/" + strTextKor.Mid(nKorNameStart+1);	
			//��⵵ ���� =��=
		}

		wcscpy(wszTransTextBuf, (LPCWSTR)strTextKor);
		return TRUE;
	}
	else return FALSE;
}