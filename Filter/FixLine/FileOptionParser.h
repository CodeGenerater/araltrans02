
/* 
 * ���� �ɼ� �ļ� - Hide_D��
 *
 * �ɼ��� ���Ϸ� �����ϴ� ��� ���
 *
 * �ɼ���
 *
 * �ɼǸ�:�ɼǰ�
 *
 * �� �⺻�̸�, ���갪����
 *
 * �ɼǸ�_n:�ɼǰ�
 * 
 * �ɼǸ�_n_n:�ɼǰ�
 * 
 * ������ ����� �� ����
 */

#pragma once

#include <map>
#include <vector>
#include "tstring.h"

using namespace std;

class CFileOptionParser
{
public:
	CFileOptionParser(tstring strFileName, bool AutoRead);
	CFileOptionParser(tstring strFileName);
	CFileOptionParser();
public:
	~CFileOptionParser(void);

public:
	void SetOptionFile(tstring strFileName);

	void OpenOptionFile();
	void SaveOptionFile();

	void Clear();

	int GetOptionInt(tstring strOptionName, int nTokCount=0, ...);
	int GetOptionInt(tstring strOptionName, vector<int>& subArgs);

	bool GetOptionBool(tstring strOptionName, int nTokCount=0, ...);

	tstring GetOptionString(tstring strOptionName, int nTokCount=0, ...);
	tstring GetOptionString(tstring strOptionName, vector<int>& subArgs);


	void SetOptionInt(int nOptionData, tstring strOptionName, int nTokCount=0, ...);
	void SetOptionInt(int nOptionData, tstring strOptionName, vector<int>& subArgs);

	void SetOptionString(tstring strOptionData, tstring strOptionName, int nTokCount=0, ...);
	void SetOptionString(tstring strOptionData, tstring strOptionName, vector<int>& subArgs);	

	bool IsInData(tstring strOptionName, int nTokCount=0, ...);
	bool IsInData(tstring strOptionName, vector<int>& subArgs);

	bool removeData(tstring strOptionName, int nTokCount=0, ...);
	bool removeData(tstring strOptionName, vector<int>& subArgs);

private:
	tstring RemoveSpace(tstring strData);

	map <tstring,tstring> m_Options;	//�ɼ� ����� ��
	tstring m_strFileName;				//�ɼ� ���ϸ�

};
