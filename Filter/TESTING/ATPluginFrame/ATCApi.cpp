// ATCApi.cpp : �ƶ� Ʈ���� �����̳� API Wrapper

#include "StdAfx.h"
#include <tchar.h>

#include "ATCApi.h"

CATCApi g_cATCApi;

CATCApi::CATCApi() : m_hContainer(NULL)
{
	ZeroMemory(&m_cEntry, sizeof(m_cEntry));

	try 
	{
		// ATCTNR.DLL �� ã�´�
		m_hContainer=GetModuleHandle(_T("ATCTNR.DLL"));
		if (!m_hContainer)	throw(_T("ATCTNR.DLL�� ã�� �� �����ϴ�."));

		// �� �Լ� �����͸� ���´�

		m_cEntry.pfnGetCurAlgorithm=(PROC_GetCurAlgorithm)GetProcAddress(m_hContainer, "GetCurAlgorithm");
		if (!m_cEntry.pfnGetCurAlgorithm) throw(_T("GetCurAlgorithm �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnGetCurTranslator=(PROC_GetCurTranslator)GetProcAddress(m_hContainer, "GetCurTranslator");
		if (!m_cEntry.pfnGetCurTranslator) throw(_T("GetCurTranslator �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnHookDllFunction=(PROC_HookDllFunction)GetProcAddress(m_hContainer, "HookDllFunction");
		if (!m_cEntry.pfnHookDllFunction) throw(_T("HookDllFunction �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnUnhookDllFunction=(PROC_UnhookDllFunction)GetProcAddress(m_hContainer, "UnhookDllFunction");
		if (!m_cEntry.pfnUnhookDllFunction) throw(_T("UnhookDllFunction �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnGetOrigDllFunction=(PROC_GetOrigDllFunction)GetProcAddress(m_hContainer, "GetOrigDllFunction");
		if (!m_cEntry.pfnGetOrigDllFunction) throw(_T("GetOrigDllFunction �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnHookCodePoint=(PROC_HookCodePoint)GetProcAddress(m_hContainer, "HookCodePoint");
		if (!m_cEntry.pfnHookCodePoint) throw(_T("HookCodePoint �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnUnhookCodePoint=(PROC_UnhookCodePoint)GetProcAddress(m_hContainer, "UnhookCodePoint");
		if (!m_cEntry.pfnUnhookCodePoint) throw(_T("UnhookCodePoint �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnTranslateText=(PROC_TranslateText)GetProcAddress(m_hContainer, "TranslateText");
		if (!m_cEntry.pfnTranslateText) throw(_T("TranslateText �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnIsAppLocaleLoaded=(PROC_IsAppLocaleLoaded)GetProcAddress(m_hContainer, "IsAppLocaleLoaded");
		if (!m_cEntry.pfnIsAppLocaleLoaded) throw(_T("IsAppLocaleLoaded �Լ��� ã�� �� �����ϴ�."));

		m_cEntry.pfnEnableAppLocale=(PROC_EnableAppLocale)GetProcAddress(m_hContainer, "EnableAppLocale");
		if (!m_cEntry.pfnEnableAppLocale) throw(_T("EnableAppLocale �Լ��� ã�� �� �����ϴ�."));
	}
	catch(TCHAR *szErrText)
	{
		OutputDebugString(szErrText);
		m_hContainer=NULL;
	}
}