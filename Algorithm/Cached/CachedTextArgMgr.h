#pragma once

#include <set>
#include <map>

using namespace std;


// ��Ʈ������ �������� �Լ� ��Ʈ��
typedef DWORD (__stdcall * PROC_GetGlyphOutline)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2*);
typedef BOOL (__stdcall * PROC_TextOut)(HDC, int, int, LPCVOID, int);
typedef BOOL (__stdcall * PROC_ExtTextOut)(HDC, int, int, UINT, CONST RECT*, LPCVOID, UINT, CONST INT*);
typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (__stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);

typedef struct _TEXT_FUNCTION_ENTRY
{

	PROC_GetGlyphOutline		pfnGetGlyphOutlineA;
	PROC_GetGlyphOutline		pfnGetGlyphOutlineW;
	PROC_TextOut				pfnTextOutA;
	PROC_TextOut				pfnTextOutW;
	PROC_ExtTextOut				pfnExtTextOutA;
	PROC_ExtTextOut				pfnExtTextOutW;
	PROC_WideCharToMultiByte	pfnOrigWideCharToMultiByte;
	PROC_MultiByteToWideChar	pfnOrigMultiByteToWideChar;

} TEXT_FUNCTION_ENTRY, *PTEXT_FUNCTION_ENTRY;

// ��ŷ ���� ����ü
typedef struct _RESERVED_HOOK_POINT
{
	UINT_PTR	pHookPoint;
	size_t		nArgDist;
	BOOL		bWideChar;
} RESERVED_HOOK_POINT, *PRESERVED_HOOK_POINT;

class CCachedText;
class CFunction;

typedef set<RESERVED_HOOK_POINT*> CReservedHooks;
typedef set<CCachedText*> CCachedTextSet;
typedef map<UINT_PTR,CFunction*> CFunctionMap;
typedef pair<UINT_PTR,size_t> CArgInfo;
typedef map<UINT_PTR,size_t> CArgInfoMap;

//////////////////////////////////////////////////////////////////////////
//
class CCachedTextArgMgr
{

private:
	static CCachedTextArgMgr* _Inst;

	BOOL					m_bApplocale;
	BOOL					m_bRunning;
	HMODULE					m_hContainer;
	HWND					m_hContainerWnd;

	HFONT					m_hFont;
	CCachedTextSet			m_setActivatedArgs;			// Ȱ��ȭ�� �ؽ�Ʈ ����
	CCachedTextSet			m_setInactivatedArgs;		// ��Ȱ��ȭ �Ǿ� �ִ� ���ڵ�
	map<size_t,int>			m_mapHitDist;				// ������ ESP�� ������ �Ÿ���
	map<long, HFONT>		m_mapFonts;
	CFunctionMap			m_mapFunc;
	CReservedHooks			m_setReservedHooks;			// ��ŷ ����
	size_t					m_distBest;					// �� ���ڴ� ������ ����
	pair<UINT_PTR,UINT_PTR> m_rangeExeModule;
	CArgInfoMap				m_mapArgInfoA;
	CArgInfoMap				m_mapArgInfoW;

	int		AddTextArg(LPVOID pText, BOOL bWideChar, BOOL bAutoTrans, UINT_PTR ptrFunc, size_t dist);	// ���ο� ���ڿ� �ĺ��� �߰��Ѵ�
	BOOL	TestCharacter(wchar_t wch, void* baseESP);					// ���ڿ� �ĺ��� ��ü�� �׽�Ʈ�Ѵ�. (���̻� ��ġ���� �ʴ� �ĺ��� �ٷ� ����)
	int		SearchStringA(INT_PTR ptrBegin, char ch1, char ch2);
	int		SearchStringW(INT_PTR ptrBegin, wchar_t wch);
	void	CheckFont(HDC hdc);
	void	ModifyHitMap(CCachedText* pCachedText, void* baseESP, int increment);
	wchar_t GetBestTranslatedCharacter();
	void	FindBestDistAndClearHitMap();
	UINT_PTR GetFuncAddrFromReturnAddr(UINT_PTR pAddr);
	BOOL	IsAutoTransPoint(size_t dist);
	void	HookAllReservedPoints();

	static void ModifyValueA(LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters);
	static void ModifyValueW(LPVOID pHookedPoint, PREGISTER_ENTRY pRegisters);

	DWORD InnerGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	DWORD InnerGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	BOOL InnerTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	BOOL InnerTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	BOOL InnerExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	BOOL InnerExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);



	//////////////////////////////////////////////////////////////////////////
	// Static Functions
	static DWORD __stdcall NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	static DWORD __stdcall NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	static BOOL __stdcall NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	static BOOL __stdcall NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	static BOOL __stdcall NewExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	static BOOL __stdcall NewExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

public:
	CONTAINER_FUNCTION_ENTRY	m_sContainerFunc;
	TEXT_FUNCTION_ENTRY			m_sTextFunc;

	CCachedTextArgMgr(void);
	~CCachedTextArgMgr(void);

	BOOL Init(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
	BOOL Start();
	BOOL Stop();
	BOOL Option();
	BOOL Close();

};
