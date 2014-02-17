#include "StdAfx.h"
#include "FLFunctionString.h"

FLFunctionString::FLFunctionString(void)
{
	//�ʱ�ȭ
	OptLen=0;
	Len=0;
	AddressLen=0;
	Type.clear();
	Text.clear();
	TOnly=false;
}

FLFunctionString::~FLFunctionString(void)
{
}

void FLFunctionString::InitFunc(int Len,int TextOnly, wstring FuncString)
{

	if(TextOnly>0)
	{
		FLFunctionString::TOnly=true;
	}
	else
	{
		FLFunctionString::TOnly=false;
	}

	OptLen=0;
	OptLen=Len;
	FLFunctionString::Len=Len;

	int Flag=-1;
	int OldFlag=-1;
	//Flag ����
	//0 : �Լ��κ�(Hex)
	//1 : �Լ��κ�(���ڿ�)
	//2 : �ؽ�Ʈ, ����(^)
	//3 : �ؽ�Ʈ, ��(*)
	//4 : �ؽ�Ʈ, ����, �̹���(!)
	wstring::size_type FSLen=FuncString.length();
	wstring::size_type SPoint=0;
	wstring::size_type Divider=0;

	bool isSet=false;
	for(Divider=0;Divider<=FSLen;Divider++)
	{
		isSet=false;

		if(Divider==FSLen)
		{
			isSet=true;
		}
		else
		{
			switch(FuncString[Divider])
			{
			case L'h':
			case L'H':
				Flag=0;
				isSet=true;
				break;
			
			case L't':
			case L'T':
				Flag=1;
				isSet=true;
				break;

			case L'^':;
				Flag=2;
				isSet=true;
				break;

			case L'*':;
				Flag=3;
				isSet=true;
				break;

			case L'!':;
				Flag=4;
				isSet=true;
				break;

			default:
				break;
			}
		}

		if(isSet)
		{
			
			//������ SPoint~Divider-1����

			if(OldFlag!=-1)
				Type.push_back(OldFlag);
			switch(OldFlag)
			{
			case 0:
			case 1:
				//hex ������
				Text.push_back(HexToString(FuncString.substr(SPoint+1,Divider-SPoint-1)));
				break;

			case 2:
			case 3:
			case 4:
				//�ϴ� ���ڿ�
				Text.push_back("");
				break;
				
			default:;
			}

			if(OldFlag!=-1)
			{
				AddressLen++;
			}
			OldFlag=Flag;
			SPoint=Divider;
		}
	}
}
bool FLFunctionString::AddText(int Address, string PushText)
{
	if(Address<AddressLen&&Type[Address]>1)
	{
		//����� �� ������ ������ ������!
		if(Type[Address]==3)
			Len-=(int)Text[Address].length();
		//���� ���̸� ����(�Ϻ��� ������ �����������!)

		if(Text[Address].length()>0&&PushText.length()==0)
		{
			//������ ������ �ִµ� �������� ������ ������
			Text[Address]="����";
			//���� ���
		}
		else
		{
			//���� �����̶��
			Text[Address]=PushText;
			//�ؽ�Ʈ�� �߰�
		}

		if(Type[Address]==3)
			Len+=(int)Text[Address].length();
		//���̸� ���Ѵ�.

		return true;
		//���������Ƿ�
	}
	else
		return false;
}

string FLFunctionString::GetSubText(queue<int>* AddressList)
{
	//Source�� �ڿ� _##*�� ���̸� �ؽ�Ʈ�� �մ´�.
	//�׸��� Queue�� �ڿ� ���� ��ȣ�� ���δ�.

	
	string Source="";
	for(int i=0;i<AddressLen;i++)
	{
		if(Type[i]==2||Type[i]==3)
		{
			//Ÿ���� �����ϴ� �ؽ��� ^,* �϶�
			//!�� �������� �����Ƿ� �����Ѵ�.
			AddressList->push(i);
			Source+="_##*";
			Source+=Text[i];
		}
	}
	AddressList->push(-1);//�̳༮�� �������� �����ٴ� ��ȣ
	return Source;
}

