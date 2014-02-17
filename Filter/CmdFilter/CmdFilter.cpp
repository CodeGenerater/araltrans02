#include "StdAfx.h"

#include "CmdFilter.h"

// debug
extern tstring g_strLogFile;

CCmdFilter g_cCmdFilter;

CCmdFilter::CCmdFilter() : m_nSize(0), m_nCutSize(0), m_bIsOverwrite(false), m_bRemoveSpace(false), m_bIsTwoByte(false)
{
//	tstring strTemp;
//	m_cDenyChecker.SetBoundary(_T("814F"));
//	m_cBodyIncludeChecker.SetBoundary(_T("0x20,0x40,0x30-0x39,0x61-0x7A"));
//	m_cBodyIncludeChecker.GetBoundary(strTemp);
//	MessageBox(NULL, strTemp.c_str(), L"TEST", MB_OK);
}

BOOL CCmdFilter::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	FILE *fp=NULL;
	
	if (!g_strLogFile.empty())
		fp=_tfopen(g_strLogFile.c_str(), _T("a"));

	string strBody;
	BOOL bRet=TRUE;
	
	m_nSize=lstrlenA(cszInJapanese);
	
	bRet=TrimCommand(cszInJapanese, m_strPrefix, strBody, m_strPostfix);
	
	if (fp)
	{
		fprintf(fp, "1st pass\n");
		fprintf(fp,	"Original=%s, size=%d\n",
			cszInJapanese, lstrlenA(cszInJapanese));
		fprintf(fp, "Prefix=%s, Body=%s, Postfix=%s\n",
			m_strPrefix.c_str(), strBody.c_str(), m_strPostfix.c_str());
		if (bRet)
		{
			if (strBody.size() <= m_nCutSize)
				fprintf(fp, "Body size is too small (size=%d) - no 2nd pass\n", strBody.size());
		}
		else
			fprintf(fp, "Denied - no 2nd pass\n");
		
		fclose(fp);
	}
	
	lstrcpyA(szOutJapanese, strBody.c_str());
	
	if (strBody.size() <= m_nCutSize)
		return FALSE;

	return bRet;
}
BOOL CCmdFilter::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	FILE *fp=NULL;
	
	if (!g_strLogFile.empty())
		fp=_tfopen(g_strLogFile.c_str(), _T("a"));

	string strRestored;
	
	string strOriginal=cszInKorean;
	
	SqueezeAndReplaceSpecialString(strOriginal);
	
	RestoreCommand(m_strPrefix, strOriginal.c_str(), m_strPostfix, strRestored, m_nSize);
	if (fp)
	{
		fprintf(fp, "2nd pass\n");
		fprintf(fp, "Prefix=%s, Body=%s, Postfix=%s\n",
			m_strPrefix.c_str(), cszInKorean, m_strPostfix.c_str());
		fprintf(fp, "Restored=%s, size = %d\n", 
			strRestored.c_str(), strRestored.size());
		fclose(fp);
	}
	
	lstrcpyA(szOutKorean, strRestored.c_str());

	return TRUE;
}

