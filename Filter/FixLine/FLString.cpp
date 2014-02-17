#include "StdAfx.h"
#include "FLString.h"

FLString::FLString(bool bFunc)
{
	m_bFunc=bFunc;
	m_nFullLen=-1;
	m_strFullString.clear();
	m_bChanged=true;
}

FLString::~FLString(void)
{
}

FLString::FLString(string strText, int nType, bool bDelete)
{
	if(nType==0)
	{
		m_bFunc=false;
	}
	else
	{
		m_bFunc=true;
	}
	PushBack(nType,strText,strText.length(),bDelete);
}

void FLString::PushBack(int nType, string strText, int nLen, bool bDelete)
{
	//������ ȣȯ��
	bool bFunc=0;
	bool bTrans=0;
	int nSize=0;

	if(nType>=4)
	{
		bFunc=true;

		if(nType==4)	nSize=nLen;
		else			nSize=-2;
	}
	else
	{
		bFunc=false;

		if(nType/2==1)	bTrans=false;
		else			bTrans=true;

		if(nType%2==0)	nSize=-1;
		else			nSize=-2;
	}

	PushBack(strText,bFunc,bTrans,nSize,bDelete);

	m_nFullLen=-1;
	m_strFullString.clear();
}

void FLString::PushBack(FLStringElement Type,string strText)
{
	if(!Type.bDel)
	{
		mElementPool.push_back(Type);
		mElementPool[mElementPool.size()-1].strText = strText;
		mElementPool[mElementPool.size()-1].nLen=strText.length();
	}
}

void FLString::PushBack(string strText, bool bFunc, bool bTrans, int nSize,
						bool bDel, bool bPass, int nMin, int nMax)
{
	FLStringElement Temp;

	Temp.strText = strText;

	Temp.bFunc=bFunc;
	Temp.bTrans=bTrans;
	Temp.nSize=nSize;

	Temp.bDel=bDel;
	Temp.bPass=bPass;
	Temp.nMin=nMin;
	Temp.nMax=nMax;
	Temp.nLen=strText.length();

	mElementPool.push_back(Temp);

	m_nFullLen=-1;
	m_strFullString.clear();

	m_bChanged=true;
}