string FLFunctionString::GetFullText()
{
	string Temp="";
	for(int i=0;i<AddressLen;i++)
	{
		Temp+=Text[i];
	}
	return Temp;
}

string FLFunctionString::GetOnlyText()
{
	string Temp="";
	for(int i=0;i<AddressLen;i++)
	{
		if(Type[i]==2)
		{
			Temp+=Text[i];
		}
	}
	return Temp;
}

int FLFunctionString::Length()
{
	return Len;
}

int FLFunctionString::FindFunc(std::string Source, int Start)
{
	//�Լ� ������ �̿��� �ҽ� ��Ʈ������ �̰��� �´°� ã�Ƴ���.
	//ã�Ƴ� ���Ŀ��� �Լ� ���̸� ��ȯ!
	//������ ���� -1

	//Head Check
	int HeadLen=(int)Text[0].length();
	int Len=HeadLen;

	int SourceLen=(int)Source.length();

	for(int i=0;i<HeadLen;i++)
	{
		if(Start+i>=SourceLen)
		{
			//���� ������ ������
			Len=-1;
			break;
		}

		if(Text[0][i]!=Source[Start+i])
		{
			Len=-1;
			break;
		}
	}
	//Head üũ �Ϸ�

	
	if(Len!=-1)
	{
		string::size_type SPoint=Start+HeadLen;
		for(int Address=1;Address<AddressLen;Address++)
		{
			if(Type[Address]<2)
			{
				SPoint=Find(Source,Text[Address],SPoint,932);
				if(SPoint==Source.npos)
				{
					Len=-1;
					break;
				}
				else
				{
					Len=(int)SPoint+(int)Text[Address].length()-Start;
				}
			}
		}
	}
	return Len;
}

void FLFunctionString::InitText(string FuncSource)
{
	//String�� ã�� �߰���Ų��.
	//ã�� �������� �߰���Ű�°� ȿ���� ������,
	//��¿ �� ����.

	//�߰��Ǵ� ������ : �Ͼ� ����

	vector <string::size_type> Where;
	Where.push_back(0);
	//�ּ� ã��
	//�Լ� ������ ã���� �װ��� �������� �ؽ�Ʈ ������ ã�� �� �ִ�.
	for(int Address=1;Address<AddressLen;Address++)
	{
		if(Type[Address]<2)
		{
			Where.push_back(Find(FuncSource,Text[Address],Where[Address-1],932));
		}
		else
		{
			Where.push_back(Where[Address-1]+Text[Address-1].length());
		}
	}

	//�ؽ�Ʈ�߰�
	for(int Address=1;Address<AddressLen-1;Address++)
	{
		if(Type[Address]>=2)
		{
			AddText(Address,FuncSource.substr(Where[Address],Where[Address+1]-Where[Address]));
		}
	}
}

bool FLFunctionString::TextOnly()
{
	return FLFunctionString::TOnly;
}

string::size_type FLFunctionString::Find(string Source,string Key, string::size_type Start,int CodePage)
{
	string::size_type Finder=Start;
	string::size_type Len=Source.length();
	string::size_type KeyLen=Key.length();

	string::size_type ReturnData=Source.npos;

	while(Finder<Len)
	{
		if(Source[Finder]==Key[0])
		{
			string::size_type KeyFinder=1;
			ReturnData=Finder;
			while(KeyFinder<KeyLen)
			{
				if(Source[Finder+KeyFinder]!=Key[KeyFinder])
				{
					ReturnData=Source.npos;
					break;
				}
				KeyFinder++;
			}
			
			if(ReturnData!=Source.npos)
			{
				//���� ã�Ҵٸ�
				break;
			}
		}
		else if(IsDBCSLeadByteEx(CodePage,Source[Finder]))
		{
			Finder+=2;
		}
		else
		{
			Finder++;
		}
	}

	return ReturnData;
}