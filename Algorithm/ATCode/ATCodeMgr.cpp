
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATCodeMgr.h"
#include "HookPoint.h"
#include "TransCommand.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "CharacterMapper.h"
#include "OptionDlg.h"
#include <process.h>

#include "Debug.h"

/*
** EncodeKor ó�� ����� �պ��� �������� �ʿ� ������ ������ ����

const WORD ko_char_map[] = {
	0x9C9A,	//��
	0x9C86,	//��
	0xAF8B,	//��
	0x0000
};
*/

CATCodeMgr*	CATCodeMgr::_Inst = NULL;

CATCodeMgr* CATCodeMgr::GetInstance()
{
	return _Inst;
}


CATCodeMgr::CATCodeMgr(void)
  : m_bRunning(FALSE), 
	m_nFontLoadLevel(0), 
	m_nUniKofilterLevel(0),
	m_nM2WLevel(0),
	m_hContainer(NULL), 
	m_hContainerWnd(NULL), 
	m_szOptionString(NULL), 
	m_nEncodeKorean(0),
	m_bUITrans(FALSE),
	m_bNoAslr(FALSE),
	m_bRunClipboardThread(FALSE),
	m_hClipboardThread(NULL),
	m_hClipTextChangeEvent(NULL),
	m_pfnLoadLibraryA(NULL),
	m_pfnLoadLibraryW(NULL),
	m_bCompareStringW(0)
{
	_Inst = this;
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_FUNCTION_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
	ZeroMemory(&m_sFontFunc, sizeof(FONT_FUNCTION_ENTRY));
	InitializeCriticalSection(&m_csClipText);
}

CATCodeMgr::~CATCodeMgr(void)
{
	DeleteCriticalSection(&m_csClipText);
	Close();
	_Inst = NULL;
}


BOOL CATCodeMgr::Init( HWND hSettingWnd, LPSTR cszOptionStringBuffer )
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
		m_sContainerFunc.pfnIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress( m_hContainer, "IsAppLocaleLoaded" );
		m_sContainerFunc.pfnEnableAppLocale		= (PROC_EnableAppLocale) GetProcAddress( m_hContainer, "EnableAppLocale" );
		m_sContainerFunc.pfnSuspendAllThread	= (PROC_SuspendAllThread) GetProcAddress( m_hContainer, "SuspendAllThread" );
		m_sContainerFunc.pfnResumeAllThread		= (PROC_ResumeAllThread) GetProcAddress( m_hContainer, "ResumeAllThread" );
		m_sContainerFunc.pfnIsAllThreadSuspended= (PROC_IsAllThreadSuspended) GetProcAddress( m_hContainer, "IsAllThreadSuspended" );
	}

	if( m_sContainerFunc.pfnGetCurAlgorithm && m_sContainerFunc.pfnGetCurTranslator
		&& m_sContainerFunc.pfnHookDllFunctionEx && m_sContainerFunc.pfnGetOrigDllFunction
		&& m_sContainerFunc.pfnUnhookDllFunction && m_sContainerFunc.pfnHookCodePointEx
		&& m_sContainerFunc.pfnUnhookCodePoint && m_sContainerFunc.pfnTranslateText
		&& m_sContainerFunc.pfnIsAppLocaleLoaded && m_sContainerFunc.pfnEnableAppLocale
		&& m_sContainerFunc.pfnSuspendAllThread && m_sContainerFunc.pfnResumeAllThread
		&& m_sContainerFunc.pfnIsAllThreadSuspended)
	{
		// ��� ������ ����
		m_sContainerFunc.pfnSuspendAllThread();

		// LoadLibrary �Լ� ��ŷ	 
		if( m_sContainerFunc.pfnHookDllFunctionEx( "kernel32.dll", "LoadLibraryA", NewLoadLibraryA, 1 ) )
		{
			m_pfnLoadLibraryA = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryA");
		}

		if( m_sContainerFunc.pfnHookDllFunctionEx( "kernel32.dll", "LoadLibraryW", NewLoadLibraryW, 1 ) )
		{
			m_pfnLoadLibraryW = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryW");
		}

		// ��Ʈ �Լ��� ��ŷ
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.dll", "CreateFontA", NewCreateFontA, 1 ) )
		{
			m_sFontFunc.pfnCreateFontA = (PROC_CreateFont) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontA");
		}

		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.dll", "CreateFontW", NewCreateFontW, 1 ) )
		{
			m_sFontFunc.pfnCreateFontW = (PROC_CreateFont) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontW");
		}

		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.dll", "CreateFontIndirectA", NewCreateFontIndirectA, 1 ) )
		{
			m_sFontFunc.pfnCreateFontIndirectA = (PROC_CreateFontIndirect) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontIndirectA");
		}

		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.dll", "CreateFontIndirectW", NewCreateFontIndirectW, 1 ) )
		{
			m_sFontFunc.pfnCreateFontIndirectW = (PROC_CreateFontIndirect) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontIndirectW");
		}


		// �ؽ�Ʈ �Լ��� ��ŷ

		// GetGlyphOutlineA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "GetGlyphOutlineA", NewGetGlyphOutlineA, 1 ) )
		{
			m_sTextFunc.pfnGetGlyphOutlineA = 
				(PROC_GetGlyphOutline) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "GetGlyphOutlineA");
		}

		// GetGlyphOutlineW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "GetGlyphOutlineW", NewGetGlyphOutlineW, 1 ) )
		{
			m_sTextFunc.pfnGetGlyphOutlineW = 
				(PROC_GetGlyphOutline) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "GetGlyphOutlineW");
		}

		// TextOutA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "TextOutA", NewTextOutA, 1 ) )
		{
			m_sTextFunc.pfnTextOutA = 
				(PROC_TextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "TextOutA");
		}

		// TextOutW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "TextOutW", NewTextOutW, 1 ) )
		{
			m_sTextFunc.pfnTextOutW = 
				(PROC_TextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "TextOutW");
		}

		// ExtTextOutA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "ExtTextOutA", NewExtTextOutA, 1 ) )
		{
			m_sTextFunc.pfnExtTextOutA = 
				(PROC_ExtTextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "ExtTextOutA");
		}

		// ExtTextOutW
		//if( m_sContainerFunc.pfnHookDllFunctionEx( "GDI32.DLL", "ExtTextOutW", NewExtTextOutW ) )
		//{
		//	m_sTextFunc.pfnExtTextOutW = 
		//		(PROC_ExtTextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "ExtTextOutW");
		//}

		// DrawTextA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextA", NewDrawTextA, 1 ) )
		{
			m_sTextFunc.pfnDrawTextA = 
				(PROC_DrawText) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextA");
		}

		// DrawTextW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextW", NewDrawTextW, 1 ) )
		{
			m_sTextFunc.pfnDrawTextW = 
				(PROC_DrawText) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextW");
		}

		// DrawTextExA
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextExA", NewDrawTextExA, 1 ) )
		{
			m_sTextFunc.pfnDrawTextExA = 
				(PROC_DrawTextEx) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextExA");
		}

		// DrawTextExW
		if( m_sContainerFunc.pfnHookDllFunctionEx( "USER32.DLL", "DrawTextExW", NewDrawTextExW, 1 ) )
		{
			m_sTextFunc.pfnDrawTextExW = 
				(PROC_DrawTextEx) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextExW");
		}

		// ���÷����� ���� �Լ�
		m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));

		m_sTextFunc.pfnOrigWideCharToMultiByte =
			(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"));

		if( m_sTextFunc.pfnTextOutA && m_sTextFunc.pfnTextOutW 
			//&& m_sTextFunc.pfnExtTextOutA && m_sTextFunc.pfnExtTextOutW
			&& m_sTextFunc.pfnGetGlyphOutlineA && m_sTextFunc.pfnGetGlyphOutlineW
			&& m_sTextFunc.pfnOrigMultiByteToWideChar && m_sTextFunc.pfnOrigWideCharToMultiByte
			&& m_sTextFunc.pfnDrawTextA	&& m_sTextFunc.pfnDrawTextW
			&& m_sTextFunc.pfnDrawTextExA && m_sTextFunc.pfnDrawTextExW
			
			&& m_pfnLoadLibraryA && m_pfnLoadLibraryW
			
			&& m_sFontFunc.pfnCreateFontA && m_sFontFunc.pfnCreateFontW
			&& m_sFontFunc.pfnCreateFontIndirectA && m_sFontFunc.pfnCreateFontIndirectW )
		{
			// Ŭ������ ����
			m_bRunClipboardThread = TRUE;
			m_hClipTextChangeEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			m_hClipboardThread = (HANDLE)_beginthreadex(NULL, 0, ClipboardThreadFunc, NULL, 0, NULL);

			// �ɼ� ��Ʈ�� �Ľ�
			m_szOptionString = cszOptionStringBuffer;

			if(m_szOptionString == NULL)
			{
				m_szOptionString = new char[4096];
				ZeroMemory(m_szOptionString, 4096);
			}

			if( m_szOptionString[0] == _T('\0') )
			{
				//strcpy(m_szOptionString, "FORCEFONT,ENCODEKOR,FONT(�ü�,-24)");
			}

#ifdef UNICODE
			wchar_t wszTmpString[4096];
			MyMultiByteToWideChar(949, 0, m_szOptionString, -1, wszTmpString, 4096);
			CString strOptionString = wszTmpString;
#else
			CString strOptionString	= m_szOptionString;		
#endif

			if( m_optionRoot.ParseChildren(strOptionString) )
			{
				bRetVal = TRUE;
			}
			
		}

		// ��� ������ �簡��
		m_sContainerFunc.pfnResumeAllThread();

	}

	if( bRetVal == TRUE )
	{
		// �ɼ� ����
		AdjustOption(&m_optionRoot);
	}
	else
	{
		Close();
	}

	return bRetVal;
}


