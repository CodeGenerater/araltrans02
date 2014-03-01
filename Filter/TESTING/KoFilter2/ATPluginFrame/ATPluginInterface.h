// ATPluginInterface - �ƶ�Ʈ���� �÷����� �������̽� �Ծ�
//

#ifndef __ATPLUGININTERFACE_H__
#define __ATPLUGININTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// �÷����� ������ �����Ǿ� ���� ������ ���� �߻�
#if !defined(USE_ARALTRANS_0_2) && !defined(USE_ARALTRANS_0_3)
#if _MSC_VER > 1200 // 1200 = Visual C++ 6.0
#error �÷������� ���� �ƶ�Ʈ���� ������ ���ǵ��� �ʾҽ��ϴ�. StdAfx.h ���� USE_ARALTRANS_0_2 Ȥ�� USE_ARALTRANS_0_3 �� �ּ��� �ϳ��� ���� �� �ּ���.
#elif
#error Plugin type is not specified. Please define at least one of these types on StdAfx.h : ATPLUGIN_ALGORITHM, ATPLUGIN_TRANSLATOR, or ATPLUGIN_FILTER.
#endif // _MSC_VER > 1200
#endif // !defined(USE_ARALTRANS_0_2) && !defined(USE_ARALTRANS_0_3)

#if !defined(ATPLUGIN_ALGORITHM) && !defined(ATPLUGIN_TRANSLATOR) && !defined(ATPLUGIN_FILTER)
#if _MSC_VER > 1200 // 1200 = Visual C++ 6.0
#error �÷����� ������ ���ǵ��� �ʾҽ��ϴ�. StdAfx.h ���� ATPLUGIN_ALGORITHM, ATPLUGIN_TRANSLATOR, Ȥ�� ATPLUGIN_FILTER �� �ּ��� �ϳ��� ���� �� �ּ���.
#elif
#error Plugin type is not specified. Please define at least one of these types on StdAfx.h : ATPLUGIN_ALGORITHM, ATPLUGIN_TRANSLATOR, or ATPLUGIN_FILTER.
#endif // _MSC_VER > 1200
#endif // !defined(ATPLUGIN_ALGORITHM) && !defined(ATPLUGIN_TRANSLATOR) && !defined(ATPLUGIN_FILTER)


#if defined(USE_ARALTRANS_0_2) && !defined(USE_ARALTRANS_0_3)
// PLUGIN_INFO ����ü (OnPluginInfo �Լ� ���ڷ� ���)
// ��� �ƶ�Ʈ���� 0.2 ������ �ʿ� ������ �׷��� ������ ����â ���Ⱑ ���ϴ�.. ����
struct PLUGIN_INFO
{
	int			cch;				// PLUGIN_INFO ����ü ������
	int			nIconID;			// Icon Resource ID
	wchar_t		wszPluginType[16];	// �÷����� Ÿ��
	wchar_t		wszPluginName[64];	// �÷����� �̸�
	wchar_t		wszDownloadUrl[256];// �÷����� �ٿ�ε� URL
};
typedef void TRANSLATION_OBJECT;	// 0.2������ ���� ������� �ʴ´�.
#else
namespace NS_ARALTRANS_0_3
{
#include "DefStruct.h"
}
using NS_ARALTRANS_0_3::PLUGIN_INFO;
using NS_ARALTRANS_0_3::TRANSLATION_OBJECT;
#endif // defined(USE_ARALTRANS_0_2) && !defined(USE_ARALTRANS_0_3)


#ifndef USE_MFC
/////////////////////////////////////////////////////////////////////////////
// CWinApp
// �� MFC �����Ͻ� ����ϴ� ��¥ CWinApp
//
// ����: �翬������ ��¥ CWinApp ó�� ����ϸ� ū�ϳ��ϴ� -_-a

class CWinApp {
public:
	// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
	HINSTANCE m_hInstance;
	
public:
	CWinApp() : m_hInstance(NULL) {}
	
