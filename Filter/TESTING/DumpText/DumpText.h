// DumpText.h : DumpText DLL�� �⺻ ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CDumpTextApp
// �� Ŭ������ ������ ������ DumpText.cpp�� �����Ͻʽÿ�.
//

class CDumpTextApp : public CWinApp
{
public:
	CDumpTextApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
