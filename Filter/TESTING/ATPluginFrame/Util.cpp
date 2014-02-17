#include "StdAfx.h"

#include "Util.h"

#include "ATPlugin.h"


const TCHAR *GetATDirectory()
{
	static TCHAR szDirectory[MAX_PATH]={_T('\0'),};
	int i;
	
	if (szDirectory[0] != _T('\0'))
		return szDirectory;

	// �ƶ�Ʈ���� ��ġ ���丮�� �˾ƿ´�.
	
	GetModuleFileName(GetModuleHandle(_T("ATCTNR.DLL")), szDirectory, MAX_PATH);
	
	for(i=lstrlen(szDirectory); i>=0; i--)
	{
		if (szDirectory[i] == _T('\\'))
		{
			szDirectory[i]=_T('\0');
			break;
		}
	}
	return szDirectory;
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

	nOptions=aOptions.size();

	for(i=0; i<nOptions; i++)
	{
		if (aOptions[i].strValue.empty())
			strOption+=aOptions[i].strKey;
		else
			strOption+=aOptions[i].strKey+_T("(")+aOptions[i].strValue+_T(")");

		if (i != nOptions-1)
			strOption+=_T(',');
	}
#ifdef _UNICODE
	Wide2Kor(strOption.c_str(), pszOption);
#else
	lstrcpyn(pszOption, strOption.c_str(), nMaxOptionLength);
#endif
}
