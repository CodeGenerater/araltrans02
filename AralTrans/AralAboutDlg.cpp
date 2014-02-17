// AralAboutDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "AralAboutDlg.h"

#include <psapi.h>
#include <tlhelp32.h>
#include <winsvc.h>

#include "Util/Misc.h"
#include "Util/AralProcessAPI/AralProcessAPI.h"
#include "Util/AralFileAPI/AralFileAPI.h"

#pragma comment(lib, "PSAPI.LIB")

// CAralAboutDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAralAboutDlg, CDialog)

CAralAboutDlg::CAralAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAralAboutDlg::IDD, pParent)
	, m_strInfo(_T("")), m_hMapFile(NULL), m_bIsSystemInfo(FALSE)
{
	m_hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security 
		PAGE_READWRITE,          // read/write access
		0,                       // max. object size 
		sizeof(TCHAR) * 20480,  // buffer size  
		_T("ATUpdateINI"));    // name of mapping object

	if (m_hMapFile)
	{
		LPTSTR szINIFile=NULL;

		szINIFile = (LPTSTR) MapViewOfFile(m_hMapFile, FILE_MAP_WRITE, 0, 0, sizeof(TCHAR) * 20480 );
		if (szINIFile) 
		{
			ZeroMemory(szINIFile, sizeof(TCHAR) * 20480);
			UnmapViewOfFile(szINIFile);
		}
	}


}

CAralAboutDlg::~CAralAboutDlg()
{
	if (m_hMapFile)
	{
		CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}
}

void CAralAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VERSIONINFO, m_strInfo);
}


BEGIN_MESSAGE_MAP(CAralAboutDlg, CDialog)
	ON_BN_CLICKED(IDC_CHANGEINFO_BUTTON, &CAralAboutDlg::OnBnClickedChangeInfoButton)
	ON_BN_CLICKED(IDC_SAVE_BUTTON, &CAralAboutDlg::OnBnClickedSaveButton)
END_MESSAGE_MAP()


// CAralAboutDlg �޽��� ó�����Դϴ�.

BOOL CAralAboutDlg::OnInitDialog()
{
	HCURSOR hOldCursor, hCursor;
	
	hCursor = LoadCursor(NULL, IDC_WAIT);
	hOldCursor = SetCursor(hCursor);

	CDialog::OnInitDialog();

	GetVersionInfo(m_strInfo);

	UpdateData(FALSE);

	SetCursor(hOldCursor);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

BOOL CAralAboutDlg::Is64bitWindows()
{
	// from MSDN

	// IsWow64Process �Լ� ������ typedef
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;

	BOOL bIsWow64 = FALSE;

	// IsWow64Process �� Win XP SP2 �̻󿡼��� �����ϹǷ�
	// Ȥ�� �𸣴� ������ ����ڸ� ���� �̷��� �ؾ߸� �Ѵ�..
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  
	if (NULL != fnIsWow64Process)
	{
		// �� ���μ����� WoW64 ���� �۵��ϴ��� Ȯ���Ѵ�.
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			;// handle error
		}
	}
	// �Լ��� ���ٸ� �翬�� 32��Ʈ

    return bIsWow64;


}

CString CAralAboutDlg::GetFilesVersion(LPCTSTR szFilesPath)
{
	CString strFilesVersion;
	CFileFind finder;
	BOOL bIsNextFileExists;

	bIsNextFileExists=finder.FindFile(szFilesPath);

	while(bIsNextFileExists)
	{
		bIsNextFileExists=finder.FindNextFile();

		if (finder.IsDirectory() || finder.IsDots())
			continue;

		strFilesVersion+=finder.GetFileName()+_T(" (")+GetFileVersion(finder.GetFilePath())+_T(")\r\n");
	}

	return strFilesVersion;
}

BOOL CAralAboutDlg::TestUpdate()
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

	// ���α׷��� �����ؾ��ϴ��� ��ȯ������ �Ǻ�
	::WaitForSingleObject( pi.hProcess, 7000 );

	::GetExitCodeProcess( pi.hProcess, &dwExitCode );

	if( dwExitCode == -1 )
	{
		return TRUE;
	}

	return FALSE;
}

