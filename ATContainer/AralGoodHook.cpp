// AralGoodHook.cpp : Defines the initialization routines for the DLL.
//
#pragma warning(disable:4731)
#pragma warning(disable:4996)

#include "stdafx.h"
#include "AralGoodHook.h"
#include "OutputDialog.h"
#include "RawHookMgr/RawHookMgr.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "AppLocMgr/AppLocMgr.h"
#include "Util.h"
#include <tlhelp32.h>

/*
#include "BugTrap/BugTrap.h"

//#pragma comment(lib, "BugTrap.lib") // Link to ANSI DLL

#pragma comment(lib, "BugTrap/BugTrapU.lib") // Link to Unicode DLL

static void SetupExceptionHandler()
{
	BT_InstallSehFilter();
	BT_SetAppName(_T("Aral Trans Exception Handler"));
	BT_SetSupportEMail(_T("webmaster@aralgood.com "));
	BT_SetFlags(BTF_DETAILEDMODE | BTF_EDITMAIL);
	BT_SetSupportServer(_T("localhost"), 9999);
	BT_SetSupportURL(_T("http://www.aralgood.com"));
}
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// ���� ��������

BEGIN_MESSAGE_MAP(CAralGoodHookApp, CWinApp)
	//{{AFX_MSG_MAP(CAralGoodHookApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#pragma code_seg()

#pragma data_seg(".ARALG")
HHOOK sg_hWinHook = NULL;
#pragma data_seg()
#pragma comment(linker, " /SECTION:.ARALG,RWS") 


/////////////////////////////////////////////////////////////////////////////
// ��������

map<DWORD,HHOOK> g_mapHookHandles;
//PROC_HookCallback g_pfnCodePointCallback = NULL;

CAralGoodHookApp theApp;
CAralGoodHookApp* CAralGoodHookApp::_gInst = NULL;	// �̱��� �ν��Ͻ�


typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (__stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);
PROC_WideCharToMultiByte g_pfnOrigWideCharToMultiByte = NULL;
PROC_MultiByteToWideChar g_pfnOrigMultiByteToWideChar = NULL;

int MyWideCharToMultiByte(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCWSTR lpWideCharStr, 
						  int cchWideChar, 
						  LPSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPCSTR lpDefaultChar, 
						  LPBOOL lpUsedDefaultChar 
						  )
{
	int nRetVal = 0;

	if( g_pfnOrigWideCharToMultiByte )
	{
		nRetVal = g_pfnOrigWideCharToMultiByte(
			CodePage, 
			dwFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpDefaultChar, 
			lpUsedDefaultChar 
			);
	}
	else
	{
		nRetVal = ::WideCharToMultiByte(
			CodePage, 
			dwFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpDefaultChar, 
			lpUsedDefaultChar 
			);
	}

	return nRetVal;
}

int MyMultiByteToWideChar(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPWSTR lpWideCharStr, 
						  int cchWideChar 
						  )
{
	int nRetVal = 0;

	if( g_pfnOrigMultiByteToWideChar )
	{
		nRetVal = g_pfnOrigMultiByteToWideChar(
			CodePage, 
			dwFlags, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpWideCharStr, 
			cchWideChar 
			);
	}
	else
	{
		nRetVal = ::MultiByteToWideChar(
			CodePage, 
			dwFlags, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpWideCharStr, 
			cchWideChar 
			);
	}

	return nRetVal;
}



//---------------------------------------------------------------------------
// WH_GETMESSAGE�� ��ŷ�� ���ν����� ����
//---------------------------------------------------------------------------
LRESULT CALLBACK GetMsgProc(
	int code,       // hook code
	WPARAM wParam,  // removal option
	LPARAM lParam   // message
	)
{
	// �׳� �ؽ�Ʈ �Ÿ� �ҷ��ش�
	return ::CallNextHookEx(sg_hWinHook, code, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
//
// CAralGoodHookApp ������
//
//////////////////////////////////////////////////////////////////////////
CAralGoodHookApp::CAralGoodHookApp()
	: m_pOutput(NULL), m_hTransDll(NULL), m_hHookingDll(NULL)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	_gInst = this;
	ZeroMemory(&m_csTextArgMgr, sizeof(CRITICAL_SECTION));
	ZeroMemory(&m_sTransFunc, sizeof(TRANS_FUNCTION_ENTRY));
	ZeroMemory(&m_hookOption, sizeof(HOOKING_OPTION));
	
	m_strTransPluginName = _T("");
	m_strHookingPluginName = _T("");

	// ���÷����� ���� �Լ�
	g_pfnOrigMultiByteToWideChar =
		(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));

	g_pfnOrigWideCharToMultiByte =
		(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"));
}


void TestCallback(REGISTER_ENTRY* pReg)
{
	//pReg->_EAX = pReg->_EAX;
	wchar_t buf[1024];
	wchar_t text[256];

	LPCWSTR strText = *((LPCWSTR*)(pReg->_ESP+24));
	size_t nCnt = *((size_t*)(pReg->_ESP+28));
	wcsncpy( text, strText, nCnt );
	text[nCnt] = L'\0';
	swprintf(buf, L"[ aral1 ] ExtTextOutW('%s') \n", text );
	OutputDebugStringW(buf);
	return;
}

BOOL IsKasperskyHookingPoint(void* pAddr)
{
	static const BYTE KHookPatern[] = {0x55, 0x8B, 0xEC, 0x90, 0x5D, 0xE9};

	BOOL bRetVal = TRUE;

	for(int i=0; i<6; i++)
	{
		if(KHookPatern[i] != ((BYTE*)pAddr)[i])
		{
			bRetVal = FALSE;
			break;
		}
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// Ư�� �����忡 ATContainer DLL ����
// ��ȯ�� : ����/����
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall SetupATContainer(DWORD dwThreadId, BOOL bSetup)
{
	BOOL bRetVal = FALSE;

	// �� ��ġ�� ���
	if(bSetup)
	{
		// ���� ��ŷ�ؾ� �Ѵٸ�
		if( g_mapHookHandles.find(dwThreadId) == g_mapHookHandles.end() )
		{

			// �ƶ�Ʈ���� ��ġ ���� ������ ����
			TCHAR szModuleName[MAX_PATH] = {0,};
			::GetModuleFileName(NULL, szModuleName, MAX_PATH);
			TCHAR* pNameStart = _tcsrchr(szModuleName, _T('\\') );
			if(pNameStart)
			{
				*pNameStart = _T('\0');
				CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AralTransHomeDir"), szModuleName);
			}


			// ���÷����� ���� �����Լ� ������ ����
			HMODULE hMod = LoadLibrary(_T("kernel32.dll"));
			if(hMod)
			{
				DWORD dwM2WAddr = (DWORD) GetProcAddress(hMod, "MultiByteToWideChar");
				CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"), dwM2WAddr);
				DWORD dwW2MAddr = (DWORD) GetProcAddress(hMod, "WideCharToMultiByte");
				CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"), dwW2MAddr);
				CAppLocMgr::SaveProcAddrToReg();
			}


			// ��� ������ ��ŷ
			HHOOK hWinHook = ::SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)(GetMsgProc), theApp.m_hInstance, dwThreadId );
			if(hWinHook)
			{
				g_mapHookHandles[dwThreadId] = hWinHook;
				bRetVal = TRUE;
			}

		}
	}

	// �� ������ ���
	else
	{
		// ��ŷ�� �����尡 �´ٸ� ����
		if( g_mapHookHandles.find(dwThreadId) != g_mapHookHandles.end() )
		{
			bRetVal = ::UnhookWindowsHookEx(g_mapHookHandles[dwThreadId]);
			g_mapHookHandles.erase(dwThreadId);
		}
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// Ư�� DLL �Լ��� ��ŷ
// ��ȯ�� : ����/����
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall HookDllFunctionEx(LPCSTR cszDllName, LPCSTR cszFuncName, LPVOID pfnNewFunc, INT nHookType)
{
	BOOL bRetVal = FALSE;

	if(CAralGoodHookApp::_gInst)
	{

#ifdef _UNICODE
		wchar_t pDllName[MAX_PATH];
		wchar_t pFuncName[MAX_PATH];
		::MyMultiByteToWideChar(949, 0, cszDllName, -1, pDllName, MAX_PATH);
		::MyMultiByteToWideChar(949, 0, cszFuncName, -1, pFuncName, MAX_PATH);
#else
		LPCSTR pDllName = cszDllName;
		LPCSTR pFuncName = cszFuncName;
#endif

		bRetVal = CRawHookMgr::Hook(pDllName, pFuncName, pfnNewFunc, nHookType);

	}

	return bRetVal;
}
extern "C" __declspec(dllexport) BOOL __stdcall HookDllFunction(LPCSTR cszDllName, LPCSTR cszFuncName, LPVOID pfnNewFunc)
{
	return HookDllFunctionEx(cszDllName, cszFuncName, pfnNewFunc, ATHOOKTYPE_COPY);
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// Ư�� DLL �Լ��� ����
// ��ȯ�� : ����/����
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall UnhookDllFunction(LPCSTR cszDllName, LPCSTR cszFuncName)
{
	BOOL bRetVal = FALSE;

	if(CAralGoodHookApp::_gInst)
	{
		HMODULE hMod = GetModuleHandleA(cszDllName);
		if(hMod)
		{
			void* pHookedFunc = (void*) GetProcAddress(hMod, cszFuncName);
			if(pHookedFunc)
			{
				bRetVal = CRawHookMgr::Unhook(pHookedFunc);
			}
		}
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// �Լ� �����Ϳ� ���� ���� DLL �Լ� ������ ��ȯ
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) void* __stdcall GetOrigDllFunction(LPCSTR cszDllName, LPCSTR cszFuncName)
{
	static 
	void* pOrigFunc = NULL;

	if(CAralGoodHookApp::_gInst)
	{
		HMODULE hMod = GetModuleHandleA(cszDllName);
		if(hMod)
		{
			void* pHookedFunc = (void*) GetProcAddress(hMod, cszFuncName);
			if(pHookedFunc)
			{
				pOrigFunc = CRawHookMgr::GetOrigFunc(pHookedFunc);

				// 2008.10.16 ī���۽�Ű�� ���� �߰�
				//if(stricmp(cszDllName, "Kernel32.dll")==0 && (strcmp(cszFuncName, "LoadLibraryA")==0 || strcmp(cszFuncName, "LoadLibraryW")==0))
				if(IsKasperskyHookingPoint(pOrigFunc)) pOrigFunc = (void*)((BYTE*)pHookedFunc + 5);				
			}
		}
	}

	return pOrigFunc;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// Ư�� ���� ��ŷ
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall HookCodePointEx(LPVOID pCodePoint, PROC_HookCallback pfnCallback, INT nHookType)
{
	BOOL bRetVal = FALSE;
	
	if(CAralGoodHookApp::_gInst)
	{
		bRetVal = CRawHookMgr::Hook(pCodePoint, PreModifyValue, TRUE, nHookType);
		if(bRetVal)
		{
			//g_pfnCodePointCallback = pfnCallback;
			CAralGoodHookApp::_gInst->m_mapCallbackTable.insert( pair<void*,void*>(pCodePoint,pfnCallback) );
		}
	}

	return bRetVal;
}
extern "C" __declspec(dllexport) BOOL __stdcall HookCodePoint(LPVOID pCodePoint, PROC_HookCallback pfnCallback)
{
	return HookCodePointEx(pCodePoint,pfnCallback, ATHOOKTYPE_COPY);
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// Ư�� ���� ����
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall UnhookCodePoint(LPVOID pCodePoint)
{
	BOOL bRetVal = FALSE;

	if(CAralGoodHookApp::_gInst)
	{
		bRetVal = CRawHookMgr::Unhook(pCodePoint);

		if(bRetVal)
		{
			//g_pfnCodePointCallback = NULL;
			CAralGoodHookApp::_gInst->m_mapCallbackTable.erase(pCodePoint);
		}
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ����
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall TranslateText(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
{
	BOOL bRetVal = FALSE;

	if(CAralGoodHookApp::_gInst && CAralGoodHookApp::_gInst->m_hTransDll)
	{
		bRetVal = CAralGoodHookApp::_gInst->Translate(cszJapanese, szKorean, nBufSize);
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ���� �ε�� �˰��� ���
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) HMODULE __stdcall GetCurAlgorithm()
{
	HMODULE hRetVal = NULL;

	if(CAralGoodHookApp::_gInst)
	{
		hRetVal = CAralGoodHookApp::_gInst->m_hHookingDll;
	}

	return hRetVal;
}



//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ���� �ε�� ������ ���
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) HMODULE __stdcall GetCurTranslator()
{
	HMODULE hRetVal = NULL;

	if(CAralGoodHookApp::_gInst)
	{
		hRetVal = CAralGoodHookApp::_gInst->m_hTransDll;
	}

	return hRetVal;
}



//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ���÷����� �ε� ���� �˻�
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall IsAppLocaleLoaded()
{
	BOOL bRetVal = 	CAppLocMgr::IsLoaded();

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ���÷����� On/Off
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall EnableAppLocale(BOOL bEnable)
{
	BOOL bRetVal = FALSE;

	if(bEnable == FALSE)
	{
		bRetVal = CAppLocMgr::Disable();
	}
	else
	{
		bRetVal = CAppLocMgr::Enable();
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ��� ������ ���� (���� �����常 ����)
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall SuspendAllThread()
{
	CRawHookMgr::SuspendAllThread();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ��� ������ �簡��
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall ResumeAllThread()
{
	CRawHookMgr::ResumeAllThread();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// ���� ��� �����尡 �����Ǿ� �ִ°�?
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall IsAllThreadSuspended()
{
	return CRawHookMgr::IsAllThreadSuspended();
}

//////////////////////////////////////////////////////////////////////////
// * EXPORT *
// Ư�� ���� ���̳ʸ� ��ġ
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall PreBinaryPatch(LPVOID pfnOld, unsigned char* cBuff, size_t copysize)
{
	BOOL bRetVal = FALSE;

	if(CAralGoodHookApp::_gInst)
	{
		bRetVal = CRawHookMgr::preHook(pfnOld, cBuff, copysize);
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// CAralGoodHookApp �ʱ�ȭ
//
//////////////////////////////////////////////////////////////////////////
BOOL CAralGoodHookApp::InitInstance() 
{
	// ���� �ڵ鷯 ��ġ
	//SetupExceptionHandler();
	
	::DisableThreadLibraryCalls( this->m_hInstance );

	// ���÷����� Off
	//BOOL bAppRes = CAppLocMgr::Disable();

	// ���� ���μ��� �̸� ���
	TCHAR szModuleName[MAX_PATH] = {0,};
	::GetModuleFileName(NULL, szModuleName, MAX_PATH);
	TCHAR* pNameStart = _tcsrchr(szModuleName, _T('\\') );
	if(pNameStart) m_strProcessName = pNameStart+1;
	else m_strProcessName = szModuleName;
	
	// ��ŷ������ ��� ���� �޸� ���� �õ�
	CString strMemName;
	strMemName.Format(_T("ATSM%d"), GetCurrentProcessId());
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, (LPCTSTR)strMemName);
	if(hMapFile)
	{
		LPHOOKING_OPTION pOption = (LPHOOKING_OPTION) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HOOKING_OPTION));
		
		// ���� �޸𸮸� ���������� �����ٸ�
		if (pOption)
		{
			// �޸𸮿� �ɼǰ� ���
			memcpy(&m_hookOption, pOption, sizeof(HOOKING_OPTION));
			UnmapViewOfFile(pOption);

			// ũ��Ƽ�� ���� ����
			InitializeCriticalSection(&m_csTextArgMgr);
			
			// ��ŷ ����
			ATStartUp();
		}


		CloseHandle(hMapFile);
	}

	// ���÷����� On
	//bAppRes = CAppLocMgr::Enable();

	return CWinApp::InitInstance();
}


int CAralGoodHookApp::ExitInstance(void)
{
	TRACE(_T("CAralGoodHookApp::ExitInstance() _gInst=0x%p \n"), _gInst);

	// AralGood.exe�� �ƴϸ� ����ȭ �ڵ�
	if( _gInst && m_strProcessName.CompareNoCase( _T("AralTrans.exe") ) != 0 )
	{
		TRACE(_T("CAralGoodHookApp::ExitInstance() Finalize.. \n"), _gInst);

		ATCleanUp();
		
		// ũ��Ƽ�� ���� ����
		DeleteCriticalSection(&m_csTextArgMgr);
	}

	_gInst = NULL;

	return CWinApp::ExitInstance();

}


BOOL CAralGoodHookApp::ATStartUp()
{
	TRACE(_T("CAralGoodHookApp::ATStartUp() begin \n"));

	// Ȩ ���丮
	m_strAralTransDir = CRegistryMgr::RegRead( _T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AralTransHomeDir") );

	if(CAppLocMgr::IsLoaded())
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastUsedAppPatch"), _T("AppLoc"));
	else if (GetModuleHandle(_T("KonJ.dll")))
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastUsedAppPatch"), _T("KonJ"));
	else if (GetModuleHandle(_T("JonK.dll")))
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastUsedAppPatch"), _T("JonK"));
	else
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastUsedAppPatch"), _T(""));

	// ���� ������ ����
	//CWnd* pParentWnd = AfxGetMainWnd();
	CWnd* pParentWnd = CWnd::FromHandle(GetDesktopWindow());
	//CString a;
	//a.Format(_T("%p"), pParentWnd);
	//MessageBox(NULL,a,a,MB_OK);

	int nShowHide = (int)CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("ShowHide"));

	m_pOutput = new COutputDialog(pParentWnd);
	m_pOutput->Create(IDD_MODALESS_TEXTOUT, pParentWnd);
	m_pOutput->UpdateWindow();

	int nCmdShow = SW_NORMAL;

	// ������ �ʱ�ȭ
	m_pOutput->CreateTrayIcon();

	// ���۽� �÷������� �־����ٸ�
	if(m_hookOption.szTranslatorName[0] 
		|| m_hookOption.szAlgorithmName[0]
		|| m_hookOption.szFilters[0])
	{
		// Ʈ���̷� ����
		if( m_pOutput->m_chkTrayMin.GetCheck() )
		{
			nCmdShow = SW_HIDE;
		}
		// �ּ�ȭ�� â ����
		else
		{
			nCmdShow = SW_MINIMIZE;
		}
	}

	if( nShowHide == 0 )
	{
		m_pOutput->ShowWindow(nCmdShow);

		// ����� ���� �ο�
		SetDebugPrivilege();

		// ��� ��� ��ȣ�Ӽ� ����
		RemoveModuleProtectionAll();
	}

	// ���̳ʸ� ��ġ
	CString regCount;
	int nCount = 0;
	int nHookPatch = (int)CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryPatch"));
	int nRestart = (int)CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryRestart"));
	while( (nHookPatch > nCount) && !nRestart )
	{
		nCount ++;
		regCount.Format(_T("Binary%d"),nCount);
		unsigned char* cBuff = (unsigned char*)malloc(0x10000);
		int bufLen = CRegistryMgr::RegReadBINARY(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, cBuff);
		regCount.Format(_T("BinaryAdr%d"),nCount);
		DWORD dModule = CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount);
		regCount.Format(_T("BinaryName%d"),nCount);
		CString str = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount);
		if( str != _T("") )
			dModule += (DWORD) GetModuleHandle(str);
		CRawHookMgr::preHook((LPVOID)dModule, cBuff, bufLen);

		free(cBuff);
	}

	// �ɼ� ���̱׷��̼� (MultiPlugin)
	if(stricmp(m_hookOption.szTranslatorName, "MultiPlugin") == 0)
	{
		// ezTransXP�� �ֳ� �˻��Ѵ�.
		size_t nOptionLen = strlen(m_hookOption.szTranslatorOption);
		LPSTR szTranslator = NULL;
		if(nOptionLen > 12) szTranslator = m_hookOption.szTranslatorOption + (nOptionLen-12);

		// �ִٸ� �����÷����� �̸����� ������
		if( szTranslator && stricmp(szTranslator, ",ezTransXP{}") == 0 )
		{
			//MessageBoxA(NULL, szTranslator+1, "11", MB_OK);
			strcpy(m_hookOption.szTranslatorName, "ezTransXP");
			*(szTranslator) = '\0';
		}
		// ���ٸ� �����÷����� �̸��� empty��
		else
		{
			*(m_hookOption.szTranslatorName) = '\0';
		}

		// �����÷����� �ɼ��� �����̸����ۿ� ����
		strcpy(m_hookOption.szFilters, m_hookOption.szTranslatorOption);

		// �����÷����� �ɼ��� ����
		*(m_hookOption.szTranslatorOption) = '\0';
	}

	// �ɼǿ� ������ ������ �÷������� �ε�
	if(m_hookOption.szTranslatorName)
	{

#ifdef UNICODE
		wchar_t cszComboText[MAX_PATH];
		MyMultiByteToWideChar(CP_ACP, 0, m_hookOption.szTranslatorName, -1, cszComboText, MAX_PATH);
#else
		LPCSTR cszComboText = m_hookOption.szTranslatorName;
#endif

		int idx = m_pOutput->m_comboTranslator.FindString(0, cszComboText);
		if(CB_ERR != idx)
		{
			CString strPluginName;
			m_pOutput->m_comboTranslator.GetLBText(idx, strPluginName);

			/*
			if( LoadTransPlugin(strPluginName) )
			{
				m_pOutput->m_comboTranslator.SetCurSel(idx);
			}
			else
			{
				m_pOutput->m_comboTranslator.SetCurSel(0);
			}
			*/

			TRACE(_T("translator : %s \n"), (LPCTSTR)strPluginName);

			m_pOutput->m_comboTranslator.PostMessage( CB_SETCURSEL, idx, 0 );
			m_pOutput->PostMessage(
				WM_COMMAND, 
				MAKEWPARAM(m_pOutput->m_comboTranslator.GetDlgCtrlID(),CBN_SELCHANGE),
				(LPARAM)m_pOutput->m_comboTranslator.m_hWnd
			);
			
		}
	}
	
	// �ɼǿ� ������ ���� �÷������� �ε� (2008.10.05)
	if(m_hookOption.szFilters)
	{
		ATPLUGIN_ARGUMENT_ARRAY args;

		GetATPluginArgsFromOptionString(m_hookOption.szFilters, args);

		if (!args.empty() && !m_cMultiPlugin.Add(args))
			return FALSE;

	}

	m_pOutput->UpdateFilterGrid();

	// �ɼǿ� ������ �˰��� �÷������� �ε�
	if(m_hookOption.szAlgorithmName)
	{

#ifdef UNICODE
		wchar_t cszComboText[MAX_PATH];
		MyMultiByteToWideChar(CP_ACP, 0, m_hookOption.szAlgorithmName, -1, cszComboText, MAX_PATH);
#else
		LPCSTR cszComboText = m_hookOption.szAlgorithmName;
#endif

		int idx = m_pOutput->m_comboHookMode.FindString(0, cszComboText);
		if(CB_ERR != idx)
		{
			EnterCriticalSection(&m_csTextArgMgr);

			// ���ο� �ؽ�Ʈ ������ ����
			
			/*
			CString strPluginName;
			m_pOutput->m_comboHookMode.GetLBText(idx, strPluginName);

			if( LoadHookingPlugin(strPluginName) )
			{
				m_pOutput->m_comboHookMode.SetCurSel(idx);
			}
			else
			{
				m_pOutput->m_comboHookMode.SetCurSel(0);
			}
			*/


			TRACE(_T("algorithm : %s \n"), (LPCTSTR)cszComboText);

			m_pOutput->m_comboHookMode.PostMessage( CB_SETCURSEL, idx, 0 );
			m_pOutput->PostMessage(
				WM_COMMAND, 
				MAKEWPARAM(m_pOutput->m_comboHookMode.GetDlgCtrlID(),CBN_SELCHANGE),
				(LPARAM)m_pOutput->m_comboHookMode.m_hWnd
			);

			LeaveCriticalSection(&m_csTextArgMgr);
			
			//m_sHookingFunc.pfnOnPluginStart();
			m_pOutput->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BTN_HOOK_ONOFF, BN_CLICKED), 0 );

		}
	}

	TRACE(_T("CAralGoodHookApp::ATStartUp() end \n"));

	return TRUE;
}

