// RawHookedModule.cpp: implementation of the CRawHookedModule class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#pragma comment (lib, "psapi.lib")
#include "RawHookedModule.h"
#include "RawHookMgr.h"

#include "../Debug.h"

//#ifdef USE_HOOKINGMODE_2

#include "disasm.h"

disasm g_disasm;
void* ptrEmptyCodeArea = NULL;
//#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRawHookedModule::CRawHookedModule()
: m_hModule(NULL), m_pOrigImg(NULL)		//, m_listFuncs(NULL)
{
	memset(m_szModuleName, 0, MAX_PATH);
	memset(&m_ModuleInfo, 0, sizeof(MODULEINFO));
}

CRawHookedModule::~CRawHookedModule()
{
	Close();
}

void *CRawHookedModule::HandledCopyMemory(void * dest, const void *src, size_t count)
{
	void *pRet=NULL;

	// memcpy() �� C ���̺귯�� �Լ��� C++ ���� ����ϴ� try-catch �δ� ���� ���� ���ٰ� �Ѵ�.. oTL
	// �� OS �� ���� �ڵ鸵�� �����ϸ� �� ��� __try-__except �ιۿ� ���� �� ����.
	//
	// NOTE: Debug �� ���� �� ������ �� ����� ���̺귯������ HEAP Corruption ������ �������� 
	//       Release ������ ������ ��(�� ����.. �Ƹ���. -_-)
	__try
	{
		pRet = memcpy(dest, src, count);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		pRet = NULL;
		NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:HandledCopyMemory error\n"));
	}
	return pRet;

}

