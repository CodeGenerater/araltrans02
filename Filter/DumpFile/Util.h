//
// Util - �÷����� ���۽� ������ ��ƿ��Ƽ �Լ� ���
//

#ifndef __UTIL_H__
#define __UTIL_H__

#include "tstring.h"

// MyCreateDirectory - ������ �����θ� ������������ ������ش�.
BOOL MyCreateDirectory(LPCTSTR lpDirectory);

// GetATDirectory - �ƶ�Ʈ���� ��ġ ���丮�� �˾ƿ´�.
const TCHAR *GetATDirectory();

// GetGameDirectory - ���� ���丮�� �˾ƿ´�.
const TCHAR *GetGameDirectory();

#endif //__UTIL_H__