#include "StdAfx.h"

#include "ATPluginContainer.h"

void CATPluginContainer::Unload()
{

	if (m_pszOption)
	{
		delete[] m_pszOption;
		m_pszOption=NULL;
	}

	if (m_hModule)
	{
		FreeLibrary(m_hModule);
		m_hModule=NULL;
	}
}

BOOL CATPluginContainer::Load(LPCTSTR pszModulePath)
{

	if (m_hModule)
		Unload();

	try 
	{
		// �÷����� �ε�
		m_hModule=LoadLibrary(pszModulePath);

		if(!m_hModule) throw(_T("LoadLibrary ����"));

		// �Լ� �ּ� �޾ƿ�

		// �⺻ �Լ�
		m_pfnOnPluginInit=(PROC_OnPluginInit)GetProcAddress(m_hModule, "OnPluginInit");
		m_pfnOnPluginOption=(PROC_OnPluginDefault)GetProcAddress(m_hModule, "OnPluginOption");
		m_pfnOnPluginClose=(PROC_OnPluginDefault)GetProcAddress(m_hModule, "OnPluginClose");

		if (!m_pfnOnPluginInit || !m_pfnOnPluginOption || !m_pfnOnPluginClose) 
			throw(_T("�⺻�Լ� �ּҸ� ã�� �� �����ϴ�"));

/*
		// ���� (�ɼ�)
		m_pfnOnPluginVersion=(PROC_OnPluginVersion)GetProcAddress(m_hModule, "OnPluginVersion");

		// ��ŷ ����
		m_pfnOnPluginStart=(PROC_OnPluginDefault)GetProcAddress(m_hModule, "OnPluginStart");
		m_pfnOnPluginStop=(PROC_OnPluginDefault)GetProcAddress(m_hModule, "OnPluginStop");

		// ���� ����
		m_pfnTranslate=(PROC_Translate)GetProcAddress(m_hModule, "Translate");
*/
		// ���� ����
		m_pfnPreTranslate = (PROC_Translate)GetProcAddress(m_hModule, "PreTranslate");
		m_pfnPostTranslate = (PROC_Translate)GetProcAddress(m_hModule, "PostTranslate");

/*
		if (!m_pfnOnPluginStart || !m_pfnOnPluginStop)
		{
			m_pfnOnPluginStart=m_pfnOnPluginStop=NULL;
		}
*/
		if (!m_pfnPreTranslate || !m_pfnPostTranslate)
		{
			m_pfnPreTranslate=m_pfnPostTranslate=NULL;
		}
		if (/*!m_pfnOnPluginStart && !m_pfnTranslate &&*/ !m_pfnPreTranslate || !m_pfnPostTranslate)
				throw(_T("�� DLL�� ���� �÷������� �ƴմϴ�."));
	}
	catch(TCHAR *errMsg)
	{
		MessageBox(NULL, errMsg, _T("����"), MB_OK | MB_ICONEXCLAMATION);
		Unload();
		return FALSE;
	}
	return TRUE;
}

BOOL CATPluginContainer::OnPluginInit(HWND hSettingWnd, LPCSTR cszOptionStringBuffer)
{
	if (!m_pfnOnPluginInit)
		return FALSE;

	// �� �÷����θ��� �ɼ��� �ʿ���
	m_pszOption=new char[4096];
	if (cszOptionStringBuffer)
		lstrcpyA(m_pszOption, cszOptionStringBuffer);
	else
		m_pszOption[0]='\0';

	return m_pfnOnPluginInit(hSettingWnd, m_pszOption);
}
