/////////////////////////////////////////////////
// MFC + v8 �� �ܼ� �ʱ�ȭ ����
/////////////////////////////////////////////////
//
// �ܼ��ϰ� ���� ������ ������
// C++�� �Լ��� JavaScript ���� �Լ��� �����ϰ�
// �ܺ� �Լ��� ���� ���η� �̾��ִ� ��ɸ� �ϴ�
// ����Դϴ�.
//
// 2010-07-26. Hide_D
/////////////////////////////////////////////////
#include "DefATContainer.h"

#pragma once

// TODO : �����ϰ��� �ϴ� �޽����� �����մϴ�.
enum MessageType {EXIT=-1,NOP=0,CALL=1};

// TODO : ������ �޽��� Ÿ���� �����մϴ�.
typedef uint32_t v8key_t;
typedef PREGISTER_ENTRY v8data_t;

struct v8message{
	MessageType type;
	v8key_t key;
	v8data_t value;
	bool async;
};

void v8RunScript(const wchar_t *);
void v8RunScript(const wchar_t ** filelist,int fileN);

bool v8SendMessage(v8message& message);

void v8Reset();