#include "StdAfx.h"
#include "resource.h"

#include "AddPluginDialog.h"

#include "MultiPlugin.h"
#include "Util.h"

#include "tstring.h"

using namespace std;

static bool bIsRedundantAllowed=false;

BOOL AddPluginDialogUpdate(HWND hwndDlg)
{
	HWND hwndList=GetDlgItem(hwndDlg, IDC_ADDPLUGINLIST);
	HWND hwndRedundantCheck=GetDlgItem(hwndDlg, IDC_REDUNDANTCHECK);
	ATPLUGIN_ARGUMENT_ARRAY aPluginArgs;
	HANDLE hFindFile;
	WIN32_FIND_DATA findData;

	int i;
	bool bIsLoaded;

	TCHAR szPath[MAX_PATH];

	EnableWindow(hwndList, FALSE);

	SendMessage(hwndList, LB_RESETCONTENT, NULL, NULL);

	wsprintf(szPath, _T("%s\\Filter\\*.DLL"), GetATDirectory());

	g_cMultiPlugin.GetPluginArgs(aPluginArgs);

	// �÷����� ���� �˻�
	hFindFile=FindFirstFile(szPath, &findData);

	if (hFindFile == INVALID_HANDLE_VALUE)
		return FALSE;
	
	while(true)
	{
		bIsLoaded=false;

		// ���ϸ��� Ȯ���� ����
		for (i=lstrlen(findData.cFileName); i>=0; i--)
		{
			if (findData.cFileName[i] == '.')
			{
				findData.cFileName[i]='\0';
				break;
			}
		}

		// �̹� �ε�� �÷������̸� ����
		// ���� ������ ���� DLL�� �ι� �ε�Ǹ� ���۵��� �� ����
		if (!lstrcmpi(findData.cFileName, _T("MultiPlugin")))
		{
			bIsLoaded=true;
		}
		else if (!bIsRedundantAllowed)
		{
			for(i=0; i<aPluginArgs.size(); i++)
			{
				if ( !lstrcmpi(findData.cFileName, aPluginArgs[i].strPluginName.c_str()) )
				{
					bIsLoaded=true;
					break;
				}
			}
		}

		// ������ ����Ʈ�� �߰�
		if (!bIsLoaded)
		{
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)findData.cFileName);
		}

		if (!FindNextFile(hFindFile, &findData))
			break;

	}	// while(true)...

	if (bIsRedundantAllowed)
		SendMessage(hwndRedundantCheck, BM_SETCHECK, BST_CHECKED, NULL);
	else
		SendMessage(hwndRedundantCheck, BM_SETCHECK, BST_UNCHECKED, NULL);
	EnableWindow(hwndList, TRUE);

	return TRUE;
}


BOOL CALLBACK AddPluginDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR *pszPluginName=NULL;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			pszPluginName=(TCHAR *)lParam;	// TCHAR szPluginName[MAX_PATH]
			AddPluginDialogUpdate(hwndDlg);
			return TRUE;
		
		case WM_COMMAND:
		{
			HWND hwndList=GetDlgItem(hwndDlg, IDC_ADDPLUGINLIST);
			int nSelected=SendMessage(hwndList, LB_GETCURSEL, 0, 0);
			switch(LOWORD(wParam))
			{
				case IDOK:
					if (nSelected == LB_ERR)
					{
						MessageBox(hwndDlg, _T("�÷������� ������ �ּ���."), _T("�÷����� �߰�"), MB_OK | MB_ICONINFORMATION);
						break;
					}
					else
						SendMessage(hwndList, LB_GETTEXT, (WPARAM)nSelected, (LPARAM) pszPluginName);
				case IDCANCEL:
					EndDialog(hwndDlg, LOWORD(wParam));
					break;

				case IDC_REDUNDANTCHECK:
					if (bIsRedundantAllowed)
					{
						bIsRedundantAllowed=false;
						AddPluginDialogUpdate(hwndDlg);
					}
					else if (MessageBox(hwndDlg,
						_T("�÷����� �ߺ� �߰��� ����ġ ���� ����� �߱��� �� �ֽ��ϴ�. ����Ͻðڽ��ϱ�?"), 
						_T("����"), 
						MB_YESNO) == IDYES
						)
					{
						bIsRedundantAllowed=true;
						AddPluginDialogUpdate(hwndDlg);
					}
					break;

				case IDC_ADDPLUGINLIST:
					if (HIWORD(wParam) == LBN_DBLCLK)
						SendMessage(hwndDlg, WM_COMMAND, IDOK, NULL);
				default:;
			}
			return TRUE;
		}
		default:;

	return FALSE;
	}
}
