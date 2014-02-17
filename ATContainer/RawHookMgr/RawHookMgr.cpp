// RawHookMgr.cpp: implementation of the CRawHookMgr class.
//
//////////////////////////////////////////////////////////////////////
#include "RawHookMgr.h"
#include "RawHookedModule.h"
#include <tlhelp32.h>

#include "../Debug.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModuleList CRawHookMgr::m_listModules;
BOOL		CRawHookMgr::m_bIsAllThreadSuspended = FALSE;
HMODULE		CRawHookMgr::m_hExecModule = NULL;

CRawHookMgr::CRawHookMgr()
{

}

CRawHookMgr::~CRawHookMgr()
{

}



LPVOID CRawHookMgr::GetFixedFuncAddr( LPVOID pFunc )
{
	LPVOID pRetFunc = NULL;
	
	// �Լ��� ����ִ� ����� �˾Ƴ���
	HMODULE hModule = HandleFromAddress(pFunc);

	// �ش� �ּҰ� ������ �״�� ������
	if(hModule)
	{
		pRetFunc = pFunc;
	}
	// �ش� �ּҰ� ������ ���� ���� ����
	else
	{
		hModule = GetExecModuleHandle();

		if(hModule && (UINT_PTR)hModule < (UINT_PTR)pFunc )
		{
			MODULEINFO mi;
			UINT_PTR dist = (UINT_PTR)pFunc - (UINT_PTR)hModule;
			BOOL bRes = ::GetModuleInformation( ::GetCurrentProcess(), hModule, &mi, sizeof(MODULEINFO) );
			if( TRUE == bRes && (DWORD)dist < mi.SizeOfImage)
			{
				pRetFunc = (LPVOID)((UINT_PTR)hModule + dist);
			}
		}
	}

	return pRetFunc;
}


//////////////////////////////////////////////////////////////////////////
//
// Ư�� �Լ� ��ŷ
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookMgr::Hook(LPVOID pfnOld, LPVOID pfnNew, BOOL bWriteCallCode, INT nHookType)
{
	NOTIFY_DEBUG_MESSAGE(_T("CRawHookMgr:Hook: called, pfnOld=%p, pfnNew=%p, bWrite=%s\n"), pfnOld, pfnNew, (bWriteCallCode?_T("TRUE"):_T("FALSE")));

	pfnOld = GetFixedFuncAddr(pfnOld);
	if(NULL == pfnOld) return FALSE;

	// �Լ��� ����ִ� ����� �˾Ƴ���
	HMODULE hModule = HandleFromAddress(pfnOld);
	if(NULL == hModule) return FALSE;

	// ���� �����ǰ� �ִ� ���� �� �� ����� �ִ� ã�ƺ���
	CRawHookedModule* pModule = FindModule(hModule);

	// ������ �� ��� ���
	if( NULL == pModule )
	{
		pModule = new CRawHookedModule();
		if( pModule->Init( hModule ) == FALSE )
		{
			delete pModule;
			return FALSE;
		}
		m_listModules.push_back(pModule);
	}

	// ��ⰴü�� �� �Լ��� ��ŷ�϶� ����
	if(nHookType == ATHOOKTYPE_SOURCE) return pModule->Hook2(pfnOld, pfnNew, bWriteCallCode);
	else return pModule->Hook(pfnOld, pfnNew, bWriteCallCode);
}

//////////////////////////////////////////////////////////////////////////
//
// Ư�� ���̳ʸ� ��ġ
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookMgr::preHook(LPVOID pfnOld, unsigned char* cBuff, size_t copysize)
{
	pfnOld = GetFixedFuncAddr(pfnOld);
	if(NULL == pfnOld) return FALSE;

	// �Լ��� ����ִ� ����� �˾Ƴ���
	HMODULE hModule = HandleFromAddress(pfnOld);
	if(NULL == hModule) return FALSE;

	// ���� �����ǰ� �ִ� ���� �� �� ����� �ִ� ã�ƺ���
	CRawHookedModule* pModule = FindModule(hModule);

	// ������ �� ��� ���
	if( NULL == pModule )
	{
		pModule = new CRawHookedModule();
		if( pModule->Init( hModule ) == FALSE )
		{
			delete pModule;
			return FALSE;
		}
		m_listModules.push_back(pModule);
	}

	return pModule->preHook(pfnOld, cBuff, copysize);
}

