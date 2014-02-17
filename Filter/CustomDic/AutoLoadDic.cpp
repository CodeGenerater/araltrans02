#include "StdAfx.h"
#include "CustomDic.h"
#include "AutoLoadDic.h"
#include "DicStruct.h"
#include "SubFunc.h"
#include "ATPlugin.h"

_rankDic nullRankDic;//�뷮�� �ʹ� Ŀ�� null�̸� �� ���� �����ȴ�.

VOID CALLBACK TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
UINT AFX_CDECL AutoLoader(LPVOID target);
volatile LONG g_Run=0;

AutoLoadDic::AutoLoadDic(void)
{
	ASSERT(false);

}

AutoLoadDic::AutoLoadDic(CustomDic* dicBase){
	m_pDicBase=dicBase;
	m_nSeqN=1;
	m_pThread=NULL;
	m_bFirstLoad=true;
}

AutoLoadDic::~AutoLoadDic(void)
{
}

void AutoLoadDic::StopAutoLoad(){
	if(g_Run==2){
		KillTimer((HWND)g_pHwnd,(UINT_PTR)this);
	}
	InterlockedExchange(&g_Run,0);

	if(m_pThread!=NULL){
		::WaitForSingleObject(m_pThread->m_hThread,INFINITE);
		m_pThread->Delete();
		delete m_pThread;
		m_pThread=NULL;
	}
}

void AutoLoadDic::StartAutoLoad(){
	if(m_pThread==NULL&&m_nInterval>0){
		InterlockedExchange(&g_Run,2);//BootUp
		if(m_bFirstLoad){
			m_bFirstLoad=false;
			SetTimer((HWND)g_pHwnd,(UINT_PTR)this,2000,TimerProc);
		}
		else{
			__StartAutoLoad();
		}
	}
}

void AutoLoadDic::__StartAutoLoad(){
	InterlockedExchange(&g_Run,1);
	m_pThread = AfxBeginThread(AutoLoader,(LPVOID)this,0,0,CREATE_SUSPENDED);
	m_pThread->m_bAutoDelete=false;
	m_pThread->ResumeThread();
}
void AutoLoadDic::SetDic(vector<fileInfo>& files){
	StopAutoLoad();

	m_csLock.Lock();

	m_arFileList.clear();
	FILETIME timeTmp;
	timeTmp.dwHighDateTime=0;
	timeTmp.dwLowDateTime=0;
	for(auto iter = files.begin();iter!=files.end();++iter){
		m_arFileList.push_back(make_pair(*iter,timeTmp));
	}

	m_csLock.Unlock();
}

void AutoLoadDic::FirstLoad(){
	g_Run=1;
	CallBackProc();
	g_Run=0;
}