void CAralAboutDlg::GetVersionInfo(CString & strVersionInfo)
{
	TCHAR szAraltransFiles[MAX_PATH] = {0,};
	if( ::GetModuleFileName( NULL, szAraltransFiles, MAX_PATH ) )
	{
		TCHAR *pFilename=_tcsrchr(szAraltransFiles, _T('\\'))+1;

		strVersionInfo = _T("�ھ� ���α׷�:\r\n");

		// �ھ� ���� #1: exe ����
		wsprintf(pFilename, _T("*.EXE"));
		strVersionInfo+= GetFilesVersion(szAraltransFiles);

		// �ھ� ���� #2: dll ����
		wsprintf(pFilename, _T("*.DLL"));
		strVersionInfo+= GetFilesVersion(szAraltransFiles);

		// �˰���
		strVersionInfo+= _T("\r\n\r\n�˰��� �÷�����:\r\n");
		wsprintf(pFilename, _T("Algorithm\\*.DLL"));
		strVersionInfo+= GetFilesVersion(szAraltransFiles);

		// ����
		strVersionInfo+= _T("\r\n\r\n���� �÷�����:\r\n");
		wsprintf(pFilename, _T("Filter\\*.DLL"));
		strVersionInfo+= GetFilesVersion(szAraltransFiles);

		// ������
		strVersionInfo+= _T("\r\n\r\n���� �÷�����:\r\n");
		wsprintf(pFilename, _T("Translator\\*.DLL"));
		strVersionInfo+= GetFilesVersion(szAraltransFiles);

	}
}