BOOL CCmdFilter::TrimCommand(const char *cszOriginal, string &strPrefix, string &strBody, string &strPostfix)
{
	int idxBodyStart, idxBodyEnd;
	int i;
	int nLen;
	string strOriginal = cszOriginal;

	WCHAR wch;

	bool bIsInBoundary;
	bool bIsInQuote = false;

	nLen=strOriginal.size();
	idxBodyStart=idxBodyEnd=0;

//	FILE *fp=NULL;
//	fp=fopen("c:\\test.txt", "w");

	// BodyStart �� ã�´�.
	for(i=0;i<nLen; i++)
	{
		// MBCS�� DBCS �� �ٲ۴�
		if (IsDBCSLeadByteEx(932, (BYTE)strOriginal[i]))
		{
			wch = MAKEWORD(strOriginal[i+1], strOriginal[i]);
		}
		else
			wch = MAKEWORD(strOriginal[i], 0);

		// �ź� üũ
		if (m_cDenyChecker.IsInBoundary(wch))
		{
			return FALSE;
		}

		// Prefix ���� üũ
		// �⺻������ 1����Ʈ�� ��� prefix true
		if (wch & 0xFF00)
			bIsInBoundary = false;
		else
			bIsInBoundary = true;

		// Prefix Include ������ ������ prefix true
		if ( !bIsInBoundary && m_cPrefixIncludeChecker.IsInBoundary(wch) )
			bIsInBoundary=true;

		// Prefix Exclude ������ ������ prefix false
		if ( bIsInBoundary && m_cPrefixExcludeChecker.IsInBoundary(wch) )
			bIsInBoundary=false;
		
		if (!bIsInBoundary)	// ������ ������ ��
			break;

		if (wch & 0xFF00)	// 2����Ʈ�� ��� �ε��� +1 ����
			i++;
	
	}

	if (i >= nLen)
	{
		// 100% ����
		strPrefix=strOriginal;
		strBody.erase();
		strPostfix.erase();
		return TRUE;
	}
	else
	{
		idxBodyStart=i;
	}

//	fprintf(fp, "nBodyStart=%d\n", idxBodyStart);

	// BodyEnd �� ã�´�.
	for (i=idxBodyStart; i<nLen; i++)
	{
//		fprintf (fp, "i=%d, checkchar=0x%02X ", i, (BYTE)strOriginal[i]);
		// 2����Ʈ ������ �պκ�?
		if (IsDBCSLeadByteEx(932, (BYTE)strOriginal[i]))
		{
			wch = MAKEWORD(strOriginal[i+1], strOriginal[i]);
			i++;	// 2����Ʈ ������ �޺κ����� �̵�
//			fprintf(fp, "is DBCSLeadByte\n");
		}
		else
		{
			wch = MAKEWORD(strOriginal[i], 0);
//			fprintf (fp, "is HalfByte %c\n",strOriginal[i]);
		}
//		char szTemp[80];
//		wsprintfA(szTemp,"wch = %04X", wch);
//		MessageBoxA(NULL, szTemp, "TEST", MB_OK);

		// �ź� üũ
		if (m_cDenyChecker.IsInBoundary(wch))
		{
			return FALSE;
		}
		
		// Body ���� üũ
		// �⺻������ 2����Ʈ�� ��� Body true
		if (wch & 0xFF00)
		{
			bIsInBoundary = true;
		}
		else
			bIsInBoundary = false;
		
		// Body Include ������ ������ Body true
		if ( !bIsInBoundary && m_cBodyIncludeChecker.IsInBoundary(wch) )
			bIsInBoundary=true;
		
		// Body Exclude ������ ������ Body false
		if ( bIsInBoundary && m_cBodyExcludeChecker.IsInBoundary(wch) )
			bIsInBoundary=false;
		
		if (!bIsInBoundary)	// ������ ������ ��
			break;

	}

	idxBodyEnd = i-1;

	if (i != nLen)
	{
		// PostFix �ź� üũ
		for(i=idxBodyEnd;i<nLen; i++)
		{
			// MBCS�� DBCS �� �ٲ۴�
			if (IsDBCSLeadByteEx(932, (BYTE)strOriginal[i]))
			{
				wch = MAKEWORD(strOriginal[i+1], strOriginal[i]);
			}
			else
				wch = MAKEWORD(strOriginal[i], 0);

			// �ź� üũ
			if (m_cDenyChecker.IsInBoundary(wch))
			{
				return FALSE;
			}
		}

	}

//	fprintf(fp, "nBodyEnd=%d\n", idxBodyEnd);
//	fclose(fp);

	// idxBodyEnd = ������ 2����Ʈ ������ �޺κ�

	// ���ڿ��� �߶󳽴�. *����-substr(����,����)
	strPrefix=strOriginal.substr(0, idxBodyStart);
	strBody=strOriginal.substr(idxBodyStart, idxBodyEnd-idxBodyStart+1);
	strPostfix=strOriginal.substr(idxBodyEnd+1);

	return TRUE;
}

