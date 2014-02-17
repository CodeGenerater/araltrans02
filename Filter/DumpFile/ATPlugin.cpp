// ATPlugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ATPlugin.h"

#include "tstring.h"
#include "Util.h"

#include <vector>

using namespace std;

// ����â ������ �ڵ�
HWND g_hSettingWnd=NULL;

// �� �÷������� ��� �ڵ� = �ν��Ͻ� �ڵ�
HINSTANCE g_hThisModule;

// �ɼ� ���� - �ɼ��� �ٲ���� �� ���⿡ ���ָ� �ƶ�Ʈ���� �ٷΰ��⿡�� ����� �� �ִ�.
// ���� - �ɼǱ��̴� �ְ� 4096 ����Ʈ.
LPSTR g_szOptionStringBuffer=NULL;

SYSTEMTIME st;
wchar_t wszFileName[MAX_PATH]=L"";

HANDLE hFileHandle=NULL;

// DLLMain
// ��ü ���ҽ��� �ִ� ��� ��� �ڵ��� �˾ƿ��� ���� �ʿ�
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hThisModule = (HINSTANCE) hModule;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// �÷����� export �Լ�

// ����
BOOL  __stdcall OnPluginInit(HWND hSettingWnd, LPSTR cszOptionStringBuffer)
{
	g_hSettingWnd = hSettingWnd;
	g_szOptionStringBuffer=cszOptionStringBuffer;

	GetLocalTime(&st);

	//���� �����
	int nTmpLne=swprintf_s(wszFileName,MAX_PATH,L"%s\\ATData\\Dump",GetGameDirectory());
	MyCreateDirectory(wszFileName);
	swprintf_s(wszFileName+nTmpLne,MAX_PATH-nTmpLne,L"\\Dump_%02d%02d%02d_%02d%02d%02d.txt",st.wYear%100,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);

	//���� ����
	hFileHandle=CreateFileW(wszFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFileHandle==INVALID_HANDLE_VALUE)
	{
		Sleep(1000);
		hFileHandle=CreateFileW(wszFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFileHandle==INVALID_HANDLE_VALUE)
		{
			::MessageBoxW(NULL,L"���� ���� ����",L"?",NULL);
			return FALSE;
		}
	}

	DWORD Tmp=0;
	WriteFile(hFileHandle,"\xFF\xFE",2,&Tmp,NULL);

	return TRUE;
}

BOOL  __stdcall OnPluginOption()
{

	::MessageBoxW(NULL,L"�ԷµǴ� �����͸� \r\nATData\\Dump\\Dump_����_�ð�.txt\r\n �� �������ִ� ���� �����Դϴ�.\r\n\r\n-Hide_D-",L"Info",NULL);
	return TRUE;
}
BOOL  __stdcall OnPluginClose()
{
	// ����: ���⼭ �޸� Ȯ�� ���� �ϸ� ���� �߻�.
	// TODO: �÷����� ���� �غ�, �ڿ� ��ȯ ���� �Ѵ�.

	if(hFileHandle)
	{
		CloseHandle(hFileHandle);
	}

	return TRUE;
}

BOOL  __stdcall PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{

	lstrcpyA(szOutJapanese, cszInJapanese);
	
	vector<wchar_t> wstrTmp;
	wstrTmp.reserve(nBufSize*4+10);
	wstrTmp.resize(nBufSize);

	int nLen=MyMultiByteToWideChar(932,NULL,szOutJapanese,-1,&wstrTmp[0],nBufSize);

	wstrTmp.resize(nLen-1);
	wstrTmp.push_back(L'\r');
	wstrTmp.push_back(L'\n');

	LPSTR pszSrc=szOutJapanese;

	while(*pszSrc!='\0')
	{
		if(IsDBCSLeadByteEx(932,(BYTE)(*pszSrc)))
		{
			BYTE up = (BYTE)(*pszSrc)/16;
			BYTE down =(BYTE)(*pszSrc)%16;
			wstrTmp.push_back(up>=0xA?L'A'+up-0xA:L'0'+up);
			wstrTmp.push_back(down>=0xA?L'A'+down-0xA:L'0'+down);
			pszSrc++;
			if(*pszSrc=='\0')break;
		}
		BYTE up = (BYTE)(*pszSrc)/16;
		BYTE down =(BYTE)(*pszSrc)%16;
		wstrTmp.push_back(up>=0xA?L'A'+up-0xA:L'0'+up);
		wstrTmp.push_back(down>=0xA?L'A'+down-0xA:L'0'+down);
		wstrTmp.push_back(L' ');
		pszSrc++;
	}
	wstrTmp.push_back(L'\r');
	wstrTmp.push_back(L'\n');

	DWORD Tmp=0;

	WriteFile(hFileHandle,&wstrTmp[0],wstrTmp.size()*sizeof(wchar_t),&Tmp,NULL);
	FlushFileBuffers(hFileHandle);

	return TRUE;
}

BOOL  __stdcall PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	lstrcpyA(szOutKorean, cszInKorean);

	vector<wchar_t> wstrTmp;
	wstrTmp.reserve(nBufSize*4+10);
	wstrTmp.resize(nBufSize);

	int nLen=MyMultiByteToWideChar(949,NULL,szOutKorean,-1,&wstrTmp[0],nBufSize);

	wstrTmp.resize(nLen-1);
	wstrTmp.push_back(L'\r');
	wstrTmp.push_back(L'\n');

	LPSTR pszSrc=szOutKorean;

	while(*pszSrc!='\0')
	{
		if(IsDBCSLeadByteEx(949,(BYTE)(*pszSrc)))
		{
			BYTE up = (BYTE)(*pszSrc)/16;
			BYTE down =(BYTE)(*pszSrc)%16;
			wstrTmp.push_back(up>=0xA?L'A'+up-0xA:L'0'+up);
			wstrTmp.push_back(down>=0xA?L'A'+down-0xA:L'0'+down);
			pszSrc++;
			if(*pszSrc=='\0')break;
		}
		BYTE up = (BYTE)(*pszSrc)/16;
		BYTE down =(BYTE)(*pszSrc)%16;
		wstrTmp.push_back(up>=0xA?L'A'+up-0xA:L'0'+up);
		wstrTmp.push_back(down>=0xA?L'A'+down-0xA:L'0'+down);
		wstrTmp.push_back(L' ');
		pszSrc++;
	}
	wstrTmp.push_back(L'\r');
	wstrTmp.push_back(L'\n');

	DWORD Tmp=0;

	WriteFile(hFileHandle,&wstrTmp[0],wstrTmp.size()*sizeof(wchar_t),&Tmp,NULL);
	FlushFileBuffers(hFileHandle);

	return TRUE;
}