void CAralGoodHookApp::ATCleanUp()
{
	TRACE(_T("CAralGoodHookApp::ATCleanUp() begin \n"));

	// ��ŷ �÷����� ����ȭ
	UnloadHookingPlugin();

	// ���� ����
	m_cMultiPlugin.RemoveAll();
	
	// ���������
	UnloadTransPlugin();

	// ���� ������ �ı�
	if( m_pOutput->GetSafeHwnd() && IsWindow(m_pOutput->GetSafeHwnd()) )
	{
		//delete m_pOutput;
		m_pOutput->CloseWindow();
		m_pOutput->DestroyWindow();
		//delete m_pOutput;
		m_pOutput = NULL;
	}

	// ����
	BOOL bRes = CRawHookMgr::Unhook();

	// ���α׷� ���� ���Ḧ ���� �̺�Ʈ ����
	CString strExitEventName;
	strExitEventName.Format(_T("ATEE%d"), GetCurrentProcessId());
	HANDLE hExitEvent = ::CreateEvent(NULL, FALSE, FALSE, strExitEventName);
	::SetEvent(hExitEvent);
	::CloseHandle(hExitEvent);

	TRACE(_T("CAralGoodHookApp::ATCleanUp() end \n"));
}

// ��ŷ �÷����� �ε�
BOOL CAralGoodHookApp::LoadHookingPlugin(CString strPluginName)
{
	BOOL bLoadRes = FALSE;


	// dll Ǯ�н� ����
	if( !m_strAralTransDir.IsEmpty() && !strPluginName.IsEmpty() )
	{
		CString strDllPath = m_strAralTransDir + _T("\\Algorithm\\") + strPluginName + _T(".dll");
		TCHAR szTmpPath[MAX_PATH];
		::GetShortPathName((LPCTSTR)strDllPath, szTmpPath, MAX_PATH);

		// dll �ε�
		m_hHookingDll = LoadLibrary(szTmpPath);
		if(m_hHookingDll)
		{
			m_sHookingFunc.pfnOnPluginInit	= (PROC_HOOK_OnPluginInit) GetProcAddress(m_hHookingDll, "OnPluginInit");
			m_sHookingFunc.pfnOnPluginStart	= (PROC_HOOK_OnPluginStart) GetProcAddress(m_hHookingDll, "OnPluginStart");
			m_sHookingFunc.pfnOnPluginStop	= (PROC_HOOK_OnPluginStop) GetProcAddress(m_hHookingDll, "OnPluginStop");
			m_sHookingFunc.pfnOnPluginOption= (PROC_HOOK_OnPluginOption) GetProcAddress(m_hHookingDll, "OnPluginOption");
			m_sHookingFunc.pfnOnPluginClose	= (PROC_HOOK_OnPluginClose) GetProcAddress(m_hHookingDll, "OnPluginClose");

			// �Լ� ������ ���� ����������
			if(m_sHookingFunc.pfnOnPluginInit && m_sHookingFunc.pfnOnPluginStart && m_sHookingFunc.pfnOnPluginStop
				&& m_sHookingFunc.pfnOnPluginOption && m_sHookingFunc.pfnOnPluginClose)
			{
				// Init
				if( m_sHookingFunc.pfnOnPluginInit(m_pOutput->GetSafeHwnd(), m_hookOption.szAlgorithmOption) )
				{
					// ��Ʈ�� Enable
					if(m_pOutput)
					{
						m_pOutput->GetDlgItem(IDC_BTN_HOOK_SETTING)->EnableWindow(TRUE);
						m_pOutput->GetDlgItem(IDC_BTN_HOOK_ONOFF)->EnableWindow(TRUE);
						m_pOutput->GetDlgItem(IDC_STATIC_HOOK_1)->EnableWindow(TRUE);
						m_pOutput->GetDlgItem(IDC_EDIT_HOOK_OPTION_STRING)->EnableWindow(TRUE);
						m_strHookingPluginName = strPluginName;
						RefreshHookingOption();
					}

					bLoadRes = TRUE;
				}

			}
			else
			{
				::MessageBox(m_pOutput->GetSafeHwnd(), _T("ȣȯ���� �ʴ� ��ŷ �÷������Դϴ�. \r\n") + strDllPath, _T("��ŷ �÷����� �ε� ����"), MB_OK);
			}
		}
		else
		{
			::MessageBox(m_pOutput->GetSafeHwnd(), _T("�÷����� ������ ã�� �� �����ϴ�. \r\n") + strDllPath, _T("��ŷ �÷����� �ε� ����"), MB_OK);
		}

	}


	if( FALSE == bLoadRes )
	{
		UnloadHookingPlugin();
	}

	return bLoadRes;
}