void AutoLoadDic::ReloadDic(fileInfo& fInfo){
	if(!g_Run)return;
	//���� ���⿡�� m_arFileList�� �������� ����!

	//���� �б�
	HANDLE hFile = CreateFile(fInfo.name.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE){
		ASSERT(false);//CallBack������ ����ߴµ� ���⿡�� �ȿ����� �ȵ���!!!
		return;
	}
	DWORD dwFileLen = GetFileSize(hFile,NULL);

	vector<BYTE> fileBuff;
	fileBuff.resize(dwFileLen);

	ReadFile(hFile,fileBuff.data(),dwFileLen,&dwFileLen,NULL);

	CloseHandle(hFile);

	vector<wchar_t> textBuff;

	//���� ��ȯ
	//Locale ��ȯ(UTF-8,UTF-16LE,BE) �� \r\n, \r, \n�� \n���� ����
	//\r\n�� �ܼ��� \n\n���� ó���ؼ� ���Ŀ� '����'�� ó���ϵ��� �Ѵ�.
	if(memcmp(fileBuff.data(),"\xFF\xFE",2)==0||
		memcmp(fileBuff.data(),"\xFE\xFF",2)==0){
		//UTF16-LE or BE
		textBuff.reserve(fileBuff.size()/2+1);

		bool bIsLE = fileBuff[0]==0xFF;
		for(size_t i=2;i<fileBuff.size();i+=2){
			wchar_t wch;
			if(bIsLE)wch = fileBuff[i]    | fileBuff[i+1]<<8;
			else	 wch = fileBuff[i]<<8 | fileBuff[i+1];
			
			switch(wch){
			case L'\r':
				textBuff.push_back(L'\n');
				break;
			case L'\0':
				//��?!
				i=fileBuff.size();
				break;
			default:
				textBuff.push_back(wch);
				break;
			}
		}
	}
	else{
		//�ƴϸ� ������ UTF-8, Shift-JIS�� CP949 �׷��� ������
		int nStart=0;
		if(memcmp(fileBuff.data(),"\xEF\xBB\xBF",3)==0){
			nStart+=3;
		}
		int nSize = MyMultiByteToWideChar(CP_UTF8,0,(LPCSTR)fileBuff.data()+nStart,-1,NULL,0);

		textBuff.resize(nSize);
		MyMultiByteToWideChar(CP_UTF8,0,(LPCSTR)fileBuff.data()+nStart,-1,textBuff.data(),nSize);

		replace(textBuff.begin(),textBuff.end(),L'\r',L'\n');
	}

	fileBuff.clear();

	int nKeyCP = fInfo.keyCP;
	int nValCP = fInfo.valCP;

	//�� ���� �Ľ�
	vector<wordInfo> wordList;
	size_t nParseIdx=0;
	wstring lineData;
	lineData.reserve(200);
	while(nParseIdx<textBuff.size()){
		lineData.resize(0);
		wchar_t wch;
		while(nParseIdx<textBuff.size()){
			wch = textBuff[nParseIdx];

			if(wch==L'\n'){
				break;
			}

			lineData.push_back(wch);
			nParseIdx++;
		}
		if(wch==L'\n')nParseIdx++;

		//�ּ�����
		auto nPos = lineData.find(L"//");
		if(nPos!=lineData.npos){
			lineData.resize(nPos);
		}

		if(lineData.length()<3)continue;//�ּ� �ѱ���\t�ѱ��� �� �Ǿ���Ѵ�
		
		wordInfo newWord;
		if(!ParseLine(lineData,newWord.key,newWord.value,newWord.aux,nKeyCP,nValCP)){
			continue;
		}

		wordList.push_back(newWord);
	}

	textBuff.clear();

	sort(wordList.begin(),wordList.end(),[] (const wordInfo& lhs,const wordInfo &rhs){
		return lhs.aux<rhs.aux;
	});

	if(fInfo.split){
		_fullDic dicPre;
		_fullDic dicPost;

		auto preIter = dicPre.end();
		auto postIter = dicPost.end();

		int nPrevRank=0x80000000-1;//-1 �� ���� int�� �ִ�ġ
		//���� �� ���� ���� �𸣴µ� ��� �ܾ �� ���ڷ� ��� �ܾ� ������ ���Ľ�Ű�� �ʾ����� ���ڴ� -_-;

		for(auto iter=wordList.begin();iter!=wordList.end();++iter){
			int nNowRank = iter->aux;//aux ��� ����

			iter->aux = GetSeqN();

			if(nPrevRank!=nNowRank){
				nPrevRank = nNowRank;

				dicPre.push_back(nullRankDic);
				dicPost.push_back(nullRankDic);

				preIter = dicPre.end();
				--preIter;
				postIter = dicPost.end();
				--postIter;
			}

			wordInfo wordPre = *iter;
			wordInfo wordPost = *iter;

			wordPre.value.clear();
			wordPost.key.clear();

			int nLen = wordPre.key.length();
			if(nLen>=MAXSTRLEN)nLen=MAXSTRLEN-1;
			BYTE chFirst = wordPre.key[0];
			UINT uHash = MakeStringHash(wordPre.key.c_str());

			(*preIter)[nLen][chFirst][uHash].push_back(wordPre);

			nLen = 0;
			chFirst = wordPost.aux%256;
			uHash = wordPost.aux/256;

			(*postIter)[nLen][chFirst][uHash].push_back(wordPost);
		}

		m_pDicBase->ChangeDic(fInfo.name,dicPre,dicPost);

	}
	else{
		_fullDic dicList;
		auto dicIter = dicList.end();

		int nPrevRank=0x80000000;

		for(auto iter=wordList.begin();iter!=wordList.end();++iter){
			int nNowRank = iter->aux;//aux ��� ����

			iter->aux = GetSeqN();

			if(nPrevRank!=nNowRank){
				nPrevRank = nNowRank;

				dicList.push_back(_rankDic());
				
				dicIter = dicList.end();
				--dicIter;
			}

			int nLen = iter->key.length();
			if(nLen>MAXSTRLEN)nLen=MAXSTRLEN;
			BYTE chFirst = iter->key[0];
			UINT uHash = MakeStringHash(iter->key.c_str());

			(*dicIter)[nLen][chFirst][uHash].push_back(*iter);
		}

		m_pDicBase->ChangeDic(fInfo.name,dicList);
	}
}

