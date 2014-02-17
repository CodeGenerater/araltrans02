
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "Cached.h"
#include "CachedTextArgMgr.h"
#include "CachedTextArg.h"
#include "CharacterMapper.h"
#include "Function.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "AralProcessAPI/AralProcessAPI.h"

#define TEXT_ARG_POOL_SIZE 100

extern CCachedTextArgMgr g_objCachedTextArgMgr;
CCachedTextArgMgr*	CCachedTextArgMgr::_Inst = NULL;


CCachedTextArgMgr::CCachedTextArgMgr(void)
	: m_hFont(NULL), m_distBest(0)
{
	_Inst = this;
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_FUNCTION_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
}


CCachedTextArgMgr::~CCachedTextArgMgr(void)
{
	_Inst = NULL;
	Close();
}

// �ʱ�ȭ
BOOL CCachedTextArgMgr::Init(HWND hSettingWnd, LPSTR cszOptionStringBuffer) 
{
	Close();

	BOOL bRetVal = FALSE;

	// �θ� ������ �ڵ� ����
	if(NULL==hSettingWnd) return FALSE;
	m_hContainerWnd = hSettingWnd;

	// �����̳� �Լ� ������ ������
	m_hContainer = GetModuleHandle("ATCTNR.DLL");
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

		// ���÷����� ���� �Լ�
		m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD("HKEY_CURRENT_USER\\Software\\AralGood", "M2WAddr");

		m_sTextFunc.pfnOrigWideCharToMultiByte =
			(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD("HKEY_CURRENT_USER\\Software\\AralGood", "W2MAddr");

		if( m_sTextFunc.pfnExtTextOutA && m_sTextFunc.pfnExtTextOutW
			&& m_sTextFunc.pfnGetGlyphOutlineA && m_sTextFunc.pfnGetGlyphOutlineW
			&& m_sTextFunc.pfnOrigMultiByteToWideChar && m_sTextFunc.pfnOrigWideCharToMultiByte
			&& m_sTextFunc.pfnTextOutA && m_sTextFunc.pfnTextOutW )
		{
			// ���÷����� �˻�
			HMODULE hApplocale = GetModuleHandleA("AlLayer.dll");
			if(hApplocale)
			{
				m_bApplocale = TRUE;
			}

			// �ؽ�Ʈ ���� Ǯ ����
			for(int i=0; i<TEXT_ARG_POOL_SIZE; i++)
			{
				m_setInactivatedArgs.insert(new CCachedText());
			}

			bRetVal = TRUE;
		}

		// ��� ������ �簡��
		m_sContainerFunc.pfnResumeAllThread();
	}

	if( FALSE == bRetVal ) Close();

	return bRetVal;
}

// ����ȭ
BOOL CCachedTextArgMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

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

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CCachedText* pCachedText = *(iter);
		delete pCachedText;
	}
	m_setActivatedArgs.clear();
	
	// ��Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CCachedTextSet::iterator iter = m_setInactivatedArgs.begin();
		iter != m_setInactivatedArgs.end();
		iter++)
	{
		CCachedText* pCachedText = *(iter);
		delete pCachedText;
	}
	m_setInactivatedArgs.clear();

	// ������ �Ÿ��� ����
	m_mapHitDist.clear();
	m_distBest = 0;

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
	for( CArgInfoMap::iterator iter3 = m_mapArgInfoA.begin();
		iter3 != m_mapArgInfoA.end();
		iter3++)
	{
		m_sContainerFunc.pfnUnhookCodePoint( (LPVOID)iter3->first );
	}
	m_mapArgInfoA.clear();

	for( CArgInfoMap::iterator iter4 = m_mapArgInfoW.begin();
		iter4 != m_mapArgInfoW.end();
		iter4++)
	{
		m_sContainerFunc.pfnUnhookCodePoint( (LPVOID)iter4->first );
	}
	m_mapArgInfoW.clear();
	

	// ��ŷ����迭 ����
	m_setReservedHooks.clear();

	// ��Ÿ ���� ����
	m_bApplocale = FALSE;
	m_hContainerWnd = NULL;

	// ��� ������ �簡��
	m_sContainerFunc.pfnResumeAllThread();

	return TRUE;

}



