//
// ATCApi - �ƶ�Ʈ���� �����̳� ���� API �� ����ϱ� ���� ���
//

#ifndef __ATCAPI_H__
#define __ATCAPI_H__

#include "DefATContainer.h"

// CATCApi : �ƶ� Ʈ���� �����̳� ���� API
//           ���� 0.2.20080518
class CATCApi {
	protected:
		HMODULE m_hContainer;
		CONTAINER_FUNCTION_ENTRY m_cEntry;

	public:
		CATCApi();

		// ����� �ʱ�ȭ �Ǿ������� �˾ƿ´�. ���� ȣ��� Ȯ������ �����Ƿ� �̸� Ȯ���� ��.
		bool IsValid() { return (m_hContainer)?TRUE:FALSE; }

		// ���� �ε�� �˰����� ��� �ڵ��� �����´�.
		HMODULE GetCurAlgorithm() 
			{ return m_cEntry.pfnGetCurAlgorithm(); }
		// ���� �ε�� �������� ��� �ڵ��� �����´�.
		HMODULE GetCurTranslator() 
			{ return m_cEntry.pfnGetCurTranslator(); }
		// Ư�� DLL �Լ��� ��ŷ�Ѵ�.
		BOOL HookDllFunction(LPCSTR cszDllName, LPCSTR cszFuncName, LPVOID pfnNewFunc) 
			{ return m_cEntry.pfnHookDllFunction(cszDllName, cszFuncName, pfnNewFunc); }
		// Ư�� DLL �Լ��� ��ŷ�� �����Ѵ�.
		BOOL UnhookDllFunction(LPCSTR cszDllName, LPCSTR cszFuncName)
			{ return m_cEntry.pfnUnhookDllFunction(cszDllName, cszFuncName); }
		// ��ŷ�� DLL�Լ��� ���� �����͸� �����´�.
		LPVOID GetOrigDllFunction(LPCSTR cszDllName, LPCSTR cszFuncName)
			{ return m_cEntry.pfnGetOrigDllFunction(cszDllName, cszFuncName); }
		// �ڵ��� Ư�������� ��ŷ�Ѵ�.
		BOOL HookCodePoint(LPVOID pCodePoint, PROC_HookCallback pfnCallback)
			{ return m_cEntry.pfnHookCodePoint(pCodePoint, pfnCallback); }
		// Ư�������� ��ŷ�� �����Ѵ�.
		BOOL UnhookCodePoint(LPVOID pCodePoint) 
			{ return m_cEntry.pfnUnhookCodePoint(pCodePoint); }
		// ���ڿ��� �����Ѵ�.
		BOOL TranslateText(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize)
			{ return m_cEntry.pfnTranslateText(cszJapanese, szKorean, nBufSize); }
		// AppLocale �� �ε�Ǿ������� �˾ƿ´�.
		BOOL IsAppLocaleLoaded()
			{ return m_cEntry.pfnIsAppLocaleLoaded(); }
		// AppLocale �� �Ѱų� ����. (���� �Ⱦ���, ���� �۵� �������)
		BOOL EnableAppLocale(BOOL bEnable)
			{ return m_cEntry.pfnEnableAppLocale(bEnable); }


};

// ATCApi ��ü
extern CATCApi g_cATCApi;

#endif //__ATCAPI_H__