//////////////////////////////////////////////////////////////////////////
//
// �־��� ����� �� �� �غ�
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::Init(HMODULE hModule)
{
	Close();

	NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:Init: called, hModule=%p\n"), hModule);

	BOOL bRes = FALSE;
	BOOL bNeedToResumeAllThread = FALSE;

	try
	{
		// ��� �̸� ���ϱ�
		TCHAR szModuleName[MAX_PATH] = {0,};
		::GetModuleFileName(hModule, szModuleName, MAX_PATH);
		TCHAR* pNameStart = _tcsrchr(szModuleName, _T('\\'));
		if(pNameStart) _tcscpy(m_szModuleName, pNameStart+1);
		else _tcscpy(m_szModuleName, szModuleName);

		// �� ����� ���� ���ϱ�
		bRes = ::GetModuleInformation( ::GetCurrentProcess(), hModule, &m_ModuleInfo, sizeof(MODULEINFO) );
		if( FALSE==bRes )
		{
			throw (GetLastError());
			//DWORD nErr = GetLastError();
			//goto ModuleInitErr;
		}

		SIZE_T curOffset = 0;

		// �޸𸮸� �Ҵ��Ͽ� ��� �̹��� ��ü�� ������ �д�
		m_pOrigImg = new BYTE[m_ModuleInfo.SizeOfImage];
		memset(m_pOrigImg, 0, m_ModuleInfo.SizeOfImage);

		// ��� ������ �����ߴ��� Ȯ��
		//if(CRawHookMgr::IsAllThreadSuspended() == FALSE)
		//{
		//	CRawHookMgr::SuspendAllThread();
		//	bNeedToResumeAllThread = TRUE;
		//}

		// ��� ����
		while(curOffset<m_ModuleInfo.SizeOfImage)	
		{

			// ������ ���� ����
			MEMORY_BASIC_INFORMATION mbi = {0,};
			SIZE_T retVal = ::VirtualQuery((LPBYTE)m_ModuleInfo.lpBaseOfDll + curOffset, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

			NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:Init: VirtualQuery: AllocBase=%p, AllocProt=%p, Base=%p, Prot=%p, RegSize=%p, State=%p, Type=%p\n"), 
				mbi.AllocationBase, mbi.AllocationProtect, mbi.BaseAddress, mbi.Protect, mbi.RegionSize, mbi.State, mbi.Type);
			
			//mbi.State : MEM_FREE(0x10000), MEM_RESERVE(0x2000), MEM_COMMIT(0x1000)
			//mbi.Type : MEM_IMAGE(0x1000000), MEM_MAPPED(0x40000), MEM_PRIVATE(0x20000)
			
			HMODULE hExecModule = CRawHookMgr::GetExecModuleHandle();
			// Protect �� ���ٰ����ϸ� ����
			if( mbi.Protect )
			{
				// ���� ������ ������
				if(mbi.AllocationBase != m_ModuleInfo.lpBaseOfDll) //if((mbi.State == MEM_FREE) || !mbi.Type)
					break;

				// �������� ��ȣ�Ӽ� ����
				DWORD dwOldProtect;
				bRes = ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);

				NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:Init: VirtualProtect: bRes=%s\n"), (bRes?_T("TRUE"):_T("FALSE")));

				if (bRes == FALSE)
				{
					DWORD dwErr = GetLastError();

					wchar_t buf[500] = {0,};
					wsprintf(buf,_T("%s : %p + %p (%p)\nVirtualQuery\nAllocBase=%p, AllocProtect=%p\n*BaseAddress=%p, Protect=%p\n*RegSize=%p, State=%p, Type=%p\nErrorNumber=%d, SizeOfImage=%p, curOffset=%p\n����ŷ������ �߻��Ͽ����ϴ�.\n���������࿡ ������ �ִٸ� �ش�â�� ĸ���Ͽ� �������ֽñ� �ٶ��ϴ�."), 
						m_szModuleName, m_ModuleInfo.lpBaseOfDll, curOffset, retVal,
						mbi.AllocationBase, mbi.AllocationProtect, mbi.BaseAddress, mbi.Protect, mbi.RegionSize, mbi.State, mbi.Type,
						dwErr, m_ModuleInfo.SizeOfImage, curOffset);
					
					MessageBox(NULL,buf,NULL,0);

					if (dwErr == ERROR_INVALID_ADDRESS)
					{
						// �� ����, m_ModuleInfo.SizeOfImage�� ����Ȯ�ϴٴ� ����
						// �Ƹ��� ������� Ȥ�� �������� ����� ���� �̷� ������ ����Ű�� ��.
						// ���� m_ModuleInfo.SizeOfImage �� ���� ���� �׳� ���⼭ ����������..
						m_ModuleInfo.SizeOfImage = curOffset;
						break;
					}
					/*else if(dwErr == ERROR_INVALID_PARAMETER)
					{
					}*/
					else
					{
						throw dwErr;
					}

				}

				NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:Init: HandledCopyMemory(%p, %p, %p)\n"), m_pOrigImg + curOffset, mbi.BaseAddress, mbi.RegionSize);

				// ����
				if (HandledCopyMemory(m_pOrigImg + curOffset, mbi.BaseAddress, mbi.RegionSize) == NULL)
				{
					// ���� ����
					// � ������� Ȥ�� �������� ����� ���� VirtualProtect ���� Pass �Ǵµ���
					// ���� memcpy() �� �غ��� AccessViolation ������ ���� ��찡 �ִ� ��.
					// ���� m_ModuleInfo.SizeOfImage�� ����Ȯ�ϴٴ� ����
					m_ModuleInfo.SizeOfImage = curOffset;
					break;
				}

				NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:Init: HandledCopyMemory OK\n"));

				// ����� ��ȣ�Ӽ� ����
				bRes = ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);

				// ����� �纻�� ��ȣ�Ӽ� ����
				bRes = ::VirtualProtect(m_pOrigImg + curOffset, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);

			}
			curOffset += mbi.RegionSize;
		}

		// ��� ������ ����
		//if(bNeedToResumeAllThread) CRawHookMgr::ResumeAllThread();

		m_hModule = hModule;
	}
	catch (DWORD dwErr)
	{
//		TRACE(_T("CRawHookedModule::Init() error\n"));
//		TRACE(_T("   GetLastError= %d\n"), dwErr);
		dwErr=dwErr;

		NOTIFY_DEBUG_MESSAGE(_T("CRawHookedModule:Init: Error %08X\n"), dwErr);

		if(m_pOrigImg)
		{
			delete [] m_pOrigImg;
			m_pOrigImg = NULL;
		}
		m_hModule = NULL;
		memset(m_szModuleName, 0, MAX_PATH);
		memset(&m_ModuleInfo, 0, sizeof(MODULEINFO));
		return FALSE;
	}

	return TRUE;