// ��ŷ �ɼ� ��Ʈ�� ����
void CAralGoodHookApp::RefreshHookingOption()
{

#ifdef _UNICODE
	wchar_t pHookOption[1024];
	MyMultiByteToWideChar(CP_ACP, 0, m_hookOption.szAlgorithmOption, -1, pHookOption, 1024);
#else
	LPCSTR pHookOption = m_hookOption.szAlgorithmOption;
#endif

	CString strOption;
	strOption.Format(_T("/a:%s{%s}"), (LPCTSTR)m_strHookingPluginName, pHookOption);
	m_pOutput->GetDlgItem(IDC_EDIT_HOOK_OPTION_STRING)->SetWindowText((LPCTSTR)strOption);
}


// ��ŷ �÷����� ��ε�
void CAralGoodHookApp::UnloadHookingPlugin()
{

	if(m_hHookingDll)
	{
		m_sHookingFunc.pfnOnPluginStop	= (PROC_HOOK_OnPluginStop) GetProcAddress(m_hHookingDll, "OnPluginStop");
		m_sHookingFunc.pfnOnPluginClose	= (PROC_HOOK_OnPluginClose) GetProcAddress(m_hHookingDll, "OnPluginClose");
		if(m_sHookingFunc.pfnOnPluginStop) m_sHookingFunc.pfnOnPluginStop();
		if(m_sHookingFunc.pfnOnPluginClose) m_sHookingFunc.pfnOnPluginClose();
		FreeLibrary(m_hHookingDll);

		m_hHookingDll = NULL;
		ZeroMemory(&m_sHookingFunc, sizeof(HOOKING_FUNCTION_ENTRY));
		ZeroMemory(&m_hookOption.szAlgorithmOption, sizeof(m_hookOption.szAlgorithmOption));
	}


	// ��Ʈ�� Disable
	if(m_pOutput && IsWindow(m_pOutput->GetSafeHwnd()))
	{
		m_pOutput->GetDlgItem(IDC_BTN_HOOK_SETTING)->EnableWindow(FALSE);
		m_pOutput->GetDlgItem(IDC_BTN_HOOK_ONOFF)->EnableWindow(FALSE);
		m_pOutput->GetDlgItem(IDC_STATIC_HOOK_1)->EnableWindow(FALSE);
		m_pOutput->GetDlgItem(IDC_EDIT_HOOK_OPTION_STRING)->SetWindowText(_T(""));
		m_pOutput->GetDlgItem(IDC_EDIT_HOOK_OPTION_STRING)->EnableWindow(FALSE);
		m_pOutput->GetDlgItem(IDC_BTN_HOOK_ONOFF)->SetWindowText( _T("����") );
	}

	m_strHookingPluginName = _T("");
}


