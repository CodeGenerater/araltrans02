
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "stdafx.h"
#include "Resource.h"
#include "ATTextArgMgr.h"
#include "ATTextArg.h"
#include "Function.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "MainDbgDlg.h"

#define TEXT_ARG_POOL_SIZE 100

extern CATTextArgMgr g_objATTextArgMgr;
LPCTSTR CATTextArgMgr::m_arrTextFuncName[] = 
{
	_T("GetGlyphOutlineA"),
	_T("GetGlyphOutlineW"),
	_T("TextOutA"),
	_T("TextOutW"),
	_T("ExtTextOutA"),
	_T("ExtTextOutW"),
	_T("DrawTextA"),
	_T("DrawTextW"),
	_T("DrawTextExA"),
	_T("DrawTextExW")
};
CATTextArgMgr*	CATTextArgMgr::_Inst = NULL;


CATTextArgMgr* CATTextArgMgr::GetInstance()
{
	return _Inst;
}

CATTextArgMgr::CATTextArgMgr(void)
	: m_pMainDbgDlg(NULL), m_pDlgThread(NULL), m_pCurBreakedPoint(0)
{
	_Inst = this;
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_FUNCTION_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
	ZeroMemory(m_aTextFuncHit, sizeof(int)*TEXT_FUNC_CNT);
}


CATTextArgMgr::~CATTextArgMgr(void)
{
	_Inst = NULL;
	Close();
}


UINT CATTextArgMgr::MainDlgThreadFunc( LPVOID pParam )
{
	CATTextArgMgr* pThis = (CATTextArgMgr*)pParam;
	pThis->m_pMainDbgDlg = new CMainDbgDlg();
	pThis->m_pMainDbgDlg->DoModal();
	delete pThis->m_pMainDbgDlg;
	pThis->m_pMainDbgDlg = NULL;

	return 0;
}

// �ʱ�ȭ
BOOL CATTextArgMgr::Init(HWND hSettingWnd, LPSTR cszOptionStringBuffer) 
{
	Close();

	BOOL bRetVal = FALSE;

	// �θ� ������ �ڵ� ����
	if(NULL==hSettingWnd) return FALSE;
	m_hContainerWnd = hSettingWnd;

	// �����̳� �Լ� ������ ������
	m_hContainer = GetModuleHandle(_T("ATCTNR.DLL"));
	if(m_hContainer)
	{
		m_sContainerFunc.pfnGetCurAlgorithm		= (PROC_GetCurAlgorithm) GetProcAddress( m_hContainer, "GetCurAlgorithm" );
		m_sContainerFunc.pfnGetCurTranslator	= (PROC_GetCurTranslator) GetProcAddress( m_hContainer, "GetCurTranslator" );
		m_sContainerFunc.pfnHookDllFunctionEx		= (PROC_HookDllFunctionEx) GetProcAddress( m_hContainer, "HookDllFunctionEx" );
		m_sContainerFunc.pfnGetOrigDllFunction	= (PROC_GetOrigDllFunction) GetProcAddress( m_hContainer, "GetOrigDllFunction" );
		m_sContainerFunc.pfnUnhookDllFunction	= (PROC_UnhookDllFunction) GetProcAddress( m_hContainer, "UnhookDllFunction" );
		m_sContainerFunc.pfnHookCodePointEx		= (PROC_HookCodePointEx) GetProcAddress( m_hContainer, "HookCodePointEx" );
		m_sContainerFunc.pfnUnhookCodePoint		= (PROC_UnhookCodePoint) GetProcAddress( m_hContainer, "UnhookCodePoint" );
		m_sContainerFunc.pfnTranslateText		= (PROC_TranslateText) GetProcAddress( m_hContainer, "TranslateText" );
		m_sContainerFunc.pfnSuspendAllThread	= (PROC_SuspendAllThread) GetProcAddress( m_hContainer, "SuspendAllThread" );
		m_sContainerFunc.pfnResumeAllThread		= (PROC_ResumeAllThread) GetProcAddress( m_hContainer, "ResumeAllThread" );
		m_sContainerFunc.pfnIsAllThreadSuspended= (PROC_IsAllThreadSuspended) GetProcAddress( m_hContainer, "IsAllThreadSuspended" );
	}

	if( m_sContainerFunc.pfnGetCurAlgorithm && m_sContainerFunc.pfnGetCurTranslator
		&& m_sContainerFunc.pfnHookDllFunctionEx && m_sContainerFunc.pfnGetOrigDllFunction
		&& m_sContainerFunc.pfnUnhookDllFunction && m_sContainerFunc.pfnHookCodePointEx
		&& m_sContainerFunc.pfnUnhookCodePoint && m_sContainerFunc.pfnTranslateText
		&& m_sContainerFunc.pfnSuspendAllThread && m_sContainerFunc.pfnResumeAllThread
		&& m_sContainerFunc.pfnIsAllThreadSuspended)
	{
		// ��� ������ ����
		m_sContainerFunc.pfnSuspendAllThread();

		// �ؽ�Ʈ �Լ��� ��ŷ		
		// GetGlyphOutlineA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "GetGlyphOutlineA", NewGetGlyphOutlineA, 0 ) )
		{
			m_sTextFunc.pfnGetGlyphOutlineA = 
				(PROC_GetGlyphOutline) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "GetGlyphOutlineA");
		}

		// GetGlyphOutlineW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "GetGlyphOutlineW", NewGetGlyphOutlineW, 0 ) )
		{
			m_sTextFunc.pfnGetGlyphOutlineW = 
				(PROC_GetGlyphOutline) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "GetGlyphOutlineW");
		}

		// TextOutA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "TextOutA", NewTextOutA, 0 ) )
		{
			m_sTextFunc.pfnTextOutA = 
				(PROC_TextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "TextOutA");
		}

		// TextOutW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "TextOutW", NewTextOutW, 0 ) )
		{
			m_sTextFunc.pfnTextOutW = 
				(PROC_TextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "TextOutW");
		}

		// ExtTextOutA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "ExtTextOutA", NewExtTextOutA, 0 ) )
		{
			m_sTextFunc.pfnExtTextOutA = 
				(PROC_ExtTextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "ExtTextOutA");
		}

		// ExtTextOutW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "ExtTextOutW", NewExtTextOutW, 0 ) )
		{
			m_sTextFunc.pfnExtTextOutW = 
				(PROC_ExtTextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "ExtTextOutW");
		}

		// DrawTextA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextA", NewDrawTextA, 0 ) )
		{
			m_sTextFunc.pfnDrawTextA = 
				(PROC_DrawText) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextA");
		}

		// DrawTextW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextW", NewDrawTextW, 0 ) )
		{
			m_sTextFunc.pfnDrawTextW = 
				(PROC_DrawText) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextW");
		}

		// DrawTextExA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextExA", NewDrawTextExA, 0 ) )
		{
			m_sTextFunc.pfnDrawTextExA = 
				(PROC_DrawTextEx) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextExA");
		}

		// DrawTextExW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextExW", NewDrawTextExW, 0 ) )
		{
			m_sTextFunc.pfnDrawTextExW = 
				(PROC_DrawTextEx) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextExW");
		}

		// ���÷����� ���� �Լ�
		m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));

		m_sTextFunc.pfnOrigWideCharToMultiByte =
			(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"));

		if( m_sTextFunc.pfnExtTextOutA && m_sTextFunc.pfnExtTextOutW
			&& m_sTextFunc.pfnGetGlyphOutlineA && m_sTextFunc.pfnGetGlyphOutlineW
			&& m_sTextFunc.pfnOrigMultiByteToWideChar && m_sTextFunc.pfnOrigWideCharToMultiByte
			&& m_sTextFunc.pfnTextOutA && m_sTextFunc.pfnTextOutW 
			&& m_sTextFunc.pfnDrawTextA	&& m_sTextFunc.pfnDrawTextW
			&& m_sTextFunc.pfnDrawTextExA && m_sTextFunc.pfnDrawTextExW )
		{
			// ���÷����� �˻�
			HMODULE hApplocale = GetModuleHandle(_T("AlLayer.dll"));
			if(hApplocale)
			{
				m_bApplocale = TRUE;
			}

			// �ؽ�Ʈ ���� Ǯ ����
			for(int i=0; i<TEXT_ARG_POOL_SIZE; i++)
			{
				m_setInactivatedArgs.insert(new CATText());
			}

			// ����� ������ ����
			m_pDlgThread = AfxBeginThread(MainDlgThreadFunc, this);
			while(m_pMainDbgDlg == NULL) Sleep(100);

			//m_pMainDbgDlg->Create(CMainDbgDlg::IDD);
			//m_pMainDbgDlg->ShowWindow(SW_SHOW);
			//m_pMainDbgDlg->UpdateWindow();

			// �̺�Ʈ ����
			m_hBreak = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			m_hResume = ::CreateEvent(NULL, TRUE, TRUE, NULL);

			bRetVal = TRUE;
		}

		// ��� ������ �簡��
		m_sContainerFunc.pfnResumeAllThread();

	}

	
	if( FALSE == bRetVal ) Close();

	return bRetVal;
}