bool AutoLoadDic::ParseLine(wstring& wstrLine,string& strKey,string& strVal,int& nRank,int KeyCP, int ValCP){
	bool bExtMode=(wstrLine[0]==L'\t');

	int nKeyStart = wstrLine.find_first_not_of(L'\t');
	int nKeyEnd   = wstrLine.find_first_of(L'\t',nKeyStart);

	int nValStart = wstrLine.find_first_not_of(L'\t',nKeyEnd);
	int nValEnd   = wstrLine.find_first_of(L'\t',nValStart);

	int nRankStart= wstrLine.find_first_not_of(L'\t',nValEnd);
	int nRankEnd  = wstrLine.find_first_of(L'\t',nRankStart);

	bool bKeyExist;
	bool bValExist;
	bool bRankExist;

	if(nKeyStart==-1){
		//OMG
		bKeyExist=false;
		bValExist=false;
		bRankExist=false;
	}
	else if(nKeyEnd==-1||nValStart==-1){
		bKeyExist=true;
		bValExist=false;
		bRankExist=false;
	}
	else if(nValEnd==-1||nRankStart==-1){
		bKeyExist=true;
		bValExist=true;
		bRankExist=false;
	}
	else{
		bKeyExist=true;
		bValExist=true;
		bRankExist=true;
	}

	if(!bKeyExist)return false;

	wstring wstrTmpKey;
	wstring wstrTmpVal;
	wstring wstrTmpRank;
	int nTmpRank;

	if(bKeyExist)wstrTmpKey = wstrLine.substr(nKeyStart,nKeyEnd-nKeyStart);
	if(bValExist)wstrTmpVal = wstrLine.substr(nValStart,nValEnd-nValStart);
	if(bRankExist)wstrTmpRank = wstrLine.substr(nRankStart,nRankEnd-nRankStart);

	//Rankó��
	if(bRankExist){
		size_t nPos=0;
		int nFlag=1;
		int nVal=0;

		bool bUseFlag=false;

		if(wstrTmpRank[0]==L'-'){
			nFlag=-1;
			nPos=1;
			bUseFlag=true;
		}
		else if(wstrTmpRank[0]==L'+'){
			nFlag=1;
			nPos=1;
			bUseFlag=true;
		}

		for(;nPos<wstrTmpRank.size();nPos++){
			wchar_t wch = wstrTmpRank[nPos];
			if(wch<L'0'||wch>L'9')break;

			nVal*=10;
			nVal+=(wch-L'0');
		}

		nVal*=nFlag;

		if(nPos==0||(nPos==1&&bUseFlag)){
			bRankExist=false;
		}
		else{
			nTmpRank=nVal;
		}
	}

	

	//Rank�� �⺻���� 10
	if(!bRankExist){
		nTmpRank=10;
	}

	string strTmpKey;
	string strTmpVal;
	vector<char> szBuff;

	int nTmpLen;

	nTmpLen=MyWideCharToMultiByte(KeyCP,0,wstrTmpKey.c_str(),-1,NULL,NULL,NULL,NULL);
	szBuff.resize(nTmpLen);
	MyWideCharToMultiByte(KeyCP,0,wstrTmpKey.c_str(),-1,szBuff.data(),nTmpLen,NULL,NULL);
	strTmpKey = szBuff.data();


	nTmpLen=MyWideCharToMultiByte(ValCP,0,wstrTmpVal.c_str(),-1,NULL,NULL,NULL,NULL);
	szBuff.resize(nTmpLen);
	MyWideCharToMultiByte(ValCP,0,wstrTmpVal.c_str(),-1,szBuff.data(),nTmpLen,NULL,NULL);
	strTmpVal = szBuff.data();

	//����, ��� -> Raw ����
	if(bKeyExist&&!bValExist){
		//���� ��ȯ ���
		strTmpVal = strTmpKey;
	}
	if(bExtMode){
		//Ȯ�� ���
		strTmpKey = parseText(strTmpKey);
		strTmpVal = parseText(strTmpVal);
	}

	strKey = strTmpKey;
	strVal = strTmpVal;
	nRank = nTmpRank;

	return true;
}