void CAralAboutDlg::GetSystemInfo(CString & strSystemInfo)
{
	// �� �κ��� Checker (http://lab.aralgood.com/35381) �� �ҽ��� (����) �׳� �����ΰ̴ϴ� ���

	LONG result;
	HKEY hKey;
	DWORD dataSize=2048;
	TCHAR szData[2048];

	// �ý��� ������ ���� ��������� �޸𸮸� Ȯ��
	strSystemInfo.Preallocate(20480);
	
	strSystemInfo=_T("�ϵ���� ����:");

	//���μ��� ����
	strSystemInfo+=_T("\r\nCPU - ");
	result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE,_T("Hardware\\Description\\System\\CentralProcessor\\0"), 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS)
	{
		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("ProcessorNameString"), NULL, NULL,(LPBYTE)szData, &dataSize);

		if(result != ERROR_SUCCESS){
			dataSize=2048;
			result = ::RegQueryValueEx (hKey, _T("Identifier"), NULL, NULL,(LPBYTE)szData, &dataSize);
		}

		strSystemInfo+=szData;
	}
	RegCloseKey (hKey);


	//OS ����
	strSystemInfo+=_T("\r\nOS - ");
	result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS)
	{
		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("ProductName"), NULL,NULL,(LPBYTE)szData, &dataSize);
		if(result == ERROR_SUCCESS){
			strSystemInfo+=szData;
		}

		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("CSDVersion"), NULL,NULL,(LPBYTE)szData, &dataSize);
		if(result == ERROR_SUCCESS){
			strSystemInfo+=_T(" ");
			strSystemInfo+=szData;
		}

		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("BuildLab"), NULL,NULL,(LPBYTE)szData, &dataSize);
		if(result == ERROR_SUCCESS){
			strSystemInfo+= _T(" [");
			strSystemInfo+= szData;
			strSystemInfo+= _T("]");
		}
		
	}
	RegCloseKey (hKey);

	if (Is64bitWindows())
		strSystemInfo+= _T(" (64bit)");
	else
		strSystemInfo+= _T(" (32bit)");


	strSystemInfo+=_T("\r\n\r\n�߿� ���丮 ���:");
	// �ƶ�Ʈ���� ���
	strSystemInfo+=_T("\r\nAralTrans ��� - ");
	if( ::GetModuleFileName( NULL, szData, MAX_PATH ) )
	{
		*(_tcsrchr(szData, _T('\\'))) = '\0';
		strSystemInfo+=szData;
	}

	// ����Ʈ���� ���
	strSystemInfo+=_T("\r\nEzTrans ��� - ");
	result = ::RegOpenKeyEx (HKEY_CURRENT_USER,_T("Software\\ChangShin\\ezTrans"), 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS)
	{
		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("FilePath"), NULL, NULL,(LPBYTE)szData, &dataSize);

		if(result != ERROR_SUCCESS){
			dataSize=2048;
			strSystemInfo+=_T("��ġ �Ǵ� ������ ���� �ʾ���");
		}
		else
			strSystemInfo+=szData;
	}
	RegCloseKey (hKey);

	// �� ���� ���
	strSystemInfo+=_T("\r\n�� ���� ��� - ");
	result = SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,szData);
	if(result == S_OK) strSystemInfo+=szData;

	// ����ȭ�� ���
	strSystemInfo+=_T("\r\n����ȭ�� ��� - ");
	result = SHGetFolderPath(NULL,CSIDL_DESKTOP,NULL,SHGFP_TYPE_CURRENT,szData);
	if(result == S_OK) strSystemInfo+=szData;

	strSystemInfo+=_T("\r\n\r\n������ ���� ����:");
	// ������ ���� ���� ���
	strSystemInfo+=_T("\r\n����� ���� - ");

	result = ::RegOpenKeyEx (HKEY_CURRENT_USER,_T("Software\\AralGood"), 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS)
	{
		dataSize=2048;
		result = ::RegQueryValueEx (hKey,  _T("LastExecPath"), NULL, NULL,(LPBYTE)szData, &dataSize);

		if(result != ERROR_SUCCESS){
			strSystemInfo+=_T("(�б� ����)");
		}
		else
		{
			if (szData[0] != _T('\0'))
			{
				strSystemInfo+=szData;
			}
			else
				strSystemInfo+=_T("(�δ� ����)");
		}

		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("LastProcName"), NULL, NULL,(LPBYTE)szData, &dataSize);

		if(result != ERROR_SUCCESS){
			strSystemInfo+=_T("(�б� ����)");
		}
		else
		{
			if (szData[0] != _T('\0'))
			{
				strSystemInfo+=_T("\r\n���� �������� - ");
				strSystemInfo+=szData;
			}
		}

		dataSize=2048;
		result = ::RegQueryValueEx (hKey, _T("LastUsedAppPatch"), NULL, NULL,(LPBYTE)szData, &dataSize);

		if(result != ERROR_SUCCESS){
			strSystemInfo+=_T("(�б� ����)");
		}
		else
		{
			if (szData[0] != _T('\0'))
			{
				strSystemInfo+=_T("\r\n���� AppPatch - ");
				strSystemInfo+=szData;
			}
		}
	}
	RegCloseKey (hKey);


	// �ƶ�Ʈ���� �������� ����
	strSystemInfo+=_T("\r\n\r\nAralTrans �������� ����:");
	result = ::RegOpenKeyEx (HKEY_CURRENT_USER,_T("Software\\AralGood\\Update"), 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS)
	{
		LPTSTR aszNames[] = { _T("AppDir"), _T("Name"), _T("UpdateInfoURL"), _T("Version"), NULL };

		int i=0;
		while(aszNames[i] != NULL)
		{
			strSystemInfo+=_T("\r\n");
			strSystemInfo+= aszNames[i];
			strSystemInfo+= _T(" - ");

			dataSize=2048;
			result = ::RegQueryValueEx (hKey, aszNames[i], NULL, NULL,(LPBYTE)szData, &dataSize);

			if(result != ERROR_SUCCESS){
				strSystemInfo+=_T("(�б� ����)");
			}
			else
				strSystemInfo+=szData;

			i++;
		}
	}
	RegCloseKey (hKey);

	// �ƶ�Ʈ���� �������� ���� ����
	strSystemInfo+=_T("\r\n������Ʈ �ʿ� ���� - ");
	if (TestUpdate())
		strSystemInfo+=_T("�ʿ�");
	else
		strSystemInfo+=_T("���ʿ�");

	if (m_hMapFile)
	{
		LPTSTR szINIFile=NULL;

		strSystemInfo+=_T("\r\n\r\nAralTrans ������Ʈ INI ���� ����:");

		if (m_hMapFile)
		{
			szINIFile = (LPTSTR) MapViewOfFile(m_hMapFile, FILE_MAP_READ, 0, 0, sizeof(TCHAR) * 20480 );
			if (szINIFile) 
			{
				CString strINIFile = szINIFile;

				strINIFile.Replace(_T("\n\n"), _T("\n"));
				strINIFile.Replace(_T("\n"), _T("\r\n"));

				strSystemInfo+=_T("\r\n");
				strSystemInfo+=strINIFile;
				UnmapViewOfFile(szINIFile);
			}
			else
				strSystemInfo+=_T(" (�б� ����)");
		}
	}
	// ���μ��� ���
	strSystemInfo+=_T("\r\n\r\n���μ��� ����:\r\n��ȣ\t   PID\t  pPID\t�̸�\t���");

	HANDLE	hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32	pe={0,};

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	pe.dwSize = sizeof( PROCESSENTRY32 );

	int n=1;
	TCHAR szBuf[1024];

	if(hSnapShot!=INVALID_HANDLE_VALUE){

		Process32First(hSnapShot, &pe);
		do
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pe.th32ProcessID);
			if(hProcess==NULL)
				continue;
			dataSize=1024;
			if(!GetModuleFileNameEx(hProcess,NULL,szBuf,dataSize))
				szBuf[0]=_T('\0');
			CloseHandle(hProcess);

			wsprintf(szData,_T("\r\n[%03d]\t%6u\t%6u\t%s\t%s"),n++,pe.th32ProcessID,pe.th32ParentProcessID,pe.szExeFile,szBuf);
			strSystemInfo+=szData;

		}while( Process32Next(hSnapShot, &pe) );

		CloseHandle(hSnapShot);	
	}

	strSystemInfo+=_T("\r\n\r\n�۵����� ���� ����:");

	SC_HANDLE SchSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ENUMERATE_SERVICE|SC_MANAGER_QUERY_LOCK_STATUS);
	LPENUM_SERVICE_STATUS_PROCESS pInfo = NULL;
	PUCHAR  pBuf    = NULL;
	ULONG  dwBufSize   = 0x00;
	ULONG  dwBufNeed   = 0x00;
	ULONG  dwNumberOfService = 0x00;

	n=0;
	if(SchSCManager){

		EnumServicesStatusEx(SchSCManager, 
			SC_ENUM_PROCESS_INFO, SERVICE_WIN32, 
			SERVICE_STATE_ALL, NULL, dwBufSize,
			&dwBufNeed,	&dwNumberOfService, NULL,NULL);

		if(dwBufNeed<0x01){
			return;
		}

		dwBufSize = dwBufNeed + 0x10;
		pBuf  = (PUCHAR) malloc(dwBufSize);

		EnumServicesStatusEx(
			SchSCManager,
			SC_ENUM_PROCESS_INFO,
			SERVICE_WIN32,  // SERVICE_DRIVER,
			SERVICE_ACTIVE,  //SERVICE_STATE_ALL,
			pBuf,
			dwBufSize,
			&dwBufNeed,
			&dwNumberOfService,
			NULL,
			NULL);

		pInfo = (LPENUM_SERVICE_STATUS_PROCESS)pBuf;


		for(DWORD i=0;i<dwNumberOfService;i++){

			if(pInfo[i].ServiceStatusProcess.dwCurrentState==SERVICE_RUNNING){
				wsprintf(szData,_T("\r\n[%d] %s (%s)"),++n,pInfo[i].lpDisplayName,pInfo[i].lpServiceName);
				strSystemInfo+=szData;
			}
		}

		free(pBuf);

		CloseServiceHandle(SchSCManager);
	}

}

