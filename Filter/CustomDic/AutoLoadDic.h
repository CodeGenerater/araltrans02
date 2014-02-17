#pragma once

#include "CDic_Proxy.h"

class AutoLoadDic
{
private:
	AutoLoadDic(void);
public:
	AutoLoadDic(CustomDic* dicBase);
	~AutoLoadDic(void);

	void StopAutoLoad();
	void StartAutoLoad();
	void __StartAutoLoad();
	void SetDic(vector<fileInfo>& files);

	void FirstLoad();

	void CallBackProc();
	void SetInterval(int nInterval);
	int GetInterval();


protected:
	int GetSeqN();
	void ReloadDic(fileInfo& fInfo);
	bool ParseLine(wstring& wstrLine,string& strKey,string& strVal,int& nRank,int KeyCP, int ValCP);

	int m_nSeqN;

	int m_nInterval;

	CustomDic* m_pDicBase;

	CWinThread* m_pThread;
	CCriticalSection m_csLock;

	vector<pair<fileInfo,FILETIME>> m_arFileList;//������ ����ȭ ���, SetDic, FirstLoad, ReloadDic, WorkerThread���� ����

	bool m_bFirstLoad;

};