// ����ȭ
BOOL CATTextArgMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	m_hContainerWnd = NULL;

	// �̺�Ʈ ����
	if(m_hBreak)
	{
		CloseHandle(m_hBreak);
		m_hBreak = NULL;
	}

	if(m_hResume)
	{
		SetEvent(m_hResume);
		CloseHandle(m_hResume);
		m_hResume = NULL;
	}


	// ���� ����� ������ �ı�
	if(m_pMainDbgDlg && ::IsWindow(m_pMainDbgDlg->m_hWnd))
	{
		m_pMainDbgDlg->PostMessage(WM_CLOSE,0,0);
		for(int k=0; k<10 && NULL != m_pMainDbgDlg; k++) Sleep(300);
	}

	// ��� ������ ����
	m_sContainerFunc.pfnSuspendAllThread();

	// GetGlyphOutlineA ����
	if( m_sTextFunc.pfnGetGlyphOutlineA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "GetGlyphOutlineA" );
		m_sTextFunc.pfnGetGlyphOutlineA = NULL;
	}

	// GetGlyphOutlineW ����
	if( m_sTextFunc.pfnGetGlyphOutlineW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "GetGlyphOutlineW" );
		m_sTextFunc.pfnGetGlyphOutlineW = NULL;
	}

	// TextOutA ����
	if( m_sTextFunc.pfnTextOutA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "TextOutA" );
		m_sTextFunc.pfnTextOutA = NULL;
	}

	// TextOutW ����
	if( m_sTextFunc.pfnTextOutW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "TextOutW" );
		m_sTextFunc.pfnTextOutW = NULL;
	}

	// ExtTextOutA ����
	if( m_sTextFunc.pfnExtTextOutA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "ExtTextOutA" );
		m_sTextFunc.pfnExtTextOutA = NULL;
	}

	// ExtTextOutW ����
	if( m_sTextFunc.pfnExtTextOutW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "ExtTextOutW" );
		m_sTextFunc.pfnExtTextOutW = NULL;
	}

	// DrawTextA ����
	if( m_sTextFunc.pfnDrawTextA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextA" );
		m_sTextFunc.pfnDrawTextA = NULL;
	}

	// DrawTextW ����
	if( m_sTextFunc.pfnDrawTextW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextW" );
		m_sTextFunc.pfnDrawTextW = NULL;
	}

	// DrawTextExA ����
	if( m_sTextFunc.pfnDrawTextExA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextExA" );
		m_sTextFunc.pfnDrawTextExA = NULL;
	}

	// DrawTextExW ����
	if( m_sTextFunc.pfnDrawTextExW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextExW" );
		m_sTextFunc.pfnDrawTextExW = NULL;
	}


	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CATText* pATText = *(iter);
		delete pATText;
	}
	m_setActivatedArgs.clear();
	
	// ��Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CATTextSet::iterator iter = m_setInactivatedArgs.begin();
		iter != m_setInactivatedArgs.end();
		iter++)
	{
		CATText* pATText = *(iter);
		delete pATText;
	}
	m_setInactivatedArgs.clear();

	// ������ �Ÿ��� ����
	m_mapHitDist.clear();

	// �Լ� ��� ����
	for( CFunctionMap::iterator iter2 = m_mapFunc.begin();
		iter2 != m_mapFunc.end();
		iter2++)
	{
		CFunction* pFunc = iter2->second;
		delete pFunc;
	}
	m_mapFunc.clear();

	// ��ŷ�� �Լ��� ����
	for( CHookedPoints::iterator iter3 = m_setHookedPoints.begin();
		iter3 != m_setHookedPoints.end();
		iter3++)
	{
		m_sContainerFunc.pfnUnhookCodePoint( (LPVOID)(*iter3) );
	}
	m_setHookedPoints.clear();
	

	// ��Ÿ ���� ����
	m_bApplocale = FALSE;
	ZeroMemory(m_aTextFuncHit, sizeof(int)*TEXT_FUNC_CNT);

	// ��� ������ �簡��
	m_sContainerFunc.pfnResumeAllThread();

	return TRUE;

}



