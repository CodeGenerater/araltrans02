// AralGoodDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AralGood.h"
#include "AralGoodDlg.h"
#include "WaitingDlg.h"
#include "OptionDlg.h"
#include "AralAboutDlg.h"
#include <tlhelp32.h>
#include <shlwapi.h>
#include "Util/AralProcessAPI/AralProcessAPI.h"
#include "Util/AralFileAPI/AralFileAPI.h"
#include "Util/RegistryMgr/cRegistryMgr.h"
#include "Util/ZipArchive/ZipArchive.h"
#include "Util/Misc.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib,"version.lib")
#pragma comment(lib,"shlwapi.lib")

/////////////////////////////////////////////////////////////////////////////
// CAralGoodDlg dialog

CAralGoodDlg::CAralGoodDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAralGoodDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAralGoodDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_bShowUI = FALSE;
	m_hDllMod = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ZeroMemory(&m_rtOrigSize, sizeof(RECT));

	m_dwPID = 0;
	m_dwADR = 0;
	m_dwDelay = 0;
	m_strHookName    = _T("");
	m_strHookOption  = _T("");
	m_strTransName   = _T("");
	m_strTransOption = _T("");
	m_strFilters	 = _T("");
	m_strGameDirectory = _T("");

	m_hHookWorkThread = NULL;

	m_dwAutoUpdate = 1;
	m_bUseATDataZip = FALSE;
	m_bDeleteLocalFiles = FALSE;
	m_bAppLocAutoConfirm = FALSE;
}

void CAralGoodDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAralGoodDlg)
	DDX_Control(pDX, IDC_LIST2, m_ctrlWindowList);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_HOME, m_linkHome);
	DDX_Control(pDX, IDC_CHECK_EXPERT, mbCheExpert);
	DDX_Control(pDX, IDC_CHECK_RESTART, mbCheRestart);
}

BEGIN_MESSAGE_MAP(CAralGoodDlg, CDialog)
	//{{AFX_MSG_MAP(CAralGoodDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_WINDOWPOSCHANGING()
	ON_BN_CLICKED(IDC_BTN_HOOK, OnBtnHook)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST2, OnDblclkList)
	ON_BN_CLICKED(IDC_BTN_REFRESH, OnBtnRefresh)
//	ON_BN_CLICKED(IDC_BUTTON1, &CAralGoodDlg::OnBnClickedButton1)
//	ON_BN_CLICKED(IDC_BTN_UNHOOK, &CAralGoodDlg::OnBnClickedBtnUnhook)
//	ON_BN_CLICKED(IDC_BUTTON_HOMEPAGE, &CAralGoodDlg::OnBnClickedButtonHomepage)
	ON_BN_CLICKED(IDC_BTN_OPTION, &CAralGoodDlg::OnBnClickedBtnOption)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_ABOUT, &CAralGoodDlg::OnBnClickedBtnAbout)
	ON_BN_CLICKED(IDC_CHECK_EXPERT, &CAralGoodDlg::OnBnClickedCheckExpert)
	ON_COMMAND(ID_HELP, &CAralGoodDlg::OnHelp)
	ON_BN_CLICKED(IDC_BTN_DIR, &CAralGoodDlg::OnBnClickedBtnDir)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CAralGoodDlg::OnLvnItemchangedList2)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CAralGoodDlg message handlers



BOOL CAralGoodDlg::IsUpdateRequired()
{
	DWORD dwExitCode = 0;

	// �ڵ� ������Ʈ ����
	CString strUpdaterDir = CAralFileAPI::GetFileDir( CAralProcessAPI::GetCurrentModulePath() );
	CString strUpdaterPath = strUpdaterDir + _T("\\AralUpdater.exe check");
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);

	CreateProcess( NULL, (LPTSTR)(LPCTSTR)strUpdaterPath, NULL, NULL, FALSE, 0, NULL, (LPTSTR)(LPCTSTR)strUpdaterDir, &si, &pi );
	TRACE( _T("[ aral1 ] [IsUpdateRequired] Launching returned HANDLE 0x%p \n"), pi.hProcess );

	// ���α׷��� �����ؾ��ϴ��� ��ȯ������ �Ǻ�
	::WaitForSingleObject( pi.hProcess, 7000 );
	::GetExitCodeProcess( pi.hProcess, &dwExitCode );
	TRACE( _T("[ aral1 ] GetExitCodeProcess() returned %d \n"), dwExitCode );
	if( dwExitCode == -1 )
	{
		TRACE( _T("[ aral1 ] [CServiceModule::Run] ������Ʈ�� �ʿ��� \n") );
		return TRUE;
	}

	return FALSE;

}


void CAralGoodDlg::UpdateClient()
{
	if( IsUpdateRequired() == TRUE )
	{
		// �ڵ� ������Ʈ ����
		CString strUpdaterDir = CAralFileAPI::GetFileDir( CAralProcessAPI::GetCurrentModulePath() );
		CString strUpdaterPath = strUpdaterDir + _T("\\AralUpdater.exe");

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFO);

		CreateProcess( NULL, (LPTSTR)(LPCTSTR)strUpdaterPath, NULL, NULL, FALSE, 0, NULL, (LPTSTR)(LPCTSTR)strUpdaterDir, &si, &pi );
	}
}