BOOL CRawHookMgr::Hook(LPCTSTR cszDllName, LPCTSTR cszExportName, LPVOID pfnNew, INT nHookType)
{
	// �ְ� ���� �ϴ� ������ ���̺귯���� �ε��Ѵ�
	::LoadLibrary( cszDllName );
	
	// ����ڵ� ���
	HMODULE hModule = ::GetModuleHandle( cszDllName );
	if(NULL==hModule) return FALSE;
	
#ifdef _UNICODE
	char szFuncName[MAX_PATH];
	::WideCharToMultiByte(CP_ACP, 0, cszExportName, -1, szFuncName, MAX_PATH, NULL, NULL);
#else
	LPCSTR szFuncName = cszExportName;
#endif

	FARPROC pOrigFunc = ::GetProcAddress( hModule, szFuncName );
	if(NULL==pOrigFunc) return FALSE;

	return CRawHookMgr::Hook(pOrigFunc, pfnNew, FALSE, nHookType);
}


///////////////////////////////////////////////////////////////////////////
//
// Ư�� �Լ� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookMgr::Unhook(LPVOID pFunc)
{
	BOOL bRetVal = FALSE;	

	pFunc = GetFixedFuncAddr(pFunc);
	if(NULL == pFunc) return FALSE;

	// ���� �����ǰ� �ִ� ������ ��ȸ�ϸ� Unhook �õ�
	CModuleList::iterator iter = m_listModules.begin();
	for( ; iter!=m_listModules.end(); iter++ )
	{
		CRawHookedModule* pTmpMod = (*iter);
		
		// ���� �õ�
		if( pTmpMod->Unhook(pFunc) )
		{
			// ���̻� ��ŷ�ϰ� �ִ� �Լ��� ������
			// -> No! ��ŷ�ϰ� ���� �ʴ��� ����Ǿ�� �� �� �����Ƿ� ������ ���� ���ܵд�.
			if( pTmpMod->m_listFuncs.empty() )
			{				
				// �� ����� ����
				pTmpMod->Close();
				delete pTmpMod;
				m_listModules.erase(iter);
			}

			bRetVal = TRUE;
			break;
		}

	}

	return bRetVal;
}

///////////////////////////////////////////////////////////////////////////
//
// ���ݱ��� �� �� �ִ� �Լ� ���� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookMgr::Unhook()
{
	// ���� �����ǰ� �ִ� ���� ���� Unhook��Ű�� ����
	CModuleList::iterator iter = m_listModules.begin();
	for( ; iter!=m_listModules.end(); iter++ )
	{
		CRawHookedModule* pTmpMod = (*iter);
		pTmpMod->Close();
		delete pTmpMod;
	}
	m_listModules.clear();
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// Ư�� �Լ��� ���� �ڵ带 ��ȯ
//
//////////////////////////////////////////////////////////////////////////
LPVOID CRawHookMgr::GetOrigFunc(LPVOID pFunc)
{
	LPVOID pfnOrig = NULL;

	pFunc = GetFixedFuncAddr(pFunc);
	if(NULL == pFunc) return NULL;

	CModuleList::iterator iter = m_listModules.begin();
	for( ; iter!=m_listModules.end(); iter++ )
	{
		CRawHookedModule* pTmpMod = (*iter);
		pfnOrig = pTmpMod->GetOrigFunc( pFunc );
		if( pfnOrig ) break;
	}

	if(NULL == pfnOrig) pfnOrig = pFunc;	// �׻� unhook�Ǿ����� �����Լ� �ּ� return
	
	return pfnOrig;
}




//////////////////////////////////////////////////////////////////////////
//
// Ư�� �ּҿ� ���Ǿ� �ִ� ��� �ڵ��� ��ȯ
//
//////////////////////////////////////////////////////////////////////////
HMODULE CRawHookMgr::HandleFromAddress(LPVOID pAddr)
{
	/*
	MEMORY_BASIC_INFORMATION mbi;

	return ((::VirtualQuery(pAddr, &mbi, sizeof(mbi)) != 0) 
	        ? (HMODULE) mbi.AllocationBase : NULL);
	*/

	HMODULE hOwnerMod = NULL;

	::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pAddr, &hOwnerMod);

	return hOwnerMod;
}



///////////////////////////////////////////////////////////////////////////
//
// �ڵ鰪���� ���� ���� ��ⰴü ã�Ƴ�
//
//////////////////////////////////////////////////////////////////////////
CRawHookedModule* CRawHookMgr::FindModule(HMODULE hModule)
{
	CRawHookedModule* pModule = NULL;

	CModuleList::iterator iter = m_listModules.begin();
	for( ; iter!=m_listModules.end(); iter++ )
	{
		CRawHookedModule* pTmpMod = (*iter);
		if( pTmpMod->GetModule() == hModule )
		{
			pModule = pTmpMod;
			break;
		}
	}

	return pModule;
}

