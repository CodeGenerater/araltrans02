#include "StdAfx.h"
#include "CustomDic2.h"
#include "tstring.h"
#include "Util.h"
#include "hash.hpp"

CAutoLoad	*m_pAutoLoad = NULL;

const char * cszDivider = "MadeByHideD";
const int nDividerLen = strlen(cszDivider);

CCustomDic2::CCustomDic2(){
	NullWord.WordLen=-1;
	NullWord.WordN=-1;

	KeyIndex.clear();
	KeyBook.clear();
	ValueList.clear();
	while(!KeyList.empty())
	{
		KeyList.pop();
	}

	m_bNoMargin=false;

	//AfxBeginThread
	m_pAutoLoad = new CAutoLoad;
}

void CCustomDic2::Init()
{
	m_pAutoLoad->StartThread();
}

void CCustomDic2::SetMarginMode(bool bNoMargin){
	m_bNoMargin=bNoMargin;
}

void CCustomDic2::End()
{
	if(m_pAutoLoad)
	{
		m_pAutoLoad->EndThread();
		delete m_pAutoLoad;
		m_pAutoLoad=NULL;
	}
}

CCustomDic2::~CCustomDic2(void)
{
	if(m_pAutoLoad)
	{
		m_pAutoLoad->EndThread();
		delete m_pAutoLoad;
		m_pAutoLoad=NULL;
	}
}

string CCustomDic2::GetValue(int KeyN)
{
	return ValueList[KeyN];
	//���
}

void CCustomDic2::FileClear()
{
	m_pAutoLoad->FileClear();
}

DicWord CCustomDic2::FindKey(LPCSTR Source, int Start, int End)
{
	//�켱 ���ڿ��� �߶󳻵��� ���� =��=;
	char FindText[2048]="";
	int _End=(int)strlen(Source)-1; //������'����'�̴ϱ�

	if(End!=-1 && End<=_End){
		//End���� -1�� �ƴϰ�(-1�� ������!)
		//End���� ���밡���� ���� ���� �ִٸ�.
		_End=End;
	}

	int TextLen=0;
	for(int i=Start;i<=End;i++)FindText[TextLen++]=Source[i];
	FindText[TextLen]='\0';
	//����


	//�˻� �۾� ����.
	map<int,int>::iterator iterIndex;
	int IndexChar=65536*TextLen+FindText[0]*256+FindText[1];

	iterIndex = KeyIndex.find(IndexChar); //1�� �˻�
	if(iterIndex==KeyIndex.end()){
		//���� ���ٸ�.
		return NullWord;
	}
	else{
		UINT Hash=MakeStringHash(FindText); //Hash��
		map<UINT,DicWord>::iterator iterBook;

		int ValuePointer=iterIndex->second;

		iterBook = KeyBook[ValuePointer].find(Hash); //2�� �˻�
		if(iterBook != KeyBook[ValuePointer].end()){
			//2�ܰ���� ��� �ִٸ�,
			return iterBook->second;
		}
		else{
			//���⵵ ���ٸ�,
			return NullWord;
		}
	}
}

void CCustomDic2::PreCustomDic(LPSTR Dest, LPCSTR Source)
{
	//Thread�κ��� ����� ������ �ִ°� Ȯ���ؼ� �޾ƿ´�.
	if(m_pAutoLoad->GetChanged())
	{
		g_cs.Lock();
		m_pAutoLoad->SetChanged();
		KeyIndex = m_pAutoLoad->GetKeyIndex();
		KeyBook = m_pAutoLoad->GetKeyBook();
		ValueList = m_pAutoLoad->GetValueList();
		g_cs.Unlock();
	}

	int SourceLen=(int)strlen(Source);
	DicWord TempWord;

	while(!KeyList.empty())
	{
		KeyList.pop();
	}

	string Temp;
	int SPoint=0;

	char atoi_tmp[10]="";

	for(int head=0;head<SourceLen;head++)
	{
		for(int tail=SourceLen-1;tail>head;tail--)
		{
			TempWord=FindKey(Source,(int)head,(int)tail);
			if(TempWord.WordLen>0){
				//���� �ִٸ�!
				//���̶�� -1 (CCustomDic2::CCustomDic2���� ����)�� �����Ե�
				
				//�պκ��� ����
				Temp+=string(Source,SPoint,head-SPoint);

				//�ܾ� �߰�
				KeyList.push(TempWord.WordN);
				Temp+= cszDivider;
				SPoint=tail+1;
				head=tail;
				break;
			}
		}
		 //2����Ʈ ó��, ����ӵ� ����
         if((BYTE)Source[head]>0x80){
              //�������ڶ��
			 if((BYTE)Source[head]<0xA0||(BYTE)Source[head]>0xDF)
              head++;
         }
	}
	Temp+=string(Source,SPoint,SourceLen-SPoint); //������

	lstrcpyA(Dest,Temp.c_str());
}

