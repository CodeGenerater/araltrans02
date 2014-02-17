// AralUpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AralUpdater.h"
#include "AralUpdaterDlg.h"
#include "HttpDownloader/HttpDownloader.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "AralFileAPI/AralFileAPI.h"
#include "AralProcessAPI/AralProcessAPI.h"
#include <afxinet.h>
#include <wingdi.h>
#include <winbase.h>
#include "unzip.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_UPDATE_INFO_URL	_T("http://www.aralgood.com/update_files/AralTransUpdate.ini")
//#define UPDATE_INFO_FILE_NAME	_T("update.ini")


/////////////////////////////////////////////////////////////////////////////
// CAralUpdaterDlg dialog

CAralUpdaterDlg::CAralUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAralUpdaterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAralUpdaterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_BkBrush.CreateSolidBrush(RGB(255, 255, 255));	// �귯�� ���� (�ڽ��� ����)
	m_nAlpha = 0;
	m_pRgn = NULL;
	m_pBG = NULL;
	m_hUserDll = NULL;
	m_pThread = NULL;
	m_pChildThread = NULL;
	m_paWorkList = NULL;
	m_nTotalSize = 0;
	m_nTranSize = 0;
	m_bStop = FALSE;
	m_BoldFont.CreateFont( 11,0,0,0,FW_NORMAL,FALSE,FALSE,0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							ANTIALIASED_QUALITY,
							DEFAULT_PITCH,
							_T("tahoma"));

	// Program Files ����
	m_strProgramFilesFolder = CRegistryMgr::RegRead( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"), _T("ProgramFilesDir"));
	if(m_strProgramFilesFolder.IsEmpty()) m_strProgramFilesFolder = _T("C:\\Program Files");
	
	// System32 ����
	TCHAR szDir[MAX_PATH];
	ZeroMemory(szDir, MAX_PATH);
	::GetSystemDirectory(szDir, MAX_PATH);
	m_strSystemFolder = szDir;

	// �������� ����
	ZeroMemory(szDir, MAX_PATH);
	::GetWindowsDirectory(szDir, MAX_PATH);
	m_strWindowsFolder = szDir;

	
	// Wow64 ���μ����ΰ�
	m_bIsWow64 = FALSE;
	typedef BOOL (__stdcall *PROC_IsWow64Process)(HANDLE,PBOOL);	
	PROC_IsWow64Process pFunc = (PROC_IsWow64Process) GetProcAddress( GetModuleHandle(_T("kernel32.dll")), "IsWow64Process" );
	if(pFunc) pFunc( ::GetCurrentProcess(), &m_bIsWow64 );

}

BOOL CAralUpdaterDlg::SetWow64Redirection(BOOL bEnable)
{	
	PVOID OldValue = NULL;
	BOOL bRet = FALSE;

	typedef BOOL (__stdcall *PROC_Wow64FsRedirection)(PVOID);
	
	PROC_Wow64FsRedirection pFunc = NULL;
	
	// �����̷��� �Ҵ�
	if( bEnable )
	{
		pFunc = (PROC_Wow64FsRedirection) GetProcAddress( GetModuleHandle(_T("kernel32.dll")), "Wow64RevertWow64FsRedirection" );
		if(pFunc) bRet = pFunc (OldValue);
	}

	// �����̷��� ����
	else
	{
		pFunc = (PROC_Wow64FsRedirection) GetProcAddress( GetModuleHandle(_T("kernel32.dll")), "Wow64DisableWow64FsRedirection" );
		if(pFunc) bRet = pFunc (&OldValue);
	}

	return bRet;
}


void CAralUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAralUpdaterDlg)
	DDX_Control(pDX, IDC_TRAN_SIZE, m_ctrlTranText);
	DDX_Control(pDX, IDC_COMMENT, m_ctrlCommentText);
	DDX_Control(pDX, IDC_PERCENT, m_ctrlPercentText);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAralUpdaterDlg, CDialog)
	//{{AFX_MSG_MAP(CAralUpdaterDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAralUpdaterDlg message handlers