	virtual BOOL InitInstance() { return TRUE; }
};
#endif // USE_MFC

///////////////////////////////////////////////////////////////////////////
// �÷����� export �Լ� ����
#ifdef __cplusplus
extern "C"
{
#endif

// ����
BOOL __stdcall OnPluginInit(HWND hAralWnd, LPVOID pvPluginOption);
BOOL __stdcall OnPluginOption();
BOOL __stdcall OnPluginClose();
#ifdef USE_ARALTRANS_0_3	// �ƶ�Ʈ���� 0.3 ����
BOOL __stdcall GetPluginInfo(PLUGIN_INFO * pPluginInfo);
#endif	// USE_ARALTRANS_0_3

#ifdef USE_ARALTRANS_0_2	// �ƶ�Ʈ���� 0.2 ����
// ��ŷ �÷����� ����
#ifdef ATPLUGIN_ALGORITHM
BOOL __stdcall OnPluginStart();
BOOL __stdcall OnPluginStop();
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
BOOL __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize);
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_FILTER
BOOL __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize);
BOOL __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize);
#endif
#endif // USE_ARALTRANS_0_2

#ifdef USE_ARALTRANS_0_3
// ���� �� ���� �÷����� ����
#if defined(ATPLUGIN_FILTER) || defined(ATPLUGIN_TRANSLATOR)
BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj);
BOOL __stdcall OnObjectClose(TRANSLATION_OBJECT* pTransObj);
BOOL __stdcall OnObjectMove(TRANSLATION_OBJECT* pTransObj);
BOOL __stdcall OnObjectOption(TRANSLATION_OBJECT* pTransObj);

#endif

BOOL __stdcall MainTranslateProcedure(TRANSLATION_OBJECT* pTransObj);

#endif // USE_ARALTRANS_0_3

// export �� ���� ��Ŀ �ɼ�
// ����
#pragma comment(linker, "/EXPORT:OnPluginInit=_OnPluginInit@8")
#pragma comment(linker, "/EXPORT:OnPluginOption=_OnPluginOption@0")
#pragma comment(linker, "/EXPORT:OnPluginClose=_OnPluginClose@0")
#ifdef USE_ARALTRANS_0_3	// �ƶ�Ʈ���� 0.3 ����
#pragma comment(linker, "/EXPORT:GetPluginInfo=_GetPluginInfo@4")
#endif	// USE_ARALTRANS_0_3

#ifdef USE_ARALTRANS_0_2	// �ƶ�Ʈ���� 0.2 ����
// ��ŷ �÷����� ����
#ifdef ATPLUGIN_ALGORITHM
#pragma comment(linker, "/EXPORT:OnPluginStart=_OnPluginStart@0")
#pragma comment(linker, "/EXPORT:OnPluginStop=_OnPluginStop@0")
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_TRANSLATOR
#pragma comment(linker, "/EXPORT:Translate=_Translate@12")
#endif

// ���� �÷����� ����
#ifdef ATPLUGIN_FILTER
#pragma comment(linker, "/EXPORT:PreTranslate=_PreTranslate@12")
#pragma comment(linker, "/EXPORT:PostTranslate=_PostTranslate@12")
#endif
#endif // USE_ARALTRANS_0_2

#ifdef USE_ARALTRANS_0_3
// ���� �� ���� �÷����� ����
#if defined(ATPLUGIN_FILTER) || defined(ATPLUGIN_TRANSLATOR)
#pragma comment(linker, "/EXPORT:OnObjectInit=_OnObjectInit@4")
#pragma comment(linker, "/EXPORT:OnObjectClose=_OnObjectClose@4")
#pragma comment(linker, "/EXPORT:OnObjectMove=_OnObjectMove@4")
#pragma comment(linker, "/EXPORT:OnObjectOption=_OnObjectOption@4")
#endif
#endif // USE_ARALTRANS_0_3


#ifdef __cplusplus
}
#endif


#endif // __ATPLUGININTERFACE_H__