FLString FLString::MakeString(string strOText,int * nLength,FLString FLType, int nStart)
{
	vector <vector <int>> FuncList;

	try
	{
		for(int i=0;i<FLType.GetSize();i++)
		{
			vector<int> SubTemp;
			string::size_type nPos=nStart;
			string strText = FLType.GetSubString(i);

			if(FLType.GetSubFunc(1))
			{
				while(1)
				{
					nPos=Find(strOText,strText,nPos,932);
					if(nPos<0) break;

					SubTemp.push_back(nPos);
					nPos++;
				}
			}
			else
			{
				SubTemp.push_back(-1);
			}

			if(FLType.GetSubFunc(i) && !FLType.GetSubPass(i) && SubTemp.size()==0)
				throw 0;

			FuncList.push_back(SubTemp);
		}
		return FLString::MakeString(strOText,nLength,FLType,FuncList,nStart);
	}
	catch (int Err)
	{
		*nLength = -1;
		FLString FLBlank;
		return FLBlank;
	}
	
}
FLString FLString::MakeString(string strOText,int * nLength,FLString FLType, vector<vector<int>> FuncList,int nStart)
{
	try{
		//���� �˻��� =_=
		vector<int> SubData;
		vector<vector<int>> CompleteList; //���⿡�� �Ϸ�� �༮���� �״´�.

		FLString::TryRecur(strOText.length(),2048,&FLType,&FuncList,&CompleteList,&SubData,0,nStart); //SubData�� �ӽ÷� �ڸ��� �����ش�.

		if(CompleteList.size()==0)
			throw 0;

		//���� ����
		int Ans=0;
		int AnsLen=99999;
		int AnsPass=9999;
		int i=0;
		int j=0;
		for(i=0;i<CompleteList.size();i++)
		{
			vector<int>* SubList = &CompleteList[i];
			int nPass=0;
			int nFuncN=SubList->size();

			while(SubList->at(nFuncN-1)==-1)	nFuncN--;

			int nLen=SubList->at(nFuncN-1);

			if(FLType.GetSubFunc(nFuncN-1))	nLen+=FLType.GetSubLen(nFuncN-1);
			else
			{
				//�ؽ�Ʈ�ε�, �� ��쿡�� �� �־�� �ϴ°� Ȯ���غ��� -_-;
				if(FLType.GetSubPass(nFuncN-1))
				{
					//�����? �׷��� -_- �� ���� ���� ��
					nPass=256;
				}
				else
				{
					//��� �ȵɶ��� �̰� ����.
					nLen=strOText.length();
				}
			}

			for(int j=0;j<SubList->size();j++)
			{
				if(SubList->at(j)==-1)
					nPass++;
			}

			if(AnsPass>=nPass && AnsLen>=nLen)
			{
				AnsPass=nPass;
				AnsLen=nLen;
				Ans=i;
			}
		}
		FLString FLTemp;
		vector<int>* FinalList = &CompleteList[Ans];
		

		i=0;
		j=0;
		for(i=0;i<FLType.GetSize()-1;i++)
		{
			if(FinalList->at(i)!=-1) //����� �ƴ� ��
			{
				for(j=i+1;j<FLType.GetSize();j++)
				{
					if(FinalList->at(j)!=-1)//�굵 ����� �ƴҶ�
						break;
				}
				if(j!=FLType.GetSize())
				{
					FLTemp.PushBack(FLType.GetSubElement(i),strOText.substr(FinalList->at(i),FinalList->at(j)-FinalList->at(i)));
					i=j-1;
				}
			}
		}
		for(i=FLType.GetSize()-1;FinalList->at(i)==-1;i--);
		if(AnsLen-FinalList->at(i)>0)
			FLTemp.PushBack(FLType.GetSubElement(i),strOText.substr(FinalList->at(i),AnsLen-FinalList->at(i)));

		*nLength=AnsLen-nStart;

		return FLTemp;
	}
	catch(int Err)
	{
		*nLength = -1;
		FLString FLBlank;
		return FLBlank;
	}
}

void FLString::TryRecur(int MaxLen,int TmpMax,FLString* FLType,vector<vector<int>>* FuncList,
					 vector<vector<int>>* CompleteList,vector<int>* ArrangedList,int nThis,int nPos)
{
	//���� �밡�� -_-+
	if(nPos<=MaxLen)
	{
		if(nThis>=FLType->GetSize())
		{
			//���� ������ �Դٴ�!!!! ��������
			CompleteList->push_back(*ArrangedList);
		}
		else
		{
			if(FLType->GetSubFunc(nThis))
			{
				//������ ���ڿ�-_-?
				//�ϴ� ������ ������
				vector<int>* ThisList=&FuncList->at(nThis);
				for(int i=0;i<ThisList->size();i++)
				{
					int nThisPos=ThisList->at(i);

					int nLast=ArrangedList->size()-1;
					for(;nLast>=0&&ArrangedList->at(nLast)==-1;nLast--); //���� �������� '����' �༮�� ã�´�
					if(nLast>=0 && FLType->GetSubFunc(nLast))
					{
						//���̰� �³� üũ������
						if(ArrangedList->at(nLast)+FLType->GetSubString(nLast).length()!=nThisPos)
						{
							//���� ���ڿ��� ������ �̹� ���ڿ��� ���� ������
							continue;
							//����
						}
					}
					else if(nThis==0)
					{
						//�����϶� �������� �ƴϸ�
						if(nThisPos!=nPos)
							continue;
					}
					if(nThisPos>=nPos&&nThisPos<=TmpMax)
					{
						//���� ���ǿ� �´°͸� ó������
						ArrangedList->push_back(nThisPos);
						TryRecur(MaxLen,2048,FLType,FuncList,CompleteList,ArrangedList,nThis+1,nThisPos+FLType->GetSubString(nThis).length());
						ArrangedList->pop_back();
					}
				}
				if(FLType->GetSubPass(nThis))
				{
					//��� ������ ���� ��������!
					ArrangedList->push_back(-1); //-1�� ����!
					TryRecur(MaxLen,TmpMax,FLType,FuncList,CompleteList,ArrangedList,nThis+1,nPos);
					ArrangedList->pop_back();
				}
			}
			else
			{
				//�ؽ�Ʈ
				ArrangedList->push_back(nPos);
				TryRecur(MaxLen,nPos+FLType->GetSubMax(nThis)-FLType->GetSubMin(nThis),FLType,FuncList,CompleteList,ArrangedList,nThis+1,nPos+FLType->GetSubMin(nThis));
				ArrangedList->pop_back();
				if(FLType->GetSubPass(nThis))
				{
					//���
					ArrangedList->push_back(-1);
					TryRecur(MaxLen,2048,FLType,FuncList,CompleteList,ArrangedList,nThis+1,nPos);// ���� �ʿ� ���� nPos+FLType->GetSubLen(nThis-1)����
					ArrangedList->pop_back();
				}
			}
		}
	}
}

