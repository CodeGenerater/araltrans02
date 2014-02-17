
#pragma warning(disable:4312)
#pragma warning(disable:4996)

#include "NonCached.h"
#include "NonCachedTextArgMgr.h"
#include "NonCachedTextArg.h"
#include "RegistryMgr/cRegistryMgr.h"

#include "Misc.h"
#include "OptionDialog.h"

#define TEXT_ARG_POOL_SIZE 100


extern CNonCachedTextArgMgr g_objNonCachedTextArgMgr;


int MyWideCharToMultiByte(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCWSTR lpWideCharStr, 
	int cchWideChar, 
	LPSTR lpMultiByteStr, 
	int cbMultiByte, 
	LPCSTR lpDefaultChar, 
	LPBOOL lpUsedDefaultChar 
);

int MyMultiByteToWideChar(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCSTR lpMultiByteStr, 
	int cbMultiByte, 
	LPWSTR lpWideCharStr, 
	int cchWideChar 
);



CNonCachedTextArgMgr::CNonCachedTextArgMgr(void)
	: m_hLastDC(NULL), m_hFont(NULL), m_hContainerWnd(NULL), m_pszOptionStringBuffer(NULL)
{
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_FUNCTION_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
}

CNonCachedTextArgMgr::~CNonCachedTextArgMgr(void)
{
	Close();
}

// �ʱ�ȭ
BOOL CNonCachedTextArgMgr::Init(HWND hSettingWnd, LPSTR cszOptionStringBuffer) 
{
	Close();

	BOOL bRetVal = FALSE;

	// �θ� ������ �ڵ� ����
	if(NULL==hSettingWnd) return FALSE;
	m_hContainerWnd = hSettingWnd;

	// �ɼ� ���� ��ġ ����
	m_pszOptionStringBuffer = cszOptionStringBuffer;

	// �ɼ� üũ
	if (!lstrcmpiA(m_pszOptionStringBuffer, "MATCHLEN"))
		g_bMatchLen=TRUE;
	else
		g_bMatchLen=FALSE;

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
				m_setInactivatedArgs.insert(new CNonCachedTextArg());
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
BOOL CNonCachedTextArgMgr::Close()
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
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CNonCachedTextArg* pNonCachedTextArg = *(iter);
		delete pNonCachedTextArg;
	}
	m_setActivatedArgs.clear();
	
	// ��Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ����
	for(CNonCachedTextArgSet::iterator iter = m_setInactivatedArgs.begin();
		iter != m_setInactivatedArgs.end();
		iter++)
	{
		CNonCachedTextArg* pNonCachedTextArg = *(iter);
		delete pNonCachedTextArg;
	}
	m_setInactivatedArgs.clear();

	// �ߺ� ī��Ʈ ���̺� �ʱ�ȭ
	ZeroMemory(m_aDupCntTable, sizeof(m_aDupCntTable));

	// ��Ÿ ���� ����
	m_bApplocale = FALSE;
	m_hContainerWnd = NULL;

	// ��Ʈ ����
	for (map<long,HFONT>::iterator it=m_mapFonts.begin(); it != m_mapFonts.end(); it++)
	{
		DeleteObject(it->second);
	}
	m_mapFonts.clear();

	// ��� ������ �簡��
	m_sContainerFunc.pfnResumeAllThread();

	return TRUE;
}



// ���ο� ���ڿ� �ĺ��� �߰��Ѵ�
BOOL CNonCachedTextArgMgr::AddTextArg(LPCWSTR wszText)
{
	BOOL bRetVal = FALSE;

	if( NULL == m_sContainerFunc.pfnTranslateText ) return FALSE;

	CNonCachedTextArg* pNonCachedTextArg = NULL;
	if(!m_setInactivatedArgs.empty()) 
	{
		CNonCachedTextArgSet::iterator iter = m_setInactivatedArgs.begin();
		pNonCachedTextArg = ( *iter );
		m_setInactivatedArgs.erase(pNonCachedTextArg);
		m_setActivatedArgs.insert(pNonCachedTextArg);
	}
	else
	{
		// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
		for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
			iter != m_setActivatedArgs.end();
			iter++)
		{
			CNonCachedTextArg* pTmpNonCachedTextArg = *(iter);

			if( NULL == pNonCachedTextArg || (pNonCachedTextArg->m_nFaultCnt < pTmpNonCachedTextArg->m_nFaultCnt) )
			{
				pNonCachedTextArg = pTmpNonCachedTextArg;
			}		
		}		
	}
		

	if( pNonCachedTextArg->SetNonCachedTextArg(wszText, m_sContainerFunc.pfnTranslateText) )
	{
		bRetVal = TRUE;
	}
	else
	{
		m_setActivatedArgs.erase(pNonCachedTextArg);
		m_setInactivatedArgs.insert(pNonCachedTextArg);
	}

	return bRetVal;
}