void CCustomDic2::PostCustomDic(LPSTR Dest, LPCSTR Source)
{
	string Temp=Source;
	string Temp2="";
	//Temp -> Temp2 �帧

	string::size_type DicTagHead = 0;
	string::size_type DicTagTail = 0;

	string::size_type LastTail = 0;

	bool Added=false;

	for(;;){
		//���� �ݺ�
		DicTagHead=Temp.find(cszDivider,DicTagTail);
		if(DicTagHead != string::npos)
		{
			Temp2 += RemoveSpace(Temp.substr(DicTagTail,DicTagHead-DicTagTail),!Added);
			DicTagTail=DicTagHead+nDividerLen;
			LastTail = DicTagTail;

			if(!KeyList.empty())
			{
				Temp2 += GetValue(KeyList.front());
				KeyList.pop();
			}
			//���� �߰�.
			Added=true;
		}
		else{
			//�������� �����ϱ�[...]
			break;
		}
	}
	Temp2 += RemoveSpace(Temp.substr(LastTail,Temp.length()-LastTail),!Added);
		
	//������[...]

	lstrcpyA(Dest,Temp2.c_str());
}
string CCustomDic2::RemoveSpace(std::string &strText,bool bHead){
	//IsDBCSLeadByteEx

	if(!m_bNoMargin)return strText;

	int nLen = strText.length();

	int nStart=0;
	int nMinus=0;

	if(!bHead&&strText[0]==' '){
		nStart++;
		nMinus++;
	}
	for(int i=0;i<nLen;i++){
		if(IsDBCSLeadByteEx(949,strText[i])){
			i++;
		}
		else if(i==nLen-1){
			if(strText[i]==' '){
				nMinus++;
			}
		}
	}

	if(nMinus==0)return strText;
	else return strText.substr(nStart,nLen-nMinus);

}
void CCustomDic2::AddDic(int Mode, LPCWSTR CustomPath)
{
	//���� ����
	wchar_t Path[4096]=L"";
	int PathLen=(int)wcslen(CustomPath);
	
	//Mode ����
	//0 - ����� ����
	//1 - GDic
	//������ - CDic

	if(Mode==0)
	{
		wcscpy_s(Path,4096,CustomPath);
		for(int i=PathLen-1;i>=0;i--){
			if(Path[i]==L'\\'){
				//������ ���丮�ΰ� �����ٸ�!
				wchar_t CharTmp=Path[i+1];
				Path[i+1]=L'\0';
				MyCreateDirectory(Path); //�ϴ� ������ ���Ѿ���!
				Path[i+1]=CharTmp;
				break;
			}
		}
	}
	else if(Mode==1)
	{
		wcscpy_s(Path,4096,GetATDirectory());
		wcscat_s(Path,4096,L"\\");
		MyCreateDirectory(Path);
		wcscat_s(Path,4096,L"CustomDic.txt");
	}
	else
	{
		wcscpy_s(Path,4096,GetGameDirectory());
		wcscat_s(Path,4096,L"\\ATData\\");
		MyCreateDirectory(Path);
		wcscat_s(Path,4096,L"CustomDic.txt");
	}

	m_pAutoLoad->AddDic(Path);
}