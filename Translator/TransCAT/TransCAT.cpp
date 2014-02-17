// TransCAT.cpp : Defines the initialization routines for the DLL.
//
#pragma warning(disable:4996)

#include "stdafx.h"
#include "TransCAT.h"
#include "hash.hpp"
#include "TransCATOptionDlg.h"
#include <stdio.h>
#include <process.h>
#include <map>

#define TRANS_BUF_SIZE 4096

//   ���� 2Byte :    KS5601 ���� ��ȣ �ڵ�
// ������ 2Byte : SHIFT-JIS ���� ��ȣ �ڵ�
const char _SHIFT_JIS_CHAR_MAP[][4] = {
	{'\xA1', '\xCD', '\x81', '\x8F'},	// '\\'
	{'\xA3', '\xC0', '\x81', '\x97'},	// '@'
	{'\xA3', '\xA8', '\x81', '\x69'},	// '('
	{'\xA3', '\xA9', '\x81', '\x6A'},	// ')'
	{'\xA3', '\xBC', '\x81', '\x83'},	// '<'
	{'\xA3', '\xBE', '\x81', '\x84'},	// '>'
	{'\xA3', '\xDB', '\x81', '\x6D'},	// '['
	{'\xA3', '\xDD', '\x81', '\x6E'},	// ']'
	{'\xA3', '\xFB', '\x81', '\x6F'},	// '{'
	{'\xA3', '\xFD', '\x81', '\x70'},	// '}'
	{'\xA2', '\xC8', '\x81', '\x4A'},	// ��
	{'\xA2', '\xC8', '\x81', '\x4B'},	// ��
	{'\xA2', '\xC8', '\x81', '\x8D'},	// ��
	{'\xA2', '\xC7', '\x81', '\x4C'},	// ��
	{'\xA2', '\xC7', '\x81', '\x4D'},	// ��
	{'\xA2', '\xC7', '\x81', '\x8C'},	// ��
	{'\xA1', '\xAE', '\x81', '\x65'},	// ��
	{'\xA1', '\xAF', '\x81', '\x66'},	// ��
	{'\xA1', '\xB0', '\x81', '\x67'},	// ��
	{'\xA1', '\xB1', '\x81', '\x68'},	// ��

	{'\x00', '\x00', '\x00', '\x00'}
};

HWND	g_hSettingWnd = NULL;
LPSTR	g_pOptStrBuf = NULL;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only CTransCATApp object

CTransCATApp theApp;


// Export Functions
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszSettingStringBuffer)
{
	return theApp.OnPluginInit(hSettingWnd, cszSettingStringBuffer);
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	return theApp.OnPluginOption();
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	return theApp.OnPluginClose();
}

extern "C" __declspec(dllexport) BOOL __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
{
	return theApp.Translate(cszJapanese, szKorean, nBufSize);
}




//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// CTransCATApp

BEGIN_MESSAGE_MAP(CTransCATApp, CWinApp)
END_MESSAGE_MAP()


// CTransCATApp construction

CTransCATApp::CTransCATApp()
	: m_bRemoveTrace(FALSE)
	, m_bRemoveDupSpace(FALSE)
	, m_hTransThread(NULL)
	, m_hRequestEvent(NULL)
	, m_hResponseEvent(NULL)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}




// CTransCATApp initialization

BOOL CTransCATApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CTransCATApp::OnPluginInit( HWND hSettingWnd, LPSTR cszSettingStringBuffer )
{
	TRACE(_T("CTransCATApp::OnPluginInit() begin \n"));

	BOOL bRetVal = FALSE;

	// �����̳� ������ ����
	g_hSettingWnd = hSettingWnd;
	
	// �ɼ� ��Ʈ�� ���� ����
	g_pOptStrBuf = cszSettingStringBuffer;

	// �ɼ� ����
	if(g_pOptStrBuf[0] == '0') m_bRemoveTrace = FALSE;
	else m_bRemoveTrace = TRUE;	// ��ȣ���Ű� ����Ʈ

	if(g_pOptStrBuf[1] == '0') m_bRemoveDupSpace = FALSE;
	else m_bRemoveDupSpace = TRUE;	// ������� ������ ����Ʈ

	// ����ȭ ��ü �ʱ�ȭ
	InitializeCriticalSection(&m_csTrans);
	m_hRequestEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hResponseEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hTransThread = (HANDLE)_beginthreadex(NULL, 0, TransThreadFunc, this, 0, NULL);
	if(NULL == m_hTransThread)
	{
		MessageBox(NULL, _T("Ʈ����Ĺ �����带 ������ �� �����ϴ�."), _T("TransCAT Initialize Error"), MB_OK | MB_TOPMOST);
	}
	else
	{
		WaitForSingleObject(m_hResponseEvent, INFINITE);
		if(!m_strErrorMsg.IsEmpty())
		{
			MessageBox(NULL, m_strErrorMsg, _T("TransCAT Initialize Error"), MB_OK | MB_TOPMOST);
		}
		else
		{
			bRetVal = TRUE;
		}

	}
	

	TRACE(_T("CTransCATApp::OnPluginInit() end \n"));

	return bRetVal;

}

