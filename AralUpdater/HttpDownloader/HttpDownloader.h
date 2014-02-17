// HttpDownloader.h: interface for the CHttpDownloader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPDOWNLOADER_H__97BEB4F4_B175_4472_A168_BE791AD820A4__INCLUDED_)
#define AFX_HTTPDOWNLOADER_H__97BEB4F4_B175_4472_A168_BE791AD820A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>

//---------------------------------------------------------------------------//
// Defines
//---------------------------------------------------------------------------//


#define HTTP_STATES_INIT			0		// ����
#define HTTP_STATES_DOWNLOADING		1		// �ٿ�ε� ��
#define HTTP_STATES_ERROR			2		// �����߻�
#define HTTP_STATES_FINALIZE		3		// ����

typedef int (*pUpdateProgressCallBack)
( 
	int		iStates,		// ���� ��Ȳ
	int		iErr,			// ���� ����
	CString strSource,		// ���� ���
	CString strTarget,		// ��� ���(���Ϸ� ������ ��츸)
	ULONGLONG TotalSize,	// ���� �� ũ��
	ULONGLONG CurrentSize	// ������� ���� �ٿ�ε� ũ��
);

#ifndef CallBackInfo

#define CallBackInfo(_pfunc, _iStates, _iErr, _strSource, _strTarget, _TotalSize, _CurrentSize)		\
		if( _pfunc ){ _pfunc( _iStates, _iErr, _strSource, _strTarget, _TotalSize, _CurrentSize); }

#endif
					



//---------------------------------------------------------------------------//
// Class
//---------------------------------------------------------------------------//

class CHttpDownloader  
{
private:
	CHttpDownloader();
	virtual ~CHttpDownloader();

public:

	//------------------------------------------//
	// HTTP �ٿ�ε� : ���Ϸ� ����
	//------------------------------------------//
	static int	Http_Download_To_File( 
		IN CString strURL, 
		IN CString strLocalPath, 
		IN PVOID pCallBack, 
		IN BOOL bCheckSize = FALSE, 
		IN ULONGLONG FileSize = 0  );

	
	//------------------------------------------//
	// HTTP �ٿ�ε� : ���ۿ� ����
	//------------------------------------------//
	static int	Http_Download_To_Buffer( 
		IN CString strURL, 
		IN OUT CString &strBuf, 
		IN PVOID pCallBack );


};

#endif // !defined(AFX_HTTPDOWNLOADER_H__97BEB4F4_B175_4472_A168_BE791AD820A4__INCLUDED_)
