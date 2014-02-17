#pragma once

struct fileInfo{
	bool split;
	int keyCP;
	int valCP;
	wstring name;
};

struct wordInfoW{
	wstring key;
	wstring value;
	int aux;
};

struct wordInfo{
	string key;
	string value;
	int aux;
	//key -> aux �Ǵ� aux -> value ����
};

struct TokenStruct{
	int aux;//<0�̸� �Ϲ� �ؽ�Ʈ, 0>=�̸� ��ȯ ��ȣ
	vector<bool> dbcs;
	string token;
};

#define MAXSTRLEN 150
typedef array<array<map<UINT,list<wordInfo>>,256>,MAXSTRLEN+1> _rankDic; //_rankDic�� ���ÿ� ���� ���� ����! ũ�Ⱑ ���� ŭ!
typedef list<_rankDic> _fullDic;
//�켱����(��ũ�帮��Ʈ) -> �ܾ����(�迭) -> ù��° ����(�迭) -> Hash���� ������� �ϴ� BinaryTree -> ����:������ 1��1 ����
//array���� ���� 0���� aux �����ͷ� �Ҵ�Ǹ�,
//�̶� ���� ù��° ���ڷ� ���̴� 256 �������� aux%256�� ���� ���� UINT�� aux/256�� ��

struct DicStruct{
	std::wstring filename;

	_fullDic dic;
};