// ���� �÷����� �ε�
BOOL CAralGoodHookApp::LoadTransPlugin(CString strPluginName)
{
	BOOL bLoadRes = FALSE;

	// ��Ƽ�÷����� �ε� ���� (2008.10.26)
	if(strPluginName.CompareNoCase(_T("MultiPlugin")) == 0)
	{
		::MessageBox(m_pOutput->GetSafeHwnd(), _T("��Ƽ �÷������� ���̻� �����Ͻ� �� �����ϴ�."), _T("�߸��� ���� �÷�����"), MB_OK);
		return FALSE;
	}

	// dll Ǯ�н� ����
	if( !m_strAralTransDir.IsEmpty() && !strPluginName.IsEmpty() )
	{
		
		CString strDllPath = m_strAralTransDir + _T("\\Translator\\") + strPluginName + _T(".dll");
		TCHAR szTmpPath[MAX_PATH];
		::GetShortPathName((LPCTSTR)strDllPath, szTmpPath, MAX_PATH);

		// dll �ε�
		HMODULE hTransDll = LoadLibrary(szTmpPath);
		if(hTransDll)
		{
			m_sTransFunc.pfnOnPluginInit	= (PROC_TRANS_OnPluginInit) GetProcAddress(hTransDll, "OnPluginInit");
			m_sTransFunc.pfnOnPluginOption	= (PROC_TRANS_OnPluginOption) GetProcAddress(hTransDll, "OnPluginOption");
			m_sTransFunc.pfnOnPluginClose	= (PROC_TRANS_OnPluginClose) GetProcAddress(hTransDll, "OnPluginClose");
			m_sTransFunc.pfnTranslate		= (PROC_TRANS_Translate) GetProcAddress(hTransDll, "Translate");

			// ������
			if( m_sTransFunc.pfnOnPluginInit && m_sTransFunc.pfnOnPluginOption
				&& m_sTransFunc.pfnOnPluginClose && m_sTransFunc.pfnTranslate )
			{
				// Init
				if( m_sTransFunc.pfnOnPluginInit(m_pOutput->GetSafeHwnd(), m_hookOption.szTranslatorOption) )
				{
					// ��Ʈ�� Enable
					if(m_pOutput)
					{
						m_pOutput->GetDlgItem(IDC_BTN_TRANS_SETTING)->EnableWindow(TRUE);
						m_pOutput->GetDlgItem(IDC_STATIC_TRANS_1)->EnableWindow(TRUE);
						m_pOutput->GetDlgItem(IDC_EDIT_TRANS_OPTION_STRING)->EnableWindow(TRUE);
						m_strTransPluginName = strPluginName;
						RefreshTransOption();						
					}

					m_hTransDll = hTransDll;
					bLoadRes = TRUE;
				}

			}
			else
			{
				::MessageBox(m_pOutput->GetSafeHwnd(), _T("ȣȯ���� �ʴ� ���� �÷������Դϴ�. \r\n") + strDllPath, _T("���� �÷����� �ε� ����"), MB_OK);
			}
		}
		else
		{
			::MessageBox(m_pOutput->GetSafeHwnd(), _T("�÷����� ������ ã�� �� �����ϴ�. \r\n") + strDllPath, _T("���� �÷����� �ε� ����"), MB_OK);
		}

	}


	if( FALSE == bLoadRes )
	{
		UnloadTransPlugin();
	}

	return bLoadRes;
}