// ���ο� ���ڿ� �ĺ��� �߰��Ѵ�
int CATTextArgMgr::AddTextArg(LPVOID pText, BOOL bWideChar, UINT_PTR ptrFunc, size_t dist)
{
	BOOL nRetVal = 0;

	CATText* pATText = NULL;
	
	// ������ ����ִ� ���ڿ����� �˻� (Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ)
	for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CATText* pTmpATText = *(iter);

		if( pTmpATText->IsWideCharacter() == bWideChar && pTmpATText->TestText(pText) )
		{
			pATText = pTmpATText;
			nRetVal = 2;
		}		
	}		


	// ������ �ʿ��ϸ� ���� �Ǵ� �־��� ��带 ������ �߰��Ѵ�
	if( NULL == pATText)
	{	
		if(!m_setInactivatedArgs.empty()) 
		{
			CATTextSet::iterator iter = m_setInactivatedArgs.begin();
			pATText = ( *iter );
			m_setInactivatedArgs.erase(pATText);
			m_setActivatedArgs.insert(pATText);
		}
		else
		{
			// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
			for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
				iter != m_setActivatedArgs.end();
				iter++)
			{
				CATText* pTmpATText = *(iter);

				if( NULL == pATText || (pATText->GetHitCount() > pTmpATText->GetHitCount()) )
				{
					pATText = pTmpATText;
				}		
			}		
		}
			

		if( pATText->SetATText(pText, bWideChar, m_sContainerFunc.pfnTranslateText) )
		{
			nRetVal = 1;
		}
		else
		{
			m_setActivatedArgs.erase( pATText );
			m_setInactivatedArgs.insert(pATText);
			pATText = NULL;
		}
	}

	// ����� �����̸� 
	if( nRetVal!=0 )
	{
		// ��� ���� ���� �߰�
		if( ptrFunc && dist )
		{
			pATText->m_setFuncArg.insert( pair<UINT_PTR,size_t>(ptrFunc,dist) );
		}
	}

	return nRetVal;
}

// ���ڿ� �ĺ��� ��ü�� �׽�Ʈ�Ѵ�. (���̻� ��ġ���� �ʴ� �ĺ��� �ٷ� ����)
BOOL CATTextArgMgr::TestCharacter(wchar_t wch, void* baseESP)
{
	BOOL bRetVal = FALSE;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ��� ��� ��ȸ
	for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CATText* pATText = (*iter);
		iter++;
		
		// �˻� ����
		int nRes = pATText->TestCharacter(wch);
		
		// ��Ʈ(����)��
		if( 0 != nRes )
		{
			if( nRes & 0x01 )
			{
				ModifyHitMap(pATText, baseESP, +1);
				bRetVal = TRUE;

				for(set<pair<UINT_PTR,size_t>>::iterator iter = pATText->m_setFuncArg.begin();
					iter != pATText->m_setFuncArg.end();
					iter++)
				{
					CFunction* pFunc = m_mapFunc[iter->first];
					size_t distArg = iter->second;
					
					if(pFunc && distArg)
					{
						pFunc->m_mapDistScores[distArg]++;
#ifdef UNICODE
						pFunc->m_strLastJapaneseText = pATText->m_wszJapaneseText;
						pFunc->m_strLastKoreanText = pATText->m_wszKoreanText;
#else
						pFunc->m_strLastJapaneseText = pATText->m_szJapaneseText;
						pFunc->m_strLastKoreanText = pATText->m_szKoreanText;
#endif

						
						// ���� Ư����ġ �̻� ���ߵǾ����� �̺κ� ��ŷ
						//if( pFunc->m_mapDistScores[distArg] > 30 
						//	&& m_sContainerFunc.pfnHookCodePointEx
						//	&& m_mapArgInfoA.find(pFunc->m_ptrFunction) == m_mapArgInfoA.end()
						//	&& m_mapArgInfoW.find(pFunc->m_ptrFunction) == m_mapArgInfoW.end() )
						//{
						//	HMODULE hExeMod = GetModuleHandle(NULL);
						//	HMODULE hHookMod = NULL;

						//	if( GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pFunc->m_ptrFunction, &hHookMod)
						//		&& hExeMod == hHookMod )
						//	{
						//		PRESERVED_HOOK_POINT pRHP = new RESERVED_HOOK_POINT;
						//		pRHP->bWideChar	= pATText->m_bWideChar;
						//		pRHP->nArgDist	= distArg;
						//		pRHP->pHookPoint	= pFunc->m_ptrFunction;
						//		m_setReservedHooks.insert(pRHP);
						//		TRACE("[ aral1 ] Function 0x%p(+%d) was reserved for hook \n", pFunc->m_ptrFunction, distArg);
						//	}
						//}

					}
				}
			}
		}
		// ����
		else
		{
			for(set<pair<UINT_PTR,size_t>>::iterator iter = pATText->m_setFuncArg.begin();
				iter != pATText->m_setFuncArg.end();
				iter++)
			{
				CFunction* pFunc = m_mapFunc[iter->first];
				size_t distArg = iter->second;

				if(pFunc && distArg)
				{
					pFunc->m_mapDistScores[distArg]--;
				}
			}

			ModifyHitMap(pATText, baseESP, -1);
			m_setActivatedArgs.erase(pATText);
			m_setInactivatedArgs.insert(pATText);
		}
	}	

	return bRetVal;
}


