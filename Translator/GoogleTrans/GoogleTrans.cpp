// GoogleTrans.cpp : DLL ���� ���α׷��� ���� ������ �Լ��� �����մϴ�.
//

#include "stdafx.h"
#include "GoogleTrans.h"
#include "resource.h"
#include "StatusDlg.h"
#include "proxy.h"

using namespace std;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hThisModule = (HINSTANCE) hModule;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


/*
 * ���� �ʱ�ȭ
 */
BOOL __stdcall OnPluginInit(HWND hSettingWnd, LPSTR szOptionStringBuffer){
	g_hSettingWnd = hSettingWnd;
	g_szOptionStringBuffer = szOptionStringBuffer;

	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_LINK_CLASS|ICC_WIN95_CLASSES;
	BOOL tr;
	tr = InitCommonControlsEx(&iccex);

	//���� ����
	if(g_hSession==NULL){
		array<wchar_t,500> szUserAgent;
		DWORD dSize=500;
		{
			array<char,500> tmpAgent;
			ObtainUserAgentString(0,tmpAgent.data(),&dSize);
			tmpAgent[dSize]='\0'; //������ ����
			MyMultiByteToWideChar(0,0,tmpAgent.data(),-1,szUserAgent.data(),500);
		}

		g_hSession = InternetOpen(szUserAgent.data(),INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);

		g_hStatusDlg = CreateDialog(g_hThisModule,MAKEINTRESOURCE(IDD_DIALOG1),NULL,StatusProc);
		ShowWindow(g_hStatusDlg,SW_SHOW);

	}
	else{
		if(::IsWindow(g_hSettingWnd)){
			::MessageBox(g_hSettingWnd,L"�÷������� ������ �ε�Ǿ����ϴ�!\r\n����ġ ���� ������ �߻��� �� �ֽ��ϴ�!",L"Google Translation Plugin",MB_ICONWARNING);
		}
	}

	return TRUE;
}

/*
 * �ɼ� ó��, ���������� �뵵�� ����.
 */
BOOL __stdcall OnPluginOption(){

	if(::IsWindow(g_hSettingWnd)){
		::MessageBox(g_hSettingWnd,L"���ٸ� �ɼ��� �������� �ʽ��ϴ�",L"Google Translation Plugin",MB_ICONINFORMATION);
	}

	return TRUE;
}

/*
 * Session�� �ݰ� Ȥ�� �����ִ� Request �ڵ��� ��� �ݴ´�.
 */
BOOL __stdcall OnPluginClose(){

	if(g_hStatusDlg!=NULL){
		DestroyWindow(g_hStatusDlg);
		g_hStatusDlg=NULL;
	}

	//Session �ݱ�
	if(g_hSession!=NULL){
		InternetCloseHandle(g_hSession);
		g_hSession=NULL;
	}

	//���� �۾��� ��� ����ɶ����� �ִ� 10�ʰ� ���
	for(int i=0;i<100;i++){
		if(g_nTransCount<=0){
			g_nTransCount=0;
			break;
		}
		Sleep(100);
	}

	//Request �ڵ��� ��� �ݴ´�. �Ϲ����� ��Ȳ���� �ڵ��� ���Ƽ� �ȵȴ�.
	auto iter = g_hRequests.begin();
	while(iter!=g_hRequests.end()){
		if(*iter != NULL){
			InternetCloseHandle(*iter);
			g_hRequests.erase(iter++); //iterator�� ++��ȣ�� ����
		}
	}

	return TRUE;
}

/*
 * ���� ���� 
 * http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&q=����(UTF-8)&langpair=ja%7Cko �� ������� GET�� �̿��� ������.
 * 
 */