void FLString::Clear()
{
	mElementPool.clear();
	m_strFullString.clear();
	m_nFullLen=-1;
}

//��ü ���ڿ� ��ȯ
string FLString::GetString()
{
	if(m_bChanged)
	{
		m_strFullString.clear();
		int nSize=mElementPool.size();

		for(int i=0;i<nSize;i++)
		{
			if(!mElementPool[i].bDel)
			{
				m_strFullString += mElementPool[i].strText;
			}
		}
	}

	m_bChanged=false;
	return m_strFullString;
}

//���� ��ȯ
int FLString::GetLength()
{
	if(m_nFullLen<0)
	{
		int nSize=mElementPool.size();

		m_nFullLen=0;
		for(int i=0;i<nSize;i++)
		{
			if(!mElementPool[i].bDel && mElementPool[i].nSize!=-2)
			{
				m_nFullLen += mElementPool[i].nLen;
			}
		}
	}

	return m_nFullLen;
}

bool FLString::GetFunc()
{
	return m_bFunc;
}

int FLString::GetType()
{
	return m_nType;
}

void FLString::SetFunc(bool bFunc)
{
	m_bFunc = bFunc;
}

void FLString::SetType(int nType)
{
	m_nType = nType;
}

void FLString::SetTrim(bool bTrim){
	m_bUseTrim = bTrim;
}

bool FLString::GetTrim(){
	return m_bUseTrim;
}

int FLString::GetSize()
{
	return mElementPool.size();
}

FLStringElement FLString::GetSubElement(int nNumber)
{
	return mElementPool[nNumber];
}

string FLString::GetSubString(int nNumber)
{
	return mElementPool[nNumber].strText;
}

bool FLString::GetSubFunc(int nNumber)
{
	return mElementPool[nNumber].bFunc;
}

int FLString::GetSubSize(int nNumber,bool bRealSize)
{
	int a=mElementPool[nNumber].nSize;
	if(bRealSize)
	{
		if(a==-1)				return mElementPool[nNumber].nLen;
		else if(a==-2)		return 0;
		else					return mElementPool[nNumber].nSize;
	}
	else return a;
}

int FLString::GetSubLen(int nNumber)
{
	return mElementPool[nNumber].nLen;
}

bool FLString::GetSubTrans(int nNumber)
{
	return mElementPool[nNumber].bTrans;
}

bool FLString::GetSubDelete(int nNumber)
{
	return mElementPool[nNumber].bDel;
}

bool FLString::GetSubPass(int nNumber)
{
	return mElementPool[nNumber].bPass;
}

int FLString::GetSubMin(int nNumber) //������ 0 �̻�
{
	int nMin=mElementPool[nNumber].nMin;
	if(nMin<0)	return 0;
	else			return nMin;
}

int FLString::GetSubMax(int nNumber) //������ 0 �̻�
{
	int nMax=mElementPool[nNumber].nMax;
	if(nMax<0)	return 99999;	//���� ���̰� 2048�̴ϱ� ����� (����)�̴�
	else			return nMax;
}


void FLString::SetSubString(int nNumber,string strText)
{
	if(strText.length()>0){
		if(m_bUseTrim){
			strText=trim(strText);
		}
		mElementPool[nNumber].strText = strText;
		mElementPool[nNumber].nLen=strText.length();
	}
	m_bChanged=true;
}