UINT_PTR CATTextArgMgr::GetFuncAddrFromReturnAddr(UINT_PTR pAddr)
{
	UINT_PTR funcAddr = NULL;

	__try
	{
		if( !IsBadReadPtr( (void*)pAddr, sizeof(void*) ) )
		{
			// �Լ��� ������� �˻�
			BYTE* pRetAddr = (BYTE*)pAddr;
			if( 0xE8 == *(pRetAddr-5) )	// call �ڵ� case 1
			{
				UINT_PTR func_dist = *( (UINT_PTR*)(pRetAddr-4) );		// �̵��Ÿ� ���ϱ�
				funcAddr = pAddr + func_dist;							// ������ �Լ��ּ�
			}
			else if( 0xFF == *(pRetAddr-6) && 0x15 == *(pRetAddr-5) )	// call �ڵ� case 2
			{
				funcAddr = **( (UINT_PTR**)(pRetAddr-4) );				// ������ �Լ��ּ�
			}


			// �Լ� ������ ��ȿ�� �˻�
			if( funcAddr && IsBadCodePtr((FARPROC)funcAddr) )
			{
				funcAddr = NULL;
			}

		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

	return funcAddr;

}


int CATTextArgMgr::SearchStringA(INT_PTR ptrBegin, char ch1, char ch2)
{
	int iRetVal = 0;
	
	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);
		LPVOID found = NULL;

		if( IsBadStringPtrA(*ppText, 1024*1024*1024)==FALSE && strlen(*ppText) < 1024 )
		{
			if('\0'==ch2)
			{
				found = (LPVOID)strchr(*ppText, ch1);
			}
			else
			{
				char tmp[4] = {0,};
				tmp[0] = ch1;
				tmp[1] = ch2;
				found = (LPVOID)strstr(*ppText, tmp);
			}
		}

		// ���ڿ��̶��
		if(found)
		{
			int nAddRes = AddTextArg( *ppText, FALSE, (pCurFunc?pCurFunc->m_ptrFunction:NULL), arg_dist );
			if( nAddRes )
			{
				// ���� ���� ���
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

			}
		}
		else
		{
			UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

			// �Լ� �����ּҶ�� ���� ������ �Լ��� ��ü�Ѵ�
			if( funcAddr )
			{
				CFunctionMap::iterator iter = m_mapFunc.find( funcAddr );
				// ���� ����Ʈ�� �����ϸ�
				if( iter != m_mapFunc.end() )
				{
					pCurFunc = iter->second;
				}
				// ������ ���� ���� & �߰�
				else
				{
					pCurFunc = new CFunction(funcAddr);
					m_mapFunc[funcAddr] = pCurFunc;
				}
				
				arg_dist = 0;
			}
		}

		dist += sizeof(void*);
		arg_dist += sizeof(void*);
	}
	
	TRACE(" [ aral1 ] ã���Ÿ�:0x%p~0x%p (%d bytes) \n", ptrBegin, ptrBegin+dist, dist);

	return iRetVal;
}

int CATTextArgMgr::SearchStringW(INT_PTR ptrBegin, wchar_t wch)
{
	int iRetVal = 0;

	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);

		// ���ڿ��̶��
		if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE && wcslen(*ppText) < 1024 && NULL!=wcschr(*ppText, wch) )
		{

			int nAddRes = AddTextArg( *ppText, TRUE, pCurFunc->m_ptrFunction, arg_dist );
			if( nAddRes )
			{
				// ���� ���� ���
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

			}
		}
		else
		{
			UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

			// �Լ� �����ּҶ�� ���� ������ �Լ��� ��ü�Ѵ�
			if( funcAddr )
			{
				CFunctionMap::iterator iter = m_mapFunc.find( funcAddr );
				// ���� ����Ʈ�� �����ϸ�
				if( iter != m_mapFunc.end() )
				{
					pCurFunc = iter->second;
				}
				// ������ ���� ���� & �߰�
				else
				{
					pCurFunc = new CFunction(funcAddr);
					m_mapFunc[funcAddr] = pCurFunc;
				}

				arg_dist = 0;
			}
		}

		dist += sizeof(void*);
		arg_dist += sizeof(void*);

	}	

	return iRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// ���� ���� ���� ó��
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::ModifyHitMap( CATText* pATText, void* baseESP, int increment ) 
{
	// ���� ���� ����
	for(map<size_t,int>::iterator iter = m_mapHitDist.begin();
		iter != m_mapHitDist.end();)
	{
		void** ppSource = (void**)( (UINT_PTR)baseESP + iter->first );
		
		// �ؽ�Ʈ���ڿ� �ش� ���ڿ� �����Ͱ� ������
		if( IsBadReadPtr(ppSource, sizeof(void*)) == FALSE
			 && pATText->m_setSourcePtr.find(*ppSource) != pATText->m_setSourcePtr.end() )
		{
			iter->second += increment;
		}

		if( iter->second > 10 )
		{
			iter->second = 10;
			iter++;
		}
		// ���̻� �ʿ������ ����
		else if( iter->second < 0 )
		{
			size_t key = iter->first;
			iter++;
			m_mapHitDist.erase(key);
		}
		// ���� �����ؾ� �Ѵٸ�
		else
		{
			iter++;
		}
	}

}



