//
// Util - �÷����� ���۽� ������ ��ƿ��Ƽ �Լ� ���
//

#ifndef __UTIL_H__
#define __UTIL_H__

#include "tstring.h"
#include <vector>

using namespace std;

// �ɼ� ����ü
typedef struct _tagATOPTION_PAIR {
	tstring strKey;	// �ɼ� Ű
	tstring strValue;	// �ɼ� ��
} ATOPTION_PAIR, *PATOPTION_PAIR;

// �ɼ� ����
typedef vector<ATOPTION_PAIR> ATOPTION_ARRAY;	

// MyCreateDirectory - ������ �����θ� ������������ ������ش�.
BOOL MyCreateDirectory(LPCTSTR lpDirectory);

// GetATDirectory - �ƶ�Ʈ���� ��ġ ���丮�� �˾ƿ´�.
const TCHAR *GetATDirectory();

// GetGameDirectory - ���� ���丮�� �˾ƿ´�.
const TCHAR *GetGameDirectory();

// GetATOptionsFromOptionString - �ɼ� ���ڿ��� �Ľ��ؼ� ATOPTION_ARRAY �� �����Ѵ�.
void GetATOptionsFromOptionString(const char *pszOption, ATOPTION_ARRAY &aOptions);

// GetOptionStringFromATOptions - ATOPTION_ARRAY �� ������ �ɼ� ���ڿ��� �ٲ��ش�.
void GetOptionStringFromATOptions(const ATOPTION_ARRAY &aOptions, char *pszOption, int nMaxOptionLength);

#endif //__UTIL_H__