/*
ModuleInitErr:
	if(m_pOrigImg)
	{
		delete [] m_pOrigImg;
		m_pOrigImg = NULL;
	}
	m_hModule = NULL;
	memset(m_szModuleName, 0, MAX_PATH);
	memset(&m_ModuleInfo, 0, sizeof(MODULEINFO));
	return FALSE;
*/
}


//////////////////////////////////////////////////////////////////////////
//
// �� ����� ���� �����Ѵ�
//
//////////////////////////////////////////////////////////////////////////
void CRawHookedModule::Close()
{
	if( m_hModule && m_pOrigImg )
	{
		// �� �Ǿ� �ִ� �Լ����� ������Ų��
		CFunctionList::iterator iter = m_listFuncs.begin();
		for( ; iter!=m_listFuncs.end(); iter++ )
		{
			HOOKED_FUNC_INFO* pHook = (*iter);
			Unhook( pHook );
			delete pHook;
		}
		
		// ���� �̹��� ����
		delete [] m_pOrigImg;

	}

	m_pOrigImg = NULL;
	m_hModule = NULL;
	// ��� ����Ʈ Ŭ����
	m_listFuncs.clear();
}

//#ifndef USE_HOOKINGMODE_2
//////////////////////////////////////////////////////////////////////////
//
// �� ����� Ư�� �Լ� �ּҸ� �ٲ���´� (��ŷ���1)
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::Hook(LPVOID pFuncAddr, LPVOID pRedirAddr, BOOL bWriteCallCode)
{
	BOOL bRetVal = FALSE;
	MEMORY_BASIC_INFORMATION mbi, mbi2;
	ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
	ZeroMemory(&mbi2, sizeof(MEMORY_BASIC_INFORMATION));
	
	// ��� ������ �����ߴ��� Ȯ��
	BOOL bNeedToResumeAllThread = FALSE;
	if(CRawHookMgr::IsAllThreadSuspended() == FALSE)
	{
		CRawHookMgr::SuspendAllThread();
		bNeedToResumeAllThread = TRUE;
	}
	
	try
	{
		// �Լ��� �� ��ⲫ�� �˻�
		if( (UINT_PTR)pFuncAddr < (UINT_PTR)m_ModuleInfo.lpBaseOfDll 
			|| (UINT_PTR)pFuncAddr > (UINT_PTR)m_ModuleInfo.lpBaseOfDll+(UINT_PTR)m_ModuleInfo.SizeOfImage) throw -1;

		// �Լ��� �����ϴ� �޸� ������ ���� ����
		::VirtualQuery(pFuncAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		// �����ؼ��� READWRITE��
		if (FALSE == ::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			PAGE_EXECUTE_READWRITE,
			&mbi.Protect)
			) throw -2;


		size_t nDistFromBase = (size_t)pFuncAddr - (size_t)m_hModule;
		LPBYTE pOrigCode = m_pOrigImg + nDistFromBase;
		size_t copysize = 5;

		// ù �ڵ带 ���� �Ǵ� ȣ��� �ٲ��ش�
		BYTE opcode;
		if( bWriteCallCode )
		{
			opcode = 0xE8;		// 32bit relative call
		}
		else
		{
			opcode = 0xE9;		// 32bit relative jmp
		}

		_asm
		{
			mov ebx, pFuncAddr;

			mov al, opcode;			// opcode
			mov byte ptr [ebx], al;

			mov eax, pRedirAddr;	// �� �Լ��� �ּҿ���
			sub eax, ebx;			// ���� �Լ� �ּҸ� ����
			sub eax, 5;				// 5�� ���� �Ÿ��� ����

			mov dword ptr [ebx+1], eax;
		}

		// �� ��ü�� �����Ͽ� ����Ʈ�� �߰�
		HOOKED_FUNC_INFO* pFuncObj	= new HOOKED_FUNC_INFO;
		pFuncObj->m_pFunction		= pFuncAddr;
		pFuncObj->m_pRedirection	= pRedirAddr;
		pFuncObj->m_nDistFromBase	= nDistFromBase;
		pFuncObj->m_nModifiedSize	= copysize;
		pFuncObj->m_pOrigCode		= pOrigCode;
		pFuncObj->m_bHookSet		= FALSE;
		m_listFuncs.push_back( pFuncObj );

		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode=nErrCode;
	}

	DWORD dwOldProtect;

	// �����ؼ��� ����
    if(mbi.BaseAddress)
		::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			mbi.Protect,
			&dwOldProtect
			);

	if(mbi2.BaseAddress)
		::VirtualProtect(
			mbi2.BaseAddress,
			mbi2.RegionSize,
			mbi2.Protect,
			&dwOldProtect
			);

	// ��� ������ ����
	if(bNeedToResumeAllThread) 
		CRawHookMgr::ResumeAllThread();


	return bRetVal;
}


