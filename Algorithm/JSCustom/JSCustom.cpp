// JSCustom.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "JSCustom.h"
#include "v8Wrapper.h"
#include "ATJSAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: �� DLL�� MFC DLL�� ���� �������� ��ũ�Ǿ� �ִ� ���
//		MFC�� ȣ��Ǵ� �� DLL���� ���������� ��� �Լ���
//		���� �κп� AFX_MANAGE_STATE ��ũ�ΰ�
//		��� �־�� �մϴ�.
//
//		��:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �Ϲ����� �Լ� ������ ���⿡ �ɴϴ�.
//		}
//
//		�� ��ũ�δ� MFC�� ȣ���ϱ� ����
//		�� �Լ��� �ݵ�� ��� �־�� �մϴ�.
//		��, ��ũ�δ� �Լ��� ù ��° ���̾�� �ϸ� 
//		��ü ������ �����ڰ� MFC DLL��
//		ȣ���� �� �����Ƿ� ��ü ������ ����Ǳ� ����
//		���;� �մϴ�.
//
//		�ڼ��� ������
//		MFC Technical Note 33 �� 58�� �����Ͻʽÿ�.
//

// CJSCustomApp

BEGIN_MESSAGE_MAP(CJSCustomApp, CWinApp)
END_MESSAGE_MAP()


// CJSCustomApp ����
// ������ CJSCustomApp ��ü�Դϴ�.

CJSCustomApp theApp;

extern "C" BOOL PASCAL EXPORT OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.OnPluginInit(hSettingWnd,szOptionStringBuffer);
}

extern "C" BOOL PASCAL EXPORT OnPluginOption()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.OnPluginOption();
}

extern "C" BOOL PASCAL EXPORT OnPluginClose()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.OnPluginClose();
}

extern "C" BOOL PASCAL EXPORT OnPluginStart()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.OnPluginStart();
}

extern "C" BOOL PASCAL EXPORT OnPluginStop()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.OnPluginStop();
}


CJSCustomApp::CJSCustomApp()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}


// CJSCustomApp �ʱ�ȭ

BOOL CJSCustomApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CJSCustomApp::OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer){
	return TRUE;
}

BOOL CJSCustomApp::OnPluginOption(){
	return TRUE;
}

BOOL CJSCustomApp::OnPluginStart(){
	//�� ���� ����
	return TRUE;
}

BOOL CJSCustomApp::OnPluginStop(){
	//�� ���� ����
	return TRUE;
}

BOOL CJSCustomApp::OnPluginClose(){
	//v8Wrapper�� �ڵ� �Ҹ��ڰ� �˾Ƽ� �Ѵ�[...]
	//��� : �� ���� ����
	return TRUE;
}
