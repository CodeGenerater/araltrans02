#include "StdAfx.h"
#include "ATPlugin.h"
#include "CharacterMapper.h"

BOOL CATPluginApp::PreTranslate(LPCSTR cszInJapanese, LPSTR szOutJapanese, int nBufSize)
{
	int nLength, i;

	m_cStringArray.clear();

	string strResult;

	string strEncodedKorean;
	bool bIsKorean = false;

	CCharacterMapper2 cCharMap;

	nLength = lstrlenA(cszInJapanese);

	for (i=0; i<nLength; i++)
	{
		if (cCharMap.IsEncodedText(cszInJapanese+i))
		{
			// EncodeKor2 �ѱ�
			if (bIsKorean == false)
			{
				// ù��° �ѱ� ����
				bIsKorean = true;
			}
			// ���ڵ��� �ѱ��� �����Ѵ�
			strEncodedKorean+=cszInJapanese[i];
			i++;
			if (i == nLength) break;
			strEncodedKorean+=cszInJapanese[i];
		}
		else
		{
			if (bIsKorean == true)
			{
				// ���ڵ��� �ѱ� ��
				bIsKorean = false;

				int nEncodedLength = strEncodedKorean.size();
				char szEncode[3]={0,}, szDecode[3]={0,};
				string strDecodedKorean;

				// ���ڵ��ϰ�
				for(int idx=0; idx < nEncodedLength; idx+=2)
				{
					szEncode[0]=strEncodedKorean[idx];
					szEncode[1]=strEncodedKorean[idx+1];

					cCharMap.DecodeJ2K(szEncode, szDecode);

					strDecodedKorean += szDecode;
				}

				// ������ �� ��ũ
				m_cStringArray.push_back(strDecodedKorean);
				strResult+=ENCODE_MARKER_STRING;
				strEncodedKorean.clear();
			}
			if (IsDBCSLeadByteEx(932, (BYTE)cszInJapanese[i]))
			{
				strResult+= cszInJapanese[i];
				i++;
			}
			if (i == nLength) break;
			strResult+=cszInJapanese[i];
		}
	}

	if (bIsKorean)
	{
		// ���� �������� �ѱ� - ������ �ѱ��� ������ ��ũ
		int nEncodedLength = strEncodedKorean.size();
		char szEncode[3]={0,}, szDecode[3]={0,};
		string strDecodedKorean;

		for(int idx=0; idx < nEncodedLength; idx+=2)
		{
			szEncode[0]=strEncodedKorean[idx];
			szEncode[1]=strEncodedKorean[idx+1];

			cCharMap.DecodeJ2K(szEncode, szDecode);

			strDecodedKorean += szDecode;
		}

		m_cStringArray.push_back(strDecodedKorean);
		strResult+=ENCODE_MARKER_STRING;
	}

	// ���ڵ��� �ѱ� �̿ܿ� ������ ���� ������ ���� ����
	nLength = strResult.size();
	for(i=0; i<nLength; i++)
	{
		if ((BYTE)strResult[i] >= 0x80)
			break;
	}
	if (i == nLength)
		return FALSE;

	lstrcpyA(szOutJapanese, strResult.c_str());


	return TRUE;
}

BOOL CATPluginApp::PostTranslate(LPCSTR cszInKorean, LPSTR szOutKorean, int nBufSize)
{
	if (m_cStringArray.empty())
	{
		// ���ڵ� ġȯ�� ���� ������ �ܼ�����
		lstrcpyA(szOutKorean, cszInKorean);
		return TRUE;
	}
	

	int idxStringId = 0;
	int i, nLength = lstrlenA(cszInKorean);
	string strResult;

	for (i=0; i<nLength; i++)
	{
		if ( (nLength - i >= ENCODE_MARKER_LENGTH) && 
			(CompareStringA(LOCALE_NEUTRAL, NULL, cszInKorean+i, ENCODE_MARKER_LENGTH, ENCODE_MARKER_STRING, ENCODE_MARKER_LENGTH) == CSTR_EQUAL) )
		{
			// ���� ��Ŀ�� ã������ ġȯ
			strResult+=m_cStringArray[idxStringId];
			idxStringId++;
			i+=ENCODE_MARKER_LENGTH;
			if (i == nLength)
				break;
		}
		strResult+=cszInKorean[i];
	}

	lstrcpyA(szOutKorean, strResult.c_str());

	return TRUE;
}