BOOL CAralGoodDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ������ ���� ũ�� ����
	this->GetWindowRect(&m_rtOrigSize);

	// �Ⱥ��̰� ����
	CRect rect(0,0,0,0);
	this->MoveWindow(rect);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// DLL �ε�
	//m_hDllMod = LoadLibrary("ATCTNR.dll");

	// ����� ���� ���
	SetDebugPrivilege();

	// ����Ʈ ��Ʈ�� �ʱ�ȭ
	InitWindowList();

	// ���� ���
	TCHAR szExeFileName[MAX_PATH] = {0,};
	if( ::GetModuleFileName( NULL, szExeFileName, MAX_PATH ) )
	{
		CString strVer = GetFileVersion(szExeFileName);
		if(!strVer.IsEmpty())
		{
			GetDlgItem(IDC_STATIC_VER)->SetWindowText(_T("Build : ") + strVer);
		}

#ifdef TEST_VERSION
		CString strTemp;
		GetWindowText(strTemp);
		strTemp+=_T(" - �׽�Ʈ ����");
		SetWindowText(strTemp);
#endif

	}
	
	// �����۸�ũ �ʱ�ȭ
	m_linkHome.SetAutoSize(TRUE);
	m_linkHome.SetColours(RGB(0,0,200), RGB(0,0,200), RGB(50,50,250));
	m_linkHome.SetURL(_T("http://www.aralgood.com/"));

	// ���� �б�
	CString strIniFileName=szExeFileName;
	strIniFileName.Replace(_T(".exe"), _T(".ini"));

	m_bUseATDataZip=(GetPrivateProfileInt(_T("AralTrans"), _T("UseATDataZip"), 1, strIniFileName)?TRUE:FALSE);
	m_bDeleteLocalFiles=(GetPrivateProfileInt(_T("AralTrans"), _T("DeleteLocalFiles"), 0, strIniFileName)?TRUE:FALSE);
	m_bAppLocAutoConfirm=(GetPrivateProfileInt(_T("AralTrans"), _T("AppLocAutoConfirm"), 1, strIniFileName)?TRUE:FALSE);

	// ������Ʈ üũ
	if( CRegistryMgr::RegExist(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AutoUpdate")) == FALSE)
	{
		CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AutoUpdate"), 1);			
	}

	m_dwAutoUpdate = CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AutoUpdate"));	
	if( m_dwAutoUpdate && IsUpdateRequired() )
	{

		BOOL bExecUpdate = FALSE;

#ifndef TEST_VERSION	// �׽�Ʈ���������� �ڵ�������Ʈ�� �ϴ� �׿����´�.
		
		if( (m_dwAutoUpdate & 2) )	// Auto Execute
		{
			bExecUpdate = TRUE;
		}
		else
		{
			if(MessageBox(_T("�ƶ�Ʈ���� ������Ұ� ������Ʈ �Ǿ����ϴ�.\r\n���� �ٿ�ε� �����ðڽ��ϱ�?"), _T("�˸�"), MB_YESNO) == IDYES)
			{
				bExecUpdate = TRUE;
			}
		}

		if(bExecUpdate)
		{
			// ���� �����ϱ�
			CString strPrevParam = ::GetCommandLine();
			TCHAR szCurDir[MAX_PATH];
			::GetCurrentDirectory(MAX_PATH, szCurDir);
			CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("PrevCmdLine"), strPrevParam);
			CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("PrevWorkDir"), szCurDir);

			// �������� �����ϰ�
			UpdateClient();

			// �ƶ�Ʈ���� ����
			::TerminateProcess( GetCurrentProcess(), 0 );
		}
#endif

	}


	// ���� ��������
	CString strParam = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("PrevCmdLine"));
	CString strWorkDir = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("PrevWorkDir"));
	CRegistryMgr::RegDelete(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("PrevCmdLine"));
	CRegistryMgr::RegDelete(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("PrevWorkDir"));

	// ������Ʈ�� ���� ���ڰ� ���ٸ�
	if(strParam.IsEmpty())
	{
		strParam = ::GetCommandLine();
		
		int nOptionIdx = strParam.Find(_T("/r:"));
		if(nOptionIdx>0)
		{
			CString strGUID = strParam.Mid(nOptionIdx+3).TrimRight();
			CString strStoredOption = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood\\StoredOption"), strGUID);
			if(!strStoredOption.IsEmpty()) strParam = strStoredOption;
		}
	}
	// ������Ʈ�� �߻��߾��ٸ�
	else
	{
		::SetCurrentDirectory(strWorkDir);
	}

	// ���̾�α� ���� ���� �ʱ�ȭ
	CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("ShowHide"), 0);
	CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryPatch"), 0);

	/*DWORD dwWrite = 0;
	const char* zoneIdentifier = "[Zonetransfer]\nZoneID=3";
	HANDLE hFile = CreateFileA("help.chm:Zone.Identifier", 
								GENERIC_WRITE,
								FILE_SHARE_READ,
								NULL,
								OPEN_ALWAYS,
								0,
								NULL);
	WriteFile(hFile, zoneIdentifier, strlen(zoneIdentifier), &dwWrite, NULL);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);*/

	// ���� �ؼ�
	CStringList tokens;
	int len = strParam.GetLength();
	int nStartIdx = 0;
	for(int i=0; i<len; i++)
	{
		if(strParam[i]==_T('\"'))
		{
			i++;
			while(i<len)
			{
				if(strParam[i]==_T('\"')) break;
				i++;
			}
		}
		else if(strParam[i]==_T('{'))
		{
			i++;
			while(i<len)
			{
				if(strParam[i]==_T('}')) break;
				i++;
			}
		}


		if((i+1)==len || strParam[i]==_T(' '))
		{
			CString token = strParam.Mid(nStartIdx, i-nStartIdx+1);
			token = token.Trim();
			token.Replace(_T("\""), _T(""));
			if(!token.IsEmpty()) tokens.AddTail(token);

			nStartIdx = i+1;
		}
	}

	
	DWORD apploc_lang = 0;
	BOOL bNtlea = 0, bRestart = 0;
	CString strKonJJonK = _T("");
	CString strGamePath;
	CString strProcName;
	CString strFileName;

	POSITION pos = tokens.GetHeadPosition();
	if(pos)
	{

		CString token = tokens.GetNext(pos);
		while(pos)
		{
			CString token = tokens.GetNext(pos);
			int nTokenLen = token.GetLength();
			
			if(nTokenLen == 0)
			{
				continue;
			}
			else if(nTokenLen >= 2 && token.Left(2).CompareNoCase(_T("/L"))==0)
			{
				if(nTokenLen>2) apploc_lang = (DWORD)_ttoi(token.Mid(2));
				if(0 == apploc_lang) apploc_lang = 411;
			}
			else if(nTokenLen >= 2 && token.Left(2).CompareNoCase(_T("/N"))==0)
			{
				bNtlea = 1;
			}
			else if(nTokenLen == 5 && (token.CompareNoCase(_T("/KonJ"))==0 || token.CompareNoCase(_T("/JonK"))==0))
			{
				strKonJJonK = token.Mid(1);
			}
			else if(nTokenLen >= 10 && token.Right(10).CompareNoCase(_T("AppLoc.exe"))==0)
			{
				
			}			
			else if(nTokenLen > 2 && token.Left(2).CompareNoCase(_T("/P"))==0)
			{				
				if(token[2] == _T('{'))
				{
					int nProcNameStart = 2;
					int nProcNameEnd = token.ReverseFind(_T('}'));
					if( nProcNameEnd > 0 )
					{
						strProcName = token.Mid(nProcNameStart+1, nProcNameEnd-nProcNameStart-1).Trim();

						// attachment type �� ��� �Ϲ������� Ư���� �δ��� �־ �δ��� ���� ����������
						// �����Ű�� ���� �ű⿡ ���̱� ���� �浵�� ���̴µ� �̷� ��� �������� ���ӽ���
						// �� �ٽ� �ƶ�Ʈ���� ������ �����ϴ� �� �ܰ踦 ���ľ� �ϹǷ� ����� ������.
						// ���� strGamePath ���� �δ���, /P {���ϸ�} ���� ���� ���������� �����ϴ� ����
						// �ùٸ��ٰ� �����Ѵ�.

					}
				}
				else
				{
					m_dwPID = (DWORD)_ttoi(token.Mid(2));
				}

			}
			else if(nTokenLen > 2 && token.Left(2).CompareNoCase(_T("/M"))==0)
			{
				if(token[2] == _T('{'))
				{
					int nProcNameStart = 2;
					int nProcNameEnd = token.ReverseFind(_T('}'));
					if( nProcNameEnd > 0 )
					{
						CString strTidAdr = token.Mid(nProcNameStart+1, nProcNameEnd-nProcNameStart-1).Trim();
						m_dwADR = (DWORD)wcstol(strTidAdr, NULL, 16);
					}
				}
			}
			else if(nTokenLen >= 2 && token.Left(2).CompareNoCase(_T("/H"))==0)
			{
				CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("ShowHide"), 1);
			}
			else if(nTokenLen >= 2 && token.Left(2).CompareNoCase(_T("/D"))==0)
			{
				m_dwDelay = (DWORD)_ttoi(token.Mid(2));
			}
			else if(nTokenLen >= 2 && token.Left(2).CompareNoCase(_T("/R"))==0)
			{
				bRestart = 1;
			}
			else if(nTokenLen > 3 && token.Left(3).CompareNoCase(_T("/a:"))==0)
			{
				int nOptStrStart = token.Find(_T('{'));
				int nOptStrEnd = token.ReverseFind(_T('}'));
				if( nOptStrStart > 0 && nOptStrEnd > 0 )
				{
					m_strHookName = token.Mid(3, nOptStrStart-3);
					m_strHookOption = token.Mid(nOptStrStart+1, nOptStrEnd-nOptStrStart-1);
				}

			}
			else if(nTokenLen > 3 && token.Left(3).CompareNoCase(_T("/t:"))==0)
			{
				int nOptStrStart = token.Find(_T('{'));
				int nOptStrEnd = token.ReverseFind(_T('}'));
				if( nOptStrStart > 0 && nOptStrEnd > 0 )
				{
					m_strTransName = token.Mid(3, nOptStrStart-3);
					m_strTransOption = token.Mid(nOptStrStart+1, nOptStrEnd-nOptStrStart-1);
				}

			}
			else if(nTokenLen > 3 && token.Left(3).CompareNoCase(_T("/f:"))==0)
			{
				m_strFilters = token.Mid(3);
			}
			else if(nTokenLen > 3 && token.Left(3).CompareNoCase(_T("/b:"))==0)
			{
				int nOptStrStart = token.Find(_T('{'));
				int nOptStrEnd = token.ReverseFind(_T('}'));
				int nOptStrFir = token.Find(_T('('));
				int nOptStrSec = token.Find(_T(')'));
				int nOptStrMid = token.Find(_T(','));
				int nCount = 0;
			
				while( nOptStrEnd > 0 )
				{
					int nOptStrLen = 0;
					DWORD dwPatchAdr;
					nCount ++;
					CString regCount, m_patchName;
					CString m_patchAdr = token.Mid(nOptStrStart+1, nOptStrFir-nOptStrStart-1);
					int nOptOffMid = m_patchAdr.Find(_T('!'));
					regCount.Format(_T("BinaryName%d"),nCount);
					if( nOptOffMid != -1 )
					{
						m_patchName.Format(_T("%s"), m_patchAdr.Mid(0,nOptOffMid));
						CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, m_patchName);
						dwPatchAdr = (DWORD)wcstol(m_patchAdr.Mid(nOptOffMid+1), NULL, 16);
					}
					else
					{
						CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, _T(""));
						dwPatchAdr = (DWORD)wcstol(m_patchAdr, NULL, 16);
					}
					CString m_patchBinary;
					unsigned char* cBuff = (unsigned char*)malloc((nOptStrSec - nOptStrFir - 1)/2);
					while( nOptStrSec - nOptStrFir - 1 >  nOptStrLen*2 )
					{
						m_patchBinary = token.Mid(nOptStrFir+1+(nOptStrLen*2), 2);
						cBuff[nOptStrLen] = (BYTE)wcstol(m_patchBinary, NULL, 16);
						nOptStrLen ++;
					}
					regCount.Format(_T("Binary%d"),nCount);
					CRegistryMgr::RegWriteBINARY(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, cBuff, nOptStrLen);
					regCount.Format(_T("BinaryAdr%d"),nCount);
					CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, dwPatchAdr);
					free(cBuff);

					if( nOptStrMid == -1 )
						break;

					nOptStrStart = nOptStrMid;
					nOptStrFir = token.Find(_T('('),nOptStrMid);
					nOptStrSec = token.Find(_T(')'),nOptStrMid);
					nOptStrMid = token.Find(_T(','),nOptStrMid+1);

					if( nOptStrFir == -1 &&
						nOptStrSec == -1 )
						break;

					if( nOptStrMid == -1 )
						nOptStrMid = nOptStrEnd;
				}
				if( nCount )
				{
					CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryPatch"), nCount);
					CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryRestart"), 0);
				}
			}
			else
			{
				if(!strGamePath.IsEmpty()) strGamePath += _T(' ');
				strGamePath += token;
				
				//char szShortPath[MAX_PATH] = {0,};
				//DWORD dwRes = ::GetShortPathName(token, szShortPath, MAX_PATH);
				//if(dwRes)
				//{
				//	strGameShortPath = szShortPath;
				//}
				//else
				//{
				//	CString strMsg;
				//	strMsg.Format("�߸��� �Ķ���� : '%s'", (LPCTSTR)token);
				//	this->MessageBox(strMsg);
				//}
			}

		}
	}

	// ���� ����
	if(0 == m_dwPID && !strGamePath.IsEmpty())
	{

		PROCESS_INFORMATION pi = {0,};
		STARTUPINFO si = {0,};
		si.cb = sizeof(STARTUPINFO);

		// ���� ���丮 ����
		m_strGameDirectory=strGamePath.Left(strGamePath.ReverseFind(_T('\\')));
		
		// ���÷����� �����
		if(apploc_lang > 0)
		{
			TCHAR szWinDir[MAX_PATH];
			::GetWindowsDirectory(szWinDir, MAX_PATH);

			CString strNewCommand;
			strNewCommand.Format(_T("%s\\AppPatch\\AppLoc.exe \"%s\" \"/L%d\""), szWinDir, (LPCTSTR)strGamePath, apploc_lang);

			::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strNewCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);



			if (m_bAppLocAutoConfirm)
			{
				// ���÷����� ��ư �ڵ�Ŭ����

				HWND hwnd=NULL;
				do
				{
					hwnd=::FindWindow(NULL, _T("Microsoft AppLocale"));
				
					if (hwnd)
						hwnd=::FindWindowEx(hwnd, NULL, _T("Button"), NULL);	// ù��° ��ư = "Ȯ��"
					else
						Sleep(300);

				} while (!hwnd);

				// ��¥ Ŭ�� �޼���
				::SendMessage(::GetParent(hwnd), WM_COMMAND, (WPARAM)MAKELONG(::GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM)hwnd);
			}


			DWORD dwWaitIdle = ::WaitForSingleObject(pi.hProcess, INFINITE);
			if(m_dwDelay)
			{
				CWaitingDlg wait_dlg;
				wait_dlg.m_dwWaitingTime = m_dwDelay;
				wait_dlg.DoModal();	
				m_dwDelay = 0;
			}
			for(int i=0; i<15; i++)
			{
				m_dwPID = GetChildProcessID(pi.dwProcessId, NULL);
				if(m_dwPID) break;

				Sleep(300);
			}

		}
		// KonJ, JonK �����
		else if(strKonJJonK.IsEmpty() == FALSE)
		{
			CString strKonJJonKDir = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("KonJJonKDir"));
			CString strNewCommand;
			strNewCommand.Format(_T("\"%s\\ApiHooks.exe\" -nq %s.dll \"%s\""), (LPCTSTR)strKonJJonKDir, (LPCTSTR)strKonJJonK, (LPCTSTR)strGamePath);
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;

			::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strNewCommand, NULL, NULL, FALSE, 0, NULL, (LPCTSTR)strKonJJonKDir, &si, &pi);
			DWORD dwWaitIdle = ::WaitForSingleObject(pi.hProcess, INFINITE);
			if(m_dwDelay)
			{
				CWaitingDlg wait_dlg;
				wait_dlg.m_dwWaitingTime = m_dwDelay;
				wait_dlg.DoModal();	
				m_dwDelay = 0;
			}
			for(int i=0; i<15; i++)
			{
				m_dwPID = GetChildProcessID(pi.dwProcessId, NULL);
				if(m_dwPID) break;

				Sleep(300);
			}

		}
		// NTLEA �����
		else if(bNtlea == TRUE)
		{
			CString strNtleaDir = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("NtleaDir"));
			CString strNewCommand;

			strNewCommand.Format(_T("\"%s\\NTLEA.exe\" \"-autorun\" \"%s\""), (LPCTSTR)strNtleaDir, (LPCTSTR)strGamePath);

			::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strNewCommand, NULL, NULL, FALSE, 0, NULL, (LPCTSTR)strNtleaDir, &si, &pi);
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;

			DWORD dwWaitIdle = ::WaitForSingleObject(pi.hProcess, INFINITE);
			if(m_dwDelay)
			{
				CWaitingDlg wait_dlg;
				wait_dlg.m_dwWaitingTime = m_dwDelay;
				wait_dlg.DoModal();	
				m_dwDelay = 0;
			}
			for(int i=0; i<15; i++)
			{
				m_dwPID = GetChildProcessID(pi.dwProcessId, NULL);
				if(m_dwPID) break;

				Sleep(300);
			}

			if ( strProcName.IsEmpty() )
			{
				int nFileNameStart = strGamePath.ReverseFind(_T('\\'));
				if( (strGamePath.GetLength() - nFileNameStart) > 0 )
					strProcName = strGamePath.Mid(nFileNameStart+1, strGamePath.GetLength()).Trim();
			}
		}
		// �Ϲ� �����
		else
		{
			if(bRestart)
			{
				// ���μ��� ���� ��ġ ������ ���
				CreateProcess(strGamePath,NULL,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&si,&pi);
				BinaryPatch (pi.hProcess);
				ResumeThread(pi.hThread);
			}
			else
			{
				::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strGamePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
				m_dwPID = pi.dwProcessId;
				DWORD dwWaitIdle = ::WaitForInputIdle(pi.hProcess, 10000);
			}
			if(m_dwDelay)
			{
				CWaitingDlg wait_dlg;
				wait_dlg.m_dwWaitingTime = m_dwDelay;
				wait_dlg.DoModal();	
				m_dwDelay = 0;
			}
		}
	}
	else if (m_dwPID)
	{
		// attachment type /P[PID]
		
		// ���� ���丮 ����
		TCHAR szGamePath[MAX_PATH];
		if (GetModulePath(m_dwPID, szGamePath, MAX_PATH))
		{
			m_strGameDirectory=szGamePath;
			m_strGameDirectory=m_strGameDirectory.Left(m_strGameDirectory.ReverseFind(_T('\\')));
		}

	}

	// attachment type /P{���ϸ�} �� ��� ó��
	if (!strProcName.IsEmpty())
	{
		DWORD dwLoaderPID=m_dwPID;
		m_dwPID=0;

		for (int j=0; j<5; j++)
		{
			// ������ ����
			FillWindowList();
			
			// ��ġ�ϴ� ���μ������� �ִ��� �˻�
			int cnt = m_ctrlWindowList.GetItemCount();
			for(int i=0; i<cnt; i++)
			{
				
				CString strProcNameTmp = m_ctrlWindowList.GetItemText(i, 0);			
				
				// ã�Ҵٸ�
				if(strProcName.CompareNoCase(strProcNameTmp) == 0)
				{
					m_dwPID = (DWORD)_ttoi( m_ctrlWindowList.GetItemText(i, 1) );
					break;
				}

				if (dwLoaderPID)
				{
					// ���� �δ��� ���������� �����Ű�� �ٷ� �����Ƿ� �����⸦ ��ٷ�����.
					// ���� ���������� ���������� ����ִ� �δ��� ���� �� �����Ƿ� ��ٷ����� �ȳ����� �������� �Ѿ��.

					HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwLoaderPID);
					
					if (hProcess) 
					{
						DWORD dwWaitIdle = ::WaitForSingleObject(hProcess, 300);
						CloseHandle(hProcess);
					}
					else
						Sleep(300);
				}
				else
					Sleep(300);

			} // of for
			if (m_dwPID)
			{
				// ���� ���丮 ����
				TCHAR szGamePath[MAX_PATH];
				if (GetModulePath(m_dwPID, szGamePath, MAX_PATH))
				{
					m_strGameDirectory=szGamePath;
					m_strGameDirectory=m_strGameDirectory.Left(m_strGameDirectory.ReverseFind(_T('\\')));
				}
				break;
			}
		}
	}

	if (m_bUseATDataZip && m_dwPID)
		ExtractATData();

	if (!strGamePath.IsEmpty() || !strProcName.IsEmpty())
	{
		// ����� �뵵�� ������ ������ ������ ������ �����صд�.
		if (!strGamePath.IsEmpty())
			CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastExecPath"), strGamePath);
		else
			CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastExecPath"), _T(""));
		if (!strProcName.IsEmpty())
			CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastProcName"), m_strGameDirectory+_T('\\')+strProcName);
		else
			CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastProcName"), _T(""));
	}


	// ��ŷ ����
	m_hHookWorkThread = (HANDLE)_beginthreadex(NULL, 0, HookWorkThreadFunc, this, 0, NULL);
	
	SetWindowPos( &wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
	return TRUE;  // return TRUE  unless you set the focus to a control
}


