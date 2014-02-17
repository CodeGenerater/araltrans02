#include "StdAfx.h"

#include "DenyWord.h"

#include <string.h>

#include <stdio.h>

extern tstring g_strDebugFile;
CDenyWord g_cDenyWord;

void CDenyWord::Load()
{
	if (!m_cDenyCheck.empty() || !m_cAllowCheck.empty() || !m_cExactCheck.empty())
	{
		m_cExactCheck.clear();
		m_cDenyCheck.clear();
		m_cAllowCheck.clear();
	}

	int i;
	char szKey[20], szData[1024];

	char szIniName[MAX_PATH]={'\0',};

	FILE *fp=NULL;
	if (!g_strDebugFile.empty())
		fp=_tfopen(g_strDebugFile.c_str(), _T("w"));

	if (fp) fprintf(fp, "Load\n");
	
	GetModuleFileNameA(NULL, szIniName, MAX_PATH);
	
	for(i=lstrlenA(szIniName); i>=0; i--)
	{
		if (szIniName[i] == _T('\\'))
		{
			szIniName[i]=_T('\0');
			break;
		}
	}
	lstrcatA(szIniName, "\\ATData\\DenyWord.ini");

	if (fp) fprintf(fp, "inifile = %s\n", szIniName);

	if (fp) fprintf(fp, "[DENY]\n");
	// [DENY]
	for (i=1;;i++)
	{
		wsprintfA(szKey, "Data%d", i);
		GetPrivateProfileStringA("DENY", szKey, "", szData, 1024, szIniName);

		if (!lstrlenA(szData))
			break;

		if (fp) fprintf(fp, "%s=%s (%02X)\n", szKey, szData, (szData[0] & 0x0FF));

		m_cDenyCheck.insert( make_pair(szData[0], string(szData)) );
	}

	if (fp) fprintf(fp, "[ALLOW]\n");	
	// [ALLOW]
	for (i=1;;i++)
	{
		wsprintfA(szKey, "Data%d", i);
		GetPrivateProfileStringA("ALLOW", szKey, "", szData, 1024, szIniName);
		
		if (!lstrlenA(szData))
			break;

		if (fp) fprintf(fp, "%s=%s (%02X)\n", szKey, szData, (szData[0] & 0x0FF));

		m_cAllowCheck.insert( make_pair(szData[0], string(szData)) );
	}

	if (fp) fprintf(fp, "[EXACT]\n");	
	// [EXACT]
	for (i=1;;i++)
	{
		wsprintfA(szKey, "Data%d", i);
		GetPrivateProfileStringA("EXACT", szKey, "", szData, 1024, szIniName);
		
		if (!lstrlenA(szData))
			break;

		if (fp) fprintf(fp, "%s=%s (%02X)\n", szKey, szData, (szData[0] & 0x0FF));

		m_cExactCheck.insert( make_pair(szData[0], string(szData)) );
	}

	if (fp) fprintf(fp, "Load end\n");
	if (fp) fclose(fp);

}