// ���� �ɼ� ��Ʈ�� ����
void CAralGoodHookApp::RefreshTransOption()
{

#ifdef _UNICODE
	wchar_t pTransOption[1024];
	MyMultiByteToWideChar(CP_ACP, 0, m_hookOption.szTranslatorOption, -1, pTransOption, 1024);
#else
	LPCSTR pTransOption = m_hookOption.szTranslatorOption;
#endif

	CString strOption;
	strOption.Format(_T("/t:%s{%s}"), (LPCTSTR)m_strTransPluginName, pTransOption);
	m_pOutput->GetDlgItem(IDC_EDIT_TRANS_OPTION_STRING)->SetWindowText((LPCTSTR)strOption);
}


// ���� �÷����� ��ε�
void CAralGoodHookApp::UnloadTransPlugin()
{
	if(m_hTransDll)
	{
		m_sTransFunc.pfnOnPluginClose	= (PROC_TRANS_OnPluginClose) GetProcAddress(m_hTransDll, "OnPluginClose");
		if(m_sTransFunc.pfnOnPluginClose) m_sTransFunc.pfnOnPluginClose();
		FreeLibrary(m_hTransDll);

		m_hTransDll = NULL;
		ZeroMemory(&m_sTransFunc, sizeof(TRANS_FUNCTION_ENTRY));
		ZeroMemory(&m_hookOption.szTranslatorOption, sizeof(m_hookOption.szTranslatorOption));
	}

	// ��Ʈ�� Disable
	if(m_pOutput && IsWindow(m_pOutput->GetSafeHwnd()))
	{
		m_pOutput->GetDlgItem(IDC_BTN_TRANS_SETTING)->EnableWindow(FALSE);
		m_pOutput->GetDlgItem(IDC_STATIC_TRANS_1)->EnableWindow(FALSE);
		m_pOutput->GetDlgItem(IDC_EDIT_TRANS_OPTION_STRING)->SetWindowText(_T(""));
		m_pOutput->GetDlgItem(IDC_EDIT_TRANS_OPTION_STRING)->EnableWindow(FALSE);
	}

	m_strTransPluginName = _T("");
}


