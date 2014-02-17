#pragma once
#include <vector>
#include <queue>
#include "tstring.h"
#include "util.h"

using namespace std;

class FLFunctionString
{
public:
	FLFunctionString(void);
public:
	~FLFunctionString(void);

	void InitFunc(int Len,int TextOnly,wstring FuncString);
	bool AddText(int Address, string PushText);

	string GetSubText(queue<int>* AddressList);

	string GetFullText();
	string GetOnlyText();

	int Length();
	bool TextOnly();

	void InitText(string FuncSource);

	int FindFunc(string Source,int Start);

private:
	int OptLen;//�ɼǽ� ���� ����
	int Len;//�ؽ�Ʈ ���� ��������
	int AddressLen;
	vector <int> Type;
	vector <string> Text;
	//Type ����
	//0 : �Լ��κ�(Hex)
	//1 : �Լ��κ�(���ڿ�)
	//2 : �ؽ�Ʈ, ����(^)
	//3 : �ؽ�Ʈ, ��(*)
	//4 : �ؽ�Ʈ, ����, �̹���(!)

	bool TOnly;

	string::size_type Find(string Source,string Key,string::size_type Start,int CodePage);
};

struct FLOption{
	int ReturnLen;
	int MaxChar;
	int MaxLine;
	int IgnoreLine;
	bool LimitLine;
	int ReturnCharLen;
	string ReturnChar;

	vector <FLFunctionString> OptionFuncStrings;
	//�ɼǿ� ���⿣ Len�� Type 2,3,4�� Text�� �̿ϼ��� ����,
	//�̰� �۰��� �ϼ��� �ϸ� �ȴٴ� ��������[...]
	int OptFuncStringN;

	bool TwoByte;
	bool RemoveSpace;
};