BOOL IsASCIIOnly(LPCTSTR cszString)
{
	if(NULL == cszString) return FALSE;

	size_t i = 0;
	while(cszString[i]) 
	{
		if( ((BYTE*)cszString)[i++] >= 0x80 ) return FALSE;
	}

	return TRUE;
}


void CTransCATApp::InitTransCAT()
{
	m_strErrorMsg = _T("");
	m_strHomeDir = _T("");

	try
	{
		while(m_strHomeDir.IsEmpty())
		{
			m_strHomeDir = GetTransCATHomeDir();
			if( m_strHomeDir.IsEmpty() )
			{
				if( ::MessageBox(
						NULL, 
						_T("TransCAT�� ��ġ�� ��θ� ã�� �� �����ϴ�.\r\n��θ� ���� ã���ðڽ��ϱ�?"), 
						_T("TransCAT Plugin"), 
						MB_YESNO | MB_TOPMOST) == IDYES )
				{
					ITEMIDLIST *pidlBrowse; 
					TCHAR pszPathname[MAX_PATH] = {0,}; 
					BROWSEINFO BrInfo;
					BrInfo.hwndOwner = NULL; 
					BrInfo.pidlRoot = NULL;

					memset(&BrInfo, 0, sizeof(BrInfo));
					BrInfo.pszDisplayName = pszPathname;
					BrInfo.lpszTitle = _T("TransCAT ��ġ ����");
					BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

					pidlBrowse = ::SHBrowseForFolder(&BrInfo);

					if( pidlBrowse != NULL)
					{
						BOOL bModalRes = ::SHGetPathFromIDList(pidlBrowse, pszPathname);
						if(bModalRes)
						{
							m_strHomeDir.Format(_T("%s"), pszPathname);
						}
					}
					
				}
				else
				{
					throw _T("TransCAT�� ��ġ�� ��θ� ã�� �� �����ϴ�.");
				}
			}
		}

		// ���� ������� �˻�
		if(IsASCIIOnly(m_strHomeDir) == FALSE)
		{
			if( ::MessageBox(
				NULL, 
				_T("TransCAT ��ο� �ѱ��� ���� ���� ��� ����� �������� ���� �� �ֽ��ϴ�.\r\n�׷��� ��� �����Ͻðڽ��ϱ�?"), 
				_T("TransCAT Plugin"), 
				MB_YESNO) == IDNO )
			{
				throw _T("TransCAT�� ���� ������ ��ġ�Ͻ� �� �ٽ� �õ��� �ֽʽÿ�.");				
			}			
		}

		// Ʈ����Ĺ �Լ���� ����ü �ʱ�ȭ
		ZeroMemory(&m_TCDLL, sizeof(m_TCDLL));

		// JK ���� ��� �ε�
		CString strJKDllPath = m_strHomeDir + _T("D_JK.dll");
		m_TCDLL.hJKMod = LoadLibrary(strJKDllPath);	
		if (m_TCDLL.hJKMod == NULL) throw _T("D_JK.dll�� �ε��� �� �����ϴ�.");

		// DLL�κ��� �Լ� ������ ���
		FARPROC pFuncAddr = NULL;

		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "JKTransStart"); 
		memcpy( &m_TCDLL.JK_TransStart, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "JKTransEndingEx"); 
		memcpy( &m_TCDLL.JK_TransEndingEx, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdDir"); 
		memcpy( &m_TCDLL.GSetJK_gdDir, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdTempGenbun"); 
		memcpy( &m_TCDLL.GSetJK_gdTempGenbun, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdTempTerget"); 
		memcpy( &m_TCDLL.GSetJK_gdTempTerget, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdTempMorph"); 
		memcpy( &m_TCDLL.GSetJK_gdTempMorph, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdTempChnGen"); 
		memcpy( &m_TCDLL.GSetJK_gdTempChnGen, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gQUESTIONMARK"); 
		memcpy( &m_TCDLL.GSetJK_gQUESTIONMARK, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gEXCLAMATION"); 
		memcpy( &m_TCDLL.GSetJK_gEXCLAMATION, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gSEMICOLONMARK"); 
		memcpy( &m_TCDLL.GSetJK_gSEMICOLONMARK, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gCOLONMARK"); 
		memcpy( &m_TCDLL.GSetJK_gCOLONMARK, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdChudanFlag"); 
		memcpy( &m_TCDLL.GSetJK_gdChudanFlag, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdUseShiftJisCode"); 
		memcpy( &m_TCDLL.GSetJK_gdUseShiftJisCode, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdDdeSpaceCut"); 
		memcpy( &m_TCDLL.GSetJK_gdDdeSpaceCut, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gnDdeTransFlag"); 
		memcpy( &m_TCDLL.GSetJK_gnDdeTransFlag, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdFileTransFlag"); 
		memcpy( &m_TCDLL.GSetJK_gdFileTransFlag, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdLineFlag"); 
		memcpy( &m_TCDLL.GSetJK_gdLineFlag, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdLineLength"); 
		memcpy( &m_TCDLL.GSetJK_gdLineLength, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdPeriodOnly"); 
		memcpy( &m_TCDLL.GSetJK_gdPeriodOnly, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdUseGairaiFlag"); 
		memcpy( &m_TCDLL.GSetJK_gdUseGairaiFlag, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gdUseKanjiFlag"); 
		memcpy( &m_TCDLL.GSetJK_gdUseKanjiFlag, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_gbUnDDE"); 
		memcpy( &m_TCDLL.GSetJK_gbUnDDE, &pFuncAddr, sizeof(FARPROC) );

		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "ControlJK_UserDic"); 
		memcpy( &m_TCDLL.ControlJK_UserDic, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_UserDicInit"); 
		memcpy( &m_TCDLL.GSetJK_UserDicInit, &pFuncAddr, sizeof(FARPROC) );
		pFuncAddr = GetProcAddress(m_TCDLL.hJKMod, "GSetJK_ITEngineEx"); 
		memcpy( &m_TCDLL.GSetJK_ITEngineEx, &pFuncAddr, sizeof(FARPROC) );

		if (m_TCDLL.JK_TransStart == NULL || m_TCDLL.JK_TransEndingEx == NULL ||
			m_TCDLL.GSetJK_gdTempGenbun == NULL || m_TCDLL.GSetJK_gdTempTerget == NULL ||
			m_TCDLL.GSetJK_gdTempMorph == NULL || m_TCDLL.GSetJK_gdTempChnGen == NULL ||
			m_TCDLL.GSetJK_gQUESTIONMARK == NULL || 
			m_TCDLL.GSetJK_gEXCLAMATION == NULL || 
			m_TCDLL.GSetJK_gSEMICOLONMARK == NULL || 
			m_TCDLL.GSetJK_gCOLONMARK == NULL || 
			m_TCDLL.GSetJK_gdChudanFlag == NULL || 
			m_TCDLL.GSetJK_gdUseShiftJisCode == NULL || 
			m_TCDLL.GSetJK_gdDdeSpaceCut == NULL || 
			m_TCDLL.GSetJK_gnDdeTransFlag == NULL || 
			m_TCDLL.GSetJK_gdFileTransFlag == NULL || 
			m_TCDLL.GSetJK_gdLineFlag == NULL || 
			m_TCDLL.GSetJK_gdLineLength == NULL || 
			m_TCDLL.GSetJK_gdPeriodOnly == NULL || 
			m_TCDLL.GSetJK_gdUseGairaiFlag == NULL || 
			m_TCDLL.GSetJK_gdUseKanjiFlag == NULL || 
			m_TCDLL.GSetJK_gbUnDDE == NULL || 
			m_TCDLL.ControlJK_UserDic == NULL || m_TCDLL.GSetJK_UserDicInit == NULL || 
			m_TCDLL.GSetJK_ITEngineEx == NULL)  throw _T("�߸��� D_JK.dll �����Դϴ�.");

		DWORD dwOldProtect, dwTmpProtect;
		BYTE editCode = 0xEB;
		HMODULE hModule = NULL;
		::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pFuncAddr, &hModule);

		::VirtualProtect((BYTE*)hModule + 0xFC80, 1, PAGE_READWRITE, &dwOldProtect);
		memset((BYTE*)hModule + 0xFC80,0xEB,1);
		::VirtualProtect((BYTE*)hModule + 0xFC80, 1, dwOldProtect, &dwTmpProtect);

		::VirtualProtect((BYTE*)hModule + 0xFC98, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		memset((BYTE*)hModule + 0xFC98,0xEB,1);
		::VirtualProtect((BYTE*)hModule + 0xFC98, 1, dwOldProtect, &dwTmpProtect);

		char szHomeDir[MAX_PATH];
		::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)m_strHomeDir, -1, szHomeDir, MAX_PATH, NULL, NULL);

		m_TCDLL.GSetJK_gdDir(1, szHomeDir);
		m_TCDLL.GSetJK_gdTempGenbun(1, "GENBUN_PJK");
		m_TCDLL.GSetJK_gdTempTerget(1, "TERGET_PJK");
		m_TCDLL.GSetJK_gdTempMorph(1, "MORPH_PJK");
		m_TCDLL.GSetJK_gdTempChnGen(1, "KRNGEN_PJK");

		m_TCDLL.GSetJK_gQUESTIONMARK(1, 0);
		m_TCDLL.GSetJK_gEXCLAMATION(1, 0);
		m_TCDLL.GSetJK_gSEMICOLONMARK(1, 0);
		m_TCDLL.GSetJK_gCOLONMARK(1, 0);
		m_TCDLL.GSetJK_gdChudanFlag(0, 0);
		m_TCDLL.GSetJK_gdUseShiftJisCode(1, 1);
		m_TCDLL.GSetJK_gdDdeSpaceCut(1, 0);
		m_TCDLL.GSetJK_gnDdeTransFlag(1, 0);
		m_TCDLL.GSetJK_gdFileTransFlag(1, 0);
		m_TCDLL.GSetJK_gdLineFlag(1, 2);
		m_TCDLL.GSetJK_gdLineLength(1, 48);
		m_TCDLL.GSetJK_gdPeriodOnly(1, 0);
		m_TCDLL.GSetJK_gdUseGairaiFlag(1, 0);
		m_TCDLL.GSetJK_gdUseKanjiFlag(1, 1);
		m_TCDLL.GSetJK_gbUnDDE(1, 0);

		m_TCDLL.ControlJK_UserDic(1);
		m_TCDLL.GSetJK_UserDicInit();

		char str[1] = {0,};
		bool bErr = m_TCDLL.GSetJK_ITEngineEx(str, str);

		if (!bErr)  throw _T("D_JK.dll ���Ϸε� ����!");
	}
	catch (LPCTSTR strErr)
	{
		m_strErrorMsg = strErr;
	}


	return;
}



void CTransCATApp::CloseTransCAT()
{
	if (m_TCDLL.hJKMod)
	{
		/*    FreeLibrary(m_TCDLL.hJKMod); */
	}

	ZeroMemory(&m_TCDLL, sizeof(m_TCDLL));

	g_hSettingWnd = NULL;

}

BOOL CTransCATApp::OnPluginOption()
{
	if(g_hSettingWnd && IsWindow(g_hSettingWnd))
	{
		CTransCATOptionDlg opt_dlg;
		opt_dlg.m_bRemoveTrace = m_bRemoveTrace;
		opt_dlg.m_bRemoveDupSpace = m_bRemoveDupSpace;
		
		if(opt_dlg.DoModal() == IDOK)
		{
			// �ɼ� ����
			m_bRemoveTrace = opt_dlg.m_bRemoveTrace;
			m_bRemoveDupSpace = opt_dlg.m_bRemoveDupSpace;
			
			g_pOptStrBuf[0] = '0' + m_bRemoveTrace;
			g_pOptStrBuf[1] = '0' + m_bRemoveDupSpace;
			g_pOptStrBuf[2] = '\0';

			// �ɼ� ���� ����?
			if(m_bRemoveTrace == TRUE && m_bRemoveDupSpace == TRUE) g_pOptStrBuf[0] = '\0';

		}
	}
	return TRUE;
}


BOOL CTransCATApp::OnPluginClose()
{
	if(m_hTransThread)
	{
		HANDLE hTmp = m_hTransThread;
		m_hTransThread = NULL;
		::SetEvent(m_hRequestEvent);
		::WaitForSingleObject(m_hTransThread, 3000);
		::CloseHandle(m_hTransThread);
	}

	// ����ȭ ��ü ����ȭ
	::CloseHandle(m_hRequestEvent);
	::CloseHandle(m_hResponseEvent);
	m_hRequestEvent = NULL;
	m_hResponseEvent = NULL;
	DeleteCriticalSection(&m_csTrans);
	
	return TRUE;
}

BOOL CTransCATApp::Translate( LPCSTR cszJapanese, LPSTR szKorean, int nBufSize )
{
	BOOL bRetVal = FALSE;

	EnterCriticalSection(&m_csTrans);

	m_nBufSize = nBufSize;
	m_pJpnText = cszJapanese;
	m_pKorText = szKorean;

	ResetEvent(m_hResponseEvent);
	BOOL bSetEventResult = ::SetEvent(m_hRequestEvent);
	TRACE(_T("[aral1] Request Trans Event ON!! (bSetEventResult : %d)"), bSetEventResult);

	DWORD dwTransWait = ::WaitForSingleObject(m_hResponseEvent, 3000);
	TRACE(_T("[aral1] Received Response of Trans Event (dwTransWait : %d)"), dwTransWait);

	if(dwTransWait == WAIT_OBJECT_0) bRetVal = TRUE;

	LeaveCriticalSection(&m_csTrans);

	return bRetVal;
}



// Ʈ����Ĺ Ȩ ���丮 �ʱ�ȭ
CString CTransCATApp::GetTransCATHomeDir()
{
	CString strRetVal = _T("");

	TCHAR szHomeDir[MAX_PATH];
	HKEY hKey;
	DWORD dwLen = MAX_PATH;

	try
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\DICO\\TRANSCAT\\TRANSCAT/jk"), 0,
			KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) throw -1;


		if (RegQueryValueEx(hKey, _T("Install Directory"), NULL, NULL, (LPBYTE)szHomeDir,
			&dwLen) != ERROR_SUCCESS) throw -2;

		RegCloseKey(hKey);

		if ( _tcslen(szHomeDir) == 0 ) throw -3;

		strRetVal.Format(_T("%s\\"), szHomeDir);
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}

	return strRetVal;
}



unsigned int __stdcall CTransCATApp::TransThreadFunc(void* pParam)
{
	TRACE(_T("[aral1] ���� ������ ����"));

	// �����ؽ�Ʈ ĳ��
	map<UINT, CTextElement*> mapCache;
	CTextElement CacheHead;
	CTextElement CacheTail;
	CacheHead.pNextLink = &CacheTail;
	CacheTail.pPrevLink = &CacheHead;
	
	// ������ ����
	char* pBuf1 = new char[TRANS_BUF_SIZE];
	char* pBuf2 = new char[TRANS_BUF_SIZE];


	CTransCATApp* pThis = (CTransCATApp*)pParam;

	pThis->InitTransCAT();
	::SetEvent(pThis->m_hResponseEvent);	// Ʈ����Ĺ �ʱ�ȭ�� �Ǿ��ٰ� �˷��ִ� �뵵

	while(pThis->m_hTransThread)
	{
		DWORD dwTransWait = WaitForSingleObject(pThis->m_hRequestEvent, INFINITE);
		TRACE(_T("[aral1] Received Request of Trans Event (dwTransWait : %d)"), dwTransWait);
		if(pThis->m_hTransThread == NULL)
		{
			TRACE(_T("[aral1] Exit (pThis->m_hTransThread == NULL)"));
			break;
		}

		if('\0' == pThis->m_pJpnText[0] || strlen(pThis->m_pJpnText) > TRANS_BUF_SIZE)
		{
			pThis->m_pKorText[0] = '\0';
		}
		else
		{
			// �� ���� �ؽ�Ʈ�� �ؽ� ���ϱ�
			UINT dwTextHash = MakeStringHash(pThis->m_pJpnText);

			// ĳ�ÿ��� ã�ƺ���
			map<UINT, CTextElement*>::iterator iter = mapCache.find(dwTextHash);
			CTextElement* pTextElem = NULL;
			
			// ĳ�ÿ� ������
			if(mapCache.end() != iter)
			{
				pTextElem = iter->second;

				// ����Ʈ���� ����������
				pTextElem->pPrevLink->pNextLink = pTextElem->pNextLink;
				pTextElem->pNextLink->pPrevLink = pTextElem->pPrevLink;

			}
			// ĳ�ÿ� ������ Ʈ����Ĺ�� ����
			else
			{
				LPCSTR cszJpnText = pThis->m_pJpnText;
				

				// ��ȣ ���� ���ڵ�
				if(pThis->m_bRemoveTrace)
				{
					pThis->EncodeTrace(cszJpnText, pBuf1);
					cszJpnText = pBuf1;
				}
				
				// ���� ���� ���ڵ�
				//pThis->EncodeTwoByte(cszJpnText, pBuf2);
				//cszJpnText = pBuf2;
				
				int nBool = pThis->m_TCDLL.JK_TransStart(cszJpnText, strlen(cszJpnText));
				char szTrans[2048] = {0,};
				bool bTrans = pThis->m_TCDLL.JK_TransEndingEx(1, szTrans, strlen(cszJpnText));

				if(szTrans && (nBool == 2) && bTrans)
				{
					LPCSTR cszKorText = szTrans;
					
					// �ߺ� ���� ����
					if(pThis->m_bRemoveDupSpace)
					{
						pThis->FilterDupSpaces(cszKorText, pBuf2);
						cszKorText = pBuf2;
					}

					// ���� ���� ���ڵ�
					//pThis->DecodeTwoByte(cszKorText, pBuf1);
					//cszKorText = pBuf1;

					// ��ȣ ���� ���ڵ�
					if(pThis->m_bRemoveTrace)
					{
						pThis->FilterTrace(cszKorText, pBuf2);
						pThis->DecodeTrace(pBuf2, pBuf1);
						cszKorText = pBuf1;
					}

					//pThis->m_TCDLL.JK_FreeMem(szTrans);

					pTextElem = new CTextElement();
					pTextElem->dwHash = dwTextHash;
					pTextElem->strTranslatedText = cszKorText;

					// ĳ�ð� �� á�ٸ� ���ڸ� Ȯ���� ����
					if(mapCache.size() >= 10000)
					{
						CTextElement* pDelElem = CacheTail.pPrevLink;
						CacheTail.pPrevLink = pDelElem->pPrevLink;

						mapCache.erase(pDelElem->dwHash);
						delete pDelElem;
					}

					// ĳ�ÿ� ����
					mapCache.insert(pair<UINT, CTextElement*>(dwTextHash, pTextElem));
				}

			}


			if(pTextElem && pThis->m_pKorText)
			{
				int nLen = (int)pTextElem->strTranslatedText.length();

				if( nLen > (pThis->m_nBufSize-1) )
				{
					strncpy(pThis->m_pKorText, pTextElem->strTranslatedText.c_str(), pThis->m_nBufSize-1);
					pThis->m_pKorText[pThis->m_nBufSize-1] = _T('\0');
				}
				else
				{
					strcpy(pThis->m_pKorText, pTextElem->strTranslatedText.c_str());
				}
				
				// ��� �������� ����
				pTextElem->pPrevLink = &CacheHead;
				pTextElem->pNextLink = CacheHead.pNextLink;

				CacheHead.pNextLink->pPrevLink = pTextElem;
				CacheHead.pNextLink = pTextElem;
			}

		}// end of else of if('\0' == pThis->m_pJpnText[0])

		BOOL bSetEventResult = ::SetEvent(pThis->m_hResponseEvent);
		TRACE(_T("[aral1] Response Trans Event ON!! (result : %d)"), bSetEventResult);
	}
	
	// ���� ����
	delete [] pBuf1;
	delete [] pBuf2;
	
	// ���� ���ҵ� ����
	TRACE(_T("[aral1] Map : %d"), mapCache.size());
	for(map<UINT, CTextElement*>::iterator iter = mapCache.begin();
		iter != mapCache.end();
		iter++)
	{
		CTextElement* pTextElem = iter->second;
		if(pTextElem) delete pTextElem;
	}


	pThis->CloseTransCAT();

	TRACE(_T("[aral1] Trans Thread Terminated!!!!!"));

	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
// �����Ⱑ ��ȣ�� ������� �ٿ������� ��� �Ϻ� ���ӿ��� ���� ����
// ���� �̷� ��� ����
// ex) "��(��)��⸦ �Ծ���." -> "�Ұ�⸦ �Ծ���."
//
//////////////////////////////////////////////////////////////////////////
void CTransCATApp::FilterTrace(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar || '\0'==cszKorSrc[0] || '\0'==szKorTar[0]) return;

	size_t len = strlen(cszKorSrc);	
	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		// 2����Ʈ ���ڸ�
		if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0')
		{
			if((BYTE)cszKorSrc[i] == 0xA1 && (BYTE)cszKorSrc[i+1] == 0xA1)
			{
				i += 2;
				szKorTar[j++] = ' ';
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
		// 1����Ʈ ���ڸ�
		else
		{
			if(cszKorSrc[i] == '(')
			{
				//// AT ���� �����̸�
				//if( strncmp(&cszKorSrc[i], "(\at", 4) == 0 )
				//{
				//	do
				//	{
				//		szKorTar[j++] = cszKorSrc[i++];
				//	}while(cszKorSrc[i-1] != ')' && i<len);
				//}
				//// �ƴ϶� �׳� '(' ��ȣ �̸�
				//else
				//{
					while(cszKorSrc[i] != ')' && i<len)
					{
						if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
						i++;
					}

					if(i<len)
					{
						i++;
						if(cszKorSrc[i]==' ') i++;
					}
				//}
			}
			else if(cszKorSrc[i] == '{')
			{
				while(cszKorSrc[i] != '}' && i<len)
				{
					if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
					i++;
				}

				if(i<len)
				{
					i++;
					if(cszKorSrc[i]==' ') i++;
				}
			}
			else if(cszKorSrc[i] == '[')
			{
				while(cszKorSrc[i] != ']' && i<len)
				{
					if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
					i++;
				}

				if(i<len)
				{
					i++;
					if(cszKorSrc[i]==' ') i++;
				}
			}
			else if(cszKorSrc[i] == '<')
			{
				while((cszKorSrc[i] != '>'  && cszKorSrc[i] != ')')&& i<len)
				{
					if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0') i++;
					i++;
				}

				if(i<len)
				{
					i++;
					if(cszKorSrc[i]==' ') i++;
					//if(j>0 && ':' == szKorText[j-1]) szKorText[j-1] = ' ';
				}
			}
			else if(cszKorSrc[i] == ':')
			{
				i++;
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<FilterTrace Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<FilterTrace End>");
#endif
}


//////////////////////////////////////////////////////////////////////////
//
// �����⸦ ��ģ �� ����(' ')�� �ʿ��̻����� �þ�� ��찡 �ִ�.
// �̷� ��� �ϳ��� ����� ��� ����
// ex) "�� , �̷���  ����"   ->   "��, �̷��� ����"
//
//////////////////////////////////////////////////////////////////////////
void CTransCATApp::FilterDupSpaces(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar || '\0'==cszKorSrc[0] || '\0'==szKorTar[0]) return;

	size_t len = strlen(cszKorSrc);	
	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		// 2����Ʈ ���ڸ�
		if((BYTE)cszKorSrc[i] >= 0x80 && (BYTE)cszKorSrc[i+1] != '\0')
		{
			if((BYTE)cszKorSrc[i] == 0xA1 &&	//�� �� �� �� 
				0xB8 <= (BYTE)cszKorSrc[i+1] && (BYTE)cszKorSrc[i+1] <= 0xBB)
			{
				if(j>0 && szKorTar[j-1] == ' ') j--;
				szKorTar[j++] = cszKorSrc[i++];
				szKorTar[j++] = cszKorSrc[i++];
				if(cszKorSrc[i] == ' ') i++;
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
				szKorTar[j++] = cszKorSrc[i++];
			}
			
		}
		// 1����Ʈ ���ڸ�
		else
		{
			if((cszKorSrc[i] == ' ') && (i+1 < len) && (cszKorSrc[i+1] == ' ' || cszKorSrc[i+1] == ','))
			{
				i++;
			}
			else
			{
				szKorTar[j++] = cszKorSrc[i++];
			}
		}
	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<FilterDupSpaces Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<FilterDupSpaces End>");
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// ���� Ư�����ڸ� Ʈ����Ĺ�� ������� ASCII�ڵ�� �ٲ������ �̸� ����
// �ϱ����� ���� ��� ��ȯ��Ŵ
// ex) "���檫��" -> "(\atA3BC)�檫(\atA3BE)"
//
//////////////////////////////////////////////////////////////////////////
void CTransCATApp::EncodeTwoByte(LPCSTR cszJpnSrc, LPSTR szJpnTar)
{
	if(NULL==cszJpnSrc || NULL==szJpnTar) return;

	if('\0'==cszJpnSrc[0])
	{
		szJpnTar[0] = '\0';
		return;
	}
	
	size_t len = strlen(cszJpnSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		if((BYTE)cszJpnSrc[i] >= 0x80 && (BYTE)cszJpnSrc[i+1] != '\0')
		{
			int nTarInc = 2;

			szJpnTar[j] = cszJpnSrc[i];
			szJpnTar[j+1] = cszJpnSrc[i+1];

			for(int k=0; _SHIFT_JIS_CHAR_MAP[k][0]; k++)
			{
				if(_SHIFT_JIS_CHAR_MAP[k][2] == cszJpnSrc[i]
					&& _SHIFT_JIS_CHAR_MAP[k][3] == cszJpnSrc[i+1])
				{
					nTarInc = sprintf(&szJpnTar[j], "_&%03u%03u&_", (BYTE)_SHIFT_JIS_CHAR_MAP[k][0], (BYTE)_SHIFT_JIS_CHAR_MAP[k][1]);
				}
			}
			
			i += 2;
			j += nTarInc;
		}
		else
		{
			szJpnTar[j++] = cszJpnSrc[i++];
		}
	}

	szJpnTar[j] = '\0';
	
#ifdef DEBUG
	OutputDebugStringA("<EncodeTwoByte Start>");
	OutputDebugStringA(cszJpnSrc);
	OutputDebugStringA(szJpnTar);
	OutputDebugStringA("<EncodeTwoByte End>");
#endif
}




//////////////////////////////////////////////////////////////////////////
//
// Ʈ����Ĺ ���� �� ��ȯ���״� ����Ư����ȣ���� ����
// ex)  "(\atA3BC)���ΰ�(\atA3BE)" -> "�����ΰ���"
//
//////////////////////////////////////////////////////////////////////////
void CTransCATApp::DecodeTwoByte(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar) return;

	if('\0'==cszKorSrc[0])
	{
		szKorTar[0] = '\0';
		return;
	}

	size_t len = strlen(cszKorSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{		
		size_t nCopyLen = len-i;
		const char* pEncPtr = strstr(&cszKorSrc[i], "_&");
				
		// "_&" �����Ƚ��� ã�Ҵٸ�
		if(NULL != pEncPtr)
		{
			// ��ȯ��ų �����Ͱ� Ȯ���Ѱ�?
			//if(	(UINT_PTR)pEncPtr - (UINT_PTR)cszKorSrc + 9 < len
			//	&& '&' == *(pEncPtr+8)
			//	&& '_' == *(pEncPtr+9) )
			if(GetEncodedLen(pEncPtr) == 10)
			{
				nCopyLen = (UINT_PTR)pEncPtr - (UINT_PTR)(&cszKorSrc[i]);
			}
			// ��ȯ��Ű�� �ȵǴ� _&���
			else
			{
				pEncPtr = NULL;
				nCopyLen = 2;
			}
		}
		else
		{
			pEncPtr = NULL;
		}

		// �Ϲ� ���ڿ� ����
		memcpy(&szKorTar[j], &cszKorSrc[i], nCopyLen);
		i += nCopyLen;
		j += nCopyLen;

		if(pEncPtr)
		{
			// Ư�� ���� ���ڵ�
			int val1, val2;
			sscanf(pEncPtr+2, "%03u%03u", &val1, &val2);
			((BYTE*)szKorTar)[j] = (BYTE)val1;
			((BYTE*)szKorTar)[j+1] = (BYTE)val2;
			
			i += 10;
			j += 2;
		}

	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<DecodeTwoByte Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<DecodeTwoByte End>");
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// �ڵ� �����Ǵ� ��ȣ�� Ȯ���� �Ǻ��ϱ� ����
// ���� �ִ� ��ȣ�� ��ȣȭ
// ex) "[�檫]" -> "(\at5B)�檫(\at5D)"
//
//////////////////////////////////////////////////////////////////////////
void CTransCATApp::EncodeTrace(LPCSTR cszJpnSrc, LPSTR szJpnTar)
{
	if(NULL==cszJpnSrc || NULL==szJpnTar) return;

	if('\0'==cszJpnSrc[0])
	{
		szJpnTar[0] = '\0';
		return;
	}

	size_t len = strlen(cszJpnSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		
		// 2����Ʈ ������ ���
		if((BYTE)cszJpnSrc[i] >= 0x80 && (BYTE)cszJpnSrc[i+1] != '\0')
		{
			szJpnTar[j++] = cszJpnSrc[i++];
			szJpnTar[j++] = cszJpnSrc[i++];
		}
		// 1����Ʈ ������ ���
		else
		{
			// ���ڰ� ��ȣ��
			if(cszJpnSrc[i] == '(' || cszJpnSrc[i] == ')'
				|| cszJpnSrc[i] == '[' || cszJpnSrc[i] == ']'
				|| cszJpnSrc[i] == '{' || cszJpnSrc[i] == '}'
				|| cszJpnSrc[i] == '<' || cszJpnSrc[i] == '>'
				|| cszJpnSrc[i] == ':')
			{
				j += sprintf(&szJpnTar[j], "_&%03u&_", (BYTE)cszJpnSrc[i++]);
			}
			// �Ϲ� ���ڸ�
			else
			{
				szJpnTar[j++] = cszJpnSrc[i++];
			}
		}
	}

	szJpnTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<EncodeTrace Start>");
	OutputDebugStringA(cszJpnSrc);
	OutputDebugStringA(szJpnTar);
	OutputDebugStringA("<EncodeTrace End>");
#endif
}




//////////////////////////////////////////////////////////////////////////
//
// Ʈ����Ĺ ���� �� ��ȯ���״� ��ȣ���� ����
// ex) "(\at5B)�檫(\at5D)" -> "[�檫]"
//
//////////////////////////////////////////////////////////////////////////
void CTransCATApp::DecodeTrace(LPCSTR cszKorSrc, LPSTR szKorTar)
{
	if(NULL==cszKorSrc || NULL==szKorTar) return;

	if('\0'==cszKorSrc[0])
	{
		szKorTar[0] = '\0';
		return;
	}

	size_t len = strlen(cszKorSrc);

	size_t i = 0;	// �������� �ε���
	size_t j = 0;	// ���� �ε���

	while(i<len)
	{
		size_t nCopyLen = len-i;
		const char* pEncPtr = strstr(&cszKorSrc[i], "_&");

		// "_&" �����Ƚ��� ã�Ҵٸ�
		if(NULL != pEncPtr)
		{
			// ��ȯ��ų �����Ͱ� Ȯ���Ѱ�?
			//if(	(UINT_PTR)pEncPtr - (UINT_PTR)cszKorSrc + 6 < len
			//	&& '&' == *(pEncPtr+5)
			//	&& '_' == *(pEncPtr+6) )
			if(GetEncodedLen(pEncPtr) == 7)
			{
				nCopyLen = (UINT_PTR)pEncPtr - (UINT_PTR)(&cszKorSrc[i]);
			}
			// ��ȯ��Ű�� �ȵǴ� _&���
			else
			{
				pEncPtr = NULL;
				nCopyLen = 2;
			}
		}
		else
		{
			pEncPtr = NULL;
		}

		// �Ϲ� ���ڿ� ����
		memcpy(&szKorTar[j], &cszKorSrc[i], nCopyLen);
		i += nCopyLen;
		j += nCopyLen;

		if(pEncPtr)
		{
			// ��ȣ ���� ���ڵ�
			int val1;
			sscanf(pEncPtr+2, "%03u", &val1);
			((BYTE*)szKorTar)[j] = (BYTE)val1;

			i += 7;
			j += 1;
		}

	}

	szKorTar[j] = '\0';

#ifdef DEBUG
	OutputDebugStringA("<DncodeTrace Start>");
	OutputDebugStringA(cszKorSrc);
	OutputDebugStringA(szKorTar);
	OutputDebugStringA("<DncodeTrace End>");
#endif
}

size_t CTransCATApp::GetEncodedLen( LPCSTR cszBytes )
{
	size_t nRetVal = 0;

	if(NULL != cszBytes)
	{
		size_t len = strlen(cszBytes);

		if(len >= 7 
			&& '_' == cszBytes[0] 
			&& '&' == cszBytes[1]
			&& isdigit(cszBytes[2])
			&& isdigit(cszBytes[3])
			&& isdigit(cszBytes[4])
			&& '&' == cszBytes[5] 
			&& '_' == cszBytes[6]) nRetVal = 7;
		else if(len >= 10 
			&& '_' == cszBytes[0] 
			&& '&' == cszBytes[1]
			&& isdigit(cszBytes[2])
			&& isdigit(cszBytes[3])
			&& isdigit(cszBytes[4])
			&& isdigit(cszBytes[5])
			&& isdigit(cszBytes[6])
			&& isdigit(cszBytes[7])
			&& '&' == cszBytes[8] 
			&& '_' == cszBytes[9]) nRetVal = 10;
	}

	return nRetVal;
}