// ����â���� �Ÿ�� ������ �Ͼ�ٸ�
void CAralGoodHookApp::OnAlgorithmChanged()
{
	
	if(m_pOutput)
	{
		EnterCriticalSection(&m_csTextArgMgr);

		// ���� ��ŷ �÷����� ����
		UnloadHookingPlugin();

		// ���ο� �ؽ�Ʈ ������ ����
		int sel = m_pOutput->m_comboHookMode.GetCurSel();
		if(sel)
		{
			CString strPluginName;
			m_pOutput->m_comboHookMode.GetLBText(sel, strPluginName);

			if( LoadHookingPlugin(strPluginName) == FALSE || m_sHookingFunc.pfnOnPluginStart() == FALSE )
			{
				m_pOutput->m_comboHookMode.SetCurSel(0);
			}
		}


		LeaveCriticalSection(&m_csTextArgMgr);

	}

}

void CAralGoodHookApp::OnAlgorithmOption()
{

	BOOL bOK = m_sHookingFunc.pfnOnPluginOption();

	// ���� ������ �ɼ� ��Ʈ�� ����
	if(bOK && m_pOutput)
	{
		CString strPluginName;
		m_pOutput->m_comboHookMode.GetLBText( m_pOutput->m_comboHookMode.GetCurSel(), strPluginName );
		
#ifdef UNICODE
		WCHAR szTmpOption[4096];
		MyMultiByteToWideChar(949, 0, m_hookOption.szAlgorithmOption, -1, szTmpOption, 4096);
#else
		LPCSTR szTmpOption = m_hookOption.szAlgorithmOption;
#endif

		CString strOption;
		strOption.Format(_T("/a:%s{%s}"), (LPCTSTR)strPluginName, szTmpOption);
		m_pOutput->GetDlgItem(IDC_EDIT_HOOK_OPTION_STRING)->SetWindowText((LPCTSTR)strOption);	
	}

}