BOOL CATCodeMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	// Ŭ������ ������ ����
	m_bRunClipboardThread = FALSE;
	if(m_hClipboardThread && m_hClipTextChangeEvent)
	{
		::SetEvent(m_hClipTextChangeEvent);
		::WaitForSingleObject(m_hClipboardThread, 3000);
		::CloseHandle(m_hClipboardThread);
		::CloseHandle(m_hClipTextChangeEvent);
	}

	// ��� ������ ����
	m_sContainerFunc.pfnSuspendAllThread();

	ResetOption();

	m_hClipboardThread = NULL;
	m_hClipTextChangeEvent = NULL;
	
	// �ɼ� ��ü �ʱ�ȭ
	m_optionRoot.ClearChildren();

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
	//if( m_sTextFunc.pfnExtTextOutW )
	//{
	//	m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "ExtTextOutW" );
	//	m_sTextFunc.pfnExtTextOutW = NULL;
	//}

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

	// MultiByteToWideChar ����
	if( m_sTextFunc.pfnOrigMultiByteToWideChar )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "MultiByteToWideChar" );
		m_sTextFunc.pfnOrigMultiByteToWideChar = NULL;
	}

	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));


	// ��Ʈ �Լ��� ����
	if( m_sFontFunc.pfnCreateFontA )
	{
		TRACE(_T("GDI32.DLL!CreateFontA Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontA" );
		m_sFontFunc.pfnCreateFontA = NULL;
	}
	if( m_sFontFunc.pfnCreateFontW )
	{
		TRACE(_T("GDI32.DLL!CreateFontW Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontW" );
		m_sFontFunc.pfnCreateFontW = NULL;
	}
	if( m_sFontFunc.pfnCreateFontIndirectA )
	{
		TRACE(_T("GDI32.DLL!CreateFontIndirectA Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontIndirectA" );
		m_sFontFunc.pfnCreateFontIndirectA = NULL;
	}
	if( m_sFontFunc.pfnCreateFontIndirectW )
	{
		TRACE(_T("GDI32.DLL!CreateFontIndirectW Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontIndirectW" );
		m_sFontFunc.pfnCreateFontIndirectW = NULL;
	}

	// LoadLibrary ����
	if( m_pfnLoadLibraryA )
	{
		TRACE(_T("kernel32.DLL!LoadLibraryA Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "LoadLibraryA" );
		m_pfnLoadLibraryA = NULL;
	}

	if( m_pfnLoadLibraryW )
	{
		TRACE(_T("kernel32.DLL!LoadLibraryW Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "LoadLibraryW" );
		m_pfnLoadLibraryW = NULL;
	}
	
	// ��Ÿ ���� ����
	m_hContainerWnd = NULL;
	m_szOptionString = NULL;

	// ��� ������ �簡��
	m_sContainerFunc.pfnResumeAllThread();

	return TRUE;
}

BOOL CATCodeMgr::Start()
{
	m_bRunning = TRUE;
	return TRUE;
}

BOOL CATCodeMgr::Stop()
{
	m_bRunning = FALSE;
	return TRUE;
}

BOOL CATCodeMgr::Option()
{
	BOOL bRetVal = TRUE;

	CString strCurOptionString = m_optionRoot.ChildrenToString();
	
	COptionNode tmpRoot;
	if( tmpRoot.ParseChildren(strCurOptionString) == FALSE ) return FALSE;

	COptionDlg od;
	od.SetRootOptionNode(&tmpRoot);
	if( od.DoModal() == IDOK )
	{
		ApplyOption(&tmpRoot);
	}

	return bRetVal;
}

BOOL CATCodeMgr::ApplyOption( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;
	NOTIFY_DEBUG_MESSAGE(_T("ApplyOption: called\n"));
	
	CString strCurOptionString = m_optionRoot.ChildrenToString();

	// �� �ɼ� ��Ʈ���� FORCEFONT�� ������ FIXFONTSIZE, FONT ��� ����
	if( pRootNode->GetChild(_T("FORCEFONT")) == NULL )
	{
		pRootNode->DeleteChild(_T("FIXFONTSIZE"));
		pRootNode->DeleteChild(_T("FONT"));
	}
	
	// �ۿ���Ѻ���
	if( AdjustOption(pRootNode) == FALSE || m_optionRoot.ParseChildren( pRootNode->ChildrenToString() ) == FALSE )
	{
		MessageBox(NULL, _T("�ɼ� ���뿡 �����Ͽ����ϴ�"), _T("ApplyOption"), MB_OK|MB_ICONEXCLAMATION);
		NOTIFY_DEBUG_MESSAGE(_T("ApplyOption: failed\n"));

		// ���и� ���󺹱�
		m_optionRoot.ParseChildren( strCurOptionString );
		AdjustOption(&m_optionRoot);
	}
	else
	{
		// ���� �����̸�
		CString strOptionString = m_optionRoot.ChildrenToString();
		NOTIFY_DEBUG_MESSAGE(_T("ApplyOption: success\n"));


#ifdef UNICODE
		MyWideCharToMultiByte(949, 0, (LPCWSTR)strOptionString, -1, m_szOptionString, 4096, NULL, NULL);
#else
		strcpy(m_szOptionString, (LPCTSTR)strOptionString);
#endif

		bRetVal = TRUE;
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// DC�� �ѱ� ��Ʈ�� ���̴� �Լ�
//
//////////////////////////////////////////////////////////////////////////
HFONT CATCodeMgr::CheckFont( HDC hdc )
{
	HFONT hRetVal = NULL;

	TEXTMETRIC tm;
	BOOL bRes = GetTextMetrics(hdc, &tm);

	// ��Ʈ �ٽ� �ε�
	if( bRes )//&& tm.tmHeight != lLastFontHeight )
	{
		
		HFONT font = NULL;
		long lFontSize = tm.tmHeight;
		
		if(m_strFontFace.IsEmpty())
		{
			m_strFontFace = _T("�ü�");
		}

		// ��Ʈ���̽����� �ٲ������ �� �ʱ�ȭ
		if(m_strLastFontFace.Compare(m_strFontFace))
		{
			for(map<long, HFONT>::iterator iter = m_mapFonts.begin();
				iter != m_mapFonts.end();
				iter++)
			{
				font = iter->second;
				DeleteObject(font);
			}
			m_mapFonts.clear();
		}
		
		// ��Ʈ ũ�� ������ ���
		if(m_lFontSize !=0 && m_bFixedFontSize)
		{
			lFontSize = m_lFontSize;
		}

		// �� ũ�⿡ �ش��ϴ� ��Ʈ�� ���� ��� ��Ʈ�� ����
		if( m_mapFonts.find(lFontSize) == m_mapFonts.end() )
		{
			font = CreateFont(lFontSize, 0, 0, 0, tm.tmWeight, tm.tmItalic, tm.tmUnderlined, tm.tmStruckOut,
				HANGEUL_CHARSET,	//ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,
				DEFAULT_PITCH,		// | FF_SWISS,
				m_strFontFace);

			m_mapFonts[lFontSize] = font;

		}
		else
		{
			font = m_mapFonts[lFontSize];
		}

		hRetVal = (HFONT)SelectObject(hdc, font);

	}


	return hRetVal;

}


//////////////////////////////////////////////////////////////////////////
//
// �� �ɼ� ���� ������ ���� �ɼǵ��� ��� �ʱ�ȭ���ִ� �Լ�
//
//////////////////////////////////////////////////////////////////////////
void CATCodeMgr::ResetOption()
{
	// UI �Ŵ��� ����
	m_UIMgr.Close();

	// ��ŷ ��õ� ���� ����� Ŭ����
	m_listRetryHook.clear();
	
	// ��ŷ�� ATCode�� ����
	for(list<CHookPoint*>::iterator iter = m_listHookPoint.begin();
		iter != m_listHookPoint.end();
		iter++)
	{
		CHookPoint* pPoint = (*iter);
		delete pPoint;
	}
	m_listHookPoint.clear();

	m_nFontLoadLevel = 0;
	m_nUniKofilterLevel = 0;
	m_nM2WLevel = 0;
	m_nEncodeKorean = 0;	
	m_bUITrans = FALSE;	
	m_bNoAslr = FALSE;	
	m_bFixedFontSize = FALSE;
	m_lFontSize = 0;
	m_strFontFace = _T("");
	m_bCompareStringW = 0;

}


//////////////////////////////////////////////////////////////////////////
//
// ���� �������� ȣȯ�� ���� �ɼ� ���̱׷��̼�
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::MigrateOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;

	BOOL bRetVal = TRUE;
	BOOL bNeedMigration = FALSE;

	BOOL bOverwrite = FALSE;
	BOOL bPtrCheat = FALSE;
	BOOL bRemoveSpace = FALSE;
	BOOL bTwoByte = FALSE;

	NOTIFY_DEBUG_MESSAGE(_T("MigrateOption: Option = %s\n"), (LPCTSTR)pRootNode->ToString());
	bool bTest=false;

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
		}
		
		// OVERWRITE �ɼ�
		else if(strValue == _T("OVERWRITE"))
		{
			bNeedMigration = TRUE;
			bOverwrite = TRUE;
		}
		// PTRCHEAT �ɼ�
		else if(strValue == _T("PTRCHEAT"))
		{
			bNeedMigration = TRUE;
			bPtrCheat = TRUE;
		}
		// REMOVESPACE �ɼ�
		else if(strValue == _T("REMOVESPACE"))
		{
			bNeedMigration = TRUE;
			bRemoveSpace = TRUE;
		}
		// TWOBYTE �ɼ�
		else if(strValue == _T("TWOBYTE"))
		{
			bNeedMigration = TRUE;
			bTwoByte = TRUE;
		}

		// HOOK(TRANS(OVERWRITE(BUFCHANGE,LENCHANGE))) �ɼ� -> HOOK(TRANS(SMSTR)) �ɼ����� ����
		else if (strValue == _T("HOOK"))
		{
			BOOL bRetnPos = 0;
			int cnt2 = pNode->GetChildCount();
			for(int i2=0; i2<cnt2; i2++)
			{
				COptionNode * pNode2 = pNode->GetChild(i2);
				CString strValue2 = pNode2->GetValue().MakeUpper();

				if(strValue2 == _T("TRANS"))
				{
					int cnt3 = pNode2->GetChildCount();
					for (int i3=0; i3 < cnt3; i3++)
					{
						COptionNode * pNode3 = pNode2->GetChild(i3);
						CString strValue3 = pNode3->GetValue().MakeUpper();

						if (strValue3 == _T("OVERWRITE"))
						{
							if (pNode3->GetChild(_T("BUFCHANGE")) || pNode3->GetChild(_T("LENCHANGE")))
							{
								// ã����. ���� ��¥ ������ oTL

								NOTIFY_DEBUG_MESSAGE(_T("  OVERWRITE->SMSTR CHANGED\n"));
								bTest = true;

								// �ɼǸ� ����
								pNode3->SetValue(_T("SMSTR"));

								// �ʿ���� �����ɼ� ����
								// NOTE: SMSTR�� IGNORE �� OVERWRITE(BUFCHANGE,IGNORE) �ʹ� �ٸ� �ǹ̸� �����Ƿ�
								//       MigrateOption() ������ �ϴ� �����Ѵ�.

								pNode3->DeleteChild(_T("BUFCHANGE"));
								pNode3->DeleteChild(_T("LENCHANGE"));
								pNode3->DeleteChild(_T("IGNORE"));

								break;
							}
						}
					}	// for (int i3 ...

				}	// if (... == "TRANS")
				else if(strValue2 == _T("RETNPOS")) bRetnPos = 1;
			}	// for (int i2...)
			if(bRetnPos == 0)
			{
				COptionNode * pNode2 = pNode->CreateChild();
				pNode2->SetValue(_T("RETNPOS"));
				COptionNode * pNode3 = pNode2->CreateChild();
				pNode3->SetValue(_T("COPY"));
				m_nHookType = 0;
			}
		}	// if (... == "HOOK")
	}	

	// DEBUG
	if (bTest) NOTIFY_DEBUG_MESSAGE(_T("  New Option = %s\n\n"), (LPCTSTR)pRootNode->ToString());
	else NOTIFY_DEBUG_MESSAGE(_T("\n"));


	// ���̱׷��̼��� �ʿ��ϸ�
	if(bNeedMigration)
	{
		// �ʿ���� ��� ����
		if (bOverwrite)
			pRootNode->DeleteChild(_T("OVERWRITE"));
		if (bPtrCheat)
			pRootNode->DeleteChild(_T("PTRCHEAT"));
		if (bRemoveSpace)
			pRootNode->DeleteChild(_T("REMOVESPACE"));
		if (bTwoByte)
			pRootNode->DeleteChild(_T("TWOBYTE"));
		
		// ������ ���� ��� HOOK ��忡 ����
		cnt = pRootNode->GetChildCount();
		for(int i=0; i<cnt; i++)
		{
			COptionNode* pNode = pRootNode->GetChild(i);
			CString strValue = pNode->GetValue().MakeUpper();

			// HOOK ���
			if(strValue == _T("HOOK"))
			{
				int cnt2 = pNode->GetChildCount();
				for(int j=0; j<cnt2; j++)
				{
					COptionNode* pTransNode = pNode->GetChild(j);
					strValue = pTransNode->GetValue().MakeUpper();

					// TRANS ���
					if(strValue == _T("TRANS"))
					{
						COptionNode* pChildNode = NULL;

						// �ؽ�Ʈ ���� ��� ����
						CString strTransMethod;
						if(bPtrCheat) strTransMethod = _T("PTRCHEAT");
						else strTransMethod = _T("OVERWRITE");

						pTransNode->DeleteChild(_T("NOP"));
						pTransNode->DeleteChild(_T("PTRCHEAT"));
						pTransNode->DeleteChild(_T("OVERWRITE"));
						pTransNode->DeleteChild(_T("REMOVESPACE"));
						pTransNode->DeleteChild(_T("TWOBYTE"));


						// �����۾� ���
						pChildNode = pTransNode->CreateChild();
						pChildNode->SetValue(strTransMethod);
							
						// ��������
						if(bRemoveSpace)
						{
							pChildNode = pTransNode->CreateChild();
							pChildNode->SetValue(_T("REMOVESPACE"));
						}

						// 1����Ʈ�� 2����Ʈ�� ��ȯ
						if(bTwoByte)
						{
							pChildNode = pTransNode->CreateChild();
							pChildNode->SetValue(_T("TWOBYTE"));
						}

					}	// TRANS ��� ��

				}
				
			} // HOOK ��� ��
		}	
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// �ɼ��� ���� ���α׷��� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::AdjustOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;
	
	ResetOption();
	MigrateOption(pRootNode);

	//FORCEFONT,HOOK(0x00434343,TRANS([ESP+0x4],ANSI,ALLSAMETEXT))
	BOOL bRetVal = TRUE;

	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();
		
		// FORCEFONT �ɼ�
		if(strValue == _T("FORCEFONT"))
		{
			COptionNode* pLevelNode = pNode->GetChild(0);
			m_nFontLoadLevel = _ttoi(pLevelNode->GetValue().Trim());
		}
		// UNIKOFILTER �ɼ�
		if(strValue == _T("UNIKOFILTER"))
		{
			COptionNode* pLevelNode = pNode->GetChild(0);
			m_nUniKofilterLevel = _ttoi(pLevelNode->GetValue().Trim());
		}
		// MULTTOWIDE �ɼ�
		if(strValue == _T("MULTTOWIDE"))
		{
			COptionNode* pLevelNode = pNode->GetChild(0);
			m_nM2WLevel = _ttoi(pLevelNode->GetValue().Trim());
		}
		// FIXFONTSIZE �ɼ�
		else if(strValue == _T("FIXFONTSIZE"))
		{
			m_bFixedFontSize = TRUE;
		}
		// FONT �ɼ�
		else if(strValue == _T("FONT") && pNode->GetChildCount() == 2)
		{
			// ��Ʈ ���̽���
			COptionNode* pFontFaceNode = pNode->GetChild(0);
			if(pFontFaceNode && m_strFontFace != pFontFaceNode->GetValue())
			{
				// ���� ��Ƴ��� ��Ʈ ��ü�� ����
				for(map<long, HFONT>::iterator iter = m_mapFonts.begin();
					iter != m_mapFonts.end();
					iter++)
				{
					DeleteObject(iter->second);
				}
				m_mapFonts.clear();

				// ���ο� ��Ʈ ���̽� ����
				m_strFontFace = pFontFaceNode->GetValue();
			}

			// ��Ʈ ������
			COptionNode* pFontSizeNode = pNode->GetChild(1);
			if(pFontSizeNode)
			{
				m_lFontSize = (long)_ttoi(pFontSizeNode->GetValue());
			}
		}
		// ENCODEKOR �ɼ�
		else if(strValue == _T("ENCODEKOR"))
		{

			if (pNode->GetChildCount())
			{
				COptionNode* pLevelNode = pNode->GetChild(0);
				m_nEncodeKorean = _ttoi(pLevelNode->GetValue().Trim());
			}
			else
				m_nEncodeKorean = 1;

			NOTIFY_DEBUG_MESSAGE(_T("ENCODEKOR(%d)\r\n"), m_nEncodeKorean);
		}
		// UITRANS �ɼ�
		else if(strValue == _T("UITRANS"))
		{
			m_bUITrans = TRUE;

			// UI ��ȯ
			// ���ۿ� ���Ƽ� �ּ�ó�� �ұ� �����..
			if(m_UIMgr.Init())
			{
				m_UIMgr.TransMenu();					
				m_UIMgr.TransTitle();
			}

		}
		// NOASLR �ɼ�
		else if(strValue == _T("NOASLR"))
		{
			m_bNoAslr = TRUE;
		}
		// HOOK ���
		else if(strValue == _T("HOOK"))
		{
			BOOL bHookRes = HookFromOptionNode(pNode);
			if(FALSE == bHookRes) m_listRetryHook.push_back(pNode);
		}
		// COMPAREJP ���
		else if(strValue == _T("COMPAREJP"))
		{
			m_bCompareStringW = TRUE;
		} // HOOK ��� ��
	}

	// MULTTOWIDE ����
	if( m_nM2WLevel >= 5 &&
		m_nM2WLevel <= 25 )
		// MultiByteToWideChar Hook
		m_sContainerFunc.pfnHookDllFunctionEx( "kernel32.dll", "MultiByteToWideChar", NewMultiByteToWideChar, 1 );
	else
	{
		if( m_sTextFunc.pfnOrigMultiByteToWideChar )
		{
			m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "MultiByteToWideChar" );
			//m_sTextFunc.pfnOrigMultiByteToWideChar =
				//(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));
			//m_sTextFunc.pfnOrigMultiByteToWideChar = NULL;
		}
	}

	// COMPAREJP ����
	if(m_bCompareStringW) m_sContainerFunc.pfnHookDllFunctionEx( "kernel32.dll", "CompareStringW", NewCompareStringW, 1 );
	else m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "CompareStringW" );

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// ���� AdjustOption ���� HOOK ��带 ������� �� �Լ��� ȣ���ؼ� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::HookFromOptionNode(COptionNode* pNode)
{
	BOOL bRetVal = FALSE;
	
	try
	{
		// ��ŷ�� �ּ�
		COptionNode* pAddrNode = pNode->GetChild(0);
		if(pAddrNode==NULL) throw -1;

		// ��ŷ ��� ����
		int cnt2 = pNode->GetChildCount();
		for(int j=1; j<cnt2; j++)
		{
			COptionNode * pNode2 = pNode->GetChild(j);
			CString strHookValue = pNode2->GetValue();
			if(strHookValue.CompareNoCase(_T("RETNPOS"))==0)
			{
				COptionNode * pNode3 = pNode2->GetChild(0);
				CString strHookValue2 = pNode3->GetValue();
				if(strHookValue2.CompareNoCase(_T("SOURCE"))==0) m_nHookType = 1;
				else m_nHookType = 0;
			}
		}

		CHookPoint* pHookPoint = CHookPoint::CreateInstance(pAddrNode->GetValue());
		if(pHookPoint==NULL)
		{
			//MessageBox(m_hContainerWnd, _T("������ �ּҸ� ��ŷ�ϴµ� �����߽��ϴ� : ") + pAddrNode->GetValue(), _T("Hook error"), MB_OK);
			//continue;
			 throw -2;
		}
		m_listHookPoint.push_back(pHookPoint);

		// �� �ּҿ� ���� ��ŷ ��ɵ� ����
		cnt2 = pNode->GetChildCount();
		for(int j=1; j<cnt2; j++)
		{
			COptionNode* pNode2 = pNode->GetChild(j);
			CString strHookValue = pNode2->GetValue();

			// ���� ���
			if(strHookValue.CompareNoCase(_T("TRANS"))==0)
			{
				if(pNode2->GetChildCount() == 0) continue;

				/*
				// ���� �Ÿ�
				COptionNode* pDistNode = pNode2->GetChild(0);

				int nDistFromESP = 0;
				CString strStorage = pDistNode->GetValue().MakeUpper();

				if(strStorage==_T("[ESP]")) nDistFromESP = 0x0;
				else if(strStorage==_T("EAX")) nDistFromESP = -0x4;
				else if(strStorage==_T("ECX")) nDistFromESP = -0x8;
				else if(strStorage==_T("EDX")) nDistFromESP = -0xC;
				else if(strStorage==_T("EBX")) nDistFromESP = -0x10;
				else if(strStorage==_T("ESP")) nDistFromESP = -0x14;
				else if(strStorage==_T("EBP")) nDistFromESP = -0x18;
				else if(strStorage==_T("ESI")) nDistFromESP = -0x1C;
				else if(strStorage==_T("EDI")) nDistFromESP = -0x20;
				else
				{

					_stscanf((LPCTSTR)strStorage, _T("[ESP+%x]"), &nDistFromESP);
					if(nDistFromESP == 0) continue;
				}

				CTransCommand* pTransCmd = pHookPoint->AddTransCmd(nDistFromESP);
				*/

				CTransCommand* pTransCmd = pHookPoint->AddTransCmd(pNode2);

				/*if( pTransCmd ->m_pTransTextBuf == NULL )
					pTransCmd ->m_pTransTextBuf = new BYTE[MAX_TEXT_LENGTH*2];*/
				ZeroMemory(pTransCmd ->m_pTransTextBuf, MAX_TEXT_LENGTH*2);
				
			}

		}	// end of for ( �� �ּҿ� ���� ��ŷ ��ɵ� ���� )

		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode; 
	}


	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// Ŭ�����忡 �ؽ�Ʈ ���� �۾��� �ϴ� ������
//
//////////////////////////////////////////////////////////////////////////
UINT __stdcall CATCodeMgr::ClipboardThreadFunc(LPVOID pParam)
{
	while(_Inst && _Inst->m_bRunClipboardThread)
	{
		DWORD dwRes = WaitForSingleObject(_Inst->m_hClipTextChangeEvent, 300);

		// ��ٷ��� ���� ���� ��
		if(WAIT_TIMEOUT == dwRes)
		{
			EnterCriticalSection(&_Inst->m_csClipText);

			// Ŭ������� ������ �� �����Ͱ� �ִٸ�
			if(_Inst->m_strClipText.IsEmpty() == FALSE)
			{
				HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (_Inst->m_strClipText.GetLength() + 1) * sizeof(TCHAR));

				LPTSTR pGlobal = (LPTSTR)GlobalLock(hGlobal);
				
				if(pGlobal)
				{
					_tcscpy(pGlobal, (LPCTSTR)_Inst->m_strClipText);
					GlobalUnlock(hGlobal);

					OpenClipboard(NULL);
					EmptyClipboard();

#ifdef UNICODE
					SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
					SetClipboardData(CF_TEXT, hGlobal);
#endif
					
					CloseClipboard();

					//GlobalFree(hGlobal);
				}

				_Inst->m_strClipText.Empty();
			}

			LeaveCriticalSection(&_Inst->m_csClipText);
		}
		// ��ٸ��� �� �ؽ�Ʈ�� �߰� �Ǿ��� ��
		else if(WAIT_OBJECT_0 == dwRes)
		{
			
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Ŭ�����忡 �ؽ�Ʈ ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::SetClipboardText(LPCTSTR cszText)
{
	BOOL bRetVal = FALSE;

	EnterCriticalSection(&m_csClipText);

	m_strClipText += cszText;

	LeaveCriticalSection(&m_csClipText);

	::SetEvent(m_hClipTextChangeEvent);

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ���ο� �ε� ���̺귯�� �Լ� (A)
// AT�ڵ尡 DLL ������ ���, �� DLL�� ���� �ε�� �� �𸣹Ƿ�
// �̰����� �����ϰ� �ִٰ� �ε�Ǵ� ���� ��ŷ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryA(LPCSTR lpFileName)
{
	wchar_t wszTmp[MAX_PATH];
	MyMultiByteToWideChar(CP_ACP, 0, lpFileName, -1, wszTmp, MAX_PATH);
	TRACE(_T("[aral1] NewLoadLibraryA('%s') \n"), wszTmp);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning)
	{
		hModule = _Inst->m_pfnLoadLibraryA(lpFileName);

		// �����ߴ� ��ŷ���� ��õ�
		for(list<COptionNode*>::iterator iter = _Inst->m_listRetryHook.begin();
			iter != _Inst->m_listRetryHook.end();)
		{
			BOOL bHookRes = _Inst->HookFromOptionNode( (*iter) );
			
			if(bHookRes) iter = _Inst->m_listRetryHook.erase(iter);
			else iter++;
		}

	}

	return hModule;
}


//////////////////////////////////////////////////////////////////////////
//
// ���ο� �ε� ���̺귯�� �Լ� (W)
// AT�ڵ尡 DLL ������ ���, �� DLL�� ���� �ε�� �� �𸣹Ƿ�
// �̰����� �����ϰ� �ִٰ� �ε�Ǵ� ���� ��ŷ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryW(LPCWSTR lpFileName)
{
	TRACE(_T("[aral1] NewLoadLibraryW('%s') \n"), lpFileName);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning)
	{
		hModule = _Inst->m_pfnLoadLibraryW(lpFileName);

		// �����ߴ� ��ŷ���� ��õ�
		for(list<COptionNode*>::iterator iter = _Inst->m_listRetryHook.begin();
			iter != _Inst->m_listRetryHook.end();)
		{
			BOOL bHookRes = _Inst->HookFromOptionNode( (*iter) );

			if(bHookRes) iter = _Inst->m_listRetryHook.erase(iter);
			else iter++;
		}

	}

	return hModule;
}


//////////////////////////////////////////////////////////////////////////
// GetGlyphOutlineA ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall CATCodeMgr::NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{	
	HFONT hOrigFont = NULL;
	char chArray[10] = {0,};
	CCharacterMapper *pcCharMap = NULL;

	if (CATCodeMgr::_Inst->m_nEncodeKorean)
	{
		if (CATCodeMgr::_Inst->m_nEncodeKorean == 2)
			pcCharMap = new CCharacterMapper2;
		else
			pcCharMap = new CCharacterMapper;
	}

	// char �迭�� ���� ����
	size_t i,j;
	j = 0;
	for(i=sizeof(/*UINT*/WCHAR); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
		}
	}

	// ��Ʈ �˻�
	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	{
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	UINT nCodePage = 932;
	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning)
	{
		// �ѱ� ���ڵ� ���� �˻�		
		if(CATCodeMgr::_Inst->m_nEncodeKorean 
			&& pcCharMap->IsEncodedText(chArray) /*0x88 <= (BYTE)chArray[0] && (BYTE)chArray[0] <= 0xEE*/
			&& 0x00 != (BYTE)chArray[1])
		{
			chArray[2] = '\0';
			char tmpbuf[10]  = {0,};

			if( pcCharMap->DecodeJ2K(chArray, tmpbuf) )
			{
				chArray[0] = tmpbuf[0];
				chArray[1] = tmpbuf[1];
				nCodePage = 949;
			}
		}  // end of if(CATCodeMgr::_Inst->m_nEncodeKorean)
		else if (!CATCodeMgr::_Inst->m_nEncodeKorean)
			nCodePage = 949;
	}

	wchar_t wchArray[10];
	int k=0;

	/*
	** m_nEncodeKorean �� ���������� DecodeJ2K �� �����ϸ� �ڵ������� 949
	** Ȥ�� m_nEncodeKorean �� ���������� �ڵ������� 949
	** �ܴ̿� ������ �ڵ������� 932 �� ó��

	//if(0x80 < (BYTE)chArray[0] && (BYTE)chArray[0] < 0xA0) nCodePage = 932;
	if(0x80 < (BYTE)chArray[0] && (BYTE)chArray[0] < 0xA0)
	{
		nCodePage = 932;
		//80~A0 ������ �ִ� �ѱ��� �ڵ������� ������ ���ϵ��� ���� ó��
		while(ko_char_map[k])
		{
			if(ko_char_map[k] == *((WORD*)chArray))
			{
				nCodePage = 949;
				break;
			}

			(BYTE)k++;
		}
	}
	*/

	MyMultiByteToWideChar(nCodePage, 0, chArray, sizeof(UINT), wchArray, 10 );
	
	DWORD dwRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnGetGlyphOutlineW(hdc, (UINT)wchArray[0], uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	
	// ��Ʈ ����
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	if (pcCharMap) delete pcCharMap;

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
// GetGlyphOutlineW ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall CATCodeMgr::NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	HFONT hOrigFont = NULL;

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	{
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	DWORD dwRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	// ��Ʈ ����
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return dwRetVal;
}



//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;
	CCharacterMapper *pcCharMap = NULL;

	if (CATCodeMgr::_Inst->m_nEncodeKorean)
	{
		if (CATCodeMgr::_Inst->m_nEncodeKorean == 2)
			pcCharMap = new CCharacterMapper2;
		else
			pcCharMap = new CCharacterMapper;
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// ��Ʈ �˻�
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && lpString && cbString > 0 )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(a_idx<cbString && lpString[a_idx])
		{
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char �迭�� ���� ����
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				UINT nCodePage = 932;
				
				// �ѱ� ���ڵ� ���� �˻�		
				//if(CATCodeMgr::_Inst->m_nEncodeKorean && 0xE0 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xFD )
				if(CATCodeMgr::_Inst->m_nEncodeKorean && pcCharMap->IsEncodedText(lpString+a_idx)/*0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE*/ )
				{
					if (pcCharMap->DecodeJ2K(&lpString[a_idx], tmpbuf))
						nCodePage = 949;
				}
				else if (!CATCodeMgr::_Inst->m_nEncodeKorean)
					nCodePage = 949;

				/*
				** m_nEncodeKorean �� ���������� DecodeJ2K �� �����ϸ� �ڵ������� 949
				** Ȥ�� m_nEncodeKorean �� ���������� �ڵ������� 949
				** �ܴ̿� ������ �ڵ������� 932 �� ó��

				nCodePage = 949;
				int k=0;

				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;
				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0)
				{
					nCodePage = 932;
					//80~A0 ������ �ִ� �ѱ��� �ڵ������� ������ ���ϵ��� ���� ó��
					while(ko_char_map[k])
					{
						if(ko_char_map[k] == *((WORD*)tmpbuf))
						{
							nCodePage = 949;
							break;
						}
						
						(BYTE)k++;
					}
				}
				*/

				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}			// 1����Ʈ ���ڸ�
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// TextOutW ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, wchArray, w_idx);
	}
	else
	{
		// �����Լ� ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}


	// ��Ʈ ����
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);
	
	if (pcCharMap) delete pcCharMap;

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	HFONT hOrigFont = NULL;

	BOOL bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);

	// ��Ʈ ����
	//if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewExtTextOutA(
	HDC hdc,          // handle to DC
	int nXStart,            // x-coordinate of reference point
	int nYStart,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString, // string
	UINT cbString,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;
	CCharacterMapper *pcCharMap = NULL;

	if (CATCodeMgr::_Inst->m_nEncodeKorean)
	{
		if (CATCodeMgr::_Inst->m_nEncodeKorean == 2)
			pcCharMap = new CCharacterMapper2;
		else
			pcCharMap = new CCharacterMapper;
	}


	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// ��Ʈ �˻�
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && lpString && cbString > 0 )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		UINT a_idx = 0;
		UINT w_idx = 0;

		while(a_idx < cbString && lpString[a_idx])
		{
			// 2����Ʈ ���ڸ�
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char �迭�� ���� ����
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				UINT nCodePage = 932;
				// �ѱ� ���ڵ� ���� �˻�		
				//if(CATCodeMgr::_Inst->m_nEncodeKorean && 0xE0 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xFD )
				/*if(CATCodeMgr::_Inst->m_nEncodeKorean && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{
					pcCharMap->DecodeJ2K(&lpString[a_idx], tmpbuf);
					nCodePage = 949;
				}
				else
				{
					nCodePage = 932;
				}*/

				if(CATCodeMgr::_Inst->m_nEncodeKorean && pcCharMap->IsEncodedText(lpString+a_idx)/*0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE*/ )
				{
					if (pcCharMap->DecodeJ2K(&lpString[a_idx], tmpbuf))
						nCodePage = 949;
				}
				else if (!CATCodeMgr::_Inst->m_nEncodeKorean)
					nCodePage = 949;


				/*
				** m_nEncodeKorean �� ���������� DecodeJ2K �� �����ϸ� �ڵ������� 949
				** Ȥ�� m_nEncodeKorean �� ���������� �ڵ������� 949
				** �ܴ̿� ������ �ڵ������� 932 �� ó��

				nCodePage = 949;
				int k=0;

				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0)
				{
					nCodePage = 932;
					//80~A0 ������ �ִ� �ѱ��� �ڵ������� ������ ���ϵ��� ���� ó��
					while(ko_char_map[k])
					{
						if(ko_char_map[k] == *((WORD*)tmpbuf))
						{
							nCodePage = 949;
							break;
						}
						
						(BYTE)k++;
					}
				}
				*/

				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1����Ʈ ���ڸ�
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// ExtTextOutW ȣ��
		//bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, wchArray, w_idx, lpDx);
		bRetVal = ExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, wchArray, w_idx, lpDx);
	}
	else
	{
		// �����Լ� ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnExtTextOutA(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbString, lpDx);
	}


	// ��Ʈ ����
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	if (pcCharMap) delete pcCharMap;

	return bRetVal;

}



//////////////////////////////////////////////////////////////////////////
// UI �Ŵ������� ��ŷ�ϹǷ� �� �Լ��� ������ �ȵȴ�
//////////////////////////////////////////////////////////////////////////
/*
BOOL __stdcall CATCodeMgr::NewExtTextOutW(
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
	
	return CATCodeMgr::_Inst->m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}
*/

//////////////////////////////////////////////////////////////////////////
// DrawTextA ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextA(
   HDC hDC,          // handle to DC
   LPCSTR lpString,  // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;
	CCharacterMapper *pcCharMap = NULL;

	if (CATCodeMgr::_Inst->m_nEncodeKorean)
	{
		if (CATCodeMgr::_Inst->m_nEncodeKorean == 2)
			pcCharMap = new CCharacterMapper2;
		else
			pcCharMap = new CCharacterMapper;
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// ��Ʈ �˻�
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hDC);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning  && lpString)	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;
		
		if(nCount == -1) nCount = strlen(lpString);

		while(a_idx<nCount && lpString[a_idx])
		{
			// 2����Ʈ ���ڸ�
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char �迭�� ���� ����
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				UINT nCodePage = 932;
				// �ѱ� ���ڵ� ���� �˻�		
				if(CATCodeMgr::_Inst->m_nEncodeKorean && pcCharMap->IsEncodedText(lpString+a_idx)/*0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE*/ )
				{
					if (pcCharMap->DecodeJ2K(&lpString[a_idx], tmpbuf))
						nCodePage = 949;
				}
				else if (!CATCodeMgr::_Inst->m_nEncodeKorean)
					nCodePage = 949;

				/*
				** m_nEncodeKorean �� ���������� DecodeJ2K �� �����ϸ� �ڵ������� 949
				** Ȥ�� m_nEncodeKorean �� ���������� �ڵ������� 949
				** �ܴ̿� ������ �ڵ������� 932 �� ó��

				nCodePage = 949;
				int k=0;

				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;
				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0)
				{
					nCodePage = 932;
					//80~A0 ������ �ִ� �ѱ��� �ڵ������� ������ ���ϵ��� ���� ó��
					while(ko_char_map[k])
					{
						if(ko_char_map[k] == *((WORD*)tmpbuf))
						{
							nCodePage = 949;
							break;
						}
						
						(BYTE)k++;
					}
				}
				*/

				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1����Ʈ ���ڸ�
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// DrawTextW ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextW(hDC, wchArray, w_idx, lpRect, uFormat);
	}
	else
	{
		// �����Լ� ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}


	// ��Ʈ ����
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	if (pcCharMap) delete pcCharMap;

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// DrawTextW ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextW(
   HDC hDC,          // handle to DC
   LPCWSTR lpString, // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
)
{
	HFONT hOrigFont = NULL;

	BOOL bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextW(hDC, lpString, nCount, lpRect, uFormat);

	// ��Ʈ ����
	//if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	return bRetVal;

}

//////////////////////////////////////////////////////////////////////////
// DrawTextExA ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextExA(
	 HDC hDC,                     // handle to DC
	 LPSTR lpString,              // text to draw
	 int nCount,                 // length of text to draw
	 LPRECT lpRect,                 // rectangle coordinates
	 UINT uFormat,             // formatting options
	 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
)
{

	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;
	CCharacterMapper *pcCharMap = NULL;

	if (CATCodeMgr::_Inst->m_nEncodeKorean)
	{
		if (CATCodeMgr::_Inst->m_nEncodeKorean == 2)
			pcCharMap = new CCharacterMapper2;
		else
			pcCharMap = new CCharacterMapper;
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// ��Ʈ �˻�
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hDC);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && lpString )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(a_idx<nCount && lpString[a_idx])
		{
			// 2����Ʈ ���ڸ�
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char �迭�� ���� ����
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				UINT nCodePage = 932;
				// �ѱ� ���ڵ� ���� �˻�		
				if(CATCodeMgr::_Inst->m_nEncodeKorean && pcCharMap->IsEncodedText(lpString+a_idx)/*0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE*/ )
				{
					if (pcCharMap->DecodeJ2K(&lpString[a_idx], tmpbuf))
						nCodePage = 949;
				}
				else if (!CATCodeMgr::_Inst->m_nEncodeKorean)
					nCodePage = 949;

				/*
				** m_nEncodeKorean �� ���������� DecodeJ2K �� �����ϸ� �ڵ������� 949
				** Ȥ�� m_nEncodeKorean �� ���������� �ڵ������� 949
				** �ܴ̿� ������ �ڵ������� 932 �� ó��

				nCodePage = 949;
				int k=0;
				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0)
				{
					nCodePage = 932;
					//80~A0 ������ �ִ� �ѱ��� �ڵ������� ������ ���ϵ��� ���� ó��
					while(ko_char_map[k])
					{
						if(ko_char_map[k] == *((WORD*)tmpbuf))
						{
							nCodePage = 949;
							break;
						}
						
						(BYTE)k++;
					}
				}
				*/

				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1����Ʈ ���ڸ�
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// DrawTextExW ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextExW(hDC, wchArray, w_idx, lpRect, uFormat, lpDTParams);
		lpRect->right -= 10;
	}
	else
	{
		// �����Լ� ȣ��
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}


	// ��Ʈ ����
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	if (pcCharMap) delete pcCharMap;

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
// DrawTextExW ��ü �Լ�
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextExW(
	 HDC hdc,                     // handle to DC
	 LPWSTR lpchText,              // text to draw
	 int cchText,                 // length of text to draw
	 LPRECT lprc,                 // rectangle coordinates
	 UINT dwDTFormat,             // formatting options
	 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
)
{
	HFONT hOrigFont = NULL;


	BOOL bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);

	// ��Ʈ ����
	//if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}