// ���ڿ� �ĺ��� ��ü�� �׽�Ʈ�Ѵ�. (���̻� ��ġ���� �ʴ� �ĺ��� �ٷ� ����)
BOOL CNonCachedTextArgMgr::TestCharacter(wchar_t wch)
{
	BOOL bRetVal = FALSE;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ��� ��� ��ȸ
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CNonCachedTextArg* pNonCachedTextArg = (*iter);
		iter++;

		// ����Ǵ� �ߺ� Ƚ�� ���ϱ�
		UINT nPreperedDupCnt = 0;
		for(UINT i=1; i<16; i++)
		{
			if( m_aDupCntTable[nPreperedDupCnt] < m_aDupCntTable[i] )
			{
				nPreperedDupCnt = i;
			}
		}
		
		// �˻� ����
		int nRes = pNonCachedTextArg->TestCharacter(wch, nPreperedDupCnt);
		
		// ������
		if( 0 != nRes )
		{
			if( nRes & 0x01 ) bRetVal = TRUE;
			if( (nRes & 0x03) != 0 )
			{
				int idx = pNonCachedTextArg->m_nDupCnt;
				if(idx<16) m_aDupCntTable[idx]++;
			}
		}
		// ����
		else
		{
			m_setActivatedArgs.erase(pNonCachedTextArg);
			m_setInactivatedArgs.insert(pNonCachedTextArg);
		}
	}	

	return bRetVal;
}

// �ְ�� Ȯ���� ���� ���� ���ڸ� ��ȯ
BOOL CNonCachedTextArgMgr::GetBestTranslatedCharacter(wchar_t* pTransResultBuf)
{
	//wchar_t wchRetVal = L'\0';
	CNonCachedTextArg* pBestArg = NULL;
	
	if( NULL == pTransResultBuf ) return FALSE;

	BOOL bRetVal = FALSE;

	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ� ��� ��ȯ
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CNonCachedTextArg* pNonCachedTextArg = *(iter);

		if( NULL == pBestArg )
		{
			if( 0 < pNonCachedTextArg->m_nHitCnt ) pBestArg = pNonCachedTextArg;
		}		
		else if( pBestArg->m_nHitCnt < pNonCachedTextArg->m_nHitCnt )
		{
			pBestArg = pNonCachedTextArg;
		}
		else if(pBestArg->m_nHitCnt == pNonCachedTextArg->m_nHitCnt)
		{
			size_t nBestRemain = pBestArg->m_nJapaneseLen - pBestArg->m_nNextTestIdx;
			size_t nTempRemain = pNonCachedTextArg->m_nJapaneseLen - pNonCachedTextArg->m_nNextTestIdx;
			if( nBestRemain < nTempRemain )
			{
				pBestArg = pNonCachedTextArg;				
			}
			else if( nBestRemain == nTempRemain && pBestArg->m_nFaultCnt > pNonCachedTextArg->m_nFaultCnt )
			{
				pBestArg = pNonCachedTextArg;
			}
		}
	}

	if(pBestArg)
	{
		// ������ ������ ��� �� ��ȯ
		if(pBestArg->m_nJapaneseLen == pBestArg->m_nNextTestIdx-1 && pBestArg->m_nKoreanLen)
		{
			wcscpy( pTransResultBuf, &pBestArg->m_wszKoreanText[pBestArg->m_nNextKorIdx] );
		}
		// �ѹ���
		else
		{
			pTransResultBuf[0] = pBestArg->GetTranslatedCharacter();
			pTransResultBuf[1] = L'\0';

		}

		if(pTransResultBuf[0]) bRetVal = TRUE;
	}

	return bRetVal;
}