//////////////////////////////////////////////////////////////////////////
//
// �극��ũ ����Ʈ�� �ݹ� �Լ�
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::BreakRoutine(LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters)
{
	if( CATTextArgMgr::_Inst )
	{
		CATTextArgMgr::_Inst->OnBreakPoint(pHookedPoint, pRegisters);
	}

}


//////////////////////////////////////////////////////////////////////////
//
// �극��ũ ����Ʈ ó��
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::OnBreakPoint( LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters )
{
	if( NULL==m_pMainDbgDlg || FALSE==IsWindow(m_pMainDbgDlg->GetSafeHwnd()) )
	{
		return;
	}

	// ���� ���ڼ� ����
	int nMainTextFunc = GetMainTextFunction();

	BOOL bWideChar;

	if( nMainTextFunc==UseGetGlyphOutlineA
		|| nMainTextFunc==UseTextOutA 
		|| nMainTextFunc==UseExtTextOutA 
		|| nMainTextFunc==UseDrawTextA 
		|| nMainTextFunc==UseDrawTextExA )
	{
		bWideChar = FALSE;
	}
	else
	{
		bWideChar = TRUE;
	}

	// Refresh Function List
	m_pCurBreakedPoint = (UINT_PTR)pHookedPoint;
	RefreshFunctionList();


	// UI ���� : ��������&����
	m_pMainDbgDlg->GetDlgItem(IDC_STATIC_REGNSTACK)->EnableWindow(TRUE);
	m_pMainDbgDlg->m_ctrlRegStackList.EnableWindow(TRUE);
	m_pMainDbgDlg->m_ctrlRegStackList.DeleteAllItems();
	
	AddRegAndStackDump(_T("EAX"), (UINT_PTR)pRegisters->_EAX, bWideChar);
	AddRegAndStackDump(_T("EBX"), (UINT_PTR)pRegisters->_EBX, bWideChar);
	AddRegAndStackDump(_T("ECX"), (UINT_PTR)pRegisters->_ECX, bWideChar);
	AddRegAndStackDump(_T("EDX"), (UINT_PTR)pRegisters->_EDX, bWideChar);
	AddRegAndStackDump(_T("ESI"), (UINT_PTR)pRegisters->_ESI, bWideChar);
	AddRegAndStackDump(_T("EDI"), (UINT_PTR)pRegisters->_EDI, bWideChar);
	AddRegAndStackDump(_T("EBP"), (UINT_PTR)pRegisters->_EBP, bWideChar);
	AddRegAndStackDump(_T("ESP"), (UINT_PTR)pRegisters->_ESP, bWideChar);
	
	for(int i = 0; i < 20; i++)
	{
		CString strAddr;
		strAddr.Format(_T("[ESP+%x]"), i*4 );
		AddRegAndStackDump(strAddr, *((UINT_PTR*)(pRegisters->_ESP + i*4)), bWideChar);
	}


	// �ݽ���
	m_pMainDbgDlg->GetDlgItem(IDC_STATIC_CALLSTACK)->EnableWindow(TRUE);
	m_pMainDbgDlg->m_ctrlCallstack.EnableWindow(TRUE);
	FillCallstackCtrl(pRegisters->_ESP);
	m_pMainDbgDlg->SetForegroundWindow();
	m_pMainDbgDlg->SetFocus();

	::ResetEvent(m_hResume);
	::WaitForSingleObject(m_hResume, INFINITE);

}

//////////////////////////////////////////////////////////////////////////
//
// �극��ũ �ɷȴ� ���α׷� �簳
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::OnResumeProgram()
{
	if(::IsWindow(m_pMainDbgDlg->m_hWnd))
	{
		// Refresh Function List
		m_pCurBreakedPoint = 0;
		RefreshFunctionList();

		// UI ���� : ��������&����
		m_pMainDbgDlg->GetDlgItem(IDC_STATIC_REGNSTACK)->EnableWindow(FALSE);
		m_pMainDbgDlg->m_ctrlRegStackList.EnableWindow(FALSE);

		// �ݽ���
		m_pMainDbgDlg->GetDlgItem(IDC_STATIC_CALLSTACK)->EnableWindow(FALSE);
		m_pMainDbgDlg->m_ctrlCallstack.EnableWindow(FALSE);

	}
	
	::SetEvent(m_hResume);
}