unsigned int __stdcall CAralGoodDlg::HookWorkThreadFunc( void* pParam )
{
	CAralGoodDlg* pThis = (CAralGoodDlg*)pParam;

	if(pThis)
	{
		pThis->Hook(
			pThis->m_dwPID, 
			pThis->m_dwADR,
			pThis->m_dwDelay, 
			pThis->m_strHookName, 
			pThis->m_strHookOption, 
			pThis->m_strTransName, 
			pThis->m_strTransOption,
			pThis->m_strFilters
			);
	}

	return 0;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAralGoodDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAralGoodDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

typedef BOOL (__stdcall * PROC_SetupATContainer) (DWORD, BOOL);

void CAralGoodDlg::OnBtnHook() 
{	
	// ����Ʈ���� ���� ���õ� �ε����� ���Ѵ�
	int nIndex = m_ctrlWindowList.GetSelectionMark();

	// ������ ADR�� ���Ѵ�
	CString strADR = m_ctrlWindowList.GetItemText(nIndex, 3);
	m_dwADR = (DWORD)wcstol(strADR,NULL,16);
	if( mbCheExpert.GetCheck() == 1 )
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("SetTidAdr"), strADR);
	else
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("SetTidAdr"), _T(""));

	// ���μ��� ID�� ���Ѵ�
	CString strPID = m_ctrlWindowList.GetItemText(nIndex, 1);
	DWORD dwProcessId = (DWORD)_ttoi( strPID );

	// ���̳ʸ� ��ġ ����
	CString m_strBiPatch;
	GetDlgItem(IDC_EDIT_BINARY)->GetWindowText(m_strBiPatch);
	m_strBiPatch.Replace(_T(" "), _T(""));
	if( m_strBiPatch.GetLength() > 0 )
	{
		int nOptStrStart = m_strBiPatch.Find(_T('{'));
		int nOptStrEnd = m_strBiPatch.ReverseFind(_T('}'));
		int nOptStrFir = m_strBiPatch.Find(_T('('));
		int nOptStrSec = m_strBiPatch.Find(_T(')'));
		int nOptStrMid = m_strBiPatch.Find(_T(','));
		int nCount = 0;

		if( nOptStrStart == -1
		 && nOptStrEnd == -1 )
		{
			MessageBox(_T("�ùٸ� ���̳ʸ� ��ġ ������ �ƴմϴ�."));
			return;
		}
	
		while( nOptStrEnd > 0 )
		{
			int nOptStrLen = 0;
			DWORD dwPatchAdr;
			nCount ++;
			CString regCount, m_patchName;
			CString m_patchAdr = m_strBiPatch.Mid(nOptStrStart+1, nOptStrFir-nOptStrStart-1);
			int nOptOffMid = m_patchAdr.Find(_T('!'));
			regCount.Format(_T("BinaryName%d"),nCount);
			if( nOptOffMid != -1 )
			{
				m_patchName.Format(_T("%s"), m_patchAdr.Mid(0,nOptOffMid));
				CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, m_patchName);
				dwPatchAdr = (DWORD)wcstol(m_patchAdr.Mid(nOptOffMid+1), NULL, 16);
			}
			else
			{
				CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, _T(""));
				dwPatchAdr = (DWORD)wcstol(m_patchAdr, NULL, 16);
			}
			CString m_patchBinary;
			unsigned char* cBuff = (unsigned char*)malloc((nOptStrSec - nOptStrFir - 1)/2);
			while( nOptStrSec - nOptStrFir - 1 >  nOptStrLen*2 )
			{
				m_patchBinary = m_strBiPatch.Mid(nOptStrFir+1+(nOptStrLen*2), 2);
				cBuff[nOptStrLen] = (BYTE)wcstol(m_patchBinary, NULL, 16);
				nOptStrLen ++;
			}
			regCount.Format(_T("Binary%d"),nCount);
			CRegistryMgr::RegWriteBINARY(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, cBuff, nOptStrLen);
			regCount.Format(_T("BinaryAdr%d"),nCount);
			CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, dwPatchAdr);
			free(cBuff);

			if( nOptStrMid == -1 )
				break;

			nOptStrStart = nOptStrMid;
			nOptStrFir = m_strBiPatch.Find(_T('('),nOptStrMid);
			nOptStrSec = m_strBiPatch.Find(_T(')'),nOptStrMid);
			nOptStrMid = m_strBiPatch.Find(_T(','),nOptStrMid+1);

			if( nOptStrFir == -1 &&
				nOptStrSec == -1 )
				break;

			if( nOptStrMid == -1 )
				nOptStrMid = nOptStrEnd;
		}
		if( nCount )
		{
			CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryPatch"), nCount);
			CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryRestart"), 0);
		}
	}

	// ���μ��� �� ���� ��ŷ�� ���
	CString str;
	GetDlgItemText(IDC_DIR, str);
	STARTUPINFO sStartupInfo;
	PROCESS_INFORMATION sProcessInfo;
	memset(&sStartupInfo,0,sizeof(STARTUPINFO));
	memset(&sProcessInfo,0,sizeof(PROCESS_INFORMATION));
	if( mbCheRestart.GetCheck() == 1 )
	{
		// ���μ��� ���� �� ����
		if( 0 != dwProcessId )
			TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId), 0);

		CreateProcess(str,NULL,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&sStartupInfo,&sProcessInfo);
		BinaryPatch (sProcessInfo.hProcess);
		if( 0 != sProcessInfo.dwProcessId ) dwProcessId = sProcessInfo.dwProcessId;

		ResumeThread(sProcessInfo.hThread);
	}
	if( 0 == dwProcessId ) return;

	if(NULL == m_hHookWorkThread)
	{
		m_dwPID = dwProcessId;
		m_strHookName    = _T("");
		m_strHookOption  = _T("");
		m_strTransName   = _T("");
		m_strTransOption = _T("");

		// ���� ���丮 ����
		TCHAR szGamePath[MAX_PATH];
		if (GetModulePath(m_dwPID, szGamePath, MAX_PATH))
		{
			m_strGameDirectory=szGamePath;
			m_strGameDirectory=m_strGameDirectory.Left(m_strGameDirectory.ReverseFind(_T('\\')));
		}

		if (m_bUseATDataZip)
			ExtractATData();

		// ����� �뵵�� ������ ������ ������ ������ �����صд�.
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastExecPath"), _T(""));
		CRegistryMgr::RegWrite(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("LastProcName"), szGamePath);


		m_hHookWorkThread = (HANDLE)_beginthreadex(NULL, 0, HookWorkThreadFunc, this, 0, NULL);
	}
}