/*
//////////////////////////////////////////////////////////////////////////
//
// ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::Unhook(HOOKED_FUNC_INFO* pHookInfo)
{	
	if(NULL==pHookInfo) return FALSE;

	BOOL bRetVal = FALSE;
	MEMORY_BASIC_INFORMATION mbi, mbi2;
	ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
	ZeroMemory(&mbi2, sizeof(MEMORY_BASIC_INFORMATION));


	// ��� ������ �����ߴ��� Ȯ��
	BOOL bNeedToResumeAllThread = FALSE;
	if(CRawHookMgr::IsAllThreadSuspended() == FALSE)
	{
		CRawHookMgr::SuspendAllThread();
		bNeedToResumeAllThread = TRUE;
	}

	try
	{
		MEMORY_BASIC_INFORMATION mbi;
		::VirtualQuery(pHookInfo->m_pFunction, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		// �����ؼ��� READWRITE��
		DWORD dwOldProtect;
		if (FALSE == ::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			PAGE_EXECUTE_READWRITE,
			&dwOldProtect)
			) throw -2;


		memcpy(pHookInfo->m_pFunction, m_pOrigImg+pHookInfo->m_nDistFromBase, 5);
		
		// �� ����Ʈ���� ����
		CFunctionList::iterator iter = m_listFuncs.begin();
		for( ; iter!=m_listFuncs.end(); iter++ )
		{
			if((*iter) == pHookInfo)
			{
				delete pHookInfo;
				m_listFuncs.erase(iter);
				bRetVal = TRUE;
				break;
			}
		}

	}
	catch (int nErrCode)
	{
		nErrCode=nErrCode;
	}

	DWORD dwOldProtect;

	// �����ؼ��� ����
	if(mbi.BaseAddress)
		::VirtualProtect(
		mbi.BaseAddress,
		mbi.RegionSize,
		mbi.Protect,
		&dwOldProtect
		);

	if(mbi2.BaseAddress)
		::VirtualProtect(
		mbi2.BaseAddress,
		mbi2.RegionSize,
		mbi2.Protect,
		&dwOldProtect
		);

	// ��� ������ ����
	if(bNeedToResumeAllThread) CRawHookMgr::ResumeAllThread();

	return bRetVal;
}*/
//#else // USE_HOOKINGMODE_2
//////////////////////////////////////////////////////////////////////////
//
// �� ����� Ư�� �Լ� �ּҸ� �ٲ���´� (��ŷ���2)
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::Hook2(LPVOID pFuncAddr, LPVOID pRedirAddr, BOOL bWriteCallCode)
{
	BOOL bRetVal = FALSE;
	MEMORY_BASIC_INFORMATION mbi, mbi2;
	ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
	ZeroMemory(&mbi2, sizeof(MEMORY_BASIC_INFORMATION));
	
	// ��� ������ �����ߴ��� Ȯ��
	BOOL bNeedToResumeAllThread = FALSE;
	if(CRawHookMgr::IsAllThreadSuspended() == FALSE)
	{
		CRawHookMgr::SuspendAllThread();
		bNeedToResumeAllThread = TRUE;
	}
	
	try
	{
		// �Լ��� �� ��ⲫ�� �˻�
		if( (UINT_PTR)pFuncAddr < (UINT_PTR)m_ModuleInfo.lpBaseOfDll 
			|| (UINT_PTR)pFuncAddr > (UINT_PTR)m_ModuleInfo.lpBaseOfDll+(UINT_PTR)m_ModuleInfo.SizeOfImage) throw -1;

		// �Լ��� �����ϴ� �޸� ������ ���� ����
		::VirtualQuery(pFuncAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		// �����ؼ��� READWRITE��
		if (FALSE == ::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			PAGE_EXECUTE_READWRITE,
			&mbi.Protect)
			) throw -2;


		// �����ڵ� ���� (2008.12.31 by sc.Choi)
		// �� �ڵ���� ã��
		LPBYTE pOrigCode = NULL;
		size_t copysize = 0;

		if(NULL==ptrEmptyCodeArea) EmptyCodeArea();

		if(ptrEmptyCodeArea)
		{
			::VirtualQuery(EmptyCodeArea, &mbi2, sizeof(MEMORY_BASIC_INFORMATION));
			if (FALSE == ::VirtualProtect(
				mbi2.BaseAddress,
				mbi2.RegionSize,
				PAGE_EXECUTE_READWRITE,
				&mbi2.Protect)
				) throw -3;

			for(size_t nOffset=0; nOffset<4096; nOffset+=32)
			{
				LPBYTE pTmpAddr = ((LPBYTE)ptrEmptyCodeArea + nOffset);
				if(0x90 == pTmpAddr[0])
				{
					pOrigCode = pTmpAddr;
					break;
				}
			}

		}
		
		if(pOrigCode)
		{
			while(copysize < 5)	// ��ŷ�ڵ庸�� ������ ��� �����ڵ带 �����Ѵ�
			{
				// �����ɾ��� ����
				LPBYTE srcLine = (LPBYTE)pFuncAddr + copysize;
				LPBYTE tarLine = (LPBYTE)pOrigCode + copysize;

				// ��ɾ� ũ�� ����
				size_t linesize = g_disasm.GetLineSize( srcLine );
				if(0==linesize)
				{
					pOrigCode = NULL;
					copysize = 0;
					break;
				}

				// ��ɾ� ����
				memcpy( tarLine, srcLine, linesize );

				// ��ɾ ���� �Ǵ� ȣ���̶��
				if( 0xE8 == srcLine[0] || 0xE9 == srcLine[0] )
				{
					// �ּ� ����
					INT_PTR jmp_dist = *(INT_PTR*)(srcLine+1);
					jmp_dist += (INT_PTR)srcLine-(INT_PTR)tarLine;
					*(INT_PTR*)(tarLine+1) = jmp_dist;
				}

				// ��ɾ� �κ��� nop��
				// memset( srcLine, 0x90, linesize );

				copysize += linesize;
			}

			if(pOrigCode)
			{
				_asm
				{
					mov ebx, pOrigCode;
					add ebx, copysize;

					mov al, 0xE9;			// opcode
					mov byte ptr [ebx], al;

					mov eax, pFuncAddr;		// ���� �Լ��� �ּҿ���
					sub eax, pOrigCode;		// �� �Լ� �ּҸ� ����
					sub eax, 5;				// 5�� ���� �Ÿ��� ����

					mov dword ptr [ebx+1], eax;
				}
			}
		}

		// ù �ڵ带 ���� �Ǵ� ȣ��� �ٲ��ش�
		BYTE opcode;
		if( bWriteCallCode )
		{
			opcode = 0xE8;		// 32bit relative call
		}
		else
		{
			opcode = 0xE9;		// 32bit relative jmp
		}

		_asm
		{
			mov ebx, pFuncAddr;

			mov al, opcode;			// opcode
			mov byte ptr [ebx], al;

			mov eax, pRedirAddr;	// �� �Լ��� �ּҿ���
			sub eax, ebx;			// ���� �Լ� �ּҸ� ����
			sub eax, 5;				// 5�� ���� �Ÿ��� ����

			mov dword ptr [ebx+1], eax;
		}

		// �� ��ü�� �����Ͽ� ����Ʈ�� �߰�
		HOOKED_FUNC_INFO* pFuncObj	= new HOOKED_FUNC_INFO;
		pFuncObj->m_pFunction		= pFuncAddr;
		pFuncObj->m_pRedirection	= pRedirAddr;
		pFuncObj->m_nDistFromBase	= (size_t)pFuncAddr - (size_t)m_hModule;
		pFuncObj->m_nModifiedSize	= copysize;
		pFuncObj->m_pOrigCode		= pOrigCode;
		pFuncObj->m_bHookSet		= TRUE;
		m_listFuncs.push_back( pFuncObj );

		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode=nErrCode;
	}

	DWORD dwOldProtect;

	// �����ؼ��� ����
    if(mbi.BaseAddress)
		::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			mbi.Protect,
			&dwOldProtect
			);

	if(mbi2.BaseAddress)
		::VirtualProtect(
			mbi2.BaseAddress,
			mbi2.RegionSize,
			mbi2.Protect,
			&dwOldProtect
			);

	// ��� ������ ����
	if(bNeedToResumeAllThread) 
		CRawHookMgr::ResumeAllThread();


	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// ���� (��ŷ���2)
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::Unhook(HOOKED_FUNC_INFO* pHookInfo)
{	
	if(NULL==pHookInfo) return FALSE;

	BOOL bRetVal = FALSE;
	MEMORY_BASIC_INFORMATION mbi, mbi2;
	ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
	ZeroMemory(&mbi2, sizeof(MEMORY_BASIC_INFORMATION));


	// ��� ������ �����ߴ��� Ȯ��
	BOOL bNeedToResumeAllThread = FALSE;
	if(CRawHookMgr::IsAllThreadSuspended() == FALSE)
	{
		CRawHookMgr::SuspendAllThread();
		bNeedToResumeAllThread = TRUE;
	}

	try
	{
		MEMORY_BASIC_INFORMATION mbi;
		::VirtualQuery(pHookInfo->m_pFunction, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		// �����ؼ��� READWRITE��
		DWORD dwOldProtect;
		if (FALSE == ::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			PAGE_EXECUTE_READWRITE,
			&dwOldProtect)
			) throw -2;


		// ���� �ּҿ� ����� �ڵ带 ������ �ִ´�. (2008.12.31 by sc.Choi)
		if(pHookInfo->m_pOrigCode && pHookInfo->m_nModifiedSize>0 && pHookInfo->m_bHookSet==FALSE)
		{
			::VirtualQuery(EmptyCodeArea, &mbi2, sizeof(MEMORY_BASIC_INFORMATION));
			if (FALSE == ::VirtualProtect(
				mbi2.BaseAddress,
				mbi2.RegionSize,
				PAGE_EXECUTE_READWRITE,
				&mbi2.Protect)
				) throw -3;

			memcpy(pHookInfo->m_pFunction, pHookInfo->m_pOrigCode, pHookInfo->m_nModifiedSize);
			memset(pHookInfo->m_pOrigCode, 0x90, 32);
		}
		else
		{
			memcpy(pHookInfo->m_pFunction, m_pOrigImg+pHookInfo->m_nDistFromBase, 5);
		}
		
		// �� ����Ʈ���� ����
		CFunctionList::iterator iter = m_listFuncs.begin();
		for( ; iter!=m_listFuncs.end(); iter++ )
		{
			if((*iter) == pHookInfo)
			{
				delete pHookInfo;
				m_listFuncs.erase(iter);
				bRetVal = TRUE;
				break;
			}
		}

	}
	catch (int nErrCode)
	{
		nErrCode=nErrCode;
	}

	DWORD dwOldProtect;

	// �����ؼ��� ����
	if(mbi.BaseAddress)
		::VirtualProtect(
		mbi.BaseAddress,
		mbi.RegionSize,
		mbi.Protect,
		&dwOldProtect
		);

	if(mbi2.BaseAddress)
		::VirtualProtect(
		mbi2.BaseAddress,
		mbi2.RegionSize,
		mbi2.Protect,
		&dwOldProtect
		);

	// ��� ������ ����
	if(bNeedToResumeAllThread) CRawHookMgr::ResumeAllThread();

	return bRetVal;
}
//#endif