// ���ο� ���ڿ� �ĺ��� �߰��Ѵ�
int CCachedTextArgMgr::AddTextArg(LPVOID pText, BOOL bWideChar, BOOL bAutoTrans, UINT_PTR ptrFunc, size_t dist)
{
	BOOL nRetVal = 0;

	CCachedText* pCachedText = NULL;
	
	// ������ ����ִ� ���ڿ����� �˻� (Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ)
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CCachedText* pTmpCachedText = *(iter);

		if( pTmpCachedText->IsWideCharacter() == bWideChar && pTmpCachedText->TestText(pText) )
		{
			pCachedText = pTmpCachedText;
			nRetVal = 2;
		}		
	}		


	// ������ �ʿ��ϸ� ���� �Ǵ� �־��� ��带 ������ �߰��Ѵ�
	if( NULL == pCachedText)
	{	
		if(!m_setInactivatedArgs.empty()) 
		{
			CCachedTextSet::iterator iter = m_setInactivatedArgs.begin();
			pCachedText = ( *iter );
			m_setInactivatedArgs.erase(pCachedText);
			m_setActivatedArgs.insert(pCachedText);
		}
		else
		{
			// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
			for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
				iter != m_setActivatedArgs.end();
				iter++)
			{
				CCachedText* pTmpCachedText = *(iter);

				if( NULL == pCachedText || (pCachedText->GetHitCount() > pTmpCachedText->GetHitCount()) )
				{
					pCachedText = pTmpCachedText;
				}		
			}		
		}
			

		if( pCachedText->SetCachedText(pText, bWideChar, m_sContainerFunc.pfnTranslateText) )
		{
			nRetVal = 1;
		}
		else
		{
			m_setActivatedArgs.erase( pCachedText );
			m_setInactivatedArgs.insert(pCachedText);
			pCachedText = NULL;
		}
	}

	// ����� �����̸� 
	if( nRetVal!=0 )
	{
		// ��� ���� ���� �߰�
		if( ptrFunc && dist )
		{
			pCachedText->m_setFuncArg.insert( pair<UINT_PTR,size_t>(ptrFunc,dist) );
		}

		// ������ �ʿ��ϸ� ����
		if( bAutoTrans )
		{
			pCachedText->Translate();
		}

		wchar_t dbg[2048];
		swprintf(dbg, L"[ aral1 ] %c[0x%p] '%s'('%s') \n",
			(pCachedText->m_bTranslated ? L'��' : L'��'),
			pText,
			pCachedText->m_wszJapaneseText, 
			pCachedText->m_wszKoreanText
			);
		//OutputDebugStringW(dbg);

	}

	return nRetVal;
}

// ���ڿ� �ĺ��� ��ü�� �׽�Ʈ�Ѵ�. (���̻� ��ġ���� �ʴ� �ĺ��� �ٷ� ����)
BOOL CCachedTextArgMgr::TestCharacter(wchar_t wch, void* baseESP)
{
	BOOL bRetVal = FALSE;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ��� ��� ��ȸ
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CCachedText* pCachedText = (*iter);
		iter++;
		
		// �˻� ����
		int nRes = pCachedText->TestCharacter(wch);
		
		// ��Ʈ(����)��
		if( 0 != nRes )
		{
			if( nRes & 0x01 )
			{
				ModifyHitMap(pCachedText, baseESP, +1);
				bRetVal = TRUE;

				for(set<pair<UINT_PTR,size_t>>::iterator iter = pCachedText->m_setFuncArg.begin();
					iter != pCachedText->m_setFuncArg.end();
					iter++)
				{
					CFunction* pFunc = m_mapFunc[iter->first];
					size_t distArg = iter->second;
					
					if(pFunc && distArg)
					{
						pFunc->m_mapDistScores[distArg]++;
						
						// ���� Ư����ġ �̻� ���ߵǾ����� �̺κ� ��ŷ
						if( pFunc->m_mapDistScores[distArg] > 30 
							&& m_sContainerFunc.pfnHookCodePointEx
							&& m_mapArgInfoA.find(pFunc->m_ptrFunction) == m_mapArgInfoA.end()
							&& m_mapArgInfoW.find(pFunc->m_ptrFunction) == m_mapArgInfoW.end() )
						{
							HMODULE hExeMod = GetModuleHandle(NULL);
							HMODULE hHookMod = NULL;

							if( GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pFunc->m_ptrFunction, &hHookMod)
								&& hExeMod == hHookMod )
							{
								/*
								PRESERVED_HOOK_POINT pRHP = new RESERVED_HOOK_POINT;
								pRHP->bWideChar	= pCachedText->m_bWideChar;
								pRHP->nArgDist	= distArg;
								pRHP->pHookPoint	= pFunc->m_ptrFunction;
								m_setReservedHooks.insert(pRHP);
								TRACE("[ aral1 ] Function 0x%p(+%d) was reserved for hook \n", pFunc->m_ptrFunction, distArg);
								*/
							}
						}

					}
				}
			}
		}
		// ����
		else
		{
			//// ���� ���� ����
			//for(set<void*>::iterator iter = pCachedText->m_setSourcePtr.begin();
			//	iter != pCachedText->m_setSourcePtr.end();
			//	iter++)
			//{
			//	void* pSource = (*iter);
			//	size_t dist = (size_t)pSource - (size_t)baseESP;
			//	ModifyHitMap(dist, -1);
			//}
			for(set<pair<UINT_PTR,size_t>>::iterator iter = pCachedText->m_setFuncArg.begin();
				iter != pCachedText->m_setFuncArg.end();
				iter++)
			{
				CFunction* pFunc = m_mapFunc[iter->first];
				size_t distArg = iter->second;

				if(pFunc && distArg)
				{
					pFunc->m_mapDistScores[distArg]--;
				}
			}

			ModifyHitMap(pCachedText, baseESP, -1);
			m_setActivatedArgs.erase(pCachedText);
			m_setInactivatedArgs.insert(pCachedText);
		}
	}	

	return bRetVal;
}


