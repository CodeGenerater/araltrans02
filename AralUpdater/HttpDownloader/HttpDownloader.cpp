// HttpDownloader.cpp: implementation of the CHttpDownloader class.
//
//////////////////////////////////////////////////////////////////////

//#include "../stdafx.h"
#include "HttpDownloader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Wininet.lib")	// for GetMACFromIP

#include <WinInet.h>
#include <IO.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define		Termination(_val_)			iReturn = _val_; goto Termination_Routine;

#ifndef tochar
	#define tochar(_str_)	(LPTSTR)(LPCTSTR)_str_
#endif

pUpdateProgressCallBack	pFunc = NULL;


CHttpDownloader::CHttpDownloader()
{

}

CHttpDownloader::~CHttpDownloader()
{

}

//---------------------------------------------------------------------------//
// <���> : ������ ��ġ(URL)�� ���� �ٿ�ε� �� ������ ���� ��ġ�� ����
// <��ȯ> : 
//			0 - ����
//
//---------------------------------------------------------------------------//
int	CHttpDownloader::Http_Download_To_File( 
	IN CString strURL, 
	IN CString strLocalPath, 
	IN PVOID pCallBack, 
	IN BOOL bCheckSize, 
	IN ULONGLONG FileSize  )
{
	int	iReturn = 0;

	HINTERNET hINetSession = NULL;
	HINTERNET hHttpConnection = NULL;

	DWORD		dwBufferSize	= sizeof(ULONGLONG);
	DWORD		dwIndex			= 0;
	ULONGLONG	dwFileSize		= 0;
	DWORD		dwRemain		= 0;
	DWORD		dwQueryBytes	= 0;

	DWORD		dwReceiveBytes	= 0;
	ULONGLONG	dwTotalReceiveBytes	= 0;

	DWORD		dwWrite			= 0;

	BOOL	bRet = TRUE;
	BYTE	byteReadBuffer[65535]	= {0, };

	HANDLE hFile = INVALID_HANDLE_VALUE;

	CString	strReceiveBuf = _T("");
	CString	strReceive = _T("");

	//pUpdateProgressCallBack	pFunc = NULL;

	pFunc = (pUpdateProgressCallBack)pCallBack;
		
	//---------------------//
	// CallBack �˸�
	//---------------------//

	TRACE(_T(" [%x] \n"), pFunc );
	CallBackInfo( pFunc, HTTP_STATES_INIT, 0, strURL, strLocalPath, 0, 0);


	//-----------------------------------------//
	// #. ���ͳ� ���� (File)
	//-----------------------------------------//
	hINetSession = InternetOpen(_T("HTTP"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0);
	if (hINetSession == NULL)
	{
		Termination(-1);	// ���ͳ� ���� �ȵ�
	}
	


	//-----------------------------------------//
	// #. URL ����
	//-----------------------------------------//
	hHttpConnection = InternetOpenUrl(hINetSession, strURL, NULL, 0, INTERNET_FLAG_TRANSFER_BINARY, 0);
	
	if(hHttpConnection == NULL)
	{			
		Termination(-2);	// URL ���� ����
	}
	

	
	//-----------------------------------------//
	// #. FileSize ����
	//-----------------------------------------//
	bRet = HttpQueryInfo(hHttpConnection, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwFileSize, &dwBufferSize, &dwIndex);
	
	if (bRet == FALSE)
	{		
		Termination(-3);	// HTTP ���� ����
	}
	

	//-----------------------------------------//
	// #. FileSize �˻� (�÷��� ������)
	//-----------------------------------------//
	if( bCheckSize == TRUE )
	{
		if( dwFileSize != FileSize)
		{
			Termination(-103);	// ���� ũ�� �ٸ�
		}
	}


	
	//-----------------------------------------//
	// #. ��� ���� ����
	//-----------------------------------------//
	
#ifdef _UNICODE
	_waccess( tochar(strLocalPath), 0 );
	if( _waccess( tochar(strLocalPath), 0 ) == 0 ) DeleteFile( tochar(strLocalPath) );
#else
	_access( tochar(strLocalPath), 0 );
	if( _access( tochar(strLocalPath), 0 ) == 0 ) DeleteFile( tochar(strLocalPath) );
#endif
	
	hFile = CreateFile(strLocalPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if( INVALID_HANDLE_VALUE == hFile )
	{
		Termination(-4);	// �ӽ����� ���� ����
	}

	do
	{
	
		//-----------------------------------------//
		// #. ���� ���� �ʱ�ȭ
		//-----------------------------------------//
		memset( byteReadBuffer, 0, 65535);
		strReceiveBuf = _T("");


		//-----------------------------------------//
		// #. ���� ���ɷ� ����
		//-----------------------------------------//
		bRet = InternetQueryDataAvailable(hHttpConnection, &dwRemain, 0, 0);
	
		if( dwRemain > 65530 )
		{
			dwQueryBytes = 65530;
		}
		else
		{
			dwQueryBytes = dwRemain;
		}
		
		//-----------------------------------------//
		// #. ������ ����
		//-----------------------------------------//
		bRet = InternetReadFile(hHttpConnection, (LPVOID)byteReadBuffer, dwQueryBytes, &dwReceiveBytes);
		
		dwTotalReceiveBytes += dwReceiveBytes;
		
		//-----------------------------------------//
		// #. ���� ������ ó��
		//-----------------------------------------//
		if (dwReceiveBytes > 0)
		{
			// CallBack �˸�
			CallBackInfo( pFunc, HTTP_STATES_DOWNLOADING, 0, strURL, strLocalPath, dwFileSize, dwTotalReceiveBytes);

			WriteFile(hFile, byteReadBuffer, dwReceiveBytes, &dwWrite, NULL);
		}	
		
	} while(dwReceiveBytes > 0);

	CloseHandle(hFile);
	Termination(0);




//-----------------------------------------//
// #. ���� ó��
//-----------------------------------------//
Termination_Routine:

	if( hINetSession ) InternetCloseHandle(hHttpConnection);
	if( hHttpConnection ) InternetCloseHandle(hINetSession);
	
	if( iReturn != 0 )
	{
		// CallBack �˸�
		CallBackInfo( pFunc, HTTP_STATES_ERROR, iReturn, strURL, strLocalPath, dwFileSize, dwTotalReceiveBytes);
	}
	else
	{
		// CallBack �˸�
		CallBackInfo( pFunc, HTTP_STATES_FINALIZE, iReturn, strURL, strLocalPath, dwFileSize, dwTotalReceiveBytes);
		
	
	}

	return iReturn;
}




//---------------------------------------------------------------------------//
// <���> : ������ ��ġ(URL)�� ���� �ٿ�ε� �� ���۷� ����
// <��ȯ> : 
//			0 - ����
//
//---------------------------------------------------------------------------//
int	CHttpDownloader::Http_Download_To_Buffer( IN CString strURL, IN OUT CString &strBuf, IN PVOID pCallBack )
{

	int	iReturn = 0;

	HINTERNET hINetSession = NULL;
	HINTERNET hHttpConnection = NULL;

	DWORD		dwBufferSize	= sizeof(ULONGLONG);
	DWORD		dwIndex			= 0;
	ULONGLONG	dwFileSize		= 0;
	DWORD		dwRemain		= 0;
	DWORD		dwQueryBytes	= 0;

	DWORD		dwReceiveBytes	= 0;
	ULONGLONG	dwTotalReceiveBytes	= 0;

	BOOL		bRet = TRUE;
	BYTE		byteReadBuffer[65535]	= {0, };

	CString		strReceive = _T("");

	//pUpdateProgressCallBack	pFunc = NULL;

	pFunc = (pUpdateProgressCallBack)pCallBack;
	
	strBuf = _T("");

	
	//---------------------//
	// CallBack �˸�
	//---------------------//
	//CallBackInfo( pFunc, HTTP_STATES_INIT, 0, strURL, "", 0, 0);


//#ifdef _UNICODE
//	char szURL[MAX_PATH];
// 	WideCharToMultiByte(CP_ACP, 0, strURL, -1, szURL, MAX_PATH, NULL, NULL);
// 	DeleteUrlCacheEntry( szURL );
// #else
	DeleteUrlCacheEntry( strURL );
//#endif


	//-----------------------------------------//
	// #. ���ͳ� ����
	//-----------------------------------------//
	hINetSession = InternetOpen(_T("HTTP"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0);
	if (hINetSession == NULL)
	{
		Termination(-1);	// ���ͳ� ���� �ȵ�
	}
	


	//-----------------------------------------//
	// #. URL ����
	//-----------------------------------------//
	hHttpConnection = InternetOpenUrl(hINetSession, strURL, NULL, 0, INTERNET_FLAG_TRANSFER_ASCII, 0);
	
	if(hHttpConnection == NULL)
	{			
		Termination(-2);	// URL ���� ����
	}
	

	
	//-----------------------------------------//
	// #. FileSize ����
	//-----------------------------------------//
	bRet = HttpQueryInfo(hHttpConnection, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwFileSize, &dwBufferSize, &dwIndex);
	
	if (bRet == FALSE)
	{		
		Termination(-3);	// HTTP ���� ����
	}

			
	do
	{
	
		//-----------------------------------------//
		// #. ���� ���� �ʱ�ȭ
		//-----------------------------------------//
		memset( byteReadBuffer, 0, 65535);


		//-----------------------------------------//
		// #. ���� ���ɷ� ����
		//-----------------------------------------//
		bRet = InternetQueryDataAvailable(hHttpConnection, &dwRemain, 0, 0);
	
		if( dwRemain > 65530 )
		{
			dwQueryBytes = 65530;
		}
		else
		{
			dwQueryBytes = dwRemain;
		}
		
		//-----------------------------------------//
		// #. ������ ����
		//-----------------------------------------//
		bRet = InternetReadFile(hHttpConnection, (LPVOID)byteReadBuffer, dwQueryBytes, &dwReceiveBytes);
		
		
		//-----------------------------------------//
		// #. ���� ������ ó��
		//-----------------------------------------//
		
		dwTotalReceiveBytes += dwReceiveBytes;

		

		if (dwReceiveBytes > 0)
		{
			// CallBack �˸�
			//CallBackInfo( pFunc, HTTP_STATES_DOWNLOADING, 0, strURL, "", dwFileSize, dwTotalReceiveBytes);

#ifdef UNICODE
			wchar_t tmpBuf[65535];
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)byteReadBuffer, -1, tmpBuf, 65535);
			strReceive += tmpBuf;			
#else
			strReceive += (LPCSTR)byteReadBuffer;
#endif
		}	
		
	}
	while(dwReceiveBytes > 0);

	Termination(0);




//-----------------------------------------//
// #. ���� ó��
//-----------------------------------------//
Termination_Routine:

	if( hINetSession ) InternetCloseHandle(hHttpConnection);
	if( hHttpConnection ) InternetCloseHandle(hINetSession);
	
	strBuf = strReceive;

	if( iReturn != 0 )
	{
		// CallBack �˸�
		//CallBackInfo( pFunc, HTTP_STATES_ERROR, iReturn, strURL, "", dwFileSize, dwTotalReceiveBytes);
	}
	else
	{
		// CallBack �˸�
		//CallBackInfo( pFunc, HTTP_STATES_FINALIZE, iReturn, strURL, "", dwFileSize, dwTotalReceiveBytes);
	}
	
	

	return iReturn;
}
