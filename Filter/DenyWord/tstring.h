//
// TSTRING - UNICODE/MBCS �� string �� ���� ���
//

#ifndef __TSTRING_H__
#define __TSTRING_H__

#include <tchar.h>
#include <string>

using namespace std;

// tstring - string/wstring ���� string
#ifndef tstring

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#endif // tstring


wstring MakeWString(const char *pszString);
WCHAR MakeWChar(const char ch);

// MakeTString - MBCS string -> tstring ��ȯ �Լ�
// MakeTChar - MBCS char -> TCHAR ��ȯ �Լ�
#ifdef _UNICODE
#define MakeTString(str) MakeWString(str)
#define MakeTChar(ch) MakeWChar(ch)
#else
#define MakeTString(str) string(str)
#define MakeTChar(ch) (ch)
#endif

// MyWideCharToMultiByte - from ATCode.cpp for ATCode plugin
int MyWideCharToMultiByte(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCWSTR lpWideCharStr, 
						  int cchWideChar, 
						  LPSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPCSTR lpDefaultChar, 
						  LPBOOL lpUsedDefaultChar 
						  );

// MyMultiByteToWideChar - from ATCode.cpp for ATCode plugin
int MyMultiByteToWideChar(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPWSTR lpWideCharStr, 
						  int cchWideChar 
						  );

// Kor2Wide - �ѱ��� ��� �����ϰ� MultiByteToWideChar ����ϱ� ���� �ζ��� �Լ�
inline int Kor2Wide(LPCSTR lpKor, LPWSTR lpWide, int nWideSize=4096)
{
	return MyMultiByteToWideChar(949, 0, lpKor, -1, lpWide, nWideSize);
}

// Kor2Wide - �ѱ��� ��� �����ϰ� WideCharToMultiByte ����ϱ� ���� �ζ��� �Լ�
inline int Wide2Kor(LPCWSTR lpWide, LPSTR lpKor, int nKorSize=4096)
{
	return MyWideCharToMultiByte(949, NULL, lpWide, -1, lpKor, nKorSize, NULL, NULL);
}

#endif // __TSTRING_H__
