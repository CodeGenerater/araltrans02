#include "../StdAfx.h"
#include "Misc.h"


//---------------------------------------------------------------------------
//	���Լ��� : GetFileVersion
//	���Լ����� : ������ ������ ������ �������ش�.
//	������ : CString �����θ� ������ �����̸�
//	�ݹ�ȯ�� : CString ������ ���� ("xx.xx.xx.xx")
//---------------------------------------------------------------------------
CString GetFileVersion(CString strFilePathName)
{
	DWORD dwHdlDest = 0;
	DWORD dwSizeDest = 0;
	DWORD dwDestLS, dwDestMS;
	CString strVersion = _T("");

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
	return strVersion;
}