void CAralGoodHookApp::OnAlgorithmOnOff()
{
	
	if(m_pOutput)
	{
		CString strBtnText;
		m_pOutput->GetDlgItem(IDC_BTN_HOOK_ONOFF)->GetWindowText(strBtnText);

		// ������Ų�Ŷ��
		if(strBtnText==_T("����"))
		{
			BOOL bRes = m_sHookingFunc.pfnOnPluginStart();
			if(bRes)
			{
				m_pOutput->GetDlgItem(IDC_BTN_HOOK_ONOFF)->SetWindowText( _T("����") );
			}
		}
		// ������Ų�Ŷ��
		if(strBtnText==_T("����"))
		{
			BOOL bRes = m_sHookingFunc.pfnOnPluginStop();
			if(bRes)
			{
				m_pOutput->GetDlgItem(IDC_BTN_HOOK_ONOFF)->SetWindowText( _T("����") );
			}
		}

	}


}

// ����â���� ������ ������ �Ͼ�ٸ�
void CAralGoodHookApp::OnTranslatorChanged()
{
	if(m_pOutput)
	{
		EnterCriticalSection(&m_csTextArgMgr);

		// ���� ���� �÷����� ����
		UnloadTransPlugin();

		// ���ο� ���� �÷����� �ε�
		int sel = m_pOutput->m_comboTranslator.GetCurSel();
		if(sel)
		{
			CString strPluginName;
			m_pOutput->m_comboTranslator.GetLBText(sel, strPluginName);

			if( LoadTransPlugin(strPluginName) == FALSE )
			{
				m_pOutput->m_comboTranslator.SetCurSel(0);
			}
		}


		LeaveCriticalSection(&m_csTextArgMgr);

	}

}