BOOL __stdcall Translate(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize){

	int nSrcLen=strlen(cszJapanese);

	//���� �� �ʿ䰡 �ִ����� Ȯ���Ѵ�.

	//1����Ʈ�� �����Ǿ��ִ��� Ȯ���Ѵ�.
	{
		bool bIs1Byte=true;

		//MBCS���� �� ���� �߿� 0x80 �̻��� ���� �ִ��� Ȯ���Ѵ�.
		for(int i=0;i<nSrcLen;i++){
			if((BYTE)cszJapanese[i]>=0x80){
				bIs1Byte=false;
				break;
			}
		}

		if(bIs1Byte){
			strcpy_s(szKorean,nBufSize,cszJapanese);
			return TRUE;
		}
	}

	//�̹� ó���Ǿ� �ִ��� Ȯ���Ѵ�.
	UINT uHash = MakeStringHash(cszJapanese);
	{
		auto iter = g_Cache.find(uHash);
		if(iter!=g_Cache.end()){
			strcpy_s(szKorean,nBufSize,iter->second.c_str());
			return TRUE;
		}
	}

	//Session�� ���� �ִ��� Ȯ���Ѵ�.
	if(g_hSession==NULL){
		strcpy_s(szKorean,nBufSize,cszJapanese);
		return TRUE;
	}

	g_nTransCount++;

	wstring strJapanese;
	{
		vector<wchar_t> strJpnBuff;
		strJpnBuff.resize(nSrcLen);
		int nLen=MyMultiByteToWideChar(932,0,cszJapanese,-1,strJpnBuff.data(),strJpnBuff.size());
		//������ nLen�� ���� �迭�� ũ���̹Ƿ� -1�� �ؾ��Ѵ�.
		nLen--;
		strJapanese.insert(strJapanese.end(),strJpnBuff.begin(),strJpnBuff.begin()+nLen);
	}

	//Status â���� ���� ����
	SetJString(strJapanese);

	//�������� ���� ���ڸ� �̸� ��ȯ
	strJapanese = EncodeUnsafeCharactor(strJapanese);

	wstring strUrl;//Http ����

	{
		strUrl=L"http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&q=";
		strUrl+=GetEscapeJSString(strJapanese);
		strUrl+=L"&langpair=ja%7Cko";
	}

	wstring strHeader=L""; //HTTP ���, �߰��� ������ ������ ����.

	//Request
	HINTERNET hRequest = InternetOpenUrl(g_hSession,strUrl.c_str(),strHeader.c_str(),strHeader.length(),INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,0);


	if(hRequest==NULL){
		strcpy_s(szKorean,nBufSize,cszJapanese);
	}
	else{
		g_hRequests.insert(hRequest);

		const int cnBuffSize = 8192;

		vector<BYTE> dataBuff;
		{
			array<BYTE,cnBuffSize> tmpBuff;
			BOOL bResult;
			while(true){
				DWORD dw=0;
				bResult = InternetReadFile(hRequest,tmpBuff.data(),cnBuffSize,&dw);

				if(bResult&&dw){
					dataBuff.insert(dataBuff.end(),tmpBuff.begin(),tmpBuff.begin()+dw);
				}
				else{
					break;
				}
			}
		}

		dataBuff.push_back('\0');

		InternetCloseHandle(hRequest);
		g_hRequests.erase(hRequest);
		//���� ����

		wstring strResponseData;

		//UTF-8 -> UTF-16LE
		{
			vector<wchar_t> transBuff;
			transBuff.resize(dataBuff.size(),L'\0');
			int nLen=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)dataBuff.data(),-1,transBuff.data(),transBuff.size());
			strResponseData.insert(strResponseData.end(),transBuff.begin(),transBuff.begin()+nLen-1);
		}

		//���� ����
		strResponseData = getTranslatedText(strResponseData);
		strResponseData = DecodeUnsafeCharactor(strResponseData);

		//������ ���
		SetKString(strResponseData);

		if(strResponseData.length()==0){
			strcpy_s(szKorean,nBufSize,cszJapanese);
		}
		else{
			MyWideCharToMultiByte(949,0,strResponseData.c_str(),-1,szKorean,nBufSize,NULL,NULL);
		}

		//ĳ�� ���
		g_Cache.insert(make_pair(uHash,string(szKorean)));

	}

	g_nTransCount--;
	return TRUE;
}


/*
 * FireFox�� urlCharType
 * ECMA 262�� ������ �� �迭���� &1 (AND 1)�� ���� �� true�̸� ��ȯ �۾��� �ʿ� ����.
 */
static const BYTE urlCharType[256] =
/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
{    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* 0x */
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* 1x */
     0,0,0,0,0,0,0,0,0,0,7,4,0,7,7,4,       /* 2x   !"#$%&'()*+,-./  */
     7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,       /* 3x  0123456789:;<=>?  */
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       /* 4x  @ABCDEFGHIJKLMNO  */
     7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,       /* 5X  PQRSTUVWXYZ[\]^_  */
     0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       /* 6x  `abcdefghijklmno  */
     7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,       /* 7X  pqrstuvwxyz{\}~  DEL */
     0, };