//////////////////////////////////////////////////////////////////////////
//
// ��������&���ÿ� �� �� �߰�
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::AddRegAndStackDump( LPCTSTR cszStorage, UINT_PTR val, BOOL bWideChar )
{
	if(NULL==m_pMainDbgDlg || NULL==cszStorage) return;

	int nIdx = m_pMainDbgDlg->m_ctrlRegStackList.GetItemCount();
	CString strVal;
	strVal.Format(_T("%p"), val);
	m_pMainDbgDlg->m_ctrlRegStackList.InsertItem(nIdx, cszStorage);
	m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 1, strVal);
	
	CATText attext;
	if( attext.SetATText((LPVOID)val, bWideChar, m_sContainerFunc.pfnTranslateText) )
	{
		CString strText;

#ifdef UNICODE
		LPCTSTR cszJpnText = attext.m_wszJapaneseText;
		LPCTSTR cszKorText = attext.m_wszKoreanText;
#else
		LPCTSTR cszJpnText = attext.m_szJapaneseText;
		LPCTSTR cszKorText = attext.m_szKoreanText;
#endif

		strText.Format(_T("%s\"%s\""), (bWideChar ? _T("L") : _T("")), cszJpnText);
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 2, (LPCTSTR)strText);
		
		strText.Format(_T("%s\"%s\""), (bWideChar ? _T("L") : _T("")), cszKorText);
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 3, (LPCTSTR)strText);
	}
	else
	{
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 2, _T("< Bad Text Pointer >"));
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 3, _T("< Bad Text Pointer >"));
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CATTextArgMgr::InnerGetGlyphOutlineA(
	  HDC hdc,             // handle to device context
	  UINT uChar,          // character to query
	  UINT uFormat,        // format of data to return
	  LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	  DWORD cbBuffer,      // size of buffer for data
	  LPVOID lpvBuffer,    // pointer to buffer for data
	  CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	  )
{
	char chArray[10] = {0,};
	wchar_t wchArray[10] = {0,};
	wchar_t wch = L'\0';

	// �ٷ� �� �� EBP�� ����
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	// char �迭�� ���� ����
	size_t i,j;
	j = 0;
	for(i=sizeof(UINT); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
		}
	}

	MyMultiByteToWideChar(932, 0, chArray, sizeof(UINT), wchArray, 10 );

	wch = wchArray[0];

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce && wch > 0x80)
	{			
		// �˻�
		int iRes = SearchStringA(_CUR_EBP, chArray[0], chArray[1]);
		if(iRes)
		{
			TestCharacter(wch, (void*)_CUR_EBP);
		}
	}

	m_aTextFuncHit[UseGetGlyphOutlineA]++;

	RefreshFunctionList();

	DWORD dwRetVal = 0;

	if(m_sTextFunc.pfnGetGlyphOutlineA)
	{
		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CATTextArgMgr::InnerGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	// �ٷ� �� �� EBP�� ����
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	wchar_t wch = (wchar_t)uChar;

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce && wch > 0x80 )
	{			

		// �˻�
		int iRes = SearchStringW(_CUR_EBP, wch);
		if(iRes)
		{
			TestCharacter(wch, (void*)_CUR_EBP);
		}
	}

	m_aTextFuncHit[UseGetGlyphOutlineW]++;

	RefreshFunctionList();

	DWORD dwRetVal = 0;

	if(m_sTextFunc.pfnGetGlyphOutlineW)
	{
		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::InnerTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = FALSE;

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	if( cbString<=2 || strlen(lpString)<=2 )
	{
		wchar_t wchArray[10] = {0,};
		MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

		wchar_t wch = wchArray[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && wch > 0x80 )
		{			
			// �˻�
			int iRes = SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

	}

	m_aTextFuncHit[UseTextOutA]++;

	RefreshFunctionList();

	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutA )
	{
		bRetVal = m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::InnerTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = FALSE;

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	if( cbString==1 || wcslen(lpString)==1 )
	{
		wchar_t wch = lpString[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && wch > 0x80 )
		{			
			// �˻�
			int iRes = SearchStringW(_CUR_EBP, wch);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

	}

	m_aTextFuncHit[UseTextOutW]++;

	RefreshFunctionList();

	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutW )
	{
		bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::InnerExtTextOutA(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	/*
	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnExtTextOutW )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;
	*/

	BOOL bRetVal = FALSE;

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	if( cbCount<=2 || strlen(lpString)<=2 )
	{
		wchar_t wchArray[10] = {0,};
		MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

		wchar_t wch = wchArray[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && wch > 0x80 )
		{			
			// �˻�
			int iRes = SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

	}

	m_aTextFuncHit[UseExtTextOutA]++;

	RefreshFunctionList();

	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutA )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutA(
			hdc,          // handle to DC
			X,            // x-coordinate of reference point
			Y,            // y-coordinate of reference point
			fuOptions,   // text-output options
			lprc, // optional dimensions
			lpString, // string
			cbCount,     // number of characters in string
			lpDx   // array of spacing values
			);
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::InnerExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnExtTextOutW )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}

int CATTextArgMgr::InnerDrawTextA(
   HDC hDC,          // handle to DC
   LPCSTR lpString,  // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
   )
{
	BOOL bRetVal = FALSE;

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	if( nCount<=2 || strlen(lpString)<=2 )
	{
		wchar_t wchArray[10] = {0,};
		MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

		wchar_t wch = wchArray[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && wch > 0x80 )
		{			
			// �˻�
			int iRes = SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

	}

	m_aTextFuncHit[UseDrawTextA]++;

	RefreshFunctionList();

	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutA )
	{
		bRetVal = m_sTextFunc.pfnDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}

	return bRetVal;
}

int CATTextArgMgr::InnerDrawTextW(
   HDC hDC,          // handle to DC
   LPCWSTR lpString, // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
   )
{
	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnDrawTextW )
	{
		bRetVal = m_sTextFunc.pfnDrawTextW(hDC, lpString, nCount, lpRect, uFormat);
	}

	return bRetVal;
}

int CATTextArgMgr::InnerDrawTextExA(
	 HDC hDC,                     // handle to DC
	 LPSTR lpString,              // text to draw
	 int nCount,                 // length of text to draw
	 LPRECT lpRect,                 // rectangle coordinates
	 UINT uFormat,             // formatting options
	 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	 )
{
	BOOL bRetVal = FALSE;

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	if( nCount<=2 || strlen(lpString)<=2 )
	{
		wchar_t wchArray[10] = {0,};
		MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

		wchar_t wch = wchArray[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && wch > 0x80 )
		{			
			// �˻�
			int iRes = SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

	}

	m_aTextFuncHit[UseDrawTextExA]++;

	RefreshFunctionList();

	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutA )
	{
		bRetVal = m_sTextFunc.pfnDrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}

	return bRetVal;
}

int CATTextArgMgr::InnerDrawTextExW(
	 HDC hDC,                     // handle to DC
	 LPWSTR lpString,              // text to draw
	 int nCount,                 // length of text to draw
	 LPRECT lpRect,                 // rectangle coordinates
	 UINT uFormat,             // formatting options
	 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	 )
{
	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnDrawTextExW )
	{
		bRetVal = m_sTextFunc.pfnDrawTextExW(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CATTextArgMgr::NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	DWORD dwRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		dwRetVal = g_objATTextArgMgr.InnerGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineA)
	{
		dwRetVal = g_objATTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CATTextArgMgr::NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	DWORD dwRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		dwRetVal = g_objATTextArgMgr.InnerGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineW)
	{
		dwRetVal = g_objATTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewTextOutA(
   HDC hdc,           // handle to DC
   int nXStart,       // x-coordinate of starting position
   int nYStart,       // y-coordinate of starting position
   LPCSTR lpString,   // character string
   int cbString       // number of characters
   )
{
	BOOL bRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		bRetVal = g_objATTextArgMgr.InnerTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnTextOutA)
	{
		bRetVal = g_objATTextArgMgr.m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewTextOutW(
   HDC hdc,           // handle to DC
   int nXStart,       // x-coordinate of starting position
   int nYStart,       // y-coordinate of starting position
   LPCWSTR lpString,   // character string
   int cbString       // number of characters
   )
{
	BOOL bRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		bRetVal = g_objATTextArgMgr.InnerTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnTextOutW)
	{
		bRetVal = g_objATTextArgMgr.m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewExtTextOutA(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	BOOL bRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		bRetVal = g_objATTextArgMgr.InnerExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnExtTextOutA)
	{
		bRetVal = g_objATTextArgMgr.m_sTextFunc.pfnExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{

	BOOL bRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		bRetVal = g_objATTextArgMgr.InnerExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnExtTextOutW)
	{
		bRetVal = g_objATTextArgMgr.m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}


//////////////////////////////////////////////////////////////////////////
// DrawTextA ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextA(
   HDC hDC,          // handle to DC
   LPCSTR lpString,  // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
   )
{
	int nRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		nRetVal = g_objATTextArgMgr.InnerDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnDrawTextA)
	{
		nRetVal = g_objATTextArgMgr.m_sTextFunc.pfnDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}

	return nRetVal;

}

//////////////////////////////////////////////////////////////////////////
// DrawTextW ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextW(
   HDC hDC,          // handle to DC
   LPCWSTR lpString, // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
   )
{
	int nRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		nRetVal = g_objATTextArgMgr.InnerDrawTextW(hDC, lpString, nCount, lpRect, uFormat);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnDrawTextW)
	{
		nRetVal = g_objATTextArgMgr.m_sTextFunc.pfnDrawTextW(hDC, lpString, nCount, lpRect, uFormat);
	}

	return nRetVal;

}

//////////////////////////////////////////////////////////////////////////
// DrawTextExA ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextExA(
	HDC hDC,                     // handle to DC
	LPSTR lpString,              // text to draw
	int nCount,                 // length of text to draw
	LPRECT lpRect,                 // rectangle coordinates
	UINT uFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	)
{
	int nRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		nRetVal = g_objATTextArgMgr.InnerDrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnDrawTextExA)
	{
		nRetVal = g_objATTextArgMgr.m_sTextFunc.pfnDrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);	
	}

	return nRetVal;

}