void CAralGoodHookApp::OnTranslatorOption()
{
	BOOL bOK = m_sTransFunc.pfnOnPluginOption();

	// ���� ������ �ɼ� ��Ʈ�� ����
	if(bOK && m_pOutput)
	{
		CString strPluginName;
		m_pOutput->m_comboTranslator.GetLBText( m_pOutput->m_comboTranslator.GetCurSel(), strPluginName );

#ifdef UNICODE
		WCHAR szTmpOption[4096];
		MyMultiByteToWideChar(949, 0, m_hookOption.szTranslatorOption, -1, szTmpOption, 4096);
#else
		LPCSTR szTmpOption = m_hookOption.szTranslatorOption;
#endif

		CString strOption;
		strOption.Format(_T("/t:%s{%s}"), (LPCTSTR)strPluginName, szTmpOption);
		m_pOutput->GetDlgItem(IDC_EDIT_TRANS_OPTION_STRING)->SetWindowText((LPCTSTR)strOption);	
	}

}


//////////////////////////////////////////////////////////////////////////
//
// ��� ��ȣ�Ӽ� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CAralGoodHookApp::RemoveModuleProtectionAll()
{
	// ���μ����� ������ �������� ����ϴ�.
	HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}


	MODULEENTRY32 me32;
	memset(&me32, 0,sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);

	char szTemp[MAX_PATH];
	memset(szTemp, 0, MAX_PATH);

	// ù ��° ���(exe ������ ���)�� ��θ� �����մϴ�.
	BOOL bNext = Module32First(hProcessSnap, &me32);
	while(bNext)
	{
		CRawHookMgr::RemoveModuleProtection(me32.hModule);
		bNext = Module32Next( hProcessSnap, &me32 );
	}
	CloseHandle (hProcessSnap);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// ����� ���� �ο�
//
//////////////////////////////////////////////////////////////////////////
void CAralGoodHookApp::SetDebugPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

	LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&luid);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);

	CloseHandle(hToken);
}


//////////////////////////////////////////////////////////////////////////
//
// ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CAralGoodHookApp::Translate( LPCSTR cszJapanese, LPSTR szKorean, int nBufSize )
{

	BOOL bRetVal = FALSE;

	if(m_sTransFunc.pfnTranslate)
	{
		bRetVal = m_cMultiPlugin.Translate(cszJapanese, szKorean, nBufSize);
		//bRetVal = m_sTransFunc.pfnTranslate(cszJapanese, szKorean, nBufSize);
	}

	return bRetVal;
}

void ModifyValue()
{
	REGISTER_ENTRY regs;
	void* pPrevFunc;


	_asm
	{
		mov eax, [ebp+4];
		mov regs._ESP, eax;
		mov eax, [ebp+8];
		mov regs._EBP, eax;
		mov eax, [ebp+12];
		mov regs._EDI, eax;
		mov eax, [ebp+16];
		mov regs._ESI, eax;
		mov eax, [ebp+20];
		mov regs._EDX, eax;
		mov eax, [ebp+24];
		mov regs._ECX, eax;
		mov eax, [ebp+28];
		mov regs._EBX, eax;
		mov eax, [ebp+32];
		mov regs._EAX, eax;
		mov eax, [ebp+36];
		mov regs._EFL, eax;


		mov ebx, regs._ESP;
		mov eax, [ebx-4];		// �����ּҿ���
		sub eax, 5;				// 5�� ����
		mov pPrevFunc, eax;		// �Լ��ּ�
	}

	// ��ŷ�Լ� ���̺��� �ݹ� �Լ� ���ϱ�
	if(CAralGoodHookApp::_gInst)
	{
		map<void*,void*>::iterator iter = CAralGoodHookApp::_gInst->m_mapCallbackTable.find(pPrevFunc);
		if( iter != CAralGoodHookApp::_gInst->m_mapCallbackTable.end() )
		{
			PROC_HookCallback pfnCallback = (PROC_HookCallback)iter->second;
			if(pfnCallback)
			{
				pfnCallback(pPrevFunc, &regs);
			}
		}

	}


	// �����ڵ�� ����
	void* pOrigFunc = CRawHookMgr::GetOrigFunc( pPrevFunc );

	_asm
	{
		mov ebx, regs._ESP;

		// �÷��� ����
		mov eax, regs._EFL;
		mov [ebx-8], eax;

		// ������ �� ����
		mov eax, pOrigFunc;
		mov [ebx-4], eax;

		// �������� ����
		mov eax, regs._EAX;
		mov ebx, regs._EBX;
		mov ecx, regs._ECX;
		mov edx, regs._EDX;
		mov esi, regs._ESI;
		mov edi, regs._EDI;
		mov esp, regs._ESP;
		mov ebp, regs._EBP;

		sub esp, 8;

		// �÷��� ����
		popfd;

		// ����
		ret;

	}
}

__declspec(naked) void PreModifyValue()
{
	_asm
	{
		pushfd;
		push eax;
		push ebx;
		push ecx;
		push edx;
		push esi;
		push edi;
		push ebp;
		mov eax, esp;
		add eax, 36;
		push eax;
		jmp ModifyValue;
	}
}