int CATCodeMgr::GetAllLoadedModules( PMODULEENTRY32 pRetBuf, int maxCnt )
{
	int curCnt = 0;

	// ��ȯ ���� �ʱ�ȭ
	ZeroMemory(pRetBuf, sizeof(PMODULEENTRY32)*maxCnt);
	
	// ���μ��� ������ �ڵ��� ����
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if(INVALID_HANDLE_VALUE == hModuleSnap) return 0;

	pRetBuf[curCnt].dwSize = sizeof(MODULEENTRY32);
	BOOL bExist = Module32First(hModuleSnap, &pRetBuf[curCnt]);

	while( bExist == TRUE && curCnt < maxCnt )
	{
		curCnt++;
		pRetBuf[curCnt].dwSize = sizeof(MODULEENTRY32);
		bExist = Module32Next(hModuleSnap, &pRetBuf[curCnt]);
	}

	CloseHandle (hModuleSnap);

	return curCnt;
}











//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontA( 
	int nHeight, 
	int nWidth, 
	int nEscapement, 
	int nOrientation, 
	int fnWeight, 
	DWORD fdwItalic, 
	DWORD fdwUnderline, 
	DWORD fdwStrikeOut, 
	DWORD fdwCharSet, 
	DWORD fdwOutputPrecision, 
	DWORD fdwClipPrecision, 
	DWORD fdwQuality, 
	DWORD fdwPitchAndFamily, 
	LPSTR lpszFace )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
	{
		wchar_t wszFace[32] = {0,};
		if(lpszFace) MyMultiByteToWideChar(932, 0, lpszFace, 32, wszFace, 32);

		hFont = CATCodeMgr::_Inst->InnerCreateFont(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			wszFace );
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontA(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpszFace );
	}


	TRACE(_T("[aral1] NewCreateFontA returns 0x%p \n"), hFont);

	return hFont;

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontW( 
	int nHeight, 
	int nWidth, 
	int nEscapement, 
	int nOrientation, 
	int fnWeight, 
	DWORD fdwItalic, 
	DWORD fdwUnderline, 
	DWORD fdwStrikeOut, 
	DWORD fdwCharSet, 
	DWORD fdwOutputPrecision, 
	DWORD fdwClipPrecision, 
	DWORD fdwQuality, 
	DWORD fdwPitchAndFamily, 
	LPWSTR lpwszFace )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 20)
	{
		hFont = CATCodeMgr::_Inst->InnerCreateFont(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpwszFace );
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontW(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpwszFace );
	}


	TRACE(_T("[aral1] NewCreateFontW returns 0x%p \n"), hFont);

	return hFont;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontIndirectA( LOGFONTA* lplf )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
	{
		LOGFONTW lfWide;
		ZeroMemory(&lfWide, sizeof(LOGFONTW));
		lfWide.lfCharSet		= lplf->lfCharSet;
		lfWide.lfClipPrecision	= lplf->lfClipPrecision;
		lfWide.lfEscapement		= lplf->lfEscapement;
		lfWide.lfHeight			= lplf->lfHeight;
		lfWide.lfItalic			= lplf->lfItalic;
		lfWide.lfOrientation	= lplf->lfOrientation;
		lfWide.lfOutPrecision	= lplf->lfOutPrecision;
		lfWide.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lfWide.lfQuality		= lplf->lfQuality;
		lfWide.lfStrikeOut		= lplf->lfStrikeOut;
		lfWide.lfUnderline		= lplf->lfUnderline;
		lfWide.lfWeight			= lplf->lfWeight;
		lfWide.lfWidth			= lplf->lfWidth;
		MyMultiByteToWideChar(932, 0, lplf->lfFaceName, 32, lfWide.lfFaceName, 32);

		hFont = CATCodeMgr::_Inst->InnerCreateFontIndirect(&lfWide);
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontIndirectA(lplf);
	}


	TRACE(_T("[aral1] NewCreateFontIndirectA returns 0x%p \n"), hFont);

	return hFont;

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontIndirectW( LOGFONTW* lplf )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 20)
	{
		hFont = CATCodeMgr::_Inst->InnerCreateFontIndirect(lplf);
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontIndirectW(lplf);
	}


	//TRACE(_T("[aral1] NewCreateFontIndirectW returns 0x%p \n"), hFont);

	return hFont;
}


