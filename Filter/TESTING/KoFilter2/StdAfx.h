// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.


#ifndef __STDAFX_H__
#define __STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////////////////////
// �÷����� �⺻ ����
//

// �ƶ�Ʈ���� 0.2 �� �÷������� ��� �����Ѵ�.
#define USE_ARALTRANS_0_2

// �ƶ�Ʈ���� 0.3 �� �÷������� ��� �����Ѵ�.
//#define USE_ARALTRANS_0_3


// �˰��� (��ŷ) �÷������� ��� �����Ѵ�.
//#define ATPLUGIN_ALGORITHM

// ���� �÷������� ��� �����Ѵ�.
//#define ATPLUGIN_TRANSLATOR

// ���� �÷������� ��� �����Ѵ�.
#define ATPLUGIN_FILTER

// MFC �� �� MFC �� �����ϴ� ����ġ
//
// ������ �����Ϸ� �ɼǿ��� MFC �� ���.. �� ������� �ϹǷ� ���⿡�� USE_MFC �� �����ϴ�
// �� ���ٴ� �����Ϸ� �ɼǿ��� �����ϴ� ���� �����մϴ�.
//#define USE_MFC

// ���� ���� �⺻ ũ��
#define TRANS_BUF_LEN 10240

//
// �÷����� �⺻ ���� ��
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// ����� ����

// FORCE_DEBUG_LOG : ���� ����� ����϶��� ����� �޼��� ���, ������ ����϶��� ����
//                   ������ FORCE_DEBUG_LOG �� ���������� ������ ����� �޼����� ���
//#define FORCE_DEBUG_LOG

// DEBUG_LOG_TYPE : ����� �޼��� ��� ��� ����
// 0 - OutputDebugString
// 1 - MessageBox
// 2 - FILE
// 3 - Log window (���� ��������)
#define DEBUG_LOG_TYPE	1

// DEBUG_WINDOW_CAPTION : ����� ������ ĸ�� (type 1, 3 ������ �ǹ� ����)
#define DEBUG_WINDOW_CAPTION "ATPlugin"

// DEBUG_LOG_FILEPATH : ����� �α� ���� ���� (type 2 ������ �ǹ� ����)
#define DEBUG_LOG_FILEPATH	"C:\\ATPlugin.log"

// _CRT_SECURE_NO_WARNINGS : fopen �� _vsnwprintf �� ���� �߻��ϴ� warning C4996 �� ���ش�.
//                           fopen_s �� _vsnwprintf_s �� ����ϸ� �ȶ����� VC++ 6.0 ���� ������.
//                           warning �� ������ ���� ����� ���� �����Ѵ�.
//#define _CRT_SECURE_NO_WARNINGS

// ����� ���� ��
//////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////
// ���� �����δ� �Ϲ����� StdAfx.h �� ������

#if _MSC_VER > 1200 // 1200 = Visual C++ 6.0
#ifndef WINVER              
#define WINVER 0x0501		// WinXP
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // WinXP
#endif
#endif // _MSC_VER

#ifndef USE_MFC	// MFC ��� ����

#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#include <windows.h>
#include <tchar.h>

#else			// MFC ���

#define VC_EXTRALEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE Ŭ�����Դϴ�.
#include <afxodlgs.h>       // MFC OLE ��ȭ ���� Ŭ�����Դϴ�.
#include <afxdisp.h>        // MFC �ڵ�ȭ Ŭ�����Դϴ�.
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC �����ͺ��̽� Ŭ�����Դϴ�.
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO �����ͺ��̽� Ŭ�����Դϴ�.
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#endif // USE_MFC

// TODO: reference additional headers your program requires here

#include "ATPluginFrame/ATPluginFrame.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __STDAFX_H__
