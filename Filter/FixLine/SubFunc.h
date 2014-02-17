#pragma once
#include "tstring.h"

#define STRLEN 2048

// Hex <-> ���ڿ� ��ȯ �Լ� ��
void atoh(LPSTR szDest,LPCSTR szSource);
string atoh(string strSource);

string itoa(int n);

/*void wtoh(LPWSTR szDest, LPCWSTR szSource);
string stl_wtoh(wstring strSource);*/

void htoa(LPSTR szDest,LPCSTR szSource);
string htoa(string strSource);

/*void htow(LPWSTR szDest, LPCSTR szSource);
wstring stl_htow(string strSource);*/

//string EncodeLine(string strSource);
string DecodeLine(string strSource);

int Find(string strSource, string strFind, int nStart, int nCodePage);

string wtoa(wstring wstrText,int nCodePage=932); //�⺻�� 932
wstring atow(string strText,int nCodePage=932); //�⺻�� 932

string trim(string& strText,int nType=0);