///////////////////////////////////////////////////////////////////////////
//
// �� �����Ͱ� �ŵ� ��⿡ �ִ°� ��ȯ
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookMgr::IsInHookedModule(LPVOID pCode)
{
	pCode = GetFixedFuncAddr(pCode);
	if(NULL == pCode) return FALSE;

	CModuleList::iterator iter = m_listModules.begin();
	for( ; iter!=m_listModules.end(); iter++ )
	{
		CRawHookedModule* pTmpMod = (*iter);
		if( pTmpMod->IsInHookedModule( pCode ) ) return TRUE;
	}

	return FALSE;	
}


///////////////////////////////////////////////////////////////////////////
//
// �� �����Ͱ� �ŵ� ��⿡ �ִ°� ��ȯ
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookMgr::RemoveModuleProtection(HMODULE hModule)
{
	MODULEINFO ModuleInfo;
	
	// �� ����� ���� ���ϱ�
	BOOL bRes = ::GetModuleInformation( ::GetCurrentProcess(), hModule, &ModuleInfo, sizeof(MODULEINFO) );
	if( FALSE==bRes )
	{
		DWORD nErr = GetLastError();
		return FALSE;
	}

	SIZE_T curOffset = 0;

	while(curOffset<ModuleInfo.SizeOfImage)	
	{

		// ������ ���� ����
		MEMORY_BASIC_INFORMATION mbi = {0,};
		SIZE_T retVal = ::VirtualQuery((LPBYTE)ModuleInfo.lpBaseOfDll + curOffset, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		// �������� ����
		if( mbi.Protect )
		{
			// �������� ��ȣ�Ӽ� ����
			DWORD dwOldProtect;
			bRes = ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		}

		curOffset += mbi.RegionSize;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// ��� �����带 ������Ų�� (���� �����常 ����)
//
//////////////////////////////////////////////////////////////////////////
void CRawHookMgr::SuspendAllThread()
{
	// ��� ��������� �������� ����ϴ�.
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if(hThreadSnap == INVALID_HANDLE_VALUE) return;


	THREADENTRY32 te32;
	memset(&te32, 0,sizeof(THREADENTRY32));
	te32.dwSize = sizeof(THREADENTRY32);

	// ���� ���μ���ID�� ������ID
	DWORD dwCurPID = GetCurrentProcessId();
	DWORD dwCurTID = GetCurrentThreadId();

	// ������ ��ȸ
	BOOL bNext = Thread32First(hThreadSnap, &te32);
	while(bNext)
	{
		if( te32.th32OwnerProcessID == dwCurPID && te32.th32ThreadID != dwCurTID )
		{
			// ������ ����
			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
			if(hThread)
			{
				SuspendThread(hThread);
				//CloseHandle(hThread);
			}
		}

		bNext = Thread32Next(hThreadSnap, &te32);
	}
	CloseHandle (hThreadSnap);

	m_bIsAllThreadSuspended = TRUE;
}



//////////////////////////////////////////////////////////////////////////
//
// ��� �����带 ������Ų��
//
//////////////////////////////////////////////////////////////////////////
void CRawHookMgr::ResumeAllThread()
{
	// ��� ��������� �������� ����ϴ�.
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if(hThreadSnap == INVALID_HANDLE_VALUE) return;


	THREADENTRY32 te32;
	memset(&te32, 0,sizeof(THREADENTRY32));
	te32.dwSize = sizeof(THREADENTRY32);

	// ���� ���μ���ID�� ������ID
	DWORD dwCurPID = GetCurrentProcessId();
	DWORD dwCurTID = GetCurrentThreadId();

	// ������ ��ȸ
	BOOL bNext = Thread32First(hThreadSnap, &te32);
	while(bNext)
	{
		if( te32.th32OwnerProcessID == dwCurPID && te32.th32ThreadID != dwCurTID )
		{
			// ������ ����
			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
			if(hThread)
			{
				ResumeThread(hThread);
				//CloseHandle(hThread);
			}
		}

		bNext = Thread32Next(hThreadSnap, &te32);
	}
	CloseHandle (hThreadSnap);

	m_bIsAllThreadSuspended = FALSE;
}

BOOL CRawHookMgr::IsAllThreadSuspended()
{
	return m_bIsAllThreadSuspended;
}