//////////////////////////////////////////////////////////////////////////
//
// ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::Unhook(void* pFunc)
{
	if(NULL==pFunc) return FALSE;

	CFunctionList::iterator iter = m_listFuncs.begin();
	for( ; iter!=m_listFuncs.end(); iter++ )
	{
		HOOKED_FUNC_INFO* pHook = (*iter);
		if( pHook->m_pFunction == pFunc || pHook->m_pRedirection == pFunc )
		{
			return Unhook(pHook);
		}
	}

	return FALSE;

}



LPVOID	CRawHookedModule::GetOrigFunc(LPVOID pFunc)
{
	CFunctionList::iterator iter = m_listFuncs.begin();
	for( ; iter!=m_listFuncs.end(); iter++ )
	{
		HOOKED_FUNC_INFO* pHook = (*iter);
		if( pHook->m_pFunction == pFunc || pHook->m_pRedirection == pFunc )
		{
			//return (LPVOID)(m_pOrigImg + pHook->m_nDistFromBase);
			return (LPVOID)pHook->m_pOrigCode;
		}
	}

	return NULL;
}


HMODULE	CRawHookedModule::GetModule()
{
	return m_hModule;
}

BOOL CRawHookedModule::IsInHookedModule(LPVOID pCode)
{
	if(NULL==m_pOrigImg) return FALSE;

	INT_PTR pFind = (INT_PTR)pCode;
	INT_PTR pBegin = (INT_PTR)m_pOrigImg;
	INT_PTR pEnd = pBegin + (INT_PTR)m_ModuleInfo.SizeOfImage;
	
	if( pBegin <= pFind && pFind <= pEnd ) return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
// �� ����� Ư�� ���̳ʸ� ���� �ٲ���´� (���̳ʸ���ġ���)
//
//////////////////////////////////////////////////////////////////////////
BOOL CRawHookedModule::preHook(LPVOID pFuncAddr, unsigned char* cBuff, size_t copysize)
{
	BOOL bRetVal = FALSE;
	MEMORY_BASIC_INFORMATION mbi;
	ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));

	// ��� ������ �����ߴ��� Ȯ��
	BOOL bNeedToResumeAllThread = FALSE;
	if(CRawHookMgr::IsAllThreadSuspended() == FALSE)
	{
		CRawHookMgr::SuspendAllThread();
		bNeedToResumeAllThread = TRUE;
	}
	
	try
	{
		// �Լ��� �� ��ⲫ�� �˻�
		if( (UINT_PTR)pFuncAddr < (UINT_PTR)m_ModuleInfo.lpBaseOfDll 
			|| (UINT_PTR)pFuncAddr > (UINT_PTR)m_ModuleInfo.lpBaseOfDll+(UINT_PTR)m_ModuleInfo.SizeOfImage) throw -1;

		// �Լ��� �����ϴ� �޸� ������ ���� ����
		::VirtualQuery(pFuncAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		// �����ؼ��� READWRITE��
		if (FALSE == ::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			PAGE_EXECUTE_READWRITE,
			&mbi.Protect)
			) throw -2;

		// ù �ڵ带 ���� �Ǵ� ȣ��� �ٲ��ش�

		_asm
		{
			mov ebx, pFuncAddr;
			mov edx, cBuff;
			mov ecx, copysize;
rout:
			cmp ecx, 0;
			je end;
			mov al, byte ptr [edx];
			mov byte ptr [ebx], al;
			inc edx;
			inc ebx;
			dec ecx;
			jmp rout;
end:
		}
		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode=nErrCode;
	}

	DWORD dwOldProtect;

	// �����ؼ��� ����
    if(mbi.BaseAddress)
		::VirtualProtect(
			mbi.BaseAddress,
			mbi.RegionSize,
			mbi.Protect,
			&dwOldProtect
			);

	// ��� ������ ����
	if(bNeedToResumeAllThread) 
		CRawHookMgr::ResumeAllThread();


	return bRetVal;
}