//////////////////////////////////////////////////////////////////////////
// DrawTextExW ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextExW(
	HDC hDC,                     // handle to DC
	LPWSTR lpString,              // text to draw
	int nCount,                 // length of text to draw
	LPRECT lpRect,                 // rectangle coordinates
	UINT uFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	)
{
	int nRetVal = 0;

	if(g_objATTextArgMgr.m_bRunning)
	{
		nRetVal = g_objATTextArgMgr.InnerDrawTextExW(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}
	else if(g_objATTextArgMgr.m_sTextFunc.pfnDrawTextExW)
	{
		nRetVal = g_objATTextArgMgr.m_sTextFunc.pfnDrawTextExW(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);	
	}

	return nRetVal;

}







BOOL CATTextArgMgr::Start()
{
	m_bRunning = TRUE;
	return TRUE;
}

BOOL CATTextArgMgr::Stop()
{
	m_bRunning = FALSE;
	return TRUE;
}

BOOL CATTextArgMgr::Option()
{
	//if(m_hContainerWnd && IsWindow(m_hContainerWnd))
	//{
	//	::MessageBox( m_hContainerWnd, "���� �������� �ʴ� ����Դϴ�.", "Cached Plugin", MB_OK );
	//}

	if( m_pMainDbgDlg && IsWindow(m_pMainDbgDlg->GetSafeHwnd()) )
	{
		m_pMainDbgDlg->ShowWindow(SW_SHOW);
		m_pMainDbgDlg->SetFocus();
	}
	return TRUE;
}


int CATTextArgMgr::GetMainTextFunction()
{
	int nRetVal = 0;
	
	for(int i=1; i<TEXT_FUNC_CNT; i++)
	{
		if( m_aTextFuncHit[nRetVal] < m_aTextFuncHit[i] )
		{
			nRetVal = i;
		}
	}

	return nRetVal;
}

void CATTextArgMgr::RefreshFunctionList()
{
	if(m_pMainDbgDlg && IsWindow(m_pMainDbgDlg->m_ctrlFuncList.GetSafeHwnd()))
	{
		m_pMainDbgDlg->m_ctrlFuncList.DeleteAllItems();
		int nIdx = 0;
		CString strTemp;

		// EXE ���
		HMODULE hExeMod = GetModuleHandle(NULL);

		// �Լ� ��� ��ȸ
		for( CFunctionMap::iterator iter = m_mapFunc.begin();
			iter != m_mapFunc.end();
			iter++)
		{
			CFunction* pFunc = iter->second;

			HMODULE hCurMod = NULL;
			BOOL bGetRes = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pFunc->m_ptrFunction, &hCurMod);

			if(FALSE == bGetRes || NULL == hCurMod) continue;

			// �Լ��ּ�
			if( hExeMod == hCurMod )
			{
				strTemp.Format(_T("0x%p"), pFunc->m_ptrFunction);
			}
			else
			{
				TCHAR szModPathName[MAX_PATH];
				GetModuleFileName(hCurMod, szModPathName, MAX_PATH);
				LPTSTR szModName = _tcsrchr(szModPathName, _T('\\'));
				if(szModName) szModName++;
				else szModName = szModPathName;
				strTemp.Format(_T("%s!0x%X"), szModName, pFunc->m_ptrFunction - (UINT_PTR)hCurMod);
			}

			// ��ŷ�� �Լ��� ���̶���Ʈ
			if( m_setHookedPoints.find(pFunc->m_ptrFunction) != m_setHookedPoints.end() )
			{
				//m_pMainDbgDlg->m_ctrlFuncList.SetTextBkColor(RGB(255,0,0));

				if(m_pCurBreakedPoint == pFunc->m_ptrFunction) strTemp = _T("��") + strTemp;
				else strTemp = _T("��") + strTemp;
			}
			m_pMainDbgDlg->m_ctrlFuncList.InsertItem(nIdx, (LPCTSTR)strTemp);
			m_pMainDbgDlg->m_ctrlFuncList.SetItemData(nIdx, pFunc->m_ptrFunction);

			// ��������
			strTemp = _T("");
			for( map<size_t,int>::iterator iter2 = pFunc->m_mapDistScores.begin();
				iter2 != pFunc->m_mapDistScores.end();
				iter2++)
			{
				CString strTemp2;
				strTemp2.Format(_T("[ESP+0x%x](%dhit)"), iter2->first, iter2->second);

				if(!strTemp.IsEmpty()) strTemp += _T(", ");
				strTemp += strTemp2;
			}
			m_pMainDbgDlg->m_ctrlFuncList.SetItemText(nIdx, 1, (LPCTSTR)strTemp);

			// ������ �ؽ�Ʈ
			m_pMainDbgDlg->m_ctrlFuncList.SetItemText(nIdx, 2, (LPCTSTR)pFunc->m_strLastJapaneseText);
			m_pMainDbgDlg->m_ctrlFuncList.SetItemText(nIdx, 3, (LPCTSTR)pFunc->m_strLastKoreanText);

			// �� ����Լ�(TextOutA����..) �̸� ǥ��
			// 2008.11.13 by sc.Choi
			CString strLblText;
			strLblText.Format(_T("Main Text Function : %s"), m_arrTextFuncName[GetMainTextFunction()]);
			m_pMainDbgDlg->m_lblMainTextFunc.SetWindowText(strLblText);

			nIdx++;
		}

	}
}