BOOL CAralUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strServerINIData;
	if( PrepareUpdate(strServerINIData) == FALSE )
	{
		CleanupUpdate();
		PostQuitMessage(0);
		OnOK();
		return FALSE;
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	CenterWindow();
	// ��Ų ��Ʈ�� �ε� (�̰��� �����ϸ� '��Ų����'���� �����Ҽ�����)
	m_pBG = new CBitmap;
	m_pBG->LoadBitmap(IDB_BG_INSTALL);
	// ��Ʈ����
	GetDlgItem(IDC_COMMENT)->SetFont(&m_BoldFont);
	GetDlgItem(IDC_TRAN_SIZE)->SetFont(&m_BoldFont);
	GetDlgItem(IDC_PERCENT)->SetFont(&m_BoldFont);
	// �ձ� �𼭸�
// 	CRect rcWindow;
// 	GetWindowRect(rcWindow);
// 	m_pRgn = new CRgn;
// 	m_pRgn->CreateRoundRectRgn (0, 0, rcWindow.Width() + 1, rcWindow.Height() + 1, 15, 15);
// 	SetWindowRgn(*m_pRgn, TRUE);

	// ���� ����
 	m_hUserDll = ::LoadLibrary(_T("USER32.dll"));
 	SetWindowLong(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	m_nAlpha = 100;
//	SetTransparent(m_hWnd, 0, 0, LWA_ALPHA );

	SetTimer(0, 50, NULL);
	
	// ���� ������ �� �� 2 ������ ����
 	m_pThread = ::AfxBeginThread(MainThread, this);
 	m_pChildThread = ::AfxBeginThread(StaticUpdateThread, this);
	//���α׷������� ������ �� ���� ����
	m_ctrlProgress.SendMessage(PBM_SETBKCOLOR, (WPARAM)0, (LPARAM)(COLORREF)RGB(230,230,230) );
	m_ctrlProgress.SendMessage(PBM_SETBARCOLOR, (WPARAM)0, (LPARAM)(COLORREF)RGB(130,190,255) ); //RGB(0x32,0x8C,0xFF)
	
	// ��� ��ư
// 	m_btnCancel.Create(NULL, BS_OWNERDRAW | WS_VISIBLE | WS_CHILD, CRect(120,170,20,20), this, IDC_WORK_STOP);
// 	m_btnCancel.LoadBitmaps(IDB_BTN_CANCEL_NORMAL, IDB_BTN_CANCEL_DOWN);
// 	m_btnCancel.SizeToContent();

	CStringA systemDir = _T("");
	CStringA FindFileName = _T("");

	::GetSystemDirectoryA(systemDir.GetBuffer(255), 255);
	FindFileName.Format("%s\\%s", systemDir, L"hhctrl.ocx");
	HINSTANCE h = ::LoadLibraryA(FindFileName);
	if (h != NULL)
		FARPROC pFunc = ::GetProcAddress(h,"DllRegisterServer");

	FILE *pData;
	pData=fopen("help.chm","rb");
	CString strdir  = _T("help.chm");
	CString strurl = _T("http://www.aralgood.com/update_files/help.chm");
	if(pData==NULL){
		DeleteUrlCacheEntryW(strurl);
		URLDownloadToFileW(NULL,strurl,strdir,NULL,NULL);
		DeleteUrlCacheEntryW(strurl);
	}
	else
	{
		fseek(pData,0,SEEK_END);
		if(ftell(pData) <= 0)
		{
			fclose(pData);
			DeleteUrlCacheEntryW(strurl);
			URLDownloadToFileW(NULL,strurl,strdir,NULL,NULL);
			DeleteUrlCacheEntryW(strurl);
		}
		else
			fclose(pData);
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CAralUpdaterDlg::PrepareUpdate(CString &strINIData)
{
	BOOL bRetVal = FALSE;

	strINIData.Empty();
	CString strServerINIData;
	
	try
	{
		// ������Ʈ���� ��ϵǾ� �ִ� update.ini ������ URL�ּҸ� ������
		CString strUpdateInfoURL = CRegistryMgr::RegRead( _T("HKEY_CURRENT_USER\\Software\\AralGood\\Update"), _T("UpdateInfoURL") );
		if(strUpdateInfoURL.IsEmpty())
		{
			strUpdateInfoURL = DEFAULT_UPDATE_INFO_URL;
			CRegistryMgr::RegWrite( _T("HKEY_CURRENT_USER\\Software\\AralGood\\Update"), _T("UpdateInfoURL"), strUpdateInfoURL );
		}

		// �������� �ֽ� ������Ʈ ����
		if( CHttpDownloader::Http_Download_To_Buffer(strUpdateInfoURL, strServerINIData, NULL) != 0 ) 
			throw _T("[ aral1 ] [Updater] Err 3 \n");

		strINIData = strServerINIData;

		// �Ľ�
		if( m_ININewVer.Parse(strServerINIData) == FALSE )
			throw _T("[ aral1 ] [Updater] Err 4 \n");

		CININode* pNode = m_ININewVer.FindNode(_T("AppInfo"));

		if( NULL == pNode ) 
			throw _T("[ aral1 ] [Updater] Err 6 \n");

		// ���� ������Ʈ ���� URL�� ����Ǿ��ٸ� �����̷��� ó��
		CString strNewInfoURL = pNode->GetAttribute(_T("UpdateInfoURL"));
		TRACE( _T("[ aral1 ] [Updater] OLD URL : '%s' \n"), (LPCTSTR)strUpdateInfoURL );
		TRACE( _T("[ aral1 ] [Updater] NEW URL : '%s' \n"), (LPCTSTR)strNewInfoURL );
		if( strUpdateInfoURL.CompareNoCase(strNewInfoURL) != 0 )
		{
			TRACE( _T("[ aral1 ] [Updater] URL redirect to '%s' \n"), (LPCTSTR)strNewInfoURL );

			m_ININewVer.Clear();
			
			// ������Ʈ���� ��ϵǾ� �ִ� update.ini ������ ���ο� URL�ּҸ� ���
			CRegistryMgr::RegWrite( _T("HKEY_CURRENT_USER\\Software\\AralGood\\Update"), _T("UpdateInfoURL"), strNewInfoURL );
			return PrepareUpdate(strINIData);
		}


		CString strServerVersion = pNode->GetAttribute(_T("Version"));

		TRACE( _T("[ aral1 ] Latest Version : '%s' \n"), strServerVersion );


		// ���û��� ���� ������Ʈ ����
		CString strRegPath = pNode->GetAttribute(_T("RegPath"));

		/*
		if( !strRegPath.IsEmpty() )
		{
		// Ŭ���̾�Ʈ ��ġ ���丮
		m_strClientFolder = CRegistryMgr::RegRead( strRegPath, "AppDir" );
		// 		if( m_strClientFolder.IsEmpty() )
		// 		{		
		// 			m_strClientFolder =  ParseSystemVariable( pNode->GetAttribute("DefaultAppDir") );
		// 			if( m_strClientFolder.IsEmpty() )
		// 			{
		// 				TRACE( _T("[ aral1 ] [Updater] Err 8 \n") );
		// 				return FALSE;
		// 			}
		// 
		// 		}
		}

		if( m_strClientFolder.IsEmpty() ) m_strClientFolder = strCurDir;
		*/

		m_strClientFolder = CAralFileAPI::GetFileDir( CAralProcessAPI::GetCurrentModulePath() );

		CString strLocalVersion = CRegistryMgr::RegRead( strRegPath, _T("Version") );

		TRACE( _T("[ aral1 ] Current App Version : '%s' \n"), strLocalVersion );

		// Ŭ���̾�Ʈ�� ���ų� ������ ������ �ٿ�ε�
		m_paWorkList = new CPtrArray();
		AddCommonFileWork(m_paWorkList);
		AddDriverFileWork(m_paWorkList);

		// �޾ƾ� �� ������ �Ѱ��� ������
		if( m_paWorkList->GetSize() == 0 )
			throw _T("[ aral1 ] [Updater] No more Update File \n");

		bRetVal = TRUE;
	}
	catch (LPCTSTR strErr)
	{
		TRACE(strErr);
		strErr = NULL;
	}


	return bRetVal;
}

BOOL CAralUpdaterDlg::AddCommonFileWork(CPtrArray* pWorkList)
{
	// �Ϲ� ����
	int cnt = 0;
	CString strNodeName;
	
	strNodeName.Format(_T("File%d"), cnt);
	CININode* pNode = m_ININewVer.FindNode(strNodeName);

	while( pNode )
	{
		CString strVersion   = pNode->GetAttribute(_T("Version"));
		CString strFilename  = ParseSystemVariable( pNode->GetAttribute(_T("Location")) );
		CString strFileURL   = pNode->GetAttribute(_T("DownloadURL"));
		
		CFileFind find;
		
		// ������ ���ų� �ֽŹ����� �ٸ��� ���� �ٿ�ε� ����
		if( find.FindFile( strFilename ) == FALSE 
			|| ( !strVersion.IsEmpty() && strVersion != GetFileVersion(strFilename) )
		)
		{
			if(strFilename.Mid(strFilename.ReverseFind(_T('\\')) + 1) == _T("help.chm"))
				CRegistryMgr::RegWrite( _T("HKEY_CURRENT_USER\\Software\\AralGood\\Update"), _T("HelpVersion"), strVersion );
			CFileWork* work = new CFileWork();
			work->m_strURL = strFileURL;
			work->m_strLocalPath = strFilename;
			pWorkList->Add(work);
			TRACE( _T("[ aral1 ] Download '%s' to '%s' \n"), work->m_strURL, work->m_strLocalPath );
		}

		// ���� ����
		cnt++;
		strNodeName.Format(_T("File%d"), cnt);
		pNode = m_ININewVer.FindNode(strNodeName);

	}

	return TRUE;
}

BOOL CAralUpdaterDlg::AddDriverFileWork(CPtrArray* pWorkList)
{
	int cnt = 0;
	CString strNodeName = _T("");
	CININode* pNode = NULL;
	
	// x86 driver ����
	if( m_bIsWow64 == FALSE )
	{

		strNodeName.Format(_T("x86Driver%d"), cnt);
		pNode = m_ININewVer.FindNode(strNodeName);

		// ����̹��� �߰�
		while( pNode )
		{
			
			CString strVersion   = pNode->GetAttribute(_T("Version"));
			CString strFileURL   = pNode->GetAttribute(_T("DownloadURL"));
			int nStrIdx = strFileURL.ReverseFind(_T('/'));
			if(nStrIdx>0)
			{
				CString strFilename  = m_strWindowsFolder + _T("\\System32\\drivers\\") + strFileURL.Mid(nStrIdx+1);				
				CFileFind find;
				
				// ������ ���ų� �ֽŹ����� �ٸ��� ���� �ٿ�ε� ����
				if( find.FindFile( strFilename ) == FALSE 
					|| ( !strVersion.IsEmpty() && strVersion != GetFileVersion(strFilename) )
				)
				{
					CFileWork* work = new CFileWork();
					work->m_strURL = strFileURL;
					work->m_strLocalPath = strFilename;
					pWorkList->Add(work);
					TRACE( _T("[ aral1 ] Download '%s' to '%s' \n"), work->m_strURL, work->m_strLocalPath );
				}
			}


			// ���� ����
			cnt++;
			strNodeName.Format(_T("x86Driver%d"), cnt);
			pNode = m_ININewVer.FindNode(strNodeName);

		}
		

	}
	// x64 driver ����
	else
	{

		int cnt = 0;
		strNodeName.Format(_T("x64Driver%d"), cnt);
		pNode = m_ININewVer.FindNode(strNodeName);

		// 64��Ʈ�� �����̷��� ����
		PVOID OldValue = NULL;
		BOOL bRet = SetWow64Redirection(FALSE);

		// ����̹��� �߰�
		while( pNode )
		{
			CString strVersion   = pNode->GetAttribute(_T("Version"));
			CString strFileURL   = pNode->GetAttribute(_T("DownloadURL"));
			int nStrIdx = strFileURL.ReverseFind('/');
			if(nStrIdx>0)
			{
				CString strFilename1  = m_strWindowsFolder + _T("\\System32\\drivers\\") + strFileURL.Mid(nStrIdx+1);
				CFileFind find;
				
				// ������ ���ų� �ֽŹ����� �ٸ��� ���� �ٿ�ε� ����
				if( find.FindFile( strFilename1 ) == FALSE 
					|| ( !strVersion.IsEmpty() && strVersion != GetFileVersion(strFilename1) )
				)
				{
					CFileWork* work = new CFileWork();
					work->m_strURL = strFileURL;
					work->m_strLocalPath = strFilename1;
					pWorkList->Add(work);
					TRACE( _T("[ aral1 ] Download '%s' to '%s' \n"), work->m_strURL, work->m_strLocalPath );
				}

				CString strFilename2  = m_strWindowsFolder + _T("\\SysWow64\\drivers\\") + strFileURL.Mid(nStrIdx+1);
				
				// ������ ���ų� �ֽŹ����� �ٸ��� ���� �ٿ�ε� ����
				if( find.FindFile( strFilename2 ) == FALSE 
					|| ( !strVersion.IsEmpty() && strVersion != GetFileVersion(strFilename2) )
				)
				{
					CFileWork* work = new CFileWork();
					work->m_strURL = strFileURL;
					work->m_strLocalPath = strFilename2;
					pWorkList->Add(work);
					TRACE( _T("[ aral1 ] Download '%s' to '%s' \n"), work->m_strURL, work->m_strLocalPath );
				}
			}


			// ���� ����
			cnt++;
			strNodeName.Format(_T("x64Driver%d"), cnt);
			pNode = m_ININewVer.FindNode(strNodeName);

		}

		// �����̷��� �Ҵ�
		bRet = SetWow64Redirection(TRUE);
	}

	return TRUE;
}


BOOL CAralUpdaterDlg::CleanupUpdate()
{
	if( m_paWorkList )
	{
		// �۾�����Ʈ ��� ���ҵ� ����
		int i, count = m_paWorkList->GetSize();
		for(i=0; i<count; i++)
		{
			if(m_paWorkList->GetAt(i)) delete (CFileWork*) m_paWorkList->GetAt(i);
		}
		m_paWorkList->RemoveAll();
		delete m_paWorkList;
		m_paWorkList = NULL;

	}

	DoRegistryWork();
	
	// ������Ʈ���� ������Ʈ ���� ����
	CININode* pNode = m_ININewVer.FindNode(_T("AppInfo"));
	if( pNode )
	{
		// ���û��� ���� ������Ʈ ����
		CString strRegPath = pNode->GetAttribute(_T("RegPath"));
		if( !strRegPath.IsEmpty() )
		{
			BOOL bRegWrite = CRegistryMgr::RegWrite( strRegPath, _T("AppDir"), m_strClientFolder );
			bRegWrite = CRegistryMgr::RegWrite( strRegPath, _T("Name"), pNode->GetAttribute(_T("Name")) );
			bRegWrite = CRegistryMgr::RegWrite( strRegPath, _T("Version"), pNode->GetAttribute(_T("Version")) );
		}

		// ���䷱ ����
		CString strAutoRun = ParseSystemVariable( pNode->GetAttribute(_T("AutoRun")) );
		if( !strAutoRun.IsEmpty() )
		{
			// exe ���� ����
 			STARTUPINFO si={0,};
 			PROCESS_INFORMATION pi;			
 			BOOL bRes = ::CreateProcess( NULL, (LPTSTR)(LPCTSTR)strAutoRun, NULL,NULL,FALSE,0,NULL, NULL, &si , &pi );
			TRACE( _T("[ aral1 ] Auto Run : %s -> Result:%d \n"), strAutoRun, bRes );
			//::ShellExecute(NULL, "open", (LPSTR)(LPCTSTR)strAutoRun, NULL, NULL, SW_SHOWNORMAL);
		}
	}

	
	return TRUE;
}






// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAralUpdaterDlg::OnPaint() 
{
	/*
	CPaintDC dc(this); // device context for painting
	CDC* pWDC = GetWindowDC();
	CRect rect;
	GetWindowRect(&rect);
	pWDC->SelectObject(GetStockObject(NULL_BRUSH));
	pWDC->SelectObject(GetStockObject(BLACK_PEN));
	ReleaseDC(pWDC);
	*/
	CDialog::OnPaint();
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAralUpdaterDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CAralUpdaterDlg::DownloadUpdateInfo()
{
	return TRUE;
}

BOOL CAralUpdaterDlg::GetSelfUpdateRequirement()
{
	return TRUE;
}


UINT CAralUpdaterDlg::MainThread(void *lParam)
{
	CAralUpdaterDlg* pThisDlg;
	CPtrArray* paWorks;
	int nIndex = 0;
	int nCount = 0;
	CFileWork* work;
	//CWinThread* pChildThread; 

	pThisDlg = (CAralUpdaterDlg*)lParam;	// �� ���̾�α��� ������
	paWorks = pThisDlg->m_paWorkList;		// �۾����
	//pChildThread = ::AfxBeginThread(StaticUpdateThread, lParam);	// ���۷��� ǥ�����ִ� �ڽ� ������ ON
	pThisDlg->m_nTotalSize = pThisDlg->GetTotalWorkSize();		// �� �����ؾ��� �� ����
	
	
	// 64��Ʈ�� �����̷��� ����
	PVOID OldValue = NULL;
	if(pThisDlg->m_bIsWow64)
	{
		BOOL bRet = SetWow64Redirection(FALSE);
	}

	nCount = paWorks->GetSize();

	// ���� �������ͺ��� ���ŵǾ�� �ϴ��� �����Ѵ�
	CString strThisUpdater = CAralProcessAPI::GetCurrentModulePath();
	if(strThisUpdater.Right(4).CompareNoCase(_T(".dmy")) == 0)
	{
		strThisUpdater = strThisUpdater.Left( strThisUpdater.GetLength()-4 ) + _T(".exe");
	}

	TCHAR szThisUpdater[MAX_PATH] = {0,};
	::GetLongPathName( (LPCTSTR)strThisUpdater, szThisUpdater, MAX_PATH );
	if(_tcslen(szThisUpdater) > 0)
	{
		strThisUpdater = szThisUpdater;
	}

	for(int i=0; i<nCount; i++)
	{
		work = (CFileWork*)paWorks->ElementAt(i);

		CString strDownFile = work->m_strLocalPath;
		TCHAR szDownFile[MAX_PATH] = {0,};
		::GetLongPathName( (LPCTSTR)strDownFile, szDownFile, MAX_PATH );
		if(_tcslen(szDownFile) > 0)
		{
			strDownFile = szDownFile;
		}

		if( strDownFile.CompareNoCase(strThisUpdater) == 0 )
		{
			// ���⼭ ���� �޴´�
			pThisDlg->WorkDownload(work);

			// �������͸� �ٽ� �����Ű��
			STARTUPINFO si={0,};
			PROCESS_INFORMATION pi;			
			BOOL bRes = ::CreateProcess( NULL, (LPTSTR)(LPCTSTR)strThisUpdater,NULL,NULL,FALSE,0,NULL, NULL, &si , &pi );

			// �̰� ����
			::TerminateProcess( GetCurrentProcess(), 0 );
		}

		if(pThisDlg->m_bStop) break;
		nIndex++;
	}

	// �ƴϸ� ������ ���� �ٿ�ε� �۾�
	nIndex = 0;
	while(nIndex<nCount)
	{
		work = (CFileWork*)paWorks->ElementAt(nIndex);

		// ���⼭ ���� ����
		pThisDlg->WorkDownload(work);

		if(pThisDlg->m_bStop) break;
		nIndex++;
	}

	// �����̷��� �Ҵ�
	if(pThisDlg->m_bIsWow64)
	{
		BOOL bRet = SetWow64Redirection(TRUE);
	}

	pThisDlg->m_pThread = NULL;
	while(pThisDlg->m_nAlpha > 0 || pThisDlg->m_pChildThread) ::Sleep(10);
	
	HWND hWnd = pThisDlg->GetSafeHwnd();
	if(pThisDlg->m_bStop)
	{
		::PostMessage(hWnd, WM_COMMAND, IDCANCEL, 0);
	}
	else
	{
		//pThisDlg->Invalidate(TRUE);
		//pThisDlg->CloseSmoothly();
		pThisDlg->CleanupUpdate();
		::PostMessage(hWnd, WM_COMMAND, IDOK, 0);
	}
	
	return 0;
}



// �ٿ�ε� �۾��� ���� �� ũ�⸦ �����ִ� �Լ�
// Byte������ ��ȯ
int CAralUpdaterDlg::GetTotalWorkSize()
{
	if(NULL==m_paWorkList) return 0;
	
	CFileWork* work = NULL;
	int nTotalSize = 0;
	int nIndex = 0;
	int nCount = m_paWorkList->GetSize();

	while(nIndex<nCount){		// �۾� ������ŭ ����
		work = (CFileWork*)m_paWorkList->ElementAt(nIndex);
		CString strURL = work->m_strURL;
		
		// ������ ũ�� ���ؼ� ����
		DWORD dwStat;
		CInternetSession* pSession = new CInternetSession;
		CHttpFile *pHttpFile = NULL;
		pHttpFile = (CHttpFile*) pSession->OpenURL(strURL, pSession->GetContext()); //, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);
		if(!pHttpFile->QueryInfoStatusCode(dwStat) || dwStat > 299 || dwStat < 200)
		{
			MessageBox(_T("������ HTTP������ ã�� �� �����ϴ� : ") + strURL);
		}

		DWORD dwSize;
		pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dwSize);
		nTotalSize += dwSize;

		pHttpFile->Close();
		delete pHttpFile;
		pSession->Close();
		delete pSession;
		nIndex++;
		::Sleep(0);
	}

	
	return nTotalSize;
}

// FTP���� ���� 1���� �ٿ�ε��Ѵ�
BOOL CAralUpdaterDlg::WorkDownload(CFileWork *pFW)
{

	CString strComment;
	int nDotIndex = pFW->m_strURL.ReverseFind( _T('.') );
	int nStartIndex = pFW->m_strURL.ReverseFind( _T('/') ) + 1;

	if(nDotIndex>0) strComment = pFW->m_strURL.Mid(nStartIndex, nDotIndex-nStartIndex);
	else strComment = pFW->m_strURL.Mid(nStartIndex);
	m_ctrlCommentText.SetWindowText(strComment + _T(" Module Request"));

	DWORD			dwService;		// ���ͳ� ���� ����
	CString			strURL;			// ������ URL
	CString			servername;		// ������
	CString			objname;		// ��ü��
	INTERNET_PORT	nPort;			// ��Ʈ��ȣ
	TCHAR strTempPath[MAX_PATH];
	TCHAR strTempFileName[MAX_PATH];
	if(!GetTempPath(MAX_PATH, strTempPath)) return FALSE;
	if(!GetTempFileName(strTempPath, _T("AralUpdateTempFile"), 0, strTempFileName)) return FALSE;

	strURL = pFW->m_strURL;
	if(!AfxParseURL(strURL, dwService, servername, objname, nPort))
	{
		CString msg;
		msg.Format( _T("invalid URL!\r\n%s"), strURL );
		MessageBox( msg );
		return FALSE;
	}


	// ���ͳ� ����
	CInternetSession* pSession = new CInternetSession;
	CHttpFile *pHttpFile = NULL;

	// HTTP ���� ����
	pHttpFile = (CHttpFile*) pSession->OpenURL(strURL, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);

	// HTTP������ ������ ���ȴٸ� ���ÿ� ���� �� ���� (�ӽ����ϸ�����)
	HANDLE hLocal = CreateFile(strTempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	if(hLocal != INVALID_HANDLE_VALUE)		// ���������� ����� ���ȴٸ�
	{

		DWORD dwWritten;
		UINT nSize = 0;
		CHAR Data[16384] = "";
		BOOL bWriteOK;

		BOOL bBegin = FALSE;

		do {
			nSize = pHttpFile->Read((LPVOID)Data, 16384);
			if(nSize){
				if(!bBegin){
					m_ctrlCommentText.SetWindowText( strComment + _T(" Module Downloading..") );
					bBegin=TRUE;
				}
				bWriteOK = WriteFile(hLocal, Data, nSize, &dwWritten, NULL);
				if(bWriteOK) m_nTranSize += dwWritten;	// �� ���۷� ����
			}

			if(m_bStop){
				CloseHandle(hLocal);				// �������� �ݱ�
				pHttpFile->Close();					// http���� �ݱ�
				DeleteFile(strTempFileName);		// �޴����� ����
				return FALSE;
			}
		} while(nSize);
		CloseHandle(hLocal);				// �������� �ݱ�
		pHttpFile->Close();					// http���� �ݱ�
		delete pHttpFile;
		pSession->Close();					// Session �ݱ�
		delete pSession;
	}
	else
	{
		pHttpFile->Close();					// http���� �ݱ�
		delete pHttpFile;
		pSession->Close();					// Session �ݱ�
		delete pSession;
		CString err = _T("Cannot create temporary file!");
		err += strTempFileName;
		this->MessageBox(err, _T("Detectron Font Updater"), MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	// �����ϱ� ���� ���丮 ����
	int nStrIdx = pFW->m_strLocalPath.ReverseFind(_T('\\'));
	if( nStrIdx > 0 )
	{
		CString strDirectory = pFW->m_strLocalPath.Left(nStrIdx);
		BOOL bCreateDir = CreateDirectoryRecursive( strDirectory );
	}

	
	
	BOOL bRetVal = FALSE;

	for(int i=0; i<5; i++)
	{
		// ���������̸� ���� ����
		if( pFW->m_strURL.Right(4).MakeUpper() == _T(".ZIP")
			&& pFW->m_strLocalPath.Right(4).MakeUpper() != _T(".ZIP") )
		{
			HZIP hz = OpenZip(strTempFileName, NULL);
			ZIPENTRY ze; 
			GetZipItem(hz,0,&ze);
			HANDLE h = CreateFile(pFW->m_strLocalPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,ze.attr,NULL);
			if (h != INVALID_HANDLE_VALUE)
			{
				if( UnzipItem(hz,0,h) == ZR_OK )
				{
					CloseHandle(h);
					CloseZip(hz);
					bRetVal = TRUE;
					break;
				}
				CloseHandle(h);
			}

			CloseZip(hz);
		}
		// ���������� �ƴϸ� ���� ����
		else
		{
			if( CopyFile(strTempFileName, pFW->m_strLocalPath, FALSE) == TRUE )
			{
				bRetVal = TRUE;
				break;
			}
		}

		Sleep(1000);
	}
	
	DeleteFile(strTempFileName);		// �ӽ����� ����

	return bRetVal;
}

BOOL CAralUpdaterDlg::CreateDirectoryRecursive(CString strDir)
{
	CFileFind find;
	if( find.FindFile( strDir ) == TRUE ) return TRUE;

	int nStrIdx = strDir.ReverseFind(_T('\\'));
	if( nStrIdx < 0 ) return FALSE;

	CString strParentDir = strDir.Left(nStrIdx);
	if( CreateDirectoryRecursive( strParentDir ) == FALSE ) return FALSE;

	return CreateDirectory( strDir, NULL );

}



// �ε巴�� ������� ���̾˷α�
BOOL CAralUpdaterDlg::SetTransparent(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
  BOOL bRet = TRUE;
  typedef BOOL (WINAPI* lpfnSetTransparent)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

  if ( m_hUserDll )
  {
    lpfnSetTransparent pFnSetTransparent = NULL;
    pFnSetTransparent =
      (lpfnSetTransparent)GetProcAddress(m_hUserDll,
                    "SetLayeredWindowAttributes");

    if (pFnSetTransparent )
       bRet = pFnSetTransparent(hWnd, crKey, bAlpha, dwFlags);
    else
       bRet = FALSE;
  } // if( m_hUserDll )

   return bRet;
}

void CAralUpdaterDlg::CloseSmoothly()
{
  for(int nPercent=100; nPercent >= 0 ;nPercent--)
  {
	 SetTransparent( m_hWnd, 0, 255 * nPercent/100, LWA_ALPHA);
	 Sleep(15);
  }
}


BOOL CAralUpdaterDlg::PreTranslateMessage(MSG* pMsg)
{
	// ������Ʈ�� ����Ǵ°��� ����
	if(pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
		return TRUE;

	if(pMsg->wParam == VK_ESCAPE)
		return TRUE;
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CAralUpdaterDlg::OnTimer(UINT nIDEvent) 
{
 	if(m_pThread==NULL)
	{
 		m_nAlpha -= 6;
 		if(m_nAlpha < 0) m_nAlpha = 0;
 	}
 	else
	{
 		m_nAlpha += 6;
 		if(m_nAlpha > 100) m_nAlpha = 100;
 	}
 	SetTransparent(m_hWnd, 0, 255 * (BYTE)m_nAlpha / 100, LWA_ALPHA);
	
	//CDialog::OnTimer(nIDEvent);
}

void CAralUpdaterDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	KillTimer(0);
	if(m_pRgn) delete m_pRgn;
	if(m_pBG) delete m_pBG; //m_pBG->DeleteObject();
}



HBRUSH CAralUpdaterDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{         
	/*
	static HBRUSH hBrush_bmp=NULL;

	HBRUSH hbr;
    pDC->SetBkMode(TRANSPARENT);  
    if(!hBrush_bmp) hBrush_bmp=CreatePatternBrush((HBITMAP)*m_pBG);
    if(hBrush_bmp) hbr=hBrush_bmp;
	else hbr = (HBRUSH)m_BkBrush; //CDialog::OnCtlColor(pDC,pWnd,nCtlColor);
    return hbr;
	*/

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

LRESULT CAralUpdaterDlg::OnNcHitTest(CPoint point) 
{
	UINT nHit = CDialog::OnNcHitTest(point);
	if(nHit == HTCLIENT) nHit = HTCAPTION;
	return nHit;
}



UINT CAralUpdaterDlg::StaticUpdateThread(void *lParam)
{
	CAralUpdaterDlg* pThisDlg;
	CString temp;

	pThisDlg = (CAralUpdaterDlg*)lParam;	// �� ���̾�α��� ������
	while(pThisDlg->m_pThread)
	{		
		// �����۷� ����		
		temp.Format(_T("%d / %d KBytes"), pThisDlg->m_nTranSize/1024, pThisDlg->m_nTotalSize/1024);
		pThisDlg->m_ctrlTranText.SetWindowText(temp);

		// ���α׷����� ����
		pThisDlg->m_ctrlProgress.SetRange(0, pThisDlg->m_nTotalSize/1024);
		pThisDlg->m_ctrlProgress.SetPos(pThisDlg->m_nTranSize/1024);

		// �ۼ�Ʈ ����
		float percent = ((float)pThisDlg->m_nTranSize / (float)pThisDlg->m_nTotalSize) * 100;
		temp.Format(_T("%d%%"), (int)percent);
		pThisDlg->m_ctrlPercentText.SetWindowText(temp);
		Sleep(0);

	}

	pThisDlg->m_pChildThread = NULL;
	return 0;
}

//---------------------------------------------------------------------------
//	���Լ��� : GetFileVersion
//	���Լ����� : ������ ������ ������ �������ش�.
//	������ : CString �����θ� ������ �����̸�
//	�ݹ�ȯ�� : CString ������ ���� ("xx.xx.xx.xx")
//---------------------------------------------------------------------------
CString CAralUpdaterDlg::GetFileVersion(CString strFilePathName)
{
	DWORD dwHdlDest = 0;
	DWORD dwSizeDest = 0;
	DWORD dwDestLS, dwDestMS;
	CString strVersion = _T("");

	TRACE( _T("[ aral1 ] CBGActiveXCtrl::GetFileVersion(\"%s\") called! \n"), strFilePathName );
	dwSizeDest = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)strFilePathName, &dwHdlDest);

	if(dwSizeDest)
	{
	   _TCHAR* pDestData = new TCHAR[dwSizeDest + 1];

	   if(GetFileVersionInfo((LPTSTR)(LPCTSTR)strFilePathName, dwHdlDest, dwSizeDest, pDestData))
	   {
		VS_FIXEDFILEINFO * pvsInfo;
		UINT uLen;
		BOOL dwRet;
		dwRet = VerQueryValue(pDestData, _T("\\"), (void**)&pvsInfo, &uLen);
		if(dwRet)
		{
			dwDestMS = pvsInfo->dwFileVersionMS;    // �ֹ���
			dwDestLS = pvsInfo->dwFileVersionLS;    // ��������

			int dwDestLS1, dwDestLS2, dwDestMS1, dwDestMS2;
			dwDestMS1 = (dwDestMS / 65536);
			dwDestMS2 = (dwDestMS % 65536);
			dwDestLS1 = (dwDestLS / 65536);
			dwDestLS2 = (dwDestLS % 65536);

			strVersion.Format( _T("%d.%d.%d.%d"),dwDestMS1,dwDestMS2,dwDestLS1,dwDestLS2);
			delete [] pDestData;
			return strVersion;
		}
	   }
	   delete [] pDestData;
	}
	else
	{
		if(strFilePathName.Mid(strFilePathName.ReverseFind(_T('\\')) + 1) == _T("help.chm"))
		{
			CString strUpdateInfoURL = CRegistryMgr::RegRead( _T("HKEY_CURRENT_USER\\Software\\AralGood\\Update"), _T("HelpVersion") );
			if( strUpdateInfoURL == _T("") )
				strVersion.Format( _T("0.0.0.0"));
			else
				strVersion = strUpdateInfoURL;
		}
	}
    return strVersion;
}

CString CAralUpdaterDlg::ParseSystemVariable(CString strSource)
{
	CString strResult = strSource;

	strResult.Replace( _T("%AppDir%"), m_strClientFolder );
	strResult.Replace( _T("%System32%"), m_strSystemFolder );
	strResult.Replace( _T("%Windows%"), m_strWindowsFolder );
	strResult.Replace( _T("%ProgramFiles%"), m_strProgramFilesFolder );

	return strResult;

}


BOOL CAralUpdaterDlg::DoRegistryWork()
{
	BOOL bRetVal = TRUE;

	// ������Ʈ�� �۾�
	int cnt = 0;
	CString strNodeName;
	
	strNodeName.Format(_T("Registry%d"), cnt);
	CININode* pNode = m_ININewVer.FindNode(strNodeName);

	while( pNode )
	{
		CString strRegPath		= pNode->GetAttribute(_T("RegPath"));
		CString strRegName		= pNode->GetAttribute(_T("RegName"));
		CString strRegValueType	= pNode->GetAttribute(_T("RegValueType"));
		CString strRegValue		= ParseSystemVariable( pNode->GetAttribute(_T("RegValue")) );

		if( strRegValueType == _T("DWORD") )
		{
			DWORD dwValue = (DWORD)_ttoi( strRegValue );
			bRetVal = bRetVal && CRegistryMgr::RegWriteDWORD( strRegPath, strRegName, dwValue );
			
		}
		else
		{
			bRetVal = bRetVal && CRegistryMgr::RegWrite( strRegPath, strRegName, strRegValue );

		}

		// ���� ������Ʈ�� �۾�
		cnt++;
		strNodeName.Format(_T("Registry%d"), cnt);
		pNode = m_ININewVer.FindNode(strNodeName);

	}

	return bRetVal;

}