void CAralGoodDlg::OnBnClickedBtnUnhook()
{
	// ����Ʈ���� ���� ���õ� �ε����� ���Ѵ�
	int nIndex = m_ctrlWindowList.GetSelectionMark();
	if(nIndex<0) return;

	// ������ ID�� ���Ѵ�
	CString strTID = m_ctrlWindowList.GetItemText(nIndex, 2);
	DWORD dwThreadId = (DWORD)_ttoi( strTID );
	if( 0 == dwThreadId ) return;

	// ���̺귯���� �ε��ϰ�
	HMODULE hLib = LoadLibrary(_T("ATCTNR.dll"));
	if( NULL == hLib ) return;

	// ����
	PROC_SetupATContainer pFunc = (PROC_SetupATContainer) ::GetProcAddress(hLib, "SetupATContainer");
	if(pFunc)
	{
		BOOL bRes = pFunc(dwThreadId, FALSE);

		CString msg;
		if(bRes) msg = _T("���� ����!");
		else msg = _T("���� ����");
		this->MessageBox(msg);
	}

	// ���̺귯�� ����
	FreeLibrary(hLib);
}


// ------------------------------------
//	����Ʈ ��Ʈ�� �ʱ�ȭ �Լ�
//
// ------------------------------------
void CAralGoodDlg::InitWindowList()
{
	LV_COLUMN	lvCol;

	SHFILEINFO sfi;
	HIMAGELIST hSmallImageList;


	hSmallImageList = (HIMAGELIST) SHGetFileInfo(_T("C:\\"), 0, 
		&sfi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	ListView_SetImageList(m_ctrlWindowList.GetSafeHwnd(), hSmallImageList, LVSIL_SMALL);

	lvCol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	
	// ���μ��� ��;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cx = 172;
	lvCol.pszText = _T("���μ��� ��");
	lvCol.iSubItem = 0;
	m_ctrlWindowList.InsertColumn(0, &lvCol);

	// PID
	lvCol.fmt = LVCFMT_RIGHT;
	lvCol.cx = 40;
	lvCol.pszText = _T("PID");
	lvCol.iSubItem = 1;
	m_ctrlWindowList.InsertColumn(1, &lvCol);

	// TID
	lvCol.fmt = LVCFMT_RIGHT;
	lvCol.cx = 40;
	lvCol.pszText = _T("TID");
	lvCol.iSubItem = 2;
	m_ctrlWindowList.InsertColumn(2, &lvCol);

	// Address
	lvCol.fmt = LVCFMT_RIGHT;
	lvCol.cx = 70;
	lvCol.pszText = _T("ADR");
	lvCol.iSubItem = 3;
	m_ctrlWindowList.InsertColumn(3, &lvCol);
/*
	// tpBasePri
	lvCol.fmt = LVCFMT_RIGHT;
	lvCol.cx = 50;
	lvCol.pszText = _T("TBP");
	lvCol.iSubItem = 2;
	m_ctrlWindowList.InsertColumn(4, &lvCol);
*/
	// Col �� ����
	ListView_SetExtendedListViewStyle(m_ctrlWindowList.GetSafeHwnd(), LVS_EX_FULLROWSELECT  );
}


BOOL CAralGoodDlg::FillWindowList()
{
	// ������ ����Ʈ�� ����
	m_ctrlWindowList.DeleteAllItems();
	
	// EnumWindows�� ȣ�����ش�
	return EnumWindows(EnumWindowsProc, (LPARAM)this);
}

BOOL CALLBACK CAralGoodDlg::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	CAralGoodDlg* pThis = (CAralGoodDlg*)lParam;
	DWORD dwProcessId = 0;
	DWORD dwThreadId = 0;
	dwThreadId = ::GetWindowThreadProcessId( hwnd, &dwProcessId );

	if( pThis && dwProcessId && dwThreadId )
	{
		// ����Ʈ������ ����ü �غ�
		LV_ITEM	lvItem = {0,};
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
		lvItem.iSubItem = 0;

		// List Index ���ϱ�
		int nIndex = pThis->m_ctrlWindowList.GetItemCount();
		TCHAR szPath[MAX_PATH]	= {0,};   
		memset(szPath, 0, MAX_PATH*sizeof(TCHAR));	
		GetModulePath(dwProcessId, szPath, MAX_PATH);
	
		for(int nItem=0; nItem<nIndex; nItem++)		// �ߺ� PID��� ��ŵ
		{
			CString strCheckPID = pThis->m_ctrlWindowList.GetItemText(nItem, 1);
			CString strCheckTID = pThis->m_ctrlWindowList.GetItemText(nItem, 2);
			if( pThis->mbCheExpert.GetCheck() == 1 )
			{
				if( (DWORD)_ttoi(strCheckPID) == dwProcessId && (DWORD)_ttoi(strCheckTID) == dwThreadId )	return TRUE;
			}
			else
			{
				if( (DWORD)_ttoi(strCheckPID) == dwProcessId )	return TRUE;
			}
		}
	
		// Icon + ���μ����� 
		lvItem.iItem = nIndex;
		lvItem.iImage = GetIconIndex(szPath);
		lvItem.pszText = _tcsrchr( szPath, _T('\\') );
		if(lvItem.pszText == NULL) lvItem.pszText = szPath;
		else lvItem.pszText++;

		if(_tcsicmp(lvItem.pszText, _T("AralTrans.exe")) == 0) return TRUE;	// AralTrans.exe ��� ��ŵ

		BOOL bRes = FALSE;
		BOOL bNoThread = FALSE;
		DWORD dwTidAdr = 0;

		if( pThis->mbCheExpert.GetCheck() == 1 )
		{
			while( dwThreadId )
			{
				CAralProcessAPI::GetThreadStartAddress(dwThreadId, &dwTidAdr);
				bNoThread = FALSE;
				nIndex = pThis->m_ctrlWindowList.GetItemCount();

				for(int nItem=0; nItem<nIndex; nItem++)		// �ߺ� ADR��� ��ŵ
				{
					CString strCheckPID = pThis->m_ctrlWindowList.GetItemText(nItem, 1);
					CString strCheckADR = pThis->m_ctrlWindowList.GetItemText(nItem, 3);
					if( ((DWORD)_ttoi(strCheckPID) == dwProcessId && (DWORD)wcstol(strCheckADR, NULL, 16) == dwTidAdr) || dwTidAdr > 0x50000000)
					//if( ((DWORD)_ttoi(strCheckPID) == dwProcessId && (DWORD)_ttoi(strCheckADR) == dwThreadId) || dwTidAdr > 0x50000000)
					{
						bNoThread = TRUE;
						break;
					}
				}
				if( bNoThread )
				{
					dwThreadId = CAralProcessAPI::GetPrimaryThreadIdNext(dwProcessId, dwThreadId);
					continue;
				}
				if( (nIndex == 0 && dwTidAdr > 0x50000000) ||
					(nIndex == 0 && dwTidAdr < 0) ) break;

				lvItem.iItem = nIndex;
				pThis->m_ctrlWindowList.InsertItem(&lvItem);

				// PID
				CString strPID;
				strPID.Format(_T("%d"), dwProcessId);
				pThis->m_ctrlWindowList.SetItemText(nIndex, 1, strPID);

				// TID
				CString strTID;
				strTID.Format(_T("%d"), dwThreadId);
				pThis->m_ctrlWindowList.SetItemText(nIndex, 2, strTID);

				// ADR
				CString strADR;
				strADR.Format(_T("%08x"), dwTidAdr);
				pThis->m_ctrlWindowList.SetItemText(nIndex, 3, strADR);

				dwThreadId = CAralProcessAPI::GetPrimaryThreadIdNext(dwProcessId, dwThreadId);
			}
		}
		else
		{
			CAralProcessAPI::GetThreadStartAddress(dwThreadId, &dwTidAdr);
			DWORD dwMainThreadId = CAralProcessAPI::GetPrimaryThreadId(dwProcessId, dwTidAdr);
			if( dwMainThreadId ) dwThreadId = dwMainThreadId;

			pThis->m_ctrlWindowList.InsertItem(&lvItem);

			// PID
			CString strPID;
			strPID.Format(_T("%d"), dwProcessId);
			pThis->m_ctrlWindowList.SetItemText(nIndex, 1, strPID);

			// TID
			CString strTID;
			strTID.Format(_T("%d"), dwThreadId);
			pThis->m_ctrlWindowList.SetItemText(nIndex, 2, strTID);

			// ADR
			CString strADR;
			strADR.Format(_T("%08x"), dwTidAdr);
			pThis->m_ctrlWindowList.SetItemText(nIndex, 3, strADR);
		}
	}

	return TRUE;
}