UINT_PTR CCachedTextArgMgr::GetFuncAddrFromReturnAddr(UINT_PTR pAddr)
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


int CCachedTextArgMgr::SearchStringA(INT_PTR ptrBegin, char ch1, char ch2)
{
	int iRetVal = 0;

	// ���ڰŸ��� ����
	//FindBestDistAndClearHitMap();
	
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
			int nAddRes = AddTextArg( *ppText, FALSE, IsAutoTransPoint(dist), (pCurFunc?pCurFunc->m_ptrFunction:NULL), arg_dist );
			if( nAddRes )
			{
				// ���� ���� ���
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

				//// �Լ� ���� ����
				//if( pCurFunc )
				//{
				//	pCurFunc->m_mapDistScores[arg_dist]++;
				//}

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
	
	//char dbg[1024];
	//sprintf(dbg, " [ aral1 ] ã���Ÿ�:0x%p~0x%p (%d bytes) \n", ptrBegin, ptrBegin+dist, dist); OutputDebugStringA(dbg);

	return iRetVal;
}

int CCachedTextArgMgr::SearchStringW(INT_PTR ptrBegin, wchar_t wch)
{
	int iRetVal = 0;

	// ���ڰŸ��� ����
	//FindBestDistAndClearHitMap();

	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);

		// ���ڿ��̶��
		if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE && wcslen(*ppText) < 1024 && NULL!=wcschr(*ppText, wch) )
		{

			int nAddRes = AddTextArg( *ppText, TRUE, IsAutoTransPoint(dist), pCurFunc->m_ptrFunction, arg_dist );
			if( nAddRes )
			{
				// ���� ���� ���
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

				//// �Լ� ���� ����
				//if( pCurFunc )
				//{
				//	pCurFunc->m_mapDistScores[arg_dist]++;
				//}

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


void CCachedTextArgMgr::CheckFont(HDC hdc) 
{
	static long lLastFontHeight = -1;

	TEXTMETRIC tm;
	//ZeroMemory(&tm, sizeof(TEXTMETRIC));
	BOOL bRes = GetTextMetrics(hdc, &tm);

	// ��Ʈ �ٽ� �ε�
	if( bRes )//&& tm.tmHeight != lLastFontHeight )
	{
		HFONT font = NULL;

		// �� ũ�⿡ �ش��ϴ� ��Ʈ�� ���� ��� ��Ʈ�� ����
		if( m_mapFonts.find(tm.tmHeight) == m_mapFonts.end() )
		{
			font = CreateFont(tm.tmHeight, 0, 0, 0, tm.tmWeight, tm.tmItalic, tm.tmUnderlined, tm.tmStruckOut,
				HANGEUL_CHARSET,	//ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,
				DEFAULT_PITCH,		// | FF_SWISS,
				"����");

			m_mapFonts[tm.tmHeight] = font;
			
		}
		else
		{
			font = m_mapFonts[tm.tmHeight];
		}

		SelectObject(hdc, font);
		lLastFontHeight = tm.tmHeight;

		ZeroMemory(&tm, sizeof(TEXTMETRIC));
		GetTextMetrics(hdc, &tm);
		hdc = hdc;

	}

}


// �ְ�� Ȯ���� ���� ���� ���ڸ� ��ȯ
wchar_t CCachedTextArgMgr::GetBestTranslatedCharacter() 
{
	wchar_t wchRetVal = L'\0';
	CCachedText* pBestArg = NULL;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
	for(CCachedTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CCachedText* pCachedTextArg = *(iter);

		if( NULL == pBestArg )
		{
			if( 0 < pCachedTextArg->GetHitCount() ) pBestArg = pCachedTextArg;
		}
		else if( pBestArg->m_bTranslated < pCachedTextArg->m_bTranslated )
		{
			pBestArg = pCachedTextArg;
		}
		else if( pBestArg->m_nHitCnt < pCachedTextArg->m_nHitCnt )
		{
			pBestArg = pCachedTextArg;
		}
		else if( pBestArg->m_nHitCnt == pCachedTextArg->m_nHitCnt )
		{
			size_t nBestRef = pBestArg->m_setSourcePtr.size();
			size_t nTempRef = pCachedTextArg->m_setSourcePtr.size();
			if( nBestRef < nTempRef )
			{
				pBestArg = pCachedTextArg;				
			}
			else if( nBestRef == nTempRef && pBestArg->m_nJapaneseLen < pCachedTextArg->m_nJapaneseLen )
			{
				pBestArg = pCachedTextArg;
			}
		}
	}

	if(pBestArg)
	{
		wchRetVal = pBestArg->GetBestTranslatedCharacter();
		//wchar_t dbg[1024];
		//swprintf(dbg, L"[ aral1 ] GetBestTranslatedCharacter() returned '%c' ('%s'[%d]) \n", (wchRetVal ? wchRetVal:L'0'), pBestArg->m_wszKoreanText, pBestArg->m_nNextTestIdx-1);
		//OutputDebugStringW(dbg);
	}

	return wchRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ���� ���� ���� ó��
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::ModifyHitMap( CCachedText* pCachedText, void* baseESP, int increment ) 
{
	// ���� ���� ����
	for(map<size_t,int>::iterator iter = m_mapHitDist.begin();
		iter != m_mapHitDist.end();)
	{
		void** ppSource = (void**)( (UINT_PTR)baseESP + iter->first );
		
		// �ؽ�Ʈ���ڿ� �ش� ���ڿ� �����Ͱ� ������
		if( IsBadReadPtr(ppSource, sizeof(void*)) == FALSE
			 && pCachedText->m_setSourcePtr.find(*ppSource) != pCachedText->m_setSourcePtr.end() )
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


	//map<size_t,int>::iterator iter = m_mapHitDist.find(dist);
	//
	//// �ű� �߰��� �ʿ��ϸ�
	//if( iter == m_mapHitDist.end() )
	//{
	//	if( increment > 0 ) m_mapHitDist.insert( pair<size_t,int>(dist, increment) );
	//}
	//// ���� �� ���۽�
	//else
	//{
	//	iter->second += increment;
	//	if( iter->second <= 0 )
	//	{
	//		m_mapHitDist.erase(iter);
	//	}
	//}
}


void CCachedTextArgMgr::FindBestDistAndClearHitMap()
{
	int nBestVal = 0;

	// �ְ� ���� ����
	for(map<size_t,int>::iterator iter = m_mapHitDist.begin();
		iter != m_mapHitDist.end();
		iter++)
	{
		if( iter->second > nBestVal && iter->second > 2 )
		{
			m_distBest = iter->first;
			nBestVal = iter->second;
		}
	}

	m_mapHitDist.clear();

}

//////////////////////////////////////////////////////////////////////////
//
// �ڵ� ������������ �Ǵ��Ѵ�
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::IsAutoTransPoint( size_t dist ) 
{
	BOOL bRetVal = FALSE;

	map<size_t,int>::iterator iter = m_mapHitDist.find( dist );
	if( iter != m_mapHitDist.end() && iter->second > 3 )
	{
		bRetVal = TRUE;
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// Ư������ ��ŷ�� �ݹ� �Լ� (��Ƽ����Ʈ��)
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::ModifyValueA(LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters)
{
	if(NULL==CCachedTextArgMgr::_Inst) return;

	// �������� ���̺��� ���� ���ϱ�
	CArgInfoMap::iterator iter = CCachedTextArgMgr::_Inst->m_mapArgInfoA.find((UINT_PTR)pHookedPoint);
	if( iter != CCachedTextArgMgr::_Inst->m_mapArgInfoA.end() )
	{
		size_t dist = iter->second;
		LPSTR* pArgText = (LPSTR*)( pRegisters->_ESP + dist );		// �ؽ�Ʈ ���� ������ ���ϱ�
		if( CCachedTextArgMgr::_Inst && IsBadReadPtr(pArgText, sizeof(LPSTR)) == FALSE 
			&& IsBadStringPtrA(*pArgText, 1024*1024*1024) == FALSE )
		{
			LPSTR pText = *pArgText;
			char ch1 = pText[0];
			char ch2 = pText[1];
			int nRes = CCachedTextArgMgr::_Inst->AddTextArg( pText, FALSE, TRUE, NULL, NULL );
			if( nRes == 1 )
			{
				CCachedTextArgMgr::_Inst->SearchStringA((INT_PTR)pRegisters->_ESP, ch1, ch2 );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Ư������ ��ŷ�� �ݹ� �Լ� (�����ڵ��)
//
//////////////////////////////////////////////////////////////////////////
void CCachedTextArgMgr::ModifyValueW(LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters)
{
	if(NULL==CCachedTextArgMgr::_Inst) return;

	// �������� ���̺��� ���� ���ϱ�
	CArgInfoMap::iterator iter = CCachedTextArgMgr::_Inst->m_mapArgInfoW.find((UINT_PTR)pHookedPoint);
	if( iter != CCachedTextArgMgr::_Inst->m_mapArgInfoW.end() )
	{
		size_t dist = iter->second;
		LPWSTR* pArgText = (LPWSTR*)( pRegisters->_ESP + dist );		// �ؽ�Ʈ ���� ������ ���ϱ�
		if( CCachedTextArgMgr::_Inst && IsBadReadPtr(pArgText, sizeof(LPWSTR)) == FALSE 
			&& IsBadStringPtrW(*pArgText, 1024*1024*1024) == FALSE )
		{
			LPWSTR pText = *pArgText;
			wchar_t wch = pText[0];
			int nRes = CCachedTextArgMgr::_Inst->AddTextArg( pText, TRUE, TRUE, NULL, NULL );
			if( nRes == 1 )
			{
				CCachedTextArgMgr::_Inst->SearchStringW((INT_PTR)pRegisters->_ESP, wch );
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CCachedTextArgMgr::InnerGetGlyphOutlineA(
	  HDC hdc,             // handle to device context
	  UINT uChar,          // character to query
	  UINT uFormat,        // format of data to return
	  LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	  DWORD cbBuffer,      // size of buffer for data
	  LPVOID lpvBuffer,    // pointer to buffer for data
	  CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	  )
{

	BOOL bDecoded = FALSE;
	char chArray[10] = {0,};
	char tmpbuf[10]  = {0,};
	wchar_t wchArray[10] = {0,};
	wchar_t wch = L'\0';

	CheckFont(hdc);

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

	// �ѱ� ���ڵ� ���� �õ�
	if( 0xE0 <= (BYTE)chArray[0] && (BYTE)chArray[0] <= 0xFC )
	{
		chArray[2] = '\0';

		if( CCharacterMapper::DecodeJ2K(chArray, tmpbuf) )
		{
			MyMultiByteToWideChar(949, 0, tmpbuf, sizeof(UINT), wchArray, 10 );
			bDecoded = TRUE;
		}

	}

	// ���ڵ� ó���� ���ڰ� �ƴ϶��
	if( FALSE == bDecoded )
	{
		MyMultiByteToWideChar(932, 0, chArray, sizeof(UINT), wchArray, 10 );
	}

	wch = wchArray[0];

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce && FALSE == bDecoded && wch > 0x80)
	{			
		// ����� ��ŷ�۾� ����
		HookAllReservedPoints();

		// �˻�
		int iRes = SearchStringA(_CUR_EBP, chArray[0], chArray[1]);
		if(iRes)
		{
			TestCharacter(wch, (void*)_CUR_EBP);
		}
		
		//sprintf(dbg, "[ aral1 ] ============== �� �˻� ��� : %d ================= \n", iRes ); OutputDebugStringA(dbg);
	}

	//wchar_t dbg[MAX_PATH];
	//swprintf(dbg, L"[ aral1 ] NewTextOutA���� ĳ�� : '%c'( %2X %2X ) \n", wch, lpString[0], lpString[1]);
	//OutputDebugStringW(dbg);


	// ���� ��ȯ
	if(bDecoded)
	{
		uChar = (UINT)(MAKEWORD(tmpbuf[1], tmpbuf[0]));
	}
	else
	{
		wchar_t wtmp[2] = {0,};
		wtmp[0] = GetBestTranslatedCharacter();
		if(wtmp[0])
		{
			MyWideCharToMultiByte(949, 0, wtmp, -1, tmpbuf, 10, NULL, NULL);
			//sprintf(dbg, "[ aral1 ] GetBestTranslatedCharacter() returned (%p) -> (%p)'%s' \n", MAKEWORD(chArray[1], chArray[0]), MAKEWORD(tmpbuf[1], tmpbuf[0]), tmpbuf ); OutputDebugStringA(dbg);
			if(tmpbuf[1])
			{
				uChar = (UINT)(MAKEWORD(tmpbuf[1], tmpbuf[0]));
			}
			else
			{
				uChar = (UINT)tmpbuf[0];
			}
		}
	}


	// �����Լ� ȣ��
	DWORD dwRetVal = 0;

	if( m_sTextFunc.pfnGetGlyphOutlineA )
	{
		//if(uChar<=0x80)
		//{
		//	uFormat = GGO_NATIVE;
		//}

		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CCachedTextArgMgr::InnerGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	CheckFont(hdc);

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
		// ����� ��ŷ�۾� ����
		HookAllReservedPoints();

		// �˻�
		int iRes = SearchStringW(_CUR_EBP, wch);
		if(iRes)
		{
			TestCharacter(wch, (void*)_CUR_EBP);
		}
		TRACE("[ aral1 ] ============== �� �˻� ��� : %d ================= \n", iRes );			
	}

	//wchar_t dbg[MAX_PATH];
	//swprintf(dbg, L"[ aral1 ] NewTextOutA���� ĳ�� : '%c'( %2X %2X ) \n", wch, lpString[0], lpString[1]);
	//OutputDebugStringW(dbg);

	wch = GetBestTranslatedCharacter();
	if(wch)
	{
		//wchar_t dbg[MAX_PATH];
		//swprintf(dbg, L"[ aral1 ] MyGetGlyphOutlineW : '%c'->'%c' \n", (wchar_t)uChar, wch);
		//OutputDebugStringW(dbg);
		uChar = (UINT)wch;
	}
	else
	{
		uChar = 0x20;
	}

	// �����Լ� ȣ��
	DWORD dwRetVal = 0;

	if( m_sTextFunc.pfnGetGlyphOutlineW )
	{
		if(uChar<=0x80)
		{
			uFormat = GGO_NATIVE;
		}

		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::InnerTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	char tmpbuf[10];

	CheckFont(hdc);

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

		// �ѱ� ���ڵ� ���� �õ�
		if( 0xE0 <= (BYTE)lpString[0] && (BYTE)lpString[0] <= 0xFC )
		{
			tmpbuf[2] = '\0';

			if( CCharacterMapper::DecodeJ2K(lpString, tmpbuf) )
			{
				MyMultiByteToWideChar(949, 0, tmpbuf, sizeof(UINT), wchArray, 10 );
				bDecoded = TRUE;
			}
			
		}

		// ���ڵ� ó���� ���ڰ� �ƴ϶��
		if( FALSE == bDecoded )
		{
			MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );
		}

		wchar_t wch = wchArray[0];

		// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
		BOOL bHitOnce = TestCharacter(wch, (void*)_CUR_EBP);

		// ���ߵ� �����Ͱ� ���ٸ� �˻�
		if( FALSE == bHitOnce && FALSE == bDecoded && wch > 0x80 )
		{			
			// ����� ��ŷ�۾� ����
			HookAllReservedPoints();
			
			// �˻�
			TRACE("[ aral1 ] \n");			
			TRACE("[ aral1 ] \n");						
			//wchar_t dbg[MAX_PATH];
			//swprintf(dbg, L"[ aral1 ] ============== �� �˻� ���� ('%c') ================= \n", wch);
			//OutputDebugStringW(dbg);
			int iRes = SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
			if(iRes)
			{
				TestCharacter(wch, (void*)_CUR_EBP);
			}
			TRACE("[ aral1 ] ============== �� �˻� ��� : (%d/%d) ================= \n", m_setActivatedArgs.size(), iRes );
		}


		// ���� ��ȯ
		wchar_t wtmp[2] = {0,};
		if( bDecoded )
		{
			wtmp[0] = wch;
		}
		else
		{
			wtmp[0] = GetBestTranslatedCharacter();
			if( L'\0' == wtmp[0] ) wtmp[0] = L' ';
		}

		wchar_t dbg2[MAX_PATH];
		if( bDecoded )
		{
			wcscpy(dbg2, L"Decoded");
		}
		else if( wtmp[0] )
		{
			wcscpy(dbg2, L"GetBestCharacter");
		}
		else
		{
			wcscpy(dbg2, L"failed..");
		}
		//swprintf(dbg, L"[ aral1 ] NewTextOutA : '%c' -> '%c' (%s) \n", wch, wtmp[0], dbg2 );
		//OutputDebugStringW(dbg);

		//lpString = tmpbuf;

		// �����Լ� ȣ��
		if( m_sTextFunc.pfnTextOutW )
		{
			bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, wtmp, 1);
		}
	}
	else
	{
		// �����Լ� ȣ��
		if( m_sTextFunc.pfnTextOutA )
		{
			bRetVal = m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
		}

	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::InnerTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnTextOutW )
	{
		bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::InnerExtTextOutA(
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
	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnExtTextOutW )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::InnerExtTextOutW(
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
	LPCWSTR lpNewString = lpString;
	//wchar_t wszNewString[1024];

	/*
	if( lpString && lpString[0] )
	{

	// m_hLastDC �� �޶����ٸ�
	if(hdc != _gInst->m_hLastDC)
	{
	_gInst->m_hLastDC = hdc;
	}

	if( NULL==_gInst->m_hFont )
	{
	_gInst->m_hFont=CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	ANSI_CHARSET,
	OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS,
	NONANTIALIASED_QUALITY,
	DEFAULT_PITCH | FF_SWISS,
	"�ü�");

	}

	SelectObject(hdc, _gInst->m_hFont);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
	mov _CUR_EBP, ebp;
	}


	size_t len = wcslen(lpString);
	if(cbCount==1)
	{
	wchar_t wch = lpString[0];

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = _gInst->m_TextArgMgr.TestCharacter(wch);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce )
	{
	size_t dist = 0;
	while( IsBadReadPtr((void*)(_CUR_EBP+dist), sizeof(void*)) == FALSE )
	{
	LPWSTR* ppText = (LPWSTR*)(_CUR_EBP+dist);
	if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE && **ppText == wch )
	{
	_gInst->m_TextArgMgr.AddTextArg(*ppText);
	}
	dist += sizeof(void*);
	}
	}

	// �ְ��� ���� ���ڸ� ����
	if( _gInst->m_TextArgMgr.GetBestTranslatedCharacter(wszNewString) )
	{
	wchar_t dbg[MAX_PATH];
	swprintf(dbg, L"[ aral1 ] MyExtTextOutW : '%c'->'%s' \n", wch, wszNewString);
	OutputDebugStringW(dbg);

	lpNewString = wszNewString;
	cbCount = wcslen(wszNewString);

	//PROC_TextOut pFunc = (PROC_TextOut) CRawHookMgr::GetOrigFunc(MyTextOutA);
	//return pFunc(hdc, nXStart, nYStart, szNewString, strlen(szNewString));
	}
	}

	else if(len < 50 && 0)
	{
	// ����
	ITranslator* pTranslator = _gInst->m_TextArgMgr.GetTranslator();
	if(pTranslator)
	{
	// �����⿡ ����־��
	char szJapanese[1024] = {0,};
	::WideCharToMultiByte(932, 0, lpString, -1, szJapanese, 1023, NULL, NULL);

	CString strKorean;
	BOOL bTrans = pTranslator->TransJ2K(szJapanese, strKorean);

	if( bTrans )
	{
	::MultiByteToWideChar(949, 0, (LPCSTR)strKorean, -1, wszNewString, 1023 );
	lpNewString = wszNewString;
	cbCount = wcslen(wszNewString);

	wchar_t dbg[MAX_PATH];
	swprintf(dbg, L"[ aral1 ] MyExtTextOutW : '%s'->'%s' \n", lpString, wszNewString);
	OutputDebugStringW(dbg);
	}

	}

	}

	}
	*/

	// �����Լ� ȣ��
	BOOL bRetVal = 0;

	if( m_sTextFunc.pfnExtTextOutW )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpNewString, cbCount, lpDx);
	}

	return bRetVal;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CCachedTextArgMgr::NewGetGlyphOutlineA(
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

	if(g_objCachedTextArgMgr.m_bRunning)
	{
		dwRetVal = g_objCachedTextArgMgr.InnerGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else if(g_objCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineA)
	{
		dwRetVal = g_objCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CCachedTextArgMgr::NewGetGlyphOutlineW(
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

	if(g_objCachedTextArgMgr.m_bRunning)
	{
		dwRetVal = g_objCachedTextArgMgr.InnerGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else if(g_objCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineW)
	{
		dwRetVal = g_objCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewTextOutA(
									   HDC hdc,           // handle to DC
									   int nXStart,       // x-coordinate of starting position
									   int nYStart,       // y-coordinate of starting position
									   LPCSTR lpString,   // character string
									   int cbString       // number of characters
									   )
{
	BOOL bRetVal = 0;

	if(g_objCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objCachedTextArgMgr.InnerTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}
	else if(g_objCachedTextArgMgr.m_sTextFunc.pfnTextOutA)
	{
		bRetVal = g_objCachedTextArgMgr.m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewTextOutW(
									   HDC hdc,           // handle to DC
									   int nXStart,       // x-coordinate of starting position
									   int nYStart,       // y-coordinate of starting position
									   LPCWSTR lpString,   // character string
									   int cbString       // number of characters
									   )
{
	BOOL bRetVal = 0;

	if(g_objCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objCachedTextArgMgr.InnerTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}
	else if(g_objCachedTextArgMgr.m_sTextFunc.pfnTextOutW)
	{
		bRetVal = g_objCachedTextArgMgr.m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewExtTextOutA(
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

	if(g_objCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objCachedTextArgMgr.InnerExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	else if(g_objCachedTextArgMgr.m_sTextFunc.pfnExtTextOutA)
	{
		bRetVal = g_objCachedTextArgMgr.m_sTextFunc.pfnExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedTextArgMgr::NewExtTextOutW(
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

	if(g_objCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objCachedTextArgMgr.InnerExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	else if(g_objCachedTextArgMgr.m_sTextFunc.pfnExtTextOutW)
	{
		bRetVal = g_objCachedTextArgMgr.m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}

BOOL CCachedTextArgMgr::Start()
{
	m_bRunning = TRUE;
	return TRUE;
}

BOOL CCachedTextArgMgr::Stop()
{
	m_bRunning = FALSE;
	return TRUE;
}

BOOL CCachedTextArgMgr::Option()
{
	if(m_hContainerWnd && IsWindow(m_hContainerWnd))
	{
		::MessageBox( m_hContainerWnd, "���� �������� �ʴ� ����Դϴ�.", "Cached Plugin", MB_OK );
	}
	return TRUE;
}

void CCachedTextArgMgr::HookAllReservedPoints()
{
	for(CReservedHooks::iterator iter = m_setReservedHooks.begin();
		iter != m_setReservedHooks.end();
		iter++)
	{

		PRESERVED_HOOK_POINT pRHP = (*iter);
		
		// �����ڵ��� ���
		if(pRHP->bWideChar)
		{

			BOOL bHooked = m_sContainerFunc.pfnHookCodePointEx( (LPVOID)pRHP->pHookPoint, ModifyValueW,0 );

			if( bHooked )
			{
				m_mapArgInfoW.insert( CArgInfo(pRHP->pHookPoint, pRHP->nArgDist) );		
				TRACE("[ aral1 ] Function 0x%p(+%d) was hooked as unicode text \n", pRHP->pHookPoint, pRHP->nArgDist);
			}

		}
		// ��Ƽ����Ʈ�� ���
		else
		{

			BOOL bHooked = m_sContainerFunc.pfnHookCodePointEx( (LPVOID)pRHP->pHookPoint, ModifyValueA,0 );

			if( bHooked )
			{
				m_mapArgInfoA.insert( CArgInfo(pRHP->pHookPoint, pRHP->nArgDist) );		
				TRACE("[ aral1 ] Function 0x%p(+%d) was hooked as multibyte text \n", pRHP->pHookPoint, pRHP->nArgDist);
			}

		}

		delete pRHP;
	}

	m_setReservedHooks.clear();


}