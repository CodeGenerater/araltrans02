// DumpText.cpp : �ش� DLL�� �ʱ�ȭ ��ƾ�� �����մϴ�.
//

#include "stdafx.h"
#include "DumpText.h"
#include "DumpDlg.h"
#include "OpQueue.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// ������ CDumpTextApp ��ü�Դϴ�.

CDumpTextApp theApp;

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

// CDumpTextApp

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

extern "C" BOOL PASCAL EXPORT PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.PreTranslate(cszInJapanese,szOutJapanese,nBufSize);
}

extern "C" BOOL PASCAL EXPORT PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.PostTranslate(cszInKorean,szOutKorean,nBufSize);
}

BEGIN_MESSAGE_MAP(CDumpTextApp, CWinApp)
END_MESSAGE_MAP()


// CDumpTextApp ����

CDumpTextApp::CDumpTextApp()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
	m_pTmpTimer=NULL;
	m_pThread=NULL;
}


// CDumpTextApp �ʱ�ȭ

BOOL CDumpTextApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	return TRUE;
}


UINT threadProc(LPVOID data){
	CDumpDlg dlg;
	dlg.DoModal();
	return 0L;
}

VOID CALLBACK TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime){
	KillTimer(NULL,idEvent);
	theApp.m_pThread = AfxBeginThread(threadProc,NULL,0,0U,CREATE_SUSPENDED);
	if(theApp.m_pThread!=NULL){
		theApp.m_pThread->m_bAutoDelete=FALSE;
		theApp.m_pThread->ResumeThread();
	}
}


BOOL CDumpTextApp::OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer){
	m_hSettingWnd = hSettingWnd;
	m_szOptionStringBuffer = szOptionStringBuffer;

	//Thread Run!
	//������ �̰� ���������� ��ü�� ������ �������� ������ ���� ����!
	//�׷��� 1�ʰ� ���� �� ����ǵ��� ����!
	m_pTmpTimer = SetTimer(NULL,3815,1000,TimerProc);

	m_szOptionStringBuffer[0]='\0';
	m_szOptionStringBuffer[1]='\0';

	return TRUE;
}

BOOL CDumpTextApp::OnPluginOption(){
	if(m_hSettingWnd!=NULL && IsWindow(m_hSettingWnd)){
		//AfxMessageBox(L"���� ������ DumpText �����쿡�� �� �� �ֽ��ϴ�.",MB_OK|MB_ICONINFORMATION);
		wstring strRegPath;
		CRegKey cReg;

		std::wstring m_strRegDefault;
		std::wstring m_strRegMy;

		m_strRegDefault = L"SOFTWARE\\AralGood\\DumpText Option";
		{
			wchar_t szFileName[2048]=L"";
			int nLen = ::GetModuleFileNameW(NULL,szFileName,2048);
			while(nLen>=0&&szFileName[nLen]!=L'\\'){
				nLen--;
			}
			m_strRegMy = m_strRegDefault;
			m_strRegMy += (szFileName+nLen);
		}

		strRegPath = m_strRegMy;

		if(cReg.Create(HKEY_CURRENT_USER,strRegPath.c_str()) != ERROR_SUCCESS){
			AfxMessageBox(L"�ɰ��� ����!\r\n������Ʈ�� �ʱ�ȭ�� �����߽��ϴ�.\r\n�����ڿ��� ���� �ʿ��մϴ�",MB_ICONSTOP);
			return TRUE;
		}
		//DOPT_SIZE
		DWORD data=0;
		wchar_t szData[2048]=L"";
		data = swprintf_s(szData,2048,L"%d|%d|%d|%d",200,200,400,500);
		cReg.SetStringValue(L"DOPT_SIZE",szData);

		cReg.Close();
		OnPluginClose();
		AfxMessageBox(L"DumpText ������ ��ġ �� ũ�⸦ �ʱ�ȭ �߽��ϴ�.\n���͸� ���� �� �ٽ� �߰����ּ���.",MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;
}

BOOL CDumpTextApp::OnPluginClose(){
	KillTimer(NULL,m_pTmpTimer);

	OpData offOp;
	offOp.type = 0x0101;

	pushOpQueue(offOp,false);
	for(int i=0;i<4;i++){
		pushOpQueue(offOp); //���� �ñ׳���... ���� ���� -_-+++
	}

	if(m_pThread!=NULL){
		::WaitForSingleObject(m_pThread->m_hThread,INFINITE);
	}

	return TRUE;

}

BOOL CDumpTextApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize){
	size_t uLen = strlen(cszInJapanese);
	strcpy_s(szOutJapanese,nBufSize,cszInJapanese);

	uLen+=2;

	vector<wchar_t> buff(uLen);

	int nLen = MyMultiByteToWideChar(932,0,cszInJapanese,-1,buff.data(),uLen);
	nLen--;

	OpData textData;
	textData.type = 0x0200;
	textData.data.insert(textData.data.end(),buff.begin(),buff.begin()+nLen);

	pushOpQueue(textData);

	return TRUE;
}

BOOL CDumpTextApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize){
	size_t uLen = strlen(cszInKorean);
	strcpy_s(szOutKorean,nBufSize,cszInKorean);

	uLen+=2;

	vector<wchar_t> buff(uLen);

	int nLen = MyMultiByteToWideChar(949,0,cszInKorean,-1,buff.data(),uLen);
	nLen--;

	OpData textData;
	textData.type = 0x0210;
	textData.data.insert(textData.data.end(),buff.begin(),buff.begin()+nLen);

	pushOpQueue(textData);

	return TRUE;
}