int CAralGoodDlg::GetIconIndex(LPCTSTR lpPath)
{
   SHFILEINFO sfi;

    if ( !SHGetFileInfo(lpPath, 0, &sfi, sizeof(SHFILEINFO),SHGFI_SYSICONINDEX) )
    {
        return -1;
    }

    return sfi.iIcon;
}

// ---------------------------------------------------------
// PID�� ���ϰ�� ã��
//		���޹��� PID�� ���� ��θ� ã�� lpFilename���� �ѱ�
//
// - ���� ����
//		DWORD pid			: PID
//		LPSTR lpFileName	: ���� ��� ��
//		UINT nSize			: �ִ� ����
// - ����
//		BOOL				: ��� ȹ�� TRUE , ���� FALSE
// --------------------------------------------------------		
BOOL CAralGoodDlg::GetModulePath(DWORD pid, LPTSTR lpFilename, int nSize)
{
    // ���μ����� ������ �������� ����ϴ�.
	HANDLE hProcessSnap = (HANDLE)-1;
	//TRACE1(_T("          [MK] CProcessDlg::GetModulePath          PID = [%d]"), pid);

	if(pid == 0)
		return TRUE;
	CString szPath;
	
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	
    if(hProcessSnap == (HANDLE)-1)
    {
		//TRACE1(_T("          [MK] CProcessDlg::GetModulePath          hProcessSnap = [%d]"), hProcessSnap);
        return FALSE;
    }


    MODULEENTRY32 me32;
	memset(&me32, 0,sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);

	TCHAR szTemp[MAX_PATH];
	memset(szTemp, 0, MAX_PATH * sizeof(TCHAR));

    // ù ��° ���(exe ������ ���)�� ��θ� �����մϴ�.
    if (Module32First(hProcessSnap, &me32) == TRUE)
    {
		
		szPath = me32.szExePath;
		
		int nCount = szPath.Find(_T("\\"), 0);
		if(nCount == 0)
		{
			CString szPathTemp;
			szPath.Delete(nCount);
			nCount = szPath.Find(_T("\\"), 0);
			szPathTemp.Format(_T("%%%s%%"),szPath.Left(nCount));
			
			ExpandEnvironmentStrings(szPathTemp ,szTemp, MAX_PATH );
			
			if(szPathTemp.Compare(szTemp))
			{
				szPath.Delete(0, nCount);
				szPath = szTemp + szPath;
			}
			else
			{
				szPath.Delete(0, nCount + 1);
			}
		}
		
		if(nSize < szPath.GetLength())
		{
			_tcsncpy(lpFilename, szPath, nSize-1);
		}
		else
		{
			_tcscpy(lpFilename, szPath);
		}
    }
    CloseHandle (hProcessSnap);
	
    return TRUE;
	
}