void CATTextArgMgr::OnSetBreakPointOnFuncList( int nIdx )
{
	if( NULL == m_pMainDbgDlg || NULL == m_sContainerFunc.pfnHookCodePointEx ) return;
	
	UINT_PTR pFunc = m_pMainDbgDlg->m_ctrlFuncList.GetItemData(nIdx);

	if( pFunc )
	{
		// ��ŷ�Ǿ� �ִ� �ڵ���
		if( m_setHookedPoints.find(pFunc) != m_setHookedPoints.end() )
		{
			// ��ŷ ����
			BOOL bUnhook = m_sContainerFunc.pfnUnhookCodePoint( (LPVOID)pFunc );
			m_setHookedPoints.erase(pFunc);
			TRACE(_T("[ aral1 ] Unhook the Code 0x%p Result : %d \n"), pFunc, bUnhook);
		}
		// ���� ��ŷ�ؾ� �� �ڵ���
		else
		{
			// ��ŷ
			BOOL bHooked = m_sContainerFunc.pfnHookCodePointEx( (LPVOID)pFunc, BreakRoutine, 0 );
			m_setHookedPoints.insert(pFunc);
			TRACE(_T("[ aral1 ] Hooking the Code 0x%p Result : %d \n"), pFunc, bHooked);
		}

		RefreshFunctionList();
	}
}

void CATTextArgMgr::FillCallstackCtrl( UINT_PTR ptrBegin )
{
	// Clear listbox control
	while(m_pMainDbgDlg->m_ctrlCallstack.GetCount())
	{
		m_pMainDbgDlg->m_ctrlCallstack.DeleteString(0);
	}

	// Create current callstack
	size_t dist = 0;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{

		UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

		// �Լ� �����ּҶ�� ���� ������ �Լ��� ��ü�Ѵ�
		if( funcAddr )
		{
			CString strFuncAddr;
			strFuncAddr.Format(_T("0x%p"), funcAddr);
			m_pMainDbgDlg->m_ctrlCallstack.AddString(strFuncAddr);
		}

		dist += sizeof(void*);
	}

}