CNonCachedTextArg* CNonCachedTextArgMgr::FindString(LPCWSTR pTestString, int nSize)
{
	CNonCachedTextArg* pMatchArg=NULL;
	// Ȱ��ȭ �ؽ�Ʈ �ν��Ͻ��� ��� ��ȸ
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CNonCachedTextArg* pNonCachedTextArg = (*iter);
		iter++;

		// �˻� ����
		int nRes = pNonCachedTextArg->TestString(pTestString, nSize);

		// ����
		if (nRes == 0)
		{
			m_setActivatedArgs.erase(pNonCachedTextArg);
			m_setInactivatedArgs.insert(pNonCachedTextArg);
		}
		// ã�ų� �ߺ���
		else if (nRes != 4)
		{
			if ( !pMatchArg || ( nSize == 1 && pMatchArg->m_nJapaneseLen == 1 && pNonCachedTextArg->m_nJapaneseLen > 1) )
				pMatchArg = pNonCachedTextArg;
		}

	}	

	return pMatchArg;
}
BOOL CNonCachedTextArgMgr::GetTranslatedStringA(INT_PTR ptrBegin, LPCSTR szOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize)
{
	CNonCachedTextArg* pBestArg = NULL;

	if( NULL == pTransResultBuf ) return FALSE;

	BOOL bRetVal = FALSE;

	wchar_t wszOrigString[1024] = { L'\0', };
	int nLen=MyMultiByteToWideChar(932, 0, szOrigString, nOrigSize, wszOrigString, 1023 );

	pBestArg = FindString(wszOrigString, nLen);

	if (!pBestArg)
	{
		int iRes = SearchStringA(ptrBegin, szOrigString[0], szOrigString[1]);
		pBestArg = FindString(wszOrigString, nLen);
	}


	if(pBestArg)
	{
		nTransSize= nTransSize / 2;

		pBestArg->GetTranslatedString(pTransResultBuf, nBufSize, nTransSize);

		if(pTransResultBuf[0]) bRetVal = TRUE;
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::GetTranslatedStringW(INT_PTR ptrBegin, LPCWSTR wszOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize)
{
	CNonCachedTextArg* pBestArg = NULL;

	if( NULL == pTransResultBuf ) return FALSE;

	BOOL bRetVal = FALSE;

	pBestArg = FindString(wszOrigString, nOrigSize);

	if (!pBestArg)
	{
		int iRes = SearchStringW(ptrBegin, wszOrigString[0]);
		pBestArg = FindString(wszOrigString, nOrigSize);
	}


	if(pBestArg)
	{
		pBestArg->GetTranslatedString(pTransResultBuf, nBufSize, nTransSize);
		
		if(pTransResultBuf[0]) bRetVal = TRUE;
	}

	return bRetVal;
}




// ���� Ȱ��ȭ �� �ؽ�Ʈ ���ڵ��� �ϳ��� ���°�?
BOOL CNonCachedTextArgMgr::IsEmpty()
{
	return m_setActivatedArgs.empty();
}



int CNonCachedTextArgMgr::SearchStringA(INT_PTR ptrBegin, char ch1, char ch2)
{
	int iRetVal = 0;

	size_t dist = 0;


	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);
		if( IsBadStringPtrA(*ppText, 1024*1024)==FALSE && (*ppText)[0]==ch1
			&&  ('\0'==ch2 || (*ppText)[1]==ch2) && strlen(*ppText) < 1024 )
		{
			wchar_t wszTmp[1024];
			MyMultiByteToWideChar(932, 0, *ppText, -1, wszTmp, 1023 );
			AddTextArg(wszTmp);
			iRetVal++;
		}
		dist += sizeof(void*);

	}

	return iRetVal;
}

int CNonCachedTextArgMgr::SearchStringW(INT_PTR ptrBegin, wchar_t wch)
{
	int iRetVal = 0;

	size_t dist = 0;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);
		if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE && **ppText == wch )
		{
			AddTextArg(*ppText);
			iRetVal++;
		}
		dist += sizeof(void*);
	}	

	return iRetVal;
}


void CNonCachedTextArgMgr::CheckFont(HDC hdc) 
{
	static long lLastFontHeight = -1;

	TEXTMETRIC tm;
	ZeroMemory(&tm, sizeof(TEXTMETRIC));
	BOOL bRes = GetTextMetrics(hdc, &tm);

	// ��Ʈ �ٽ� �ε�
	if( bRes && tm.tmHeight != lLastFontHeight )
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

	/*
	// ��Ʈ �ε�
	if( NULL==m_hFont )
	{
		TEXTMETRIC tm;

		ZeroMemory(&tm, sizeof(TEXTMETRIC));
		GetTextMetrics(hdc, &tm);

		m_hFont=CreateFont(tm.tmHeight, tm.tmHeight/2, 0, 0, tm.tmWeight, tm.tmItalic, tm.tmUnderlined, tm.tmStruckOut,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			NONANTIALIASED_QUALITY,
			DEFAULT_PITCH | FF_SWISS,
			"����");

		ZeroMemory(&tm, sizeof(TEXTMETRIC));
		GetTextMetrics(hdc, &tm);
		hdc = hdc;
	}

	SelectObject(hdc, m_hFont);
	*/
}