bool CDenyWord::IsDenied(const char *pszJapanese)
{
	int nSize = lstrlenA(pszJapanese);
	int i=0;

	bool bIsDenied=false;
	bool bNoKanji=true; //2����Ʈ ī��,���� �����
	int nKanji=0;		//2����Ʈ ī��,���� �����
	int nChar=0;
	//bool bNoLeadByte=true;

	string strDenied, strAllowed, strExact;

	FILE *fp=NULL;
	if (!g_strDebugFile.empty())
		fp=_tfopen(g_strDebugFile.c_str(), _T("a"));

	if (m_cExactCheck.count(pszJapanese[0]))
	{
		// 0��° ���ڷ� �����ϴ� Exact �ܾ ����
		CCheckMap::iterator itBegin=m_cExactCheck.lower_bound(pszJapanese[0]);
		CCheckMap::iterator itEnd=m_cExactCheck.upper_bound(pszJapanese[0]);

		CCheckMap::iterator it;

		// ExactCheck

		for (it=itBegin; it != itEnd; it++)
		{
			strExact = it->second;
			
			if (!strcmp(strExact.c_str(), pszJapanese))
			{
				// ���� ��ġ
				bIsDenied=true;

				// Loop�� ����	
				nSize=0;

				break;
			}
		}
	} // if (m_cExactCheck..)

	for (i=0; i<nSize; i++)
	{
		if (IsDBCSLeadByteEx(932, (BYTE)pszJapanese[i]))
		{
			//bNoLeadByte=false;

			//2����Ʈ ī��,���� ����

			//Shift-JIS ��¥ �ڵ� �����׿�.
			//���ڸ��� ������ ���� ��-

			WORD Char=(BYTE)pszJapanese[i]*256+(BYTE)pszJapanese[i+1];
			if(
				!m_bNotKanji && (
				(0x8141 == Char) ||							//,
				(0x8142 == Char) ||							//.
				(0x8163 == Char) ||							//��
				((0x8175 <= Char) && (Char <= 0x82FA)) ||	//����ȣ
				(0x8158 == Char) ||							//���ȣ
				((0x829F <= Char) && (Char <= 0x82FA)) ||	//���󰡳�
				((0x8340 <= Char) && (Char <= 0x8396)) ||	//��Ÿī��
				((0x889F <= Char) && (Char <= 0xEAA4)) ||	//���� ��1~2����
				((0xED40 <= Char) && (Char <= 0xEEEC))		//���� ��3����
				))
			{
				nKanji++;
			}
			else if(
				m_bNotKanji && (
				(0x8141 == Char) ||							//,
				(0x8142 == Char) ||							//.
				(0x8163 == Char) ||							//��
				((0x8175 <= Char) && (Char <= 0x82FA)) ||	//����ȣ
				(0x8158 == Char) ||							//���ȣ
				((0x829F <= Char) && (Char <= 0x82FA)) ||	//���󰡳�
				((0x8340 <= Char) && (Char <= 0x8396))// ||	//��Ÿī��
				//((0x8940 <= Char) && (Char <= 0xEAA4)) ||	//���� ��1~2����
				//((0xED40 <= Char) && (Char <= 0xEEEC))		//���� ��3����
				))
			{
				nKanji++;
			}
			bNoKanji = false;
			i++;
		}

		nChar++;
	}	// for (i=0;...)

	if( ((nChar*m_nPassKanji/100) >= nKanji) && nKanji)
		bIsDenied = true;

	if( bNoKanji && nSize)
		bIsDenied = true;

	for (i=0; i<nSize; i++)
	{
		CCheckMap::iterator itBegin, itEnd, it;

		if (m_cDenyCheck.count(pszJapanese[i]))
		{
			// �ش� ���ڷ� ���۵Ǵ� Deny �ܾ ����
			itBegin=m_cDenyCheck.lower_bound(pszJapanese[i]);
			itEnd=m_cDenyCheck.upper_bound(pszJapanese[i]);

			// DenyCheck
			for (it=itBegin; it != itEnd; it++)
			{
				strDenied = it->second;
				
				if ( !strncmp(strDenied.c_str(), (pszJapanese+i), strDenied.size()) )
				{
					// ���� ��ġ
					bIsDenied=true;
					break;
				}
			}
		} // if (m_cDenyCheck..)

		// ���������� denied �� ��� ����
		if (bIsDenied)
			break;

		if (IsDBCSLeadByteEx(932, (BYTE)pszJapanese[i]))
			i++;
	}

	if (bIsDenied)
	{
		for (i=0; i<nSize; i++)
		{
			CCheckMap::iterator itBegin, itEnd, it;

			if (m_cAllowCheck.count(pszJapanese[i]))
			{
				// �ش� ���ڷ� ���۵Ǵ� Allow �ܾ ����
				itBegin=m_cAllowCheck.lower_bound(pszJapanese[i]);
				itEnd =m_cAllowCheck.upper_bound(pszJapanese[i]);

				// AllowCheck
				for (it=itBegin; it != itEnd; it++)
				{
					strAllowed = it->second;

					if ( !strncmp(strAllowed.c_str(), (pszJapanese+i), strAllowed.size()) )
					{
						// ���� ��ġ
						bIsDenied = false;
						break;
					}
				}
			}

			// ���������� Allow �� ��� ����
			if (!bIsDenied)
				break;

			if (IsDBCSLeadByteEx(932, (BYTE)pszJapanese[i]))
				i++;
		}
	}
	

	if (fp)
	{
		fprintf(fp, "-------------------------------\n");
		fprintf(fp, "Check=%s\n", pszJapanese);

		if (bIsDenied)
		{
			fprintf(fp, "      ");
			for (int j=0; j < i; j++)
				fprintf(fp, " ");
			fprintf(fp, "^ %s\nFailed.\n", strDenied.c_str());
		}
		else if (bNoKanji)
			fprintf(fp, "No Kana,Kanji Charactor - Ignored.\n");
			//fprintf(fp, "1 byte characters only - Ignored.\n");
		else if (nSize <= m_nCut)
			fprintf(fp, "Size is too small (Cut=%d) - Ignored.\n", m_nCut);
		else
			fprintf(fp, "Passed.\n");

		fclose(fp);
	}

	if (nSize <= m_nCut)
		bIsDenied = true;

//	if (bNoKanji)
//		return true;
	
	return bIsDenied;
}