HFONT CATCodeMgr::InnerCreateFont(
	int nHeight,               // height of font
	int nWidth,                // average character width
	int nEscapement,           // angle of escapement
	int nOrientation,          // base-line orientation angle
	int fnWeight,              // font weight
	DWORD fdwItalic,           // italic attribute option
	DWORD fdwUnderline,        // underline attribute option
	DWORD fdwStrikeOut,        // strikeout attribute option
	DWORD fdwCharSet,          // character set identifier
	DWORD fdwOutputPrecision,  // output precision
	DWORD fdwClipPrecision,    // clipping precision
	DWORD fdwQuality,          // output quality
	DWORD fdwPitchAndFamily,   // pitch and family
	LPWSTR lpszFace           // typeface name
	)
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
	{
		fdwCharSet = HANGEUL_CHARSET;	//ANSI_CHARSET,
		fdwOutputPrecision = OUT_DEFAULT_PRECIS;
		fdwClipPrecision = CLIP_DEFAULT_PRECIS;
		fdwQuality = ANTIALIASED_QUALITY;
		fdwPitchAndFamily = DEFAULT_PITCH;		// | FF_SWISS,

		if(m_strFontFace.IsEmpty())
		{
			m_strFontFace = _T("Gungsuh");
		}

#ifdef UNICODE			
		if(lpszFace) wcscpy(lpszFace, (LPCWSTR)m_strFontFace);
#else
		if(lpszFace) MyMultiByteToWideChar(949, 0, m_strFontFace, -1, lpszFace, 32);
#endif


		// ��Ʈ ũ�� ������ ���
		if(m_lFontSize !=0 && m_bFixedFontSize)
		{
			nHeight = m_lFontSize;
		}

	}


	hFont = m_sFontFunc.pfnCreateFontW(
		nHeight, 
		nWidth, 
		nEscapement, 
		nOrientation, 
		fnWeight, 
		fdwItalic, 
		fdwUnderline, 
		fdwStrikeOut, 
		fdwCharSet, 
		fdwOutputPrecision, 
		fdwClipPrecision, 
		fdwQuality, 
		fdwPitchAndFamily, 
		lpszFace );


	return hFont;	
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT CATCodeMgr::InnerCreateFontIndirect( LOGFONTW* lplf )
{
	//static CString strLastFontFace = _T("");

	HFONT hFont = NULL;

	if(lplf)
	{
		if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
		{
			lplf->lfCharSet = HANGEUL_CHARSET;	//ANSI_CHARSET,
			lplf->lfOutPrecision = OUT_DEFAULT_PRECIS;
			lplf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lplf->lfQuality = ANTIALIASED_QUALITY;
			lplf->lfPitchAndFamily = DEFAULT_PITCH;		// | FF_SWISS,

			if(m_strFontFace.IsEmpty())
			{
				m_strFontFace = _T("Gungsuh");
			}
			
#ifdef UNICODE			
			wcscpy(lplf->lfFaceName, (LPCWSTR)m_strFontFace);
#else
			MyMultiByteToWideChar(949, 0, m_strFontFace, -1, lplf->lfFaceName, 32);
#endif
		
			// ��Ʈ ũ�� ������ ���
			if(m_lFontSize !=0 && m_bFixedFontSize)
			{
				lplf->lfHeight = m_lFontSize;
			}
		}

			
		hFont = m_sFontFunc.pfnCreateFontIndirectW(lplf);

	}


	//TRACE(_T("[aral1] NewCreateFontIndirectW returns 0x%p \n"), hFont);

	return hFont;	
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

int CATCodeMgr::NewCompareStringW(
	int nLocaleid,              // Localeid
	DWORD nOptions,            // options
	LPCSTR lpszText,           // String1
	int nSize,				   // text size
	LPWSTR lpszBuf,            // String2
	int bufSize                // buf size
	)
{
	int nRetVal = 0;
	
	if(CATCodeMgr::_Inst->m_bCompareStringW && (nLocaleid == 0x800))
	{
		nLocaleid = 0x411;
	}

	nRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar(
			nLocaleid, 
			nOptions, 
			lpszText, 
			nSize, 
			lpszBuf, 
			bufSize
			);

	return nRetVal;
}

int CATCodeMgr::NewMultiByteToWideChar(
	int nCodePage,             // code page
	DWORD nOptions,            // options
	LPCSTR lpszText,           // text
	int nSize,				   // text size
	LPWSTR lpszBuf,            // buf
	int bufSize
	)
{
	int nRetVal = 0;
	BOOL bANset = false;

	// ���÷����� ���� �Լ�
	if( LoadLibrary(_T("AlLayer.DLL")) != NULL )
	{
		CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));
		bANset = true;
	}
	// NTLEA �Լ� ����
	else if( LoadLibrary(_T("ntleah.dll")) != NULL )
	{
		CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));
		DWORD aMtW = (DWORD) CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar;
		__asm ADD aMtW, 6;
		CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar = (PROC_MultiByteToWideChar) aMtW;
		bANset = true;
	}
	else
		CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar = 
			(PROC_MultiByteToWideChar) CATCodeMgr::_Inst->m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "MultiByteToWideChar");

	if(nCodePage==CP_ACP || nCodePage== CP_OEMCP || nCodePage == 949 || nCodePage == 932)
	{
		if(CATCodeMgr::GetInstance()->IsKorean(lpszText))
			nCodePage = 0x3B5;
		else if(bANset)
			nCodePage = 0x3A4;
	}

	nRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar(
		nCodePage, 
		nOptions, 
		lpszText, 
		nSize, 
		lpszBuf, 
		bufSize
		);
	if( LoadLibrary(_T("ntleah.dll")) != NULL )
	{
		DWORD aMtW = (DWORD) CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar;
		__asm SUB aMtW, 6;
		CATCodeMgr::_Inst->m_sTextFunc.pfnOrigMultiByteToWideChar = (PROC_MultiByteToWideChar) aMtW;
	}
	
	return nRetVal;
}