void CAralAboutDlg::OnBnClickedChangeInfoButton()
{
	HCURSOR hOldCursor, hCursor;
	
	hCursor = LoadCursor(NULL, IDC_WAIT);
	hOldCursor = SetCursor(hCursor);


	m_bIsSystemInfo=!m_bIsSystemInfo;

	if (m_bIsSystemInfo)
	{
		GetDlgItem(IDC_CHANGEINFO_BUTTON)->SetWindowText(_T("���� ����"));
		GetDlgItem(IDC_GROUPBOX)->SetWindowText(_T("�ý��� ����"));
		GetSystemInfo(m_strInfo);
	}
	else
	{
		GetDlgItem(IDC_CHANGEINFO_BUTTON)->SetWindowText(_T("�ý��� ����"));
		GetDlgItem(IDC_GROUPBOX)->SetWindowText(_T("���� ����"));
		GetVersionInfo(m_strInfo);
	}
	UpdateData(FALSE);
	
	SetCursor(hOldCursor);
}

void CAralAboutDlg::OnBnClickedSaveButton()
{
	CString strDest, strTemp, strInfo;
	CFileDialog dlg(false,_T("txt"),_T("AralSystemInfo"),OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("�ؽ�Ʈ ����(*.txt)|*.txt|��� ����(*.*)|*.*||"));

	// ���� ���μ����� �۾� ���丮 ����
	CString strWorkDir; 
	::GetCurrentDirectory(MAX_PATH, strWorkDir.GetBufferSetLength(MAX_PATH));
	strWorkDir.ReleaseBuffer();

	if(dlg.DoModal()==IDOK){
		strDest=dlg.GetPathName();
		HANDLE hFile= CreateFile(strDest,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

		DWORD size=0;

		if(hFile==INVALID_HANDLE_VALUE){
			AfxMessageBox(_T("������ �����ϴ� �� �����߽��ϴ�."));
			return;
		}

		HCURSOR hOldCursor, hCursor;
	
		hCursor = LoadCursor(NULL, IDC_WAIT);
		hOldCursor = SetCursor(hCursor);


		strInfo.Preallocate(40960);

		strInfo=_T("== Araltrans ���� ����==\r\n");
		GetVersionInfo(strTemp);
		strInfo+=strTemp;
		strInfo+=_T("\r\n\r\n\r\n\r\n== �ý��� ���� ==\r\n");
		GetSystemInfo(strTemp);
		strInfo+=strTemp+_T("\r\n");

		WriteFile(hFile,"\xFF\xFE",2,&size,0);
		WriteFile(hFile,strInfo,strInfo.GetLength()*sizeof(TCHAR),&size,0);

		CloseHandle(hFile);
		SetCursor(hOldCursor);

		MessageBox(strDest+_T(" �� ���������� ����Ǿ����ϴ�."), _T("AralTrans"), MB_OK | MB_ICONINFORMATION);
	}

	// �۾� ���丮 ����
	::SetCurrentDirectory(strWorkDir);
}