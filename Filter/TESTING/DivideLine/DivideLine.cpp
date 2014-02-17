#include "StdAfx.h"
#include "DivideLine.h"

CDivideLine g_cDivideLine;

CDivideLine::CDivideLine(void)
{
	m_nReturnLen	= 2048;
	m_nMaxChar		= 2048;
	m_nMaxLine		= 6;
	m_nIgnoreLine	= 0;

	m_strReturnChar	= "\n";
	m_nReturnCharLen= 1;

	m_bRemoveSpace	= FALSE;
	m_bTwoByte		= FALSE;
}

CDivideLine::~CDivideLine(void)
{
}

void CDivideLine::SetBase(int nIgnoreLine, int nMaxChar, int nMaxLine,
					   int nReturnLen, string strReturnChar)
{
	m_nIgnoreLine	= nIgnoreLine;
	m_nMaxChar		= nMaxChar;
	m_nMaxLine		= nMaxLine;
	m_nReturnLen	= nReturnLen;

	m_strReturnChar	= strReturnChar;
	m_nReturnCharLen= (int)m_strReturnChar.length();
}

int CDivideLine::SplitLines(vector <string>* NewLines,vector <string> Lines)
{
	//FixLine���� �״�� ������
	//�Լ�ó�� �κи� ��¦ �����

	NewLines->clear();
	int NewLineN=0;

	int IgnoreModeFuncN=0;
	int FuncN=0;
	int LineN=(int)Lines.size();
	//�ű���!
	for(int i=0;i<LineN;i++)
	{
		string::size_type SPoint=0;
		string::size_type Divider=0;
		string::size_type OldDivider=0;
		int CharN=0;
		while(1)
		{
			OldDivider=Divider;

			if(Divider>=Lines[i].length())
			{
				NewLines->push_back(Lines[i].substr(SPoint,Lines[i].length()-SPoint));
				NewLineN++;
				break;
			}
			//���б�

			if(Divider<Lines[i].length()-6&&
				//_*&n,Len&*_
				Lines[i][Divider]=='_'&&
				Lines[i][Divider+1]=='*'&&
				Lines[i][Divider+2]=='&')
			{
				//�Լ���� On
				//�� �����ϱ� ������ [....]

				string::size_type DividerEnd=Lines[i].find("&*_",Divider+3);

				string FuncNTemp=Lines[i].substr(Divider+3,DividerEnd-Divider-3);
				
				//DivideLine���� �߰��� �κ�
				string::size_type nTab=FuncNTemp.find(',');
				string strFuncNLen=FuncNTemp.substr(nTab+1,FuncNTemp.length()-nTab-1);
				int nFuncLen=atoi(strFuncNLen.c_str());

				Divider=DividerEnd+3;
				CharN+=nFuncLen;
			}
			else if(IsDBCSLeadByteEx(949,Lines[i][Divider])){
				//����
				Divider+=2;
				CharN+=2;
			}
			else
			{
				//�ݰ�
				Divider+=1;
				CharN+=1;
			}

			if(CharN>m_nMaxChar)
			{
				NewLines->push_back(Lines[i].substr(SPoint,OldDivider-SPoint));
				SPoint=OldDivider;
				NewLineN++;

				CharN=0;

				if(NewLineN<=m_nIgnoreLine)
				{
					//���ö����̸�, ���̻� �Է����� �ʰ� �ڸ�
					break;
				}
			}
		}//while����
	}//for ����
	return NewLineN;
}

void CDivideLine::PreDivideLine(LPSTR szDest, LPCSTR szSource)
{
	string strSource=szSource;
	string strDest="";

	//��ó��
	//�� �� �����̸� ���๮�� ���� �̾� �������� ������ �ǵ��� �Ѵ�.
	
	//FixLine���� �ۿԴ�.

	//���Ϳ� �ٷ� ������
	vector <string> Lines;
	//���๮�ڷ� �ɰ� Lines�� ��´�.
	string::size_type SPoint=0;
	string::size_type Divider=0;
	int LineN=0;
	while(1)
	{

		LineN++;

		Divider=strSource.find(m_strReturnChar,SPoint);
		if(Divider==strSource.npos)
		{
			//���̻� ������
			Lines.push_back(strSource.substr(SPoint,strSource.length()-SPoint));
			break;
		}
		else
		{
			//���๮�ڰ� ������
			Lines.push_back(strSource.substr(SPoint,Divider-SPoint));
			SPoint=Divider+m_nReturnCharLen;
		}
	}
	
	//���̸� üũ�ؼ� ���쿡�� ���� �Ͱ� �̾ ����Ѵ�.
	//FixLine �״�� �ۿԴ�.
	for(int i=0;i<LineN;i++)
	{
		bool AddReturnChar=false;
		strDest+=Lines[i];
		//�ϴ� �߰��ϰ�
		if(i<m_nIgnoreLine)
		{
			//���ö����̸�
			if(i<LineN-1)
			{
				AddReturnChar=true;
			}
		}
		else
		{
			//��������̶�� ����üũ
			string::size_type thisLen=0;
			string::size_type SPoint=0;
			string::size_type Divider=0;
			while(1)
			{
				//�Լ��� ã�Ƽ� ���̸� ���� ���Ѵ�
				Divider = Lines[i].find("_*&",SPoint);
				if(Divider==Lines[i].npos)
				{
					//���̻� �Լ��� ���ٸ�!
					thisLen+=Lines[i].length()-SPoint;
					break;
				}
				else
				{
					//�Լ��� �ִٸ�
					string::size_type DividerEnd=Lines[i].find("&*_",Divider+3);

					string FuncNTemp=Lines[i].substr(Divider+3,DividerEnd-Divider-3);


					//DivideLine���� �߰��� �κ�
					string::size_type nTab=FuncNTemp.find(',');
					string strFuncNLen=FuncNTemp.substr(nTab+1,FuncNTemp.length()-nTab-1);
					int nFuncLen=atoi(strFuncNLen.c_str());

					thisLen+=Divider-SPoint;
					thisLen+=nFuncLen;

					SPoint=DividerEnd+3;
				}
			}

			//����üũ!
			if((int)thisLen<m_nReturnLen&&i<LineN-1)
			{
				//������ �ʾҴٸ� �����߰�
				AddReturnChar=true;
			}
		}
		
		if(AddReturnChar)
		{
			//�����ȣ �߰��� �����Ǿ��ٸ�
			strDest+='\n';
		}
	}
	
}

