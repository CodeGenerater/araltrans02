// AralGoodHook.h : main header file for the ARALGOODHOOK DLL
//

#if !defined(AFX_ARALGOODHOOK_H__36D69F35_0369_4248_894B_291A8D21B40F__INCLUDED_)
#define AFX_ARALGOODHOOK_H__36D69F35_0369_4248_894B_291A8D21B40F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "DefStruct.h"
#include "MultiPlugin.h"
#include <map>

using namespace std;

typedef struct _HOOKING_OPTION
{
	char szAlgorithmName[256];
	char szAlgorithmOption[4096];
	char szTranslatorName[256];
	char szTranslatorOption[4096];
	char szFilters[4096];
} HOOKING_OPTION, *LPHOOKING_OPTION;


void PreModifyValue();
void ModifyValue();


/////////////////////////////////////////////////////////////////////////////
// CAralGoodHookApp
// See AralGoodHook.cpp for the implementation of this class
//
class COutputDialog;

class CAralGoodHookApp : public CWinApp
{
private:
	CRITICAL_SECTION	m_csTextArgMgr;
	CString				m_strProcessName;
	CString				m_strHookingPluginName;
	CString				m_strTransPluginName;
	HOOKING_OPTION		m_hookOption;



	BOOL LoadHookingPlugin(CString strPluginName);	// ��ŷ �÷����� �ε�
	void RefreshHookingOption();
	void UnloadHookingPlugin();						// ��ŷ �÷����� ��ε�

	BOOL LoadTransPlugin(CString strPluginName);	// ���� �÷����� �ε�
	void RefreshTransOption();
	void UnloadTransPlugin();						// ���� �÷����� ��ε�

	BOOL RemoveModuleProtectionAll();
	void SetDebugPrivilege();

public:
	COutputDialog*		m_pOutput;
	static CAralGoodHookApp* _gInst;
	map<void*,void*>		m_mapCallbackTable;		// ��ŷ �ݹ��Լ� ���̺�

	// �ε��� ��ŷ �÷����� DLL
	HMODULE					m_hHookingDll;
	HOOKING_FUNCTION_ENTRY	m_sHookingFunc;

	// �ε��� ���� �÷����� DLL
	HMODULE					m_hTransDll;
	TRANS_FUNCTION_ENTRY	m_sTransFunc;

	// �ƶ� Ʈ���� Ȩ���丮
	CString				m_strAralTransDir;

	// ���� ������
	CMultiPlugin		m_cMultiPlugin;

	CAralGoodHookApp();
	
	void OnAlgorithmChanged();
	void OnAlgorithmOption();
	void OnAlgorithmOnOff();
	void OnTranslatorChanged();
	void OnTranslatorOption();

	BOOL ATStartUp();
	void ATCleanUp();
	BOOL Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAralGoodHookApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance(void);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAralGoodHookApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_ARALGOODHOOK_H__36D69F35_0369_4248_894B_291A8D21B40F__INCLUDED_)