BOOL CATCodeMgr::IsKorean(LPCSTR pszTestString)
{
	volatile BYTE szBuf[3] = { '\0', };
	WORD wch;
	int nKoCount, nJpCount;

	int i, nLen;

	BOOL bIsKorean, bIsnKorean;

	nLen=lstrlenA(pszTestString);

	// �⺻ üũ ��ƾ
	nKoCount=nJpCount=0;
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
		if (( (0x889F <= wch) && (wch <= 0x9872) ) ||	// 0x889F - 0x9872,
			( (0x989F <= wch) && (wch <= 0x9FFC) ) ||	// 0x989F - 0x9FFC,
			( (0xE040 <= wch) && (wch <= 0xEAA4) ) ||	// 0xE040 - 0xEAA4,
			( (0xED40 <= wch) && (wch <= 0xEEEC) ))		// 0xED40 - 0xEEEC
		{
			// �ѱ��ΰ�? (EncodeKor)

			// Decode �غ���
			CATCodeMgr::GetInstance()->Decode(pszTestString+i, (char *)szBuf);
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

		if (bIsKorean)
		{
			// �ѱ�
			// if (IsDBCSLeadByteEx(949, pszTestString[i]))
				nKoCount++;
			i++; // �̹� üũ�����Ƿ� ���� ���ڷ� �Ѿ	
		}
		else if(bIsnKorean)
			i++;
		else
		{
			// �Ͼ�
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
				{
					nJpCount++;
				}
			}
			i++;
		}
		// ��Ÿ ���ڴ� ����
	}
	
	// ���� ����
	bIsKorean=FALSE;
	switch(CATCodeMgr::GetInstance()->m_nM2WLevel)
	{
		case 0:		// ������� ����
			break;

		case 5:		// �Ͼ 1�ڶ� ������ �ڵ������� �������
			if (!nJpCount)
				bIsKorean=TRUE;
			break;

		case 10:	// �ѱ�x2 > �Ͼ� �϶� �ڵ������� ����
			if (nKoCount*2 > nJpCount)
				bIsKorean=TRUE;
			break;

		case 15:	// �ѱ� > �Ͼ� �϶� �ڵ������� ����
			if (nKoCount > nJpCount)
				bIsKorean=TRUE;
			break;

		case 20:	// �ѱ� > �Ͼ�x2 �϶� �ڵ������� ����
			if (nKoCount > nJpCount*2)
				bIsKorean=TRUE;
			break;

		case 25:	// �ѱ��� 1�ڶ� ������ �ڵ������� ����
			if (nKoCount)
				bIsKorean=TRUE;
			break;

		default:
			break;
	}
	return bIsKorean;
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////