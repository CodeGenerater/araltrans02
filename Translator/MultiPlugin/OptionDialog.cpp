#include "StdAfx.h"
#include "resource.h"

#include "OptionDialog.h"
#include "MultiPlugin.h"

#include "ATPlugin.h"
#include "AddPluginDialog.h"
#include "DirectOptionDialog.h"

#include "Util.h"

#include <commctrl.h>



BOOL OptionDialogInit(HWND hwndDlg);
void OptionDialogUpdateListItems(HWND hwndDlg);

BOOL CALLBACK OptionDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{
		case WM_INITDIALOG:
			return OptionDialogInit(hwndDlg);

		case WM_COMMAND:
		{
			static TCHAR cszNotSelected[]=_T("�÷������� ���� ������ �ּ���");
			HWND hwndList=GetDlgItem(hwndDlg, IDC_PLUGINLIST);
			int nSelected=ListView_GetSelectionMark(hwndList);

			switch(LOWORD(wParam))
			{
				case IDCANCEL:
				case IDOK:
					EndDialog(hwndDlg, IDOK);
					return TRUE;

				case IDC_MOVETOPBUTTON:
					if (nSelected < 0) 
					{
						MessageBox(hwndDlg, cszNotSelected, g_szPluginName, MB_OK | MB_ICONINFORMATION );
						break;
					}
					if (g_cMultiPlugin.MoveTop(nSelected))
						OptionDialogUpdateListItems(hwndDlg);
					break;

				case IDC_MOVEUPBUTTON:
					if (nSelected < 0) 
					{
						MessageBox(hwndDlg, cszNotSelected, g_szPluginName, MB_OK | MB_ICONINFORMATION );
						break;
					}
					if (g_cMultiPlugin.MoveUp(nSelected))
						OptionDialogUpdateListItems(hwndDlg);
					break;
				case IDC_MOVEDOWNBUTTON:
					if (nSelected < 0) 
					{
						MessageBox(hwndDlg, cszNotSelected, g_szPluginName, MB_OK | MB_ICONINFORMATION );
						break;
					}
					if (g_cMultiPlugin.MoveDown(nSelected))
						OptionDialogUpdateListItems(hwndDlg);
					break;
				case IDC_MOVEBOTTOMBUTTON:
					if (nSelected < 0) 
					{
						MessageBox(hwndDlg, cszNotSelected, g_szPluginName, MB_OK | MB_ICONINFORMATION );
						break;
					}
					if (g_cMultiPlugin.MoveBottom(nSelected))
						OptionDialogUpdateListItems(hwndDlg);
					break;
				case IDC_OPTIONBUTTON:
					if (nSelected < 0) 
					{
						MessageBox(hwndDlg, cszNotSelected, g_szPluginName, MB_OK | MB_ICONINFORMATION );
						break;
					}
					if (g_cMultiPlugin.OnPluginOption(nSelected))
						OptionDialogUpdateListItems(hwndDlg);
					break;
				case IDC_ADDBUTTON:
				{
					TCHAR szPluginName[MAX_PATH];
					szPluginName[0]=_T('\0');

					if ( DialogBoxParam((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_ADDPLUGINDIALOG), 
						hwndDlg, AddPluginDialogProc, (LPARAM)szPluginName) == IDOK )
					{
						ATPLUGIN_ARGUMENT arg;
						arg.strPluginName=szPluginName;
						g_cMultiPlugin.Add(arg, false);
						OptionDialogUpdateListItems(hwndDlg);
					}

					break;
				}

				case IDC_REMOVEBUTTON:
					if (nSelected < 0) 
					{
						MessageBox(hwndDlg, cszNotSelected, g_szPluginName, MB_OK | MB_ICONINFORMATION );
						break;
					}

					if (MessageBox(hwndDlg, _T("�ش� �÷������� �����Ͻðڽ��ϱ�?"), _T("�÷����� ����"), MB_YESNO) == IDYES)
					{
						g_cMultiPlugin.Remove(nSelected);
						OptionDialogUpdateListItems(hwndDlg);
					}
					break;

				case IDC_DIRECTOPTIONBUTTON:
					if (DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_DIRECTOPTIONDIALOG), hwndDlg, DirectOptionDialogProc) == IDOK)
					{
						ATPLUGIN_ARGUMENT_ARRAY aPluginArgs;

						GetATPluginArgsFromOptionString(g_szOptionStringBuffer, aPluginArgs);

						g_cMultiPlugin.RemoveAll();
						g_cMultiPlugin.Add(aPluginArgs);
						OptionDialogUpdateListItems(hwndDlg);

					}
					break;

				case IDC_TRANSLATORCOMBO:
					
					if (HIWORD(wParam) == CBN_SELENDOK)
					{
						HWND hwndCombo = (HWND) lParam;
						nSelected=SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);

						g_cMultiPlugin.Remove(-1);


						if (nSelected > 0)
						{
							TCHAR szPluginName[MAX_PATH];

							SendMessage(hwndCombo, CB_GETLBTEXT, (WPARAM)nSelected, (LPARAM) szPluginName);

							ATPLUGIN_ARGUMENT arg;
							arg.strPluginName=szPluginName;
							g_cMultiPlugin.Add(arg, true);
							
						}
					}
					return TRUE;

				case IDC_TRANSLATOROPTIONBUTTON:
					if (g_cMultiPlugin.OnPluginOption(-1))
						OptionDialogUpdateListItems(hwndDlg);
					break;

				default:;
			}
			SetFocus(hwndDlg);
			return TRUE;	
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmh=(LPNMHDR) lParam;


			// ����Ŭ�� = �ɼǹ�ư�� ����
			if ( (lpnmh->idFrom == IDC_PLUGINLIST) && (lpnmh->code == NM_DBLCLK) )
				SendMessage(hwndDlg, WM_COMMAND, IDC_OPTIONBUTTON, NULL);
		}

		default:;
	}


	return FALSE;
}