void AutoLoadDic::CallBackProc(){

	vector<fileInfo> dicList;
	dicList.reserve(m_arFileList.size());

	m_csLock.Lock();
	for(auto iter=m_arFileList.begin();iter!=m_arFileList.end();++iter){
		HANDLE hFile = CreateFile(iter->first.name.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			
		if(hFile==INVALID_HANDLE_VALUE){
			continue;
		}

		FILETIME nowTime;

		if(GetFileTime(hFile,NULL,NULL,&nowTime)==0){
			CloseHandle(hFile);
			continue;
		}

		if(memcmp(&iter->second,&nowTime,sizeof(FILETIME))!=0){
			iter->second = nowTime;
			dicList.push_back(iter->first);
		}

		CloseHandle(hFile);
	}

	m_csLock.Unlock();

	if(dicList.size()!=0){
		for(size_t i=0;i<dicList.size();i++){
			ReloadDic(dicList[i]);
		}
	}
}

//�ƶ�Ʈ���� 0.2���� �ʱ�ȭ�� Init����� �����尡 �ʱ�ȭ ���� �ʴ°��� ȸ��
VOID CALLBACK TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime){
	ASSERT(hwnd==(HWND)g_pHwnd);
	KillTimer(hwnd,idEvent);
	AutoLoadDic * pThis = (AutoLoadDic*)idEvent;// ����
	pThis->__StartAutoLoad();
}

UINT AutoLoader(LPVOID target){
	AutoLoadDic* p_cAutoLoadDic = reinterpret_cast<AutoLoadDic*>(target);
	int nInverval = p_cAutoLoadDic->GetInterval();
	int nNowTime=nInverval; //�ܺο��� ��� �� ����ǹǷ� �ٷ� üũ�ؾ��� �ʿ䰡 �ִ�.
	while(g_Run&&nInverval>0){
		if(nNowTime>=nInverval){
			nNowTime=0;
			p_cAutoLoadDic->CallBackProc();
		}
		nNowTime+=20;
		Sleep(20);
	}
	return 0L;
}

int AutoLoadDic::GetSeqN(){
	ASSERT(0<=m_nSeqN);

	return m_nSeqN++;
}

int AutoLoadDic::GetInterval(){
	return m_nInterval;
}

void AutoLoadDic::SetInterval(int nInterval){
	if(nInterval<100){
		nInterval*=1000;
	}

	m_nInterval=nInterval;
}