//////////////////////////////////////////////////////////////////////////
//
// GetGlyphOutlineA
//
//////////////////////////////////////////////////////////////////////////
DWORD CNonCachedTextArgMgr::InnerGetGlyphOutlineA(
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

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}


	//CString strWndText;
	//_gInst->m_pOutput->m_editOutput.GetWindowText(strWndText);

	//wchar_t wch = (wchar_t)uChar;
	char chArray[5] = {0,};
	wchar_t wchArray[10] = {0,};
	size_t i,j;
	j = 0;
	for(i=sizeof(UINT); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
			//strWndText += one_ch;
		}
	}

	//_gInst->m_pOutput->m_editOutput.SetWindowText(strWndText);


	MyMultiByteToWideChar(932, 0, chArray, sizeof(UINT), wchArray, 10 );

	wchar_t wch = wchArray[0];

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce )
	{
		int iRes = SearchStringA(_CUR_EBP, chArray[0], chArray[1]);
		//iRes += SearchStringW(_CUR_EBP, wch);
	}

	// �ְ��� ���� ���ڸ� ����
	wchar_t wchNewChar[1024];

	DWORD dwRetVal = 0;

	if( GetBestTranslatedCharacter(wchNewChar) )
	{
		uChar = wchNewChar[0];

		if(uChar<=0x20)
		{
			uFormat = GGO_NATIVE;
		}

		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

		if(dwRetVal==GDI_ERROR)
		{
			TRACE("[ aral1 ] GetGlyphOutlineA(0x%x) failed (ErrCode:%d) \n", uChar, GetLastError());
		}

		/*
		wchArray[0] = wchNewChar[0];
		wchArray[1] = L'\0';

		MyWideCharToMultiByte(949, 0, wchArray, -1, chArray, 4, NULL, NULL );

		if(chArray[1])
		{
		uChar = MAKEWORD( chArray[1], chArray[0] );
		}
		else
		{
		uChar = chArray[0];
		}
		*/

		//wchar_t dbg[MAX_PATH];
		//swprintf(dbg, L"[ aral1 ] MyGetGlyphOutlineA : '%c'->'%s' \n", wch, wchNewChar);
		//OutputDebugStringW(dbg);
	}
	// �����Լ� ȣ��
	else if( m_sTextFunc.pfnGetGlyphOutlineA )
	{
		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;	
}


//////////////////////////////////////////////////////////////////////////
//
// GetGlyphOutlineW
//
//////////////////////////////////////////////////////////////////////////
DWORD CNonCachedTextArgMgr::InnerGetGlyphOutlineW(
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

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}


	wchar_t wch = (wchar_t)uChar;

	// �˻����� �ؽ�Ʈ �����͵� ��� ��ȸ
	BOOL bHitOnce = TestCharacter(wch);

	// ���ߵ� �����Ͱ� ���ٸ� �˻�
	if( FALSE == bHitOnce )
	{
		int iRes = SearchStringW(_CUR_EBP, wch);
	}

	// �ְ��� ���� ���ڸ� ����
	wchar_t wchNewChar[1024];

	if( GetBestTranslatedCharacter(wchNewChar) )
	{
		uChar = wchNewChar[0];
	}

	wchar_t dbg[MAX_PATH];
	swprintf(dbg, L"[ aral1 ] MyGetGlyphOutlineW : '%c'->'%s' \n", wch, wchNewChar);
	OutputDebugStringW(dbg);

	// �����Լ� ȣ��
	DWORD dwRetVal = 0;
	if( m_sTextFunc.pfnGetGlyphOutlineW )
	{
		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}

BOOL CNonCachedTextArgMgr::InnerTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = 0;
	//	char szNewString[1024];

	CheckFont(hdc);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	int nLen;
//	wchar_t wszJapanese[1024] = { L'\0', };
//	nLen=MyMultiByteToWideChar(932, 0, lpString, cbString, wszJapanese, 1023 );

