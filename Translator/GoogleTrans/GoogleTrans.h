#pragma once

BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer);
BOOL __stdcall OnPluginOption();
BOOL __stdcall OnPluginClose();
BOOL __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize);


//�Ϲ� �Լ�
std::wstring GetQueryInfo(HINTERNET hRequest,DWORD dwInfoLevel);

std::wstring GetEscapeJSString(std::wstring strSource);
std::wstring GetUnescapeJSString(std::wstring strSource);

bool isHex(wchar_t ch);
wchar_t convertHexToWchar(wchar_t* psz,int nCount=4);

std::wstring getTranslatedText(std::wstring strSource);

std::wstring EncodeUnsafeCharactor(std::wstring strSource);
std::wstring DecodeUnsafeCharactor(std::wstring strSource);

HINSTANCE g_hThisModule;

//������ �ڵ�
HWND g_hSettingWnd=NULL;

//�ɼ� ��Ʈ��
LPSTR g_szOptionStringBuffer=NULL;


//���ͳ� �ڵ�
HINTERNET   g_hSession=NULL;
std::set<HINTERNET> g_hRequests;

volatile int g_nTransCount=0;
volatile bool g_bLock=false;//�Ƚᵵ �Ǵµ� ���� ������...

std::map<UINT,std::string> g_Cache;

std::wstring g_strUnsafeChars  =L"<>{}��������������������";

std::wstring g_strPreToken = L"<tok:";
std::wstring g_strPostToken = L">";
