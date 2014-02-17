#include "StdAfx.h"
#include "ATPlugin.h"
#include "DumpTextDlg.h"

HANDLE	hDumpThread;
CRITICAL_SECTION g_cs;
volatile bool g_bContinue;
queue <wstring> TextQueue;

HWND hEdit;
BOOL CALLBACK DumpTextProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			hEdit=GetDlgItem(hwndDlg, IDC_EDIT1);
			SendMessage( hEdit, EM_LIMITTEXT, -1, -1 );
			
			//������ ����
			InitializeCriticalSection(&g_cs);
			//EnterCriticalSection(&g_cs);
			g_bContinue = true;
			//LeaveCriticalSection(&g_cs);
			hDumpThread = (HANDLE)_beginthreadex(NULL, 0, CallThreadHandlerProc, NULL, 0, NULL);		
			return 0;
		case WM_SIZE:
			MoveWindow(hEdit,0,0,LOWORD(lParam),HIWORD(lParam),TRUE);
			return 0;
		case WM_USER:
			AddString((LPWSTR)lParam);

			return 0;
		case WM_USER+1:
			if((BOOL)lParam!=0)
				SetWindowPos(hwndDlg,HWND_TOPMOST,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
			else
				SetWindowPos(hwndDlg,HWND_NOTOPMOST,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
			return 0;
		case WM_DESTROY:
			//Thread ����
			EnterCriticalSection(&g_cs);
			g_bContinue = false;
			LeaveCriticalSection(&g_cs);
			WaitForSingleObject(hDumpThread,INFINITE);
			DeleteCriticalSection(&g_cs);
			return 0;
		default:;
	}
	return FALSE;
}

void AddString(const wchar_t * pstr )
{
	wchar_t pszTmp[2048]=L"";

	// ���ڿ��� �߰�
	wsprintf( pszTmp, L"%s\r\n", pstr );
	EnterCriticalSection(&g_cs);
	TextQueue.push(pszTmp);
	LeaveCriticalSection(&g_cs);
}

unsigned _stdcall CallThreadHandlerProc(void  *pThreadHandler)
{
	wstring strTemp;
	wchar_t szTemp[2048]=L"";
	while (g_bContinue == true)
	{
		//������ �սô�.(Do Something -_-;;)
		EnterCriticalSection(&g_cs);
		if(!TextQueue.empty())
		{
			strTemp = TextQueue.front();
			TextQueue.pop();
			LeaveCriticalSection(&g_cs);
			wcscpy_s(szTemp,2048,strTemp.c_str());

			 // Ŀ���� �� ������ �ű�� ��ƾ
			SendMessage( hEdit, EM_SETSEL, 0, -1 );
			SendMessage( hEdit, EM_SETSEL, -1, -1 );

			// �� �߰�!
			SendMessage( hEdit, EM_REPLACESEL, 0,(LPARAM)szTemp  );


		}
		else
		{
			LeaveCriticalSection(&g_cs);
			Sleep(1);
		}

		
	}
	/*_endthreadex�Լ��� ȣ���Ͽ� �����尡 ���������� ���� �ǰ� �մϴ�.*/
	DWORD exitCode;
	GetExitCodeThread(hDumpThread, &exitCode);
	_endthreadex(exitCode);
	return 0;
}