//	if (IsJapaneseW(wszJapanese, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];
		nLen=cbString;

		if( GetTranslatedStringA(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{
			bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, wszNewString, nLen);
			return bRetVal;
		}
	}


	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutA )
	{
		bRetVal = m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::InnerTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = 0;

	CheckFont(hdc);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}
	int nLen = cbString;

	if (IsJapaneseW(lpString, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{

			bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, wszNewString, nLen);
			return bRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnTextOutW )
	{
		bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::InnerExtTextOutA(
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

	CheckFont(hdc);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	int nLen;
	//	wchar_t wszJapanese[1024] = { L'\0', };
	//	nLen=MyMultiByteToWideChar(932, 0, lpString, cbString, wszJapanese, 1023 );

	//	if (IsJapaneseW(wszJapanese, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];
		nLen=(int)cbCount;

		if( GetTranslatedStringA(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{

			bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, wszNewString, nLen, lpDx);
			return bRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnExtTextOutA )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::InnerExtTextOutW(
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
	CheckFont(hdc);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}
	int nLen = (int)cbCount;

	if (IsJapaneseW(lpString, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{
			bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, wszNewString, nLen, lpDx);
			return bRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnExtTextOutW )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;
}

int CNonCachedTextArgMgr::InnerDrawTextA(
				   HDC hDC,          // handle to DC
				   LPCSTR lpString,  // text to draw
				   int nCount,       // text length
				   LPRECT lpRect,    // formatting dimensions
				   UINT uFormat      // text-drawing options
				   )
{
	int nRetVal = 0;

	CheckFont(hDC);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	int nLen;
	//	wchar_t wszJapanese[1024] = { L'\0', };
	//	nLen=MyMultiByteToWideChar(932, 0, lpString, cbString, wszJapanese, 1023 );

	//	if (IsJapaneseW(wszJapanese, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];
		nLen=nCount;

		if( GetTranslatedStringA(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{
			nRetVal = m_sTextFunc.pfnDrawTextW(hDC, wszNewString, nLen, lpRect, uFormat);
			return nRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnDrawTextA )
	{
		nRetVal = m_sTextFunc.pfnDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}

	return nRetVal;
}

int CNonCachedTextArgMgr::InnerDrawTextW(
				   HDC hDC,          // handle to DC
				   LPCWSTR lpString, // text to draw
				   int nCount,       // text length
				   LPRECT lpRect,    // formatting dimensions
				   UINT uFormat      // text-drawing options
				   )
{
	int nRetVal = 0;

	CheckFont(hDC);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}
	int nLen=nCount;

	if (IsJapaneseW(lpString, nCount))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{
			nRetVal = m_sTextFunc.pfnDrawTextW(hDC, wszNewString, nLen, lpRect, uFormat);
			return nRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnDrawTextW )
	{
		nRetVal = m_sTextFunc.pfnDrawTextW(hDC, lpString, nLen, lpRect, uFormat);
	}

	return nRetVal;
}

int CNonCachedTextArgMgr::InnerDrawTextExA(
					 HDC hdc,                     // handle to DC
					 LPSTR lpchText,              // text to draw
					 int cchText,                 // length of text to draw
					 LPRECT lprc,                 // rectangle coordinates
					 UINT dwDTFormat,             // formatting options
					 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
					 )
{
	int nRetVal = 0;

	CheckFont(hdc);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	int nLen;
	//	wchar_t wszJapanese[1024] = { L'\0', };
	//	nLen=MyMultiByteToWideChar(932, 0, lpString, cbString, wszJapanese, 1023 );

	//	if (IsJapaneseW(wszJapanese, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];
		nLen=cchText;

		if( GetTranslatedStringA(_CUR_EBP, lpchText, nLen, wszNewString, 1024, nLen) )
		{
			nRetVal = m_sTextFunc.pfnDrawTextExW(hdc, wszNewString, nLen, lprc, dwDTFormat, lpDTParams);
			return nRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnDrawTextExA )
	{
		nRetVal = m_sTextFunc.pfnDrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}

	return nRetVal;
}

int CNonCachedTextArgMgr::InnerDrawTextExW(
					 HDC hdc,                     // handle to DC
					 LPWSTR lpchText,              // text to draw
					 int cchText,                 // length of text to draw
					 LPRECT lprc,                 // rectangle coordinates
					 UINT dwDTFormat,             // formatting options
					 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
					 )
{
	int nRetVal = 0;

	CheckFont(hdc);

	// �ٷ� �� �� EBP�� ���ؼ�
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	int nLen=cchText;

	if (IsJapaneseW(lpchText, nLen))
	{

		// �ְ��� ���� ���ڸ� ����
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpchText, nLen, wszNewString, 1024, nLen) )
		{
			nRetVal = m_sTextFunc.pfnDrawTextExW(hdc, wszNewString, nLen, lprc, dwDTFormat, lpDTParams);
			return nRetVal;
		}
	}
	// �����Լ� ȣ��
	if( m_sTextFunc.pfnDrawTextExW )
	{
		nRetVal = m_sTextFunc.pfnDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}

	return nRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CNonCachedTextArgMgr::NewGetGlyphOutlineA(
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

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		dwRetVal = g_objNonCachedTextArgMgr.InnerGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineA)
	{
		dwRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CNonCachedTextArgMgr::NewGetGlyphOutlineW(
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

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		dwRetVal = g_objNonCachedTextArgMgr.InnerGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineW)
	{
		dwRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CNonCachedTextArgMgr::NewTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = 0;

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objNonCachedTextArgMgr.InnerTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnTextOutA)
	{
		bRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CNonCachedTextArgMgr::NewTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = 0;

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objNonCachedTextArgMgr.InnerTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnTextOutW)
	{
		bRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CNonCachedTextArgMgr::NewExtTextOutA(
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

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objNonCachedTextArgMgr.InnerExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnExtTextOutA)
	{
		bRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CNonCachedTextArgMgr::NewExtTextOutW(
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

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		bRetVal = g_objNonCachedTextArgMgr.InnerExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnExtTextOutW)
	{
		bRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;

}

int CNonCachedTextArgMgr::NewDrawTextA(
								  HDC hDC,          // handle to DC
								  LPCSTR lpString,  // text to draw
								  int nCount,       // text length
								  LPRECT lpRect,    // formatting dimensions
								  UINT uFormat      // text-drawing options
								  )
{

	int nRetVal = 0;

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		nRetVal = g_objNonCachedTextArgMgr.InnerDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextA)
	{
		nRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}

	return nRetVal;

}

int CNonCachedTextArgMgr::NewDrawTextW(
								  HDC hDC,          // handle to DC
								  LPCWSTR lpString, // text to draw
								  int nCount,       // text length
								  LPRECT lpRect,    // formatting dimensions
								  UINT uFormat      // text-drawing options
								  )
{

	int nRetVal = 0;

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		nRetVal = g_objNonCachedTextArgMgr.InnerDrawTextW(hDC, lpString, nCount, lpRect, uFormat);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextW)
	{
		nRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextW(hDC, lpString, nCount, lpRect, uFormat);
	}

	return nRetVal;

}

int CNonCachedTextArgMgr::NewDrawTextExA(
									HDC hdc,                     // handle to DC
									LPSTR lpchText,              // text to draw
									int cchText,                 // length of text to draw
									LPRECT lprc,                 // rectangle coordinates
									UINT dwDTFormat,             // formatting options
									LPDRAWTEXTPARAMS lpDTParams  // more formatting options
									)
{

	int nRetVal = 0;

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		nRetVal = g_objNonCachedTextArgMgr.InnerDrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextExA)
	{
		nRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}

	return nRetVal;

}

int CNonCachedTextArgMgr::NewDrawTextExW(
									HDC hdc,                     // handle to DC
									LPWSTR lpchText,              // text to draw
									int cchText,                 // length of text to draw
									LPRECT lprc,                 // rectangle coordinates
									UINT dwDTFormat,             // formatting options
									LPDRAWTEXTPARAMS lpDTParams  // more formatting options
									)
{

	int nRetVal = 0;

	if(g_objNonCachedTextArgMgr.m_bRunning)
	{
		nRetVal = g_objNonCachedTextArgMgr.InnerDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}
	else if(g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextExW)
	{
		nRetVal = g_objNonCachedTextArgMgr.m_sTextFunc.pfnDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}

	return nRetVal;

}

BOOL CNonCachedTextArgMgr::Start()
{
	m_bRunning = TRUE;
	return TRUE;
}

BOOL CNonCachedTextArgMgr::Stop()
{
	m_bRunning = FALSE;
	return TRUE;
}

BOOL CNonCachedTextArgMgr::Option()
{
	if(m_hContainerWnd && IsWindow(m_hContainerWnd))
	{
		COptionDialog od;
		if (od.DoModal() == IDOK)
		{
			g_bMatchLen = od.m_bMatchLen;
			if (g_bMatchLen)
				lstrcpyA(m_pszOptionStringBuffer, "MATCHLEN");
			else
				m_pszOptionStringBuffer[0]='\0';
			
		}
	}
	return TRUE;
}
