#pragma once

/* ���� â�� */
struct OpData{
	DWORD type;
	/* Type �����ʹ� 16������! ��������!
	 *
	 * 0x01xx : �ý��� �ñ׳�
	 *     00 : �ʱ�ȭ(��� ����)
	 *     01 : ���� ����
	 *	   02 : ���� �Ϸ�(!!!)
	 *     03 : ����(��� ����)
	 *
	 * 0x02xx : �ؽ�Ʈ �ñ׳�
	 *     00 : ����(932)
	 *     0x : ����(Reserved)
	 *     10 : ������(949)
	 *     1x : ������(Reserved)
	 *
	 * 0xFFxx : ���� �߻�
	 *     00 : Critical Section Lock ����
	 *     01 : Queue�� �������
	 */
	std::wstring data; //�ּ� 32Byte
};

bool pushOpQueue(OpData& sOP,bool isBack=true);
bool pushOpQueue(DWORD type,std::wstring data,bool isBack=true);

OpData popOpQueue();

bool isOpQueueEmpty();