BOOL OptionDialogInit(HWND hwndDlg)
{
	TCHAR szPluginInfo[80];

	HWND hwndList=GetDlgItem(hwndDlg, IDC_PLUGINLIST);
	HWND hwndCombo= GetDlgItem(hwndDlg, IDC_TRANSLATORCOMBO);
	
	LVCOLUMN lvColumn;

	ZeroMemory(&lvColumn, sizeof(lvColumn));

	wsprintf(szPluginInfo, _T("%s v%d.%d.%d"), g_szPluginName, 
		g_pluginVer.PluginVersion.Major, g_pluginVer.PluginVersion.Minor, g_pluginVer.PluginVersion.BuildDate);

	SetDlgItemText(hwndDlg, IDC_INFOSTATIC, szPluginInfo);

	// ����Ʈ�� ��Ÿ�� = ���� ����, �׸������
	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// ����Ʈ�� �÷� ���
	lvColumn.mask=LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt=LVCFMT_LEFT;
	lvColumn.cx=150;
	lvColumn.pszText=_T("�÷����� �̸�");
	lvColumn.iSubItem=0;
	ListView_InsertColumn(hwndList, 0, &lvColumn);

	lvColumn.pszText=_T("�ɼ�");
	lvColumn.cx=300;
	lvColumn.iSubItem=1;
	ListView_InsertColumn(hwndList, 1, &lvColumn);

	TCHAR szPath[MAX_PATH];
	HANDLE hFindFile;
	WIN32_FIND_DATA findData;
	int i;

	EnableWindow(hwndCombo, FALSE);

	SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)_T("<none>"));

	wsprintf(szPath, _T("%s\\Translator\\*.DLL"), GetATDirectory());

	// ���� �÷����� ���� �˻�
	hFindFile=FindFirstFile(szPath, &findData);

	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		while(true)
		{
			// ���ϸ��� Ȯ���� ����
			for (i=lstrlen(findData.cFileName); i>=0; i--)
			{
				if (findData.cFileName[i] == '.')
				{
					findData.cFileName[i]='\0';
					break;
				}
			}
			if (lstrcmpi(findData.cFileName, _T("MultiPlugin")))
				SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) findData.cFileName);

			if (!FindNextFile(hFindFile, &findData))
				break;
		}

	}
	EnableWindow(hwndCombo, TRUE);

	OptionDialogUpdateListItems(hwndDlg);

	return TRUE;
}

void OptionDialogUpdateListItems(HWND hwndDlg)
{
	HWND hwndList=GetDlgItem(hwndDlg, IDC_PLUGINLIST);
	HWND hwndCombo= GetDlgItem(hwndDlg, IDC_TRANSLATORCOMBO);
	
	LVITEM lvItem;
	
	ATPLUGIN_ARGUMENT_ARRAY aPluginArgs;
	int i;
	TCHAR szBuffer[1024];
	
	EnableWindow(hwndList, FALSE);

	// ��� ����Ʈ �����ͼ�
	g_cMultiPlugin.GetPluginArgs(aPluginArgs);
	
	ZeroMemory(&lvItem, sizeof(lvItem));
	
	lvItem.mask=LVIF_TEXT;
	lvItem.pszText=szBuffer;
	
	// ����Ʈ�� ��ü ����
	ListView_DeleteAllItems(hwndList);

	// ����
	for(i=0; i<aPluginArgs.size()-1; i++)
	{
		lvItem.iItem=i;
		lvItem.iSubItem=0;
		lstrcpyn(szBuffer, aPluginArgs[i].strPluginName.c_str(), 1024);
		ListView_InsertItem(hwndList, &lvItem);
		
		lstrcpyn(szBuffer, aPluginArgs[i].strOption.c_str(), 4096);
		ListView_SetItemText(hwndList, i, 1, szBuffer);
	}

	if (!aPluginArgs[i].strPluginName.empty())
	{
		SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) aPluginArgs[i].strPluginName.c_str());
		SetDlgItemText(hwndDlg, IDC_TRANSLATOROPTIONEDIT, aPluginArgs[i].strOption.c_str());
	}
	else
		SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);


	EnableWindow(hwndList, TRUE);

	return;
}