void CDivideLine::PostDivideLine(LPSTR szDest, LPCSTR szSource)
{
	//FixLine���� �Ϻκи� ������
	string strSource=szSource;


	string strStep1;
	if(m_bRemoveSpace==FALSE&&m_bTwoByte==FALSE)
	{
		//�ɼ� �ƹ��͵� ���Ѹ� �� �۾� �� �ʿ䰡 ���� =��=;
		strStep1=strSource;
	}
	else
	{
		string::size_type Len=strSource.length();
		string::size_type SPoint=0;
		string::size_type Divider=0;
		while(SPoint<Len)
		{
			Divider=strSource.find("_*&",SPoint);
			if(Divider==strSource.npos)
			{
				Divider=Len;
			}

			for(string::size_type i=SPoint;i<Divider;i++)
			{
				if(strSource[i]==' '&&m_bRemoveSpace!=FALSE)
				{
					//�ƹ��͵� ���� �ʴ´�.
				}
				else if(IsDBCSLeadByteEx(949,strSource[i]))
				{
					//����
					strStep1+=strSource[i];
					i++;
					strStep1+=strSource[i];
				}
				else if(m_bTwoByte!=FALSE)
				{
					strStep1+=(char)0xA3;
					strStep1+=strSource[i]+(char)0x80;
				}
				else
				{
					strStep1+=strSource[i];
				}
			}

			if(Divider!=Len)
			{
				string::size_type TempDivider=Divider;
				Divider=strSource.find("&*_",TempDivider);

				strStep1+=strSource.substr(TempDivider,Divider-TempDivider+3);

				SPoint = Divider+3;
			}
			else
			{
				SPoint=Divider;
			}
		}
	}

	//��ó��
	//���� FixLine���� ������

	//���ɰ���
	vector <string> Lines;
	int LineN=0;
	bool OverText=false;
	{

		//string strStep1
		string::size_type SPoint=0;
		string::size_type Divider=0;
		while(1)
		{

			LineN++;

			Divider=strStep1.find('\n',SPoint);
			if(Divider==strStep1.npos)
			{
				//���̻� ������
				Lines.push_back(strStep1.substr(SPoint,strStep1.length()-SPoint));
				break;
			}
			else
			{
				//���๮�ڰ� ������
				Lines.push_back(strStep1.substr(SPoint,Divider-SPoint));
				SPoint=Divider+1;
			}
		}
	}

	//�ɰ���
	//���� FixLine ��������
	vector <string> NewLines;
	if(SplitLines(&NewLines,Lines)>m_nMaxLine)
	{
		//��2���
		//IgnoreLine�� ������� ó���ϰ�,
		//���� ���� ���� �̾���δ�.

		
		vector <string> TempLines;
		for(int j=0;j<m_nIgnoreLine;j++)
		{
			TempLines.push_back(Lines[j]);
		}
		string Mode2Source="";
		for(int j=m_nIgnoreLine;j<LineN;j++)
		{
			Mode2Source+=Lines[j];
		}
		TempLines.push_back(Mode2Source);
		//�� �̾���̰�

		//�ѹ� �� ��ȯ =��=;
		SplitLines(&NewLines,TempLines);
	}

	string strDest="";
	int i=0;
	for(i=0;i<(int)NewLines.size()-1&&i<m_nMaxLine-1;i++)
	{
		strDest+=NewLines[i];
		strDest+=m_strReturnChar;
	}
	strDest+=NewLines[i];

	lstrcpyA(szDest,strDest.c_str());
}