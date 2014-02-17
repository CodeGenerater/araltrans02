#include "StdAfx.h"
#include "OutRuby.h"

COutRuby::COutRuby(void)
{
}

COutRuby::~COutRuby(void)
{
}

void COutRuby::SetRubyString(LPCSTR szRuby)
{
	//������ �ణ �޶� ������ ����Ѵ�.
	strRuby=szRuby;
	wstring strRubyEnd;
	int Len=(int)strRuby.length();

	bool isHead=true;
	for(int i=0;i<Len;i++)
	{
		if(isHead)
		{
			strRubyEnd+=L"H";
			isHead=false;
		}
		strRubyEnd+=(wchar_t)strRuby[i];

		if(strRuby[i]=='!' || strRuby[i]=='*' || strRuby[i]=='^')
		{
			isHead=true;
		}
	}

	RubyString.InitFunc(0,true,strRubyEnd);
}

LPCSTR COutRuby::GetRubyString()
{
	return strRuby.c_str();
}

void COutRuby::PreOutRuby(LPSTR szDest, LPCSTR szSource)
{

	//FixLine���� �Ϻθ� ������

	string strSource=szSource;
	string strDest;

	string::size_type Len=strSource.length();
	string::size_type Finder=0;


	
	while(Finder<Len)
	{

		//�ϴ� �̺κ��� �Լ����� Ȯ���غ���
		int Len=RubyString.FindFunc(strSource,(int)Finder);

		if(Len==-1)
		{
			//�׳� �Ϲ� ���ڿ��̶��
			if(IsDBCSLeadByteEx(932,strSource[Finder])){
				//����?
				strDest.push_back(strSource[Finder]);
				strDest.push_back(strSource[Finder+1]);
				Finder+=2;
			}
			else
			{
				//�ݰ�?
				strDest.push_back(strSource[Finder]);
				Finder++;
			}
		}
		else
		{
			//�Լ���� =��=;
			FLFunctionString TempFunc=RubyString;
			TempFunc.InitText(strSource.substr(Finder,Len));
			strDest+=TempFunc.GetOnlyText();
			Finder+=Len;
		}
	}

	lstrcpyA(szDest,strDest.c_str());
}