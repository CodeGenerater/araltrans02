#include "StdAfx.h"

#include "RLCmd.h"

// debug
extern tstring g_strLogFile;

CRLCmd g_cRLCmd;

BOOL CRLCmd::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	FILE *fp=NULL;
	
	if (!g_strLogFile.empty())
		fp=_tfopen(g_strLogFile.c_str(), _T("a"));

	string strBody;
	
	m_nSize=lstrlenA(cszInJapanese);
	
	TrimCommand(cszInJapanese, m_strPrefix, strBody, m_strPostfix);
	
	if (fp)
	{
		fprintf(fp, "1st pass\n");
		fprintf(fp,	"Original=%s, size=%d\n",
			cszInJapanese, lstrlenA(cszInJapanese));
		fprintf(fp, "Prefix=%s, Body=%s, Postfix=%s\n",
			m_strPrefix.c_str(), strBody.c_str(), m_strPostfix.c_str());
		if (strBody.empty())
			fprintf(fp, "No body - no 2nd pass\n");
		
		fclose(fp);
	}
	
	lstrcpyA(szOutJapanese, strBody.c_str());
	
	if (strBody.empty())
		return FALSE;

	return TRUE;
}
BOOL CRLCmd::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
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

void CRLCmd::TrimCommand(const char *cszOriginal, string &strPrefix, string &strBody, string &strPostfix)
{
	int idxBodyStart, idxBodyEnd;
	int i;
	int nLen;
	string strOriginal = cszOriginal;

	bool bIsInQuote = false;


	if ( (strOriginal[0] == '\"') && (strOriginal[1] == '\"') )
	{
		// �������
		strPrefix=strOriginal;
		strBody.erase();
		strPostfix.erase();
		return;
	}
	nLen=strOriginal.size();
	idxBodyStart=idxBodyEnd=0;

//	FILE *fp=NULL;
//	fp=fopen("c:\\test.txt", "w");

	// BodyStart �� ã�´�.
	for(i=0;i<nLen; i++)
	{
		// ù��° 2����Ʈ ����
		if (IsDBCSLeadByteEx(932, (BYTE)strOriginal[i]))
			break;

		if (strOriginal[i] == '\"')
			break;
	}

	if (i >= nLen)
	{
		// 100% ����
		strPrefix=strOriginal;
		strBody.erase();
		strPostfix.erase();
		return;
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
			i++;	// 2����Ʈ ������ �޺κ����� �̵�
//			fprintf(fp, "is DBCSLeadByte\n");
		}
		else
		{
//			fprintf (fp, "is HalfByte %c\n",strOriginal[i]);
			if ( ('A' <= strOriginal[i] ) && (strOriginal[i] <= 'Z') ||
				('a' <= strOriginal[i] ) && (strOriginal[i] <= 'z') )
				continue;

			if ( (strOriginal[i] == ' ') || (strOriginal[i] == '!')
				)
				continue;

			if (strOriginal[i] == '\"')
			{
				bIsInQuote=!bIsInQuote;
//				fprintf(fp, "bIsInQuote=%s\n", (bIsInQuote)?"true":"false");
				continue;
			}

			if (!bIsInQuote)
			{
				idxBodyEnd=i-1;	// idxBodyEnd= 2����Ʈ ������ �޺κ�
				break;
			}
		}
		
	}

	if (i == nLen)
		idxBodyEnd=nLen-1;

//	fprintf(fp, "nBodyEnd=%d\n", idxBodyEnd);
//	fclose(fp);

	// idxBodyEnd = ������ 2����Ʈ ������ �޺κ�

	// ���ڿ��� �߶󳽴�. *����-substr(����,����)
	strPrefix=strOriginal.substr(0, idxBodyStart);
	strBody=strOriginal.substr(idxBodyStart, idxBodyEnd-idxBodyStart+1);
	strPostfix=strOriginal.substr(idxBodyEnd+1);

}

void CRLCmd::RestoreCommand(string &strPrefix, const char *cszTranslated, string &strPostfix, string &strRestored, int nMaxSize)
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
			if (m_bRemoveSpace)
				strRestored+="��";
			else
				strRestored+="  ";
		}
		strRestored+="��";	
	}

	// ����
	strRestored=strPrefix+strRestored+strPostfix;
}

void CRLCmd::SqueezeAndReplaceSpecialString(string &strData)
{

	int i;

	bool bIsSpecial=false;
	
	string strOriginal=strData;
	strData.erase();
	
	for(i=0; i<strOriginal.size(); i++)
	{
		if (!IsDBCSLeadByteEx(949,(BYTE)strOriginal[i] ) )
		{
			switch (strOriginal[i])
			{
				case '*':
					strData+= (char) 0x081;
					strData+= (char) 0x096;
					bIsSpecial=true;
					break;
				case '%':
					strData+= (char) 0x081;
					strData+= (char) 0x093;
					bIsSpecial=true;
					break;

				case ' ':
					bIsSpecial=false;
					if (!m_bRemoveSpace)
					{
						if ( (strOriginal[i+1] == ',') || (strOriginal[i+1] == ' ') )
							break;
						else
							strData+=strOriginal[i];
					}
					break;

				case '.':
					bIsSpecial=false;
					if (i == strOriginal.size()-1)
						strData+= "��";
					else
						strData+=strOriginal[i];
					break;

				case ',':
					bIsSpecial=false;
					strData+="��";
					if (strOriginal[i+1] == ' ')
						i++;
					break;

				case '!':
					bIsSpecial=false;
					strData+="��";
					break;

				case '\'':	// Ȭ����ǥ, �����ǥ ����
				case '\"':
					break;

				default:

					if (bIsSpecial && ('A' <= strOriginal[i]) && (strOriginal[i] <= 'Z'))
					{
						strData+= (char) 0x082;
						strData+= (char) ((BYTE)strOriginal[i] + 0x1F );
					}
					else
					{
						bIsSpecial=false;
						strData+=strOriginal[i];
					}
			}	// switch...
		}
		else
		{
			WORD wch=MAKEWORD(strOriginal[i+1], strOriginal[i]);
			bIsSpecial=false;
			
			if ( wch == 0xA1BC)	// ��
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
			{
				strData+=strOriginal[i];
				strData+=strOriginal[i+1];
			}
			i++;
		}	// if(...)
	}	// for..

	if (m_bIsNoFreeze && !m_bRemoveSpace)
	{
		// ��, ���� Ȯ��ϼ��� ���� ���� ���� ������ ��� ������ �����ϱ� ���� 1����Ʈ,2����Ʈ ��ĭ�� �߰�
		strData+=" ��";
	}
}

