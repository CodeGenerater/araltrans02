#include "StdAfx.h"

#include "Util.h"

#include "ATPlugin.h"

BOOL MyCreateDirectory(LPCTSTR lpDirectory)
{
	DWORD pos,len;
	LPTSTR str;

	if (!lpDirectory) return FALSE;

	len=lstrlen(lpDirectory);
	str=new TCHAR [len+1];
	lstrcpy(str,lpDirectory);
	for (pos=0; str[pos]; pos++)
	{
		if (str[pos]==_T('\\'))
		{
			str[pos]=0;
			CreateDirectory(str,NULL);
			str[pos]=_T('\\');
		}
	}
	CreateDirectory(str,NULL);

	delete [] str;
	return CreateDirectory(lpDirectory,NULL) || ((pos=GetFileAttributes(lpDirectory)) != 0xFFFFFFFF && (pos & FILE_ATTRIBUTE_DIRECTORY));
}


const TCHAR *GetATDirectory()
{
	static TCHAR szATDirectory[MAX_PATH]={_T('\0'),};
	int i;
	
	if (szATDirectory[0] != _T('\0'))
		return szATDirectory;

	// �ƶ�Ʈ���� ��ġ ���丮�� �˾ƿ´�.
	
	GetModuleFileName(GetModuleHandle(_T("ATCTNR.DLL")), szATDirectory, MAX_PATH);
	
	for(i=lstrlen(szATDirectory); i>=0; i--)
	{
		if (szATDirectory[i] == _T('\\'))
		{
			szATDirectory[i]=_T('\0');
			break;
		}
	}
	return szATDirectory;
}

const TCHAR *GetGameDirectory()
{
	static TCHAR szGameDirectory[MAX_PATH]={_T('\0'),};
	int i;
	
	if (szGameDirectory[0] != _T('\0'))
		return szGameDirectory;

	//���� ���丮�� �˾ƿ´�.
	
	GetModuleFileName(GetModuleHandle(NULL), szGameDirectory, MAX_PATH);
	
	for(i=lstrlen(szGameDirectory); i>=0; i--)
	{
		if (szGameDirectory[i] == _T('\\'))
		{
			szGameDirectory[i]=_T('\0');
			break;
		}
	}
	return szGameDirectory;
}

void GetATOptionsFromOptionString(const char *pszOption, ATOPTION_ARRAY &aOptions)
{
	tstring strNow;

	int nParentheseCount;

	int i, nOptionSize;

	ATOPTION_PAIR option;

	aOptions.clear();

	nOptionSize=lstrlenA(pszOption);
	if (nOptionSize == 0)
		return;


	strNow.reserve(nOptionSize);
	nParentheseCount=0;

	for(i=0; i<nOptionSize; i++)
	{

		switch(pszOption[i])
		{
			case ',':
				if (nParentheseCount)
				{
					// �ɼ� ���� ,
					strNow+=MakeTChar(pszOption+i);
				}
				else
				{
					// () �� ���� ��� : ex) aOption,bOption
					if (!strNow.empty())
					{
						// value ���� �÷��������� �����ϰ� ���
						option.strKey = strNow;
						option.strValue.erase();
						aOptions.push_back(option);

						option.strKey.erase();
						strNow.erase();
					}
				}
				break;

			case '(':
				if (!nParentheseCount)
				{
					nParentheseCount++;
					// �ɼ� Key->�ɼ� Value �Ѿ
					option.strKey=strNow;
					strNow.erase();
				}
				else
				{
					// Value ������ (
					nParentheseCount++;
					strNow+=MakeTChar(pszOption+i);
				}
				break;

			case ')':
				if (nParentheseCount == 1)
				{
					// �ɼ� 1�� �Ϸ� - ���
					nParentheseCount--;
					option.strValue=strNow;
					aOptions.push_back(option);
	
					// ����Ÿ ����
					option.strKey.erase();
					option.strValue.erase();
					strNow.erase();
				}
				else if (nParentheseCount)
				{
					nParentheseCount--;
					// Value ������ )
					strNow+=MakeTChar(pszOption+i);
				}
				break;	// �� �� - ����

			default:
				strNow+=MakeTChar(pszOption+i);
				if (IsDBCSLeadByteEx(949, (BYTE) pszOption[i]))
					i++;
		}
	}
	// ���� strNow �� ���� ���������� �׳� ��Ͻ�Ų��.
	if (!strNow.empty())
	{
		if (nParentheseCount)
		{
			option.strValue=strNow;
			aOptions.push_back(option);
		}
		else
		{
			option.strKey=strNow;
			option.strValue.erase();
			aOptions.push_back(option);
		}

	}
}

void GetOptionStringFromATOptions(const ATOPTION_ARRAY &aOptions, char *pszOption, int nMaxOptionLength)
{
	tstring strOption;

	int i, nOptions;

	nOptions=(int)aOptions.size();

	for(i=0; i<nOptions; i++)
	{
		if (aOptions[i].strValue.empty())
			strOption+=aOptions[i].strKey;
		else
			strOption+=aOptions[i].strKey+_T("(")+aOptions[i].strValue+_T(")");

		if (i != nOptions-1)
			strOption+=_T(',');
	}
	Wide2Kor(strOption.c_str(), pszOption);
}
