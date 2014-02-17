// Debug - ����׿� ���� ǥ�� ���̺귯��
//

#ifndef __DEBUG_H__
#define __DEBUG_H__

// FORCE_DEBUG_LOG : ���� ����� ����϶��� ����� �޼��� ���, ������ ����϶��� ����
//                   ������ FORCE_DEBUG_LOG �� ���������� ������ ����� �޼����� ���
//#define FORCE_DEBUG_LOG

#define DEBUG_LOG_TYPE	2
#define DEBUG_LOG_FILEPATH	"D:\\ATCTNR.log"

// �����ڵ� : wLow(������ �����ڵ�), wHigh(���ι�ȣ)
#define ERROR_CODE(e) MAKELONG(e, __LINE__)

// SetLastError : ������ �����ڵ� + 29��° ��Ʈ on (=application-defined error code)
#define SETLASTERROR(error_code) SetLastError(LOWORD(error_code) | 0x20000000)

#if defined(_DEBUG) || defined(FORCE_DEBUG_LOG)
// ����� ��� ������

#include <stdio.h>
#include <stdarg.h>

class CDebugLog {
	FILE *m_fp;

	public:
		CDebugLog();
		~CDebugLog() { if (m_fp) fclose(m_fp); }

		void Log(LPCTSTR cszFormat, ...);
};

extern CDebugLog g_cDebugLog;

#define NOTIFY_ERROR(error_code)	NotifyError(_T(__FILE__), HIWORD(error_code), LOWORD(error_code))	// ���� �˸�
#define NOTIFY_DEBUG_MESSAGE	g_cDebugLog.Log	// ����� �޼��� �˸� (printf ó�� ���)

void NotifyError(LPCTSTR cszFileName, DWORD dwLineNo, DWORD dwErrCode);



#else
// ������ ��� ������

#define NOTIFY_ERROR(error_code)	// �ƹ� �ϵ� ���� �ʴ´�.
#define NOTIFY_DEBUG_MESSAGE(...)

#endif // _DEBUG

#endif // __DEBUG_H__