void CCmdFilter::RestoreCommand(string &strPrefix, const char *cszTranslated, string &strPostfix, string &strRestored, int nMaxSize)
{
	int i;

	// Prefix, Postfix ��뷮�� �ִ�ũ�⿡�� ����
	nMaxSize= nMaxSize-strPrefix.size()-strPostfix.size();
	strRestored=cszTranslated;

	if ( m_bIsOverwrite && (strRestored.size() >= nMaxSize) )	// ���̰� �ʹ� ���.
	{
		// ĿƮ ��ġ �˻�
		for(i=0; i<nMaxSize; i++)
		{
			// 2����Ʈ ������ �պκ�?
			if (IsDBCSLeadByteEx(949, (BYTE)strRestored[i]))
			{
				if (i+1 == nMaxSize)
					break;	// 2����Ʈ ���ڰ� ���� �ȵ��� - �ش� ���� ����
				else
					i++;
			}
			else if (i+2 == nMaxSize)	// 2���ڹۿ� �ȳ�������
			{
				if (!IsDBCSLeadByteEx(949, (BYTE)strRestored[i+1]))	// ������ 1����Ʈ ����
				{
					// 2����Ʈ ��ĭ���� ���´�.
					strRestored[i]=(char) 0x0A1;
					strRestored[i+1]=(char) 0x0A1;
				}
			}
			else if (i+1 == nMaxSize)	// 1����Ʈ ���ڷ� ���� ���ڿ� - �ش� ���� ����
				break;
		}
		strRestored=strRestored.substr(0,i);	// ĿƮ
	}

	// ������ Ȧ¦�� �����ش�
	if ( (strRestored.size() % 2) != (nMaxSize % 2) )
	{
		// ��ĭ�� �ϳ� �߰��� ������ Ȧ¦�� �����ش�
		for (i=0; i<strRestored.size(); i++)
		{
			if ( (BYTE)strRestored[i] < 0x80)
			{
				strRestored.insert(i+1, 1, ' ');
				break;
			}
		}
		
		// ��ĭ�� ����
		if ( (strRestored.size() % 2) != (nMaxSize % 2) )
			strRestored.insert(2, 1, ' ');	// ������ ����
		
	}

	// ���� strRestored ���̴� nMaxSize ���� �۰ų� ����.

	// strRestored ���̰� nMaxSize ���� �۴�
	if (strRestored.size() < nMaxSize)
	{
		int nDiff= nMaxSize - strRestored.size();

		for(i=0; i<nDiff-2; i+=2)	// ���ڸ� ä���ֱ�
		{
			if (m_bRemoveSpace || m_bIsTwoByte)
				strRestored+="��";
			else
				strRestored+="  ";
		}
		strRestored+="��";	// ���ڸ� ä���ֱ�
	}

	// ����
	strRestored=strPrefix+strRestored+strPostfix;
}

void CCmdFilter::SqueezeAndReplaceSpecialString(string &strData)
{

	int i;

	bool bIsSpecial=false;
	
	string strOriginal=strData;
	strData.erase();
	
	for(i=0; i<strOriginal.size(); i++)
	{
		if (!IsDBCSLeadByteEx(949,(BYTE)strOriginal[i] ) )
		{
			char ch='\0';	// TWOBYTE �� ��ȯ�� �ʿ��� �� ���ڸ� ����
			switch (strOriginal[i])
			{

				case ' ':
					bIsSpecial=false;
					if (!m_bRemoveSpace)
					{
						if ( (strOriginal[i+1] == ',') || (strOriginal[i+1] == '.') || (strOriginal[i+1] == ' ') )
							break;
						else if ( m_bIsTwoByte && 
							((strOriginal[i-1] == ',') || (strOriginal[i-1] == '.') || (strOriginal[i+1] == ' ')) )
							break;
						else
							ch=strOriginal[i];
					}
					break;

				case '.':
					bIsSpecial=false;
					if (i == strOriginal.size()-1)
						strData+= "��";
					else
						ch=strOriginal[i];
					break;

				case '!':
					bIsSpecial=false;
					strData+="��";
					break;

				case '\'':	// Ȭ����ǥ, �����ǥ ����
				case '\"':
					break;

				default:
					ch = strOriginal[i];
			}	// switch...

			if (ch)
			{
				if (m_bIsTwoByte)	// TWOBYTE ���� ��
				{
					// 1����Ʈ -> 2����Ʈ ��ȯ
					// From CCharacterMapper::Ascii2KS5601() for ATCode �÷�����
					if ( (0x020 <= (BYTE)strOriginal[i]) && ((BYTE)strOriginal[i] <= 0x07E) )
					{
						if (strOriginal[i] == ' ')
						{
							strData+="��";
						}
						else
						{
							strData+=(char) 0x0A3;
							strData+=(char) (0x080+(BYTE)strOriginal[i]);
						}
					}
				}
				else
					strData+=ch;
			}
		}	// if (!IsDBCSLeadByteEX(...))
		else
		{
			WORD wch=MAKEWORD(strOriginal[i+1], strOriginal[i]);
			bIsSpecial=false;
			
/*			if ( wch == 0xA1BC)	// ��
			{
				if (!m_bIsBracket)
				{
					if (!m_bIsOverwrite)
					{
						strData+= (char) 0x081;
						strData+= (char) 0x079;
					}
				}
				else
				{
					strData+=strOriginal[i];
					strData+=strOriginal[i+1];
				}
			}
			else if (wch == 0xA1BD)	// ��
			{
				if (!m_bIsBracket)
				{
					if (!m_bIsOverwrite)
					{
						strData+= (char) 0x081;
						strData+= (char) 0x07A;
					}
					else
					{
						strData+= ' ';
					}
				}
				else
				{
					strData+=strOriginal[i];
					strData+=strOriginal[i+1];
				}

			}
			else
*/			{
				strData+=strOriginal[i];
				strData+=strOriginal[i+1];
			}
			i++;
		}	// if(...)
	}	// for..
}