//////////////////////////////////////////////////////////////////////////
//
// Ư�� ���μ����� �ڽ� ���μ����� ��ȯ
// (�ڽ� ���μ����� ���ϸ���� �ָ� �� ������ �Ǻ�)
//
//////////////////////////////////////////////////////////////////////////
DWORD CAralGoodDlg::GetChildProcessID(DWORD dwParentPID, LPCTSTR cszChildModulePathName)
{
	DWORD dwChildPID = 0;
	
	try
	{
		// ���μ��� ������ �ڵ��� ����
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwParentPID);
		if(INVALID_HANDLE_VALUE == hProcessSnap) throw -1;
		
		PROCESSENTRY32 pe32;
		memset(&pe32, 0,sizeof(PROCESSENTRY32));
		pe32.dwSize = sizeof(PROCESSENTRY32);
		
		for(BOOL bExist = Process32First(hProcessSnap, &pe32);
			bExist == TRUE;
			bExist = Process32Next(hProcessSnap, &pe32))
		{
			if(pe32.th32ParentProcessID == dwParentPID)
			{
				dwChildPID = pe32.th32ProcessID;
				break;
			}
		}

		CloseHandle (hProcessSnap);

	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}

	return dwChildPID;
}



void CAralGoodDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnBtnHook();
	*pResult = 0;
}

void CAralGoodDlg::OnBtnRefresh() 
{
	// TODO: Add your control notification handler code here
	FillWindowList();
}

void CAralGoodDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	//CTransTestDlg dlg;
	//dlg.DoModal();
	DWORD* pTest1 = NULL;
	__asm mov pTest1, esp;
	TRACE(_T("[ aral1 ] Cur esp = 0x%p \n"), pTest1);

	DWORD dwVal = 0;
	while(1)
	{
		__try
		{
			dwVal = *pTest1;
			pTest1++;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return;
		}
	}
}

// --------------------------------------
// ���� ���~~
// --------------------------------------
void CAralGoodDlg::SetDebugPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

	//SE_DEBUG_NAME
	LookupPrivilegeValue(NULL, _T("SeDebugPrivilege"), &luid);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);

	CloseHandle(hToken);
}

void CAralGoodDlg::OnBnClickedButtonHomepage()
{
	// TODO: Add your control notification handler code here
	ShellExecute(NULL, _T("open"), _T("iexplore.exe"), _T("http://www.aralgood.com/"), NULL, SW_MAXIMIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// ������ �Ⱥ��̰� ����
//
//////////////////////////////////////////////////////////////////////////
void CAralGoodDlg::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	if(m_bShowUI)
	{
		lpwndpos->flags |= SWP_SHOWWINDOW;
	}
	else
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}
	CDialog::OnWindowPosChanging(lpwndpos);
}

//////////////////////////////////////////////////////////////////////////
//
// ��
//
//////////////////////////////////////////////////////////////////////////
void CAralGoodDlg::Hook(
	DWORD dwProcessId,
	DWORD dwAddress,
	DWORD dwDelay,
	LPCTSTR cszAlgorithmName,
	LPCTSTR cszAlgorithmOption,
	LPCTSTR cszTranslatorName,
	LPCTSTR cszTranslatorOption,
	LPCTSTR cszFilters)
{
	// �ɼ� ������ ���� ���� �޸� �غ�
	typedef struct _HOOKING_OPTION
	{
		char szAlgorithmName[256];
		char szAlgorithmOption[4096];
		char szTranslatorName[256];
		char szTranslatorOption[4096];
		char szFilters[4096];
	} HOOKING_OPTION, *LPHOOKING_OPTION;
	
	HMODULE hLib = NULL;
	HANDLE hMapFile = NULL;
	LPHOOKING_OPTION pOption = NULL;

	// ��ŷ ��ŸƮ!
	try
	{
		if(0 == dwProcessId) throw _T("");

		// �־��� �ð���ŭ ���
		if(dwDelay)
		{
			CWaitingDlg wait_dlg;
			wait_dlg.m_dwWaitingTime = dwDelay;
			wait_dlg.DoModal();			
		}

		// ���̺귯���� �ε��ϰ�
		CString strDllPath = CAralFileAPI::GetFileDir( CAralProcessAPI::GetCurrentModulePath() );
		strDllPath += _T("\\ATCTNR.DLL");
		TCHAR szShortPath[MAX_PATH];
		::GetShortPathName((LPCTSTR)strDllPath, szShortPath, MAX_PATH);
		hLib = LoadLibrary(szShortPath);
		if(NULL == hLib) throw _T("�ƶ� Ʈ���� �����̳�(ATCTNR.dll) ����� �ε��� �� �����ϴ�.");

		// ����� �Լ� ���
		PROC_SetupATContainer pFunc = (PROC_SetupATContainer) ::GetProcAddress(hLib, "SetupATContainer");
		if(NULL == pFunc) throw _T("ATCTNR.dll���� SetupATContainer �Լ��� ã�� �� �����ϴ�.");
		
		// ���� �޸� ����
		CString strMemName;
		strMemName.Format(_T("ATSM%d"), dwProcessId);
		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security 
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size 
			sizeof(HOOKING_OPTION),  // buffer size  
			(LPCTSTR)strMemName);    // name of mapping object

		if (hMapFile == NULL) throw _T("���� �޸𸮸� ������ �� �����ϴ�.");

		// �޸𸮿� �ɼǰ� ���
		pOption = (LPHOOKING_OPTION) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HOOKING_OPTION));
		if (pOption == NULL) throw _T("���� �޸𸮿� ������ �� �����ϴ�.");
		
		ZeroMemory(pOption, sizeof(HOOKING_OPTION));

#ifdef UNICODE
		if(cszAlgorithmName) WideCharToMultiByte(949, 0, cszAlgorithmName, -1, pOption->szAlgorithmName, 256, NULL, NULL);
		if(cszAlgorithmOption) WideCharToMultiByte(949, 0, cszAlgorithmOption, -1, pOption->szAlgorithmOption, 4096, NULL, NULL);
		if(cszTranslatorName) WideCharToMultiByte(949, 0, cszTranslatorName, -1, pOption->szTranslatorName, 256, NULL, NULL);
		if(cszTranslatorOption) WideCharToMultiByte(949, 0, cszTranslatorOption, -1, pOption->szTranslatorOption, 4096, NULL, NULL);
		if(cszFilters) WideCharToMultiByte(949, 0, cszFilters, -1, pOption->szFilters, 4096, NULL, NULL);
#else
		if(cszAlgorithmName) strcpy(pOption->szAlgorithmName, cszAlgorithmName);
		if(cszAlgorithmOption) strcpy(pOption->szAlgorithmOption, cszAlgorithmOption);
		if(cszTranslatorName) strcpy(pOption->szTranslatorName, cszTranslatorName);
		if(cszTranslatorOption) strcpy(pOption->szTranslatorOption, cszTranslatorOption);
		if(cszFilters) strcpy(pOption->szFilters, cszFilters);
