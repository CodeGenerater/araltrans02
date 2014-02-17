#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

class CControlChar
{
public:
	CControlChar(void);
public:
	~CControlChar(void);
public:
	// �ʿ� ��Ʈ�� ���ڿ� �Է�
	bool AddCtrStr(LPCSTR ControlHex,bool isViewed);

	multimap <char,string> ConrolCharMap; 
public:
	bool PreControlChar(LPSTR Dest, LPCSTR Source);
public:
	bool PostControlChar(LPSTR Dest, LPCSTR Source);
};