/*
 * �ش� ���ڿ��� JavaScript�� Escape��ɰ� �����ϰ� �����Ѵ�
 * ECMA 262�� ������, urlCharType�� �ش��ϴ� ����, %uFFFF, %FF�� �������� �̷������.
 */
wstring GetEscapeJSString(wstring strSource){
	vector<wchar_t> buff;
	buff.reserve(strSource.length()*6+10);

	char buff3[5];
	for(UINT i=0;i<strSource.length();i++){
		unsigned short ch=strSource[i];
		if(ch==L'\0'){
			break;
		}
		else if(ch>=0x256){
			sprintf_s(buff3,5,"%04X",ch);
			buff.push_back(L'%');
			buff.push_back(L'u');
			buff.push_back(buff3[0]);
			buff.push_back(buff3[1]);
			buff.push_back(buff3[2]);
			buff.push_back(buff3[3]);
		}
		else if(urlCharType[ch]&1){
			buff.push_back(ch);
		}
		else{
			sprintf_s(buff3,5,"%02X",ch);
			buff.push_back(L'%');
			buff.push_back(buff3[0]);
			buff.push_back(buff3[1]);
		}
	}

	if(buff.size()==0){
		return wstring();
	}
	else{
		return wstring(buff.begin(),buff.end());
	}
}

/*
 * � ���ڰ� HEX�� ���̴� �������� �Ǵ��Ѵ�.
 */
bool isHex(wchar_t ch){
	return (('0'<=ch&&ch<='9')||('a'<=ch&&ch<='f')||('A'<=ch&&ch<='F'));
}

/*
 * 00A0, FF�� ���� Hex�� �̷���� ���ڿ��� �ϳ��� ���ڷ� ��ȯ�Ѵ�.
 * �ش��ϴ� ������ ������ Hex ���ڷ� �̷���� �ִٰ� �����Ѵ�.
 */
wchar_t convertHexToWchar(wchar_t* psz,int nCount){ //�ѱ��ڸ�!
	if(nCount>4)nCount=4;
	wchar_t cRet=0;
	for(int i=0;i<nCount;i++){
		wchar_t ch=psz[i];
		cRet<<=4;
		if(L'0'<=ch&&ch<=L'9'){
			cRet+=ch-'0';
		}
		else if(L'a'<=ch&&ch<=L'f'){
			cRet+=ch-'a'+0xa;
		}
		else if(L'A'<=ch&&ch<=L'F'){
			cRet+=ch-'A'+0xa;
		}
		else{
			cRet>>=8;
			break;
		}

	}
	return cRet;
}

/*
 * �ش� ���ڿ��� JavaScript�� Unescape��ɰ� �����ϰ� �����Ѵ�
 * \FF \uFFFF �Ϲݹ��� �� �������� �̷�����ִ�.
 */
wstring GetUnescapeJSString(wstring strSource){
	vector<wchar_t> src;
	src.insert(src.end(),strSource.begin(),strSource.end());

	vector<wchar_t> buff;
	buff.reserve(src.size());

	for(UINT i=0;i<src.size();i++){
		if(strSource[i]==L'\0'){
			break;
		}
		else if(i<=src.size()-6&&
			src[i]=='\\'&&
			src[i+1]=='u'&&
			isHex(src[i+2])&&
			isHex(src[i+3])&&
			isHex(src[i+4])&&
			isHex(src[i+5]))
		{
			buff.push_back(convertHexToWchar(&src[i+2],4));
			i+=6-1;
		}
		else if(i<=src.size()-3&&
			src[i]=='\\'&&
			isHex(src[i+1])&&
			isHex(src[i+2]))
		{
			buff.push_back(convertHexToWchar(&src[i+1],2));
			i+=3-1;
		}
		else{
			buff.push_back(src[i]);
		}
	}

	if(buff.size()==0){
		return wstring();
	}
	else{
		return wstring(buff.begin(),buff.end());
	}
}

/*
 * Google Translation API�� ������� �Ľ��ϴ� �Լ�
 * JSON���� �� �����( {"responseData": {"translatedText":"�����"}, "responseDetails": null, "responseStatus": 200} )�� ������ �ɰ� ��
 * &quot; &amp; &lt; &gt; &nbsp; �� ���� ��ȯ�� ������ ���� ������� ��ȯ�Ѵ�.
 */
