�ƶ� Ʈ���� �÷����� ���� �����ӿ�ũ 20080810 for AralTrans 0.2.20080704

* ���� ����
	ATPlugin.h - �ƶ�Ʈ���� �÷����� �԰ݿ� �´� �ͽ���Ʈ ����.
	ATPlugin.cpp - �ƶ�Ʈ���� �÷������� �����ؾ� �ϴ� ���� �Լ� ����
	DefATContainer.h - �ƶ�Ʈ���� �����̳ʰ� �����ϴ� �Լ��� ���� (ATCode �÷����ο��� ��½ ^^)
	ATCApi.h / ATCApi.cpp - �ƶ�Ʈ���� �����̳� ���� �Լ��� ����ϱ� ���� Wrapping�� Ŭ����
	tstring.h / tstring.cpp - �����ڵ�� ��ȯ �Լ� �� string ���� ���
	Util.h / Util.cpp - �÷����� ���۽� ������ ��ƿ��Ƽ �Լ� ����
	resource.h / ATPlugin.rc - �÷����� ���� ǥ�� ���ҽ�
	ATPlugin.dsw / ATPlugin.dsp - Visual C++ 6.0 �� ������Ʈ ����

* ��ŷ �÷������� ���� �� 
	ATPlugin.h �� #define ATPLUGIN_HOOKER �� �ּ� �����Ͻð� �����ϼ���.

* ���� �÷������� ���� ��
	ATPlugin.h �� #define ATPLUGIN_TRANSLATOR �� �ּ� �����Ͻð� �����ϼ���.

* ���� �÷������� ���� ��
	ATPlugin.h �� #define ATPLUGIN_FILTER �� �ּ� �����Ͻð� �����ϼ���.

* ATPLUGIN_VERSION �� OnPluginVersion()
	0.2.20080518 ���� �÷����� �԰��� �ƴ����� ���� �÷����� ����� ������ �� �ƶ�Ʈ����
	�����ڲ��� ���� �����ϵ��� �߰��߽��ϴ�;

	(20080726 �߰� - AralUpdater �� �÷������� ���� ���� ���ҽ��� �⺻���� ������Ʈ�Ѵٴ� ����
	 �˾Ƴ����Ƿ� OnPluginVersion()�� �ʿ���� �� �����ϴٸ�.. �ϴ� ���ܵӴϴ�.
	 �߰���.. �⺻������ �÷������� ���� ���� ���ҽ��� �߰��ϵ��� ATPlugin.rc �� �����Ͽ�����
	 �� ������ ���� ���ҽ��� ���� ����ϵ��� �Ͻñ� �ٶ��ϴ�)

* ATCApi ����ϱ� ���� �غ�
	1. ATCApi.h �� include �ϸ� ���� ��ü�� g_cATCApi�� �����ϴ�.
	2. g_cATCApi�� IsValid() �� TRUE �̸� ��� ����.

* �����ڵ� ��� ������Ʈ�� ���� ��
	Visual C++ 6.0 ������ �����Ϸ� �ɼ� define �� _MBCS �� ����� UNICODE, _UNICODE �� ��� ������ �˴ϴ�.
	������ ������Ʈ ���Ͽ��� �����ڵ��/MBCS���� ������ �� �� �ֽ��ϴ�.
	�ٸ� ������ �����Ϸ� ���ÿ��� �� ������ �����ڵ� ���� ����� �����ϼ���.

* tstring ����� �Լ� ���� ����
	tstring - �����ڵ� ������ ���� string Ȥ�� wstring ���� �����մϴ�.
	tstring MakeTString(const char *pszString) - char ������ ���ڿ��� tstring ���� �ٲ��ݴϴ�.
	TCHAR MakeTChar(const char ch) - char ������ ���ڸ� TCHAR �� �ٲ��ݴϴ�.. �����δ� �ʿ��������;
	MyWideCharToMultiByte() - WideCharToMultiByte ��� �Լ��Դϴ�. ���÷����϶����� �̷��� ����ؾ� �Ѵٰ�..
	MyMultiByteToWideChar() - MyMultiByteToWideChar ��� �Լ��Դϴ�. ���÷����϶����� �̷��� ����ؾ� �Ѵٰ�..
	Kor2Wide(LPCSTR lpKor, LPWSTR lpWide, int nWideSize) - �ѱ��� ��� MyMultiByteToWideChar�� ���� ���� ���� �����Լ��Դϴ�.
	Wide2Kor(LPCWSTR lpWide, LPSTR lpKor, int nKorSize) - �ѱ��� ��� MyWideCharToMultiByte�� ���� ���� ���� �����Լ��Դϴ�.


* Util �Լ� ���� ����
	const char *GetATDirectory()
	- �ƶ�Ʈ���� ��ġ ���丮�� �˾ƿ´�.
	void GetATOptionsFromOptionString(const char *pszOption, ATOPTION_ARRAY &aOptions)
	- ������ �ɼ� �ļ�. �ɼ��� key1(value1),key2(value2) ���� Ȥ�� key1,key2 �� ����.
	void GetOptionStringFromATOptions(const ATOPTION_ARRAY &aOptions, char *pszOption, int nMaxOptionLength)
	- ���� �ɼ� �ļ��� �ݴ�� �ɼ� ������ �ɼ� ���ڿ��� �ٲ��ִ� �Լ�.