#endif

		UnmapViewOfFile(pOption);

		// ADR �� ���ϰ�
		for (int j=0; !dwAddress; j++)
		{
			// ������ ����
			FillWindowList();
			
			// ��ġ�ϴ� ���μ������� �ִ��� �˻�
			int cnt = m_ctrlWindowList.GetItemCount();
			for(int i=0; i<cnt; i++)
			{
				// ã�Ҵٸ�
				if( dwProcessId == (DWORD)_ttoi( m_ctrlWindowList.GetItemText(i, 1)) )
				{
					if( (DWORD)wcstol( m_ctrlWindowList.GetItemText(i, 3), NULL, 16 ) < 0x50000000 )
					{
						dwAddress = (DWORD)wcstol( m_ctrlWindowList.GetItemText(i, 3), NULL, 16 );
						break;
					}
				}
			}
			if( dwAddress )
				break;
			Sleep(300);
		}

		// �� ��ġ
		int retry = 0;
		DWORD dwThreadId = CAralProcessAPI::GetPrimaryThreadId(dwProcessId, dwAddress);
		for(retry=0; retry<10 && dwThreadId==NULL; retry++)
		{
			Sleep(500);
			dwThreadId = CAralProcessAPI::GetPrimaryThreadId(dwProcessId, dwAddress);
		}
		if(NULL == dwThreadId) throw _T("�ش� ���μ����� ������ ���̵� ���� �� �����ϴ�.");

		

		BOOL bRes = FALSE;
		DWORD dwThreadIdAll = dwThreadId;
		while( !bRes )
		{
			for(retry=0; dwThreadIdAll; retry++)
			{
				if( bRes == TRUE )
					break;
				bRes = pFunc(dwThreadIdAll, TRUE);
				if( bRes == FALSE )
					dwThreadIdAll = CAralProcessAPI::GetPrimaryThreadIdHook(dwProcessId, dwThreadIdAll);
			}
			if( bRes == TRUE ) break;
			dwThreadIdAll = dwThreadId;
			Sleep(500);
		}
		if(FALSE == bRes) throw _T("DLL ����ǿ� �����߽��ϴ�.");

		// ������ �����
		m_bShowUI = FALSE;
		this->MoveWindow(CRect(0,0,0,0));
		this->ShowWindow(SW_HIDE);

		// ���� ������� ���
		//HANDLE hProcess = ::OpenProcess(SYNCHRONIZE, FALSE, dwProcessId);
		//::WaitForSingleObject(hProcess, INFINITE);
		
		// �ƶ�Ʈ���� ���� ��ȣ ������ ���� ���
		//CString strExitEventName;
		//strExitEventName.Format(_T("ATEE%d"), dwProcessId);
		//HANDLE hExitEvent = ::CreateEvent(NULL, FALSE, FALSE, strExitEventName);
		

		HANDLE hExitSignal[2];
		CString strExitEventName;
		strExitEventName.Format(_T("ATEE%d"), dwProcessId);
		hExitSignal[0] = ::CreateEvent(NULL, FALSE, FALSE, strExitEventName);
		hExitSignal[1] = ::OpenProcess(SYNCHRONIZE, FALSE, dwProcessId);

		::WaitForMultipleObjects(2, hExitSignal, FALSE, INFINITE);
		::CloseHandle(hExitSignal[0]);

		// ATData ����
		if (this->m_bUseATDataZip)
			this->ArchiveATData();

		// �ƶ� Ʈ���� ����
		this->PostMessage(WM_CLOSE);

	}
	// ��ŷ ����������
	catch(LPCTSTR cszErrMsg)
	{
		CString msg;
		msg.Format(_T("%s \r\n�����ڵ� : %d"), cszErrMsg, GetLastError());
		if(cszErrMsg[0]) this->MessageBox((LPCTSTR)msg, _T("��ŷ ����"));

		// ������ ����
		FillWindowList();

		// ������ ���̱�
		if(m_bShowUI == FALSE)
		{
			m_bShowUI = TRUE;
			this->MoveWindow(&m_rtOrigSize);
			this->ShowWindow(SW_NORMAL);
			this->CenterWindow();
		}

		m_dwPID = 0;
		m_dwADR = 0;
		m_strHookName    = _T("");
		m_strHookOption  = _T("");
		m_strTransName   = _T("");
		m_strTransOption = _T("");
		m_hHookWorkThread = NULL;
	}

	FreeLibrary(hLib);

	// ���� �޸� ����
	CloseHandle(hMapFile);

}

// ATData.zip ���� ������ �����ɴϴ�
BOOL CAralGoodDlg::ExtractATData(void)
{

	// ����: �� ��ƾ�� ���������� ���� ��� �Ǿ����� �ʽ��ϴ�. 2011.10.22 ���� �������� ���� �÷�������
	//       �⸮�⸮�ۿ� ���� �⸮�⸮ ��ũ��Ʈ�� ���������� ���� �������� ���� �÷������� ���´ٸ�
	//       �� ��ƾ�� �����ؾ� �մϴ�.

	if (m_strGameDirectory.IsEmpty())
	{
		MessageBox(_T("���� ���丮�� �����Ǿ����� �ʽ��ϴ�"));
		return FALSE;
	}

	CString strATDataZipPath=m_strGameDirectory+_T("\\ATData.zip");
	CString strATDataDirectory=m_strGameDirectory+_T("\\ATData\\");
	
	// ATData.zip �� ������ �� �ʿ� ����
	if (!PathFileExists(strATDataZipPath))
		return FALSE;

	CZipArchive cATDataZip;
	CZipFileHeader *pfhInfo=NULL;
	FILETIME ftimeZippedFile;
	CString strZippedFileName;

	HANDLE hFile=INVALID_HANDLE_VALUE;
	FILETIME ftimeC, ftimeA, ftimeW;

	bool bNeedToExtract;

	int i, nTotalIndex;

	try {

		// zip ������ ����
		cATDataZip.Open(strATDataZipPath, CZipArchive::zipOpenReadOnly);

		// ����Ǯ�� ������ ������ ����
		cATDataZip.SetRootPath(strATDataDirectory);

		// �� �������� ������ �о�ͼ�
		nTotalIndex = cATDataZip.GetCount();

		for (i=0; i < nTotalIndex; i++)
		{
			bNeedToExtract=false;

			// ������ ���� ������ �о�ͼ���
			pfhInfo=cATDataZip.GetFileInfo(i);

			// ���丮�� �н�
			if (pfhInfo->GetSystemAttr() & FILE_ATTRIBUTE_DIRECTORY) continue;

			// ������ Script ���丮 �ȿ� ������ �н� (�⸮�⸮)
			strZippedFileName=pfhInfo->GetFileName();
			if (strZippedFileName.Left(7).Compare(_T("Script\\")) == 0) continue;

			// ���� ���� ������ ������ �̸��� ������ ����
			hFile=CreateFile(strATDataDirectory+strZippedFileName, GENERIC_READ, FILE_SHARE_READ, 
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				// ������ �ۼ��ð��� �о�� ����
				TimetToFileTime(pfhInfo->GetTime(), &ftimeZippedFile);	// ��������
				GetFileTime(hFile, &ftimeC, &ftimeA, &ftimeW);	// ��������

				// �������� ���� ������ �������Ϻ��� �ֱ��̸� �ش������� ������ Ǭ��.
				if (CompareFileTime(&ftimeZippedFile, &ftimeW) > 0)
					bNeedToExtract=true;

				CloseHandle(hFile);
				hFile=INVALID_HANDLE_VALUE;

			}
			else
			{
				// ���� ����? ���� Ǯ��.
				bNeedToExtract=true;
			}

			// ���� ����
			if (bNeedToExtract)
			{
				cATDataZip.ExtractFile(i, strATDataDirectory);
			}

		}

		cATDataZip.Close();

	}
#ifdef _ZIP_IMPL_STL
	catch (CZipException &ex)
	{
		// CZipArchive �� STL ������ ��� �������� ���� �ڵ鷯

		TCHAR szBuf[256], szWhy[200];
		ex.GetErrorMessage(szWhy, 200);
		wsprintf(szBuf, _T("����(%d:%d:%s) %s"), ex.m_iCause, ex.m_iSystemError, ex.m_szFileName, szWhy);
		MessageBox(szBuf, _T("AralTrans:ExtractATData"), MB_OK | MB_ICONEXCLAMATION);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		cATDataZip.Close(CZipArchive::afAfterException);
		return FALSE;
	}
#else
	catch(CZipException *pex)
	{
		// CZipArchive �� MFC ������ ��� �������� ���� �ڵ鷯
		TCHAR szBuf[256], szWhy[200];
		pex->GetErrorMessage(szWhy, 200);
		wsprintf(szBuf, _T("����(%d:%d:%s) %s"), pex->m_iCause, pex->m_iSystemError, pex->m_szFileName, szWhy);
		MessageBox(szBuf, _T("AralTrans:ExtractATData"), MB_OK | MB_ICONEXCLAMATION);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		cATDataZip.Close(CZipArchive::afAfterException);
		return FALSE;
	}
#endif


	return TRUE;
}

// ATData.zip�� ������ �����մϴ�
BOOL CAralGoodDlg::ArchiveATData(void)
{
	// ����: �� ��ƾ�� ���������� ���� ��� �Ǿ����� �ʽ��ϴ�. 2011.10.22 ���� �������� ���� �÷�������
	//       �⸮�⸮�ۿ� ���� �⸮�⸮ ��ũ��Ʈ�� ���������� ���� �������� ���� �÷������� ���´ٸ�
	//       �� ��ƾ�� �����ؾ� �մϴ�.

	if (m_strGameDirectory.IsEmpty())
	{
		MessageBox(_T("���� ���丮�� �����Ǿ����� �ʽ��ϴ�"));
		return FALSE;
	}

	CString strATDataZipPath=m_strGameDirectory+_T("\\ATData.zip");
	CString strATDataDirectory=m_strGameDirectory+_T("\\ATData\\");

	// ATData ���丮�� ������ �ƿ� �� �ʿ䰡 ����
	if (!PathIsDirectory(strATDataDirectory))
		return FALSE;

	CZipArchive cATDataZip;
	CZipFileHeader *pfhInfo=NULL;
	FILETIME ftimeZippedFile;

	HANDLE hFile=INVALID_HANDLE_VALUE;
	CString strFileName;
	FILETIME ftimeC, ftimeA, ftimeW;

	CFileFind cFileFind;
	BOOL bFileExist;

	bool bNeedToArchive;

	int i;

	try {

		// zip ������ ����
		if (PathFileExists(strATDataZipPath))
			cATDataZip.Open(strATDataZipPath, CZipArchive::zipOpen);
		else
			cATDataZip.Open(strATDataZipPath, CZipArchive::zipCreate);
		
		// ������ ������ �����ϰ�
		cATDataZip.SetRootPath(strATDataDirectory);

		// ���� ã�� ����
		bFileExist=cFileFind.FindFile(strATDataDirectory+_T("*.*"));

		while(bFileExist)
		{
			bFileExist=cFileFind.FindNextFile();
			
			// ���丮�� �н�
			if (cFileFind.IsDirectory() || cFileFind.IsDots() ) continue;

			// ������ ������
			strFileName=cFileFind.GetFileName();

			bNeedToArchive=false;

			// �������Ͽ� �ش� ������ �ִ��� ã�Ƽ�
			i=cATDataZip.FindFile(strFileName);

			if (i != ZIP_FILE_INDEX_NOT_FOUND)
			{
				// ������ ���� ������ �о�ͼ���
				pfhInfo=cATDataZip.GetFileInfo(i);
				
				// ���� ������ ������ ����
				hFile=CreateFile(strATDataDirectory+strFileName, GENERIC_READ, FILE_SHARE_READ, 
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					// ������ �ۼ��ð��� �о�� ����
					// time_t(1��)�� FILETIME(100������)�� ���е� ������ ������ ���� ���������� ��� ������ �Ǵ� ������ �߻�
					// ���������� �ð��� �� 2�� ���� �÷��� �ʿ���� ������ ����
					// ������ 2�ʳ��� ���� �����ġ�� �� �θ����� �ʰ��� -_-
					TimetToFileTime(pfhInfo->GetTime()+2, &ftimeZippedFile);	// ��������
					GetFileTime(hFile, &ftimeC, &ftimeA, &ftimeW);	// ��������

					// �������� ���� ���Ϻ��� ���������� �ֱ��̸� �������Ϸ� ��ü�����Ѵ�.
					if (CompareFileTime(&ftimeZippedFile, &ftimeW) < 0)
					{
						cATDataZip.RemoveFile(i);
						bNeedToArchive=true;
					}

					CloseHandle(hFile);
					hFile=INVALID_HANDLE_VALUE;
				}
			}
			else
			{
				// ������ �߰� 
				bNeedToArchive=true;
			}


			if (bNeedToArchive)
			{
				cATDataZip.AddNewFile(cFileFind.GetFilePath(), 1, false);
			}
		}


		cATDataZip.Close(CZipArchive::afNoException, true);


		// ����� ATData ������ ���� ����
		if (m_bDeleteLocalFiles)
		{

			// ���� �� ������ ã�Ƽ�
			bFileExist=cFileFind.FindFile(strATDataDirectory+_T("*.*"));
			while (bFileExist)
			{
				bFileExist=cFileFind.FindNextFile();

				if ( cFileFind.IsDots() || cFileFind.IsDirectory() ) continue;

				// ����
				DeleteFile(cFileFind.GetFilePath());
			}

			// ������ ������ ������ ������ ����
			if (PathIsDirectoryEmpty(strATDataDirectory))
				RemoveDirectory(strATDataDirectory);
		}

	}
#ifdef _ZIP_IMPL_STL
	catch (CZipException &ex)
	{
		// CZipArchive �� STL ������ ��� �������� ���� �ڵ鷯

		TCHAR szBuf[256], szWhy[200];
		ex.GetErrorMessage(szWhy, 200);
		wsprintf(szBuf, _T("����(%d:%d:%s) %s"), ex.m_iCause, ex.m_iSystemError, ex.m_szFileName, szWhy);
		MessageBox(szBuf, _T("AralTrans:ArchiveATData"), MB_OK | MB_ICONEXCLAMATION);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		cATDataZip.Close(CZipArchive::afAfterException);
		return FALSE;
	}
#else
	catch(CZipException *pex)
	{
		// CZipArchive �� MFC ������ ��� �������� ���� �ڵ鷯
		TCHAR szBuf[256], szWhy[200];
		pex->GetErrorMessage(szWhy, 200);
		wsprintf(szBuf, _T("����(%d:%d:%s) %s"), pex->m_iCause, pex->m_iSystemError, pex->m_szFileName, szWhy);
		MessageBox(szBuf, _T("AralTrans:ArchiveATData"), MB_OK | MB_ICONEXCLAMATION);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		cATDataZip.Close(CZipArchive::afAfterException);
		return FALSE;
	}
#endif

	return TRUE;
}

void CAralGoodDlg::OnBnClickedBtnOption()
{
	COptionDlg dlg;

	dlg.m_nAutoUpdate=m_dwAutoUpdate;
	dlg.m_bUseATDataZip=m_bUseATDataZip;
	dlg.m_bDeleteLocalFiles = m_bDeleteLocalFiles;
	dlg.m_bAppLocAutoConfirm = m_bAppLocAutoConfirm;

	if (dlg.DoModal() == IDOK)
	{
#ifdef TEST_VERSION
		if (m_dwAutoUpdate != (DWORD)dlg.m_nAutoUpdate)
			MessageBox(_T("�׽�Ʈ ���������� �ڵ� ������Ʈ ������ ������ �� ������ ���� �ڵ� ������Ʈ�� ���� �ʽ��ϴ�."), _T("����"), MB_OK | MB_ICONINFORMATION);
#endif
		m_dwAutoUpdate=(DWORD)dlg.m_nAutoUpdate;
		m_bUseATDataZip = dlg.m_bUseATDataZip;
		m_bDeleteLocalFiles = dlg.m_bDeleteLocalFiles;
		m_bAppLocAutoConfirm = dlg.m_bAppLocAutoConfirm;

		CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("AutoUpdate"), m_dwAutoUpdate);
		TCHAR szExeFileName[MAX_PATH] = {0,};
		if( ::GetModuleFileName( NULL, szExeFileName, MAX_PATH ) )
		{
			CString strIniFileName=szExeFileName;
			strIniFileName.Replace(_T(".exe"), _T(".ini"));

			WritePrivateProfileString(_T("AralTrans"), _T("UseATDataZip"),(m_bUseATDataZip?_T("1"):_T("0")), strIniFileName);
			WritePrivateProfileString(_T("AralTrans"), _T("DeleteLocalFiles"),(m_bDeleteLocalFiles?_T("1"):_T("0")), strIniFileName);
			WritePrivateProfileString(_T("AralTrans"), _T("AppLocAutoConfirm"),(m_bAppLocAutoConfirm?_T("1"):_T("0")), strIniFileName);
		}


	}
}

void CAralGoodDlg::OnBnClickedBtnAbout()
{
	CAralAboutDlg dlg;
	dlg.DoModal();
}

void CAralGoodDlg::OnBnClickedCheckExpert()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	FillWindowList();
}

void CAralGoodDlg::OnHelp()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CString path = "help.chm";
	::ShellExecute(::GetDesktopWindow(), L"open", path , 0, 0, SW_SHOWDEFAULT);
}

void CAralGoodDlg::OnBnClickedBtnDir()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"���� ���� (*.exe)|*.exe||" );
	
	if( fd.DoModal() != IDCANCEL )
	{
		SetDlgItemText(IDC_DIR, fd.GetPathName());
		m_ctrlWindowList.SetSelectionMark(-1);
	}
}

void CAralGoodDlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int idx = pNMListView -> iItem;

	CString str;
	CString strPID = m_ctrlWindowList.GetItemText(idx, 1);
	DWORD dwProcessId = (DWORD)_ttoi( strPID );

	TCHAR szGamePath[MAX_PATH];
	if (GetModulePath(dwProcessId, szGamePath, MAX_PATH))
		SetDlgItemText(IDC_DIR, szGamePath);
	*pResult = 0;
}

void CAralGoodDlg::BinaryPatch( HANDLE hProcess )
{
	// ���̳ʸ� ��ġ
	CString regCount;
	int nCount = 0;
	int nHookPatch = (int)CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryPatch"));
	while( nHookPatch > nCount )
	{
		nCount ++;
		regCount.Format(_T("Binary%d"),nCount);
		unsigned char* cBuff = (unsigned char*)malloc(0x10000);
		int bufLen = CRegistryMgr::RegReadBINARY(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount, cBuff);
		regCount.Format(_T("BinaryAdr%d"),nCount);
		DWORD dModule = CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount);
		regCount.Format(_T("BinaryName%d"),nCount);
		CString str = CRegistryMgr::RegRead(_T("HKEY_CURRENT_USER\\Software\\AralGood"), regCount);
		if( str != _T("") )
			dModule += (DWORD) GetModuleHandle(str);
		
		DWORD fOldProtect = 0;
		BYTE byChar = 0;
		int i = 0;
		VirtualProtectEx(hProcess, (LPVOID) dModule, bufLen, PAGE_READWRITE, &fOldProtect);
		while(i < bufLen)
		{
			byChar = cBuff[i];
			WriteProcessMemory(hProcess, (LPVOID) dModule, &byChar, 1, 0);
			i++;
			dModule++;
		}
		VirtualProtectEx(hProcess, (LPVOID)(dModule - i), bufLen, fOldProtect, &fOldProtect);

		free(cBuff);
	}
	CRegistryMgr::RegWriteDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("BinaryRestart"), nCount);
}