wstring getTranslatedText(wstring strSource){

	//Tokenizing

	const wchar_t preTok[]=L"\"translatedText\":\"";
	wstring::size_type preIdx= strSource.find(preTok);

	if(strSource.npos==preIdx)return L"";

	preIdx+=sizeof(preTok)/sizeof(wchar_t)-1;

	wstring::size_type postIdx= strSource.find(L'"',preIdx);

	if(strSource.npos==postIdx)return L"";

	wstring strRawText = strSource.substr(preIdx,postIdx-preIdx);

	//Unescape
	strRawText = GetUnescapeJSString(strRawText);

	//Symbolic HTML entity
	vector<wchar_t> tmpBuff=vector<wchar_t>(strRawText.begin(),strRawText.end());
	vector<wchar_t> tmpRet;
	tmpRet.reserve(tmpBuff.size()*3/2);

	for(UINT i=0;i<tmpBuff.size();i++){
		if(tmpBuff[i]==L'&'){
			if(_wcsnicmp(&tmpBuff[i],L"&quot;",6)==0){
				i+=6-1;
				tmpRet.push_back(L'"');
			}
			else if(_wcsnicmp(&tmpBuff[i],L"&amp;",5)==0){
				i+=5-1;
				tmpRet.push_back(L'&');
			}
			else if(_wcsnicmp(&tmpBuff[i],L"&lt;",4)==0){
				i+=4-1;
				tmpRet.push_back(L'<');
			}
			else if(_wcsnicmp(&tmpBuff[i],L"&gt;",4)==0){
				i+=4-1;
				tmpRet.push_back(L'>');
			}
			else if(_wcsnicmp(&tmpBuff[i],L"&nbsp;",6)==0){
				i+=6-1;
				tmpRet.push_back(L' ');
			}
			else{
				tmpRet.push_back(tmpBuff[i]);
			}
		}
		else{
			tmpRet.push_back(tmpBuff[i]);
		}
	}

	return wstring(tmpRet.begin(),tmpRet.end());
}

/*
 * g_strUnsafeChars�� �ش��ϴ� ���ڸ� <tok:#>�� �������� ��ȯ�Ѵ�.
 */
wstring EncodeUnsafeCharactor(wstring strSource){
	wstring strRet;
	strRet.reserve(strSource.length()*2);

	wchar_t buff[11]=L"0";

	for(UINT i=0;i<strSource.length();i++){
		UINT idx=g_strUnsafeChars.find(strSource[i]);
		if(idx==g_strUnsafeChars.npos){
			strRet.push_back(strSource[i]);
		}
		else{
			strRet+=g_strPreToken;
			_itow_s(idx,buff,10,10);
			strRet+=buff;
			strRet+=g_strPostToken;
		}
	}

	return strRet;
}

/*
 * <tok:#>�� �������� ������ ���ڸ� g_strUnSafeChars �� ������ �� ��ȯ�Ѵ�.
 */
wstring DecodeUnsafeCharactor(wstring strSource){
	wstring strRet;
	strRet.reserve(strSource.length());

	wstring::size_type last_idx = 0;
	wstring::size_type pre_idx = 0;
	wstring::size_type post_idx = 0;

	while(true){
		if(last_idx==strSource.length()){
			break;
		}

		pre_idx = strSource.find(g_strPreToken,last_idx);

		if(pre_idx == strSource.npos){
			strRet.insert(strRet.end(),strSource.begin()+last_idx,strSource.end());
			break;
		}
		
		if(pre_idx>last_idx){
			strRet.insert(strRet.end(),strSource.begin()+last_idx,strSource.begin()+pre_idx);
		}

		last_idx=pre_idx;

		pre_idx+=g_strPreToken.length();

		post_idx = strSource.find(g_strPostToken,pre_idx);

		if(post_idx == strSource.npos){
			strRet.insert(strRet.end(),strSource.begin()+last_idx,strSource.begin()+pre_idx);
			continue;
		}

		while(strRet.length()>0&&strRet.back()==L' '){
			strRet.pop_back();
		}

		wstring strNumber = strSource.substr(pre_idx,post_idx-pre_idx);
		UINT nWord = (UINT)_wtoi(strNumber.c_str());
		if(nWord<g_strUnsafeChars.size()){
			strRet.push_back(g_strUnsafeChars[nWord]);
		}
		else{
			strRet+=strNumber;
		}

		last_idx=post_idx+g_strPostToken.length();

		while(last_idx<strSource.length()&&strSource[last_idx]==L' '){
			last_idx++;
		}

	}

	return strRet;
}
