
#include "stdafx.h"
#include "CharacterMapper.h"


BYTE CCharacterMapper::_SHIFT_JIS_MAP[0x10000];
BOOL CCharacterMapper::bMapInitialized = FALSE;

const BYTE _MAP[][4]={
	{ '\xBE', '\xC6', '\xA8', '\xA1' },	// ('��')
	{ '\xB7', '\xB8', '\xA8', '\xA2' },	// ('��')
	{ '\xB8', '\xD5', '\xA8', '\xA3' },	// ('��')
	{ '\x86', '\x9C', '\xA8', '\xA4' },	// ('��')
	{ '\x9A', '\x9C', '\xA8', '\xA5' },	// ('��')
	{ '\x8B', '\xAF', '\xA8', '\xA6' },	// ('��')
	{ '\x00', '\x00', '\x00', '\x00' }
};

//////////////////////////////////////////////////////////////////////////
//
// �� �ѱ��ڵ带 SHIFTJIS �� ��� �󵵰� ���� �����ڵ念���� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CCharacterMapper::EncodeK2J(LPCSTR cszKorCode, LPSTR szJpnCode)
{
	BOOL bRetVal = FALSE;

	for(int k=3;_MAP[k][0];k++)
	{
	if(cszKorCode && cszKorCode && 0xA0 <= (BYTE)cszKorCode[0] && 0xA0 <= (BYTE)cszKorCode[1]
		|| _MAP[k][0] == (BYTE)cszKorCode[0] && _MAP[k][1] == (BYTE)cszKorCode[1]	)
	{
		// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
		// 95~3074, 3119~4535, 16632~18622, 19089~19450
		BYTE hiKorByte = (BYTE)cszKorCode[0];
		BYTE loKorByte = (BYTE)cszKorCode[1];
		/*
		// ���� ('��')
		if(hiKorByte==0xBE && loKorByte==0xC6){ hiKorByte = 0xA8; loKorByte = 0xA1; }
		// ���� ('��')
		if(hiKorByte==0xB7 && loKorByte==0xB8){ hiKorByte = 0xA8; loKorByte = 0xA2; }
		// ���� ('��')
		if(hiKorByte==0xB8 && loKorByte==0xD5){ hiKorByte = 0xA8; loKorByte = 0xA3; }
		// ���� ('��')
		if(hiKorByte==0x86 && loKorByte==0x9C){ hiKorByte = 0xA8; loKorByte = 0xA4; }
		// ���� ('��')
		if(hiKorByte==0x9A && loKorByte==0x9C){ hiKorByte = 0xA8; loKorByte = 0xA5; }
		// ���� ('��')
		if(hiKorByte==0x8B && loKorByte==0xAF){ hiKorByte = 0xA8; loKorByte = 0xA6; }
		*/

		int z=0;
		while(_MAP[z][0])
		{
			if(hiKorByte==_MAP[z][0] && loKorByte==_MAP[z][1])
			{
				hiKorByte = _MAP[z][2];
				loKorByte = _MAP[z][3];
				break;
			}
			(BYTE)z++;
		}

		WORD dwKorTmp = ( hiKorByte - 0xA0) * 0xA0;	// 96*96
		dwKorTmp += (loKorByte - 0xA0);

		dwKorTmp += 95;
		if(dwKorTmp > 3074) dwKorTmp += 44;
		if(dwKorTmp > 4535) dwKorTmp += 12096;
		if(dwKorTmp > 18622) dwKorTmp += 466;
		if(dwKorTmp > 19450) return FALSE;
		
		BYTE hiByte = (dwKorTmp / 189) + 0x88;
		BYTE loByte = (dwKorTmp % 189) + 0x40;
		dwKorTmp = MAKEWORD( loByte, hiByte );

		TRACE("[ aral1 ] EncodeK2J : '%s'(%p) -> (%p) \n", cszKorCode, MAKEWORD(cszKorCode[1], cszKorCode[0]), dwKorTmp );

		if( 0x889F <= dwKorTmp && dwKorTmp <= 0xEEEC )
		{
			szJpnCode[0] = HIBYTE(dwKorTmp);
			szJpnCode[1] = LOBYTE(dwKorTmp);

			bRetVal = TRUE;
		}
	}
	}
	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// �� ���εǾ��� �ѱ��ڵ带 ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CCharacterMapper::DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode)
{
	BOOL bRetVal = FALSE;

	// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
	// 95~3074, 3119~4535, 16632~18622, 19089~19450
	if	( cszJpnCode && szKorCode && 
			(
				(0x88 <= (BYTE)cszJpnCode[0] && (BYTE)cszJpnCode[0] <= 0x9F) ||
				(0xE0 <= (BYTE)cszJpnCode[0] && (BYTE)cszJpnCode[0] <= 0xEE)
			)
		)
	{


		WORD dwKorTmp = (WORD)( (BYTE)cszJpnCode[0] - 0x88 ) * 189;
		dwKorTmp += (WORD)( (BYTE)cszJpnCode[1] - 0x40 );

		if(dwKorTmp > 19450) return FALSE;
		if(dwKorTmp > 18622) dwKorTmp -= 466;
		if(dwKorTmp > 4535) dwKorTmp -= 12096;
		if(dwKorTmp > 3074) dwKorTmp -= 44;
		dwKorTmp -= 95;

		szKorCode[0] = (char)( ((UINT_PTR)dwKorTmp/(UINT_PTR)0xA0) + (UINT_PTR)0xA0);
		szKorCode[1] = (char)( ((UINT_PTR)dwKorTmp%(UINT_PTR)0xA0) + (UINT_PTR)0xA0);

		TRACE("[ aral1 ] DecodeJ2K : (%p) -> (%p)'%s' \n", (UINT_PTR)(MAKEWORD(cszJpnCode[1], cszJpnCode[0])), (UINT_PTR)(MAKEWORD(szKorCode[1], szKorCode[0])), szKorCode);

		for(int k=3;_MAP[k][0];k++)
		{
		if( 0xA0 <= (BYTE)szKorCode[0] && 0xA0 <= (BYTE)szKorCode[1]
			|| _MAP[k][0] == (BYTE)szKorCode[0] && _MAP[k][1] == (BYTE)szKorCode[1]	)
		{
			int z=0;
			while(_MAP[z][0])
			{
				if( (BYTE)szKorCode[0]==(BYTE)_MAP[z][2] && (BYTE)szKorCode[1]==(BYTE)_MAP[z][3] )
				{
					*((BYTE*)szKorCode) = (BYTE)_MAP[z][0];
					*((BYTE*)szKorCode+1) = (BYTE)_MAP[z][1];
					break;
				}
				(BYTE)z++;
			}
			/*
			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA1 )
			{
				*((BYTE*)szKorCode) = 0xBE;
				*((BYTE*)szKorCode+1) = 0xC6;
			}

			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA2 )
			{
				*((BYTE*)szKorCode) = 0xB7;
				*((BYTE*)szKorCode+1) = 0xB8;
			}

			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA3 )
			{
				*((BYTE*)szKorCode) = 0xB8;
				*((BYTE*)szKorCode+1) = 0xD5;
			}
			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA4 )
			{
				*((BYTE*)szKorCode) = 0x86;
				*((BYTE*)szKorCode+1) = 0x9C;
			}
			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA5 )
			{
				*((BYTE*)szKorCode) = 0x9A;
				*((BYTE*)szKorCode+1) = 0x9C;
			}
			// ���� ('��')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA6 )
			{
				*((BYTE*)szKorCode) = 0x8B;
				*((BYTE*)szKorCode+1) = 0xAF;
			}
			*/

			bRetVal = TRUE;
		}
		}
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// �� �Լ��� �ӿ�����...? ��,.��;;
//
//////////////////////////////////////////////////////////////////////////
WORD CCharacterMapper::Ascii2KS5601( char chAscii )
{
	WORD wChar = 0;

	if( 0x20 <= (BYTE)chAscii && (BYTE)chAscii <= 0x7E )
	{	
		if(' ' == chAscii) wChar = 0xA1A1;
		else wChar = MAKEWORD( 0x80 + (BYTE)chAscii, 0xA3 );
	}

	return wChar;	
}


//////////////////////////////////////////////////////////////////////////
//
// _SHIFT_JIS_MAP �迭�� ����Ͽ� �־��� ���ڿ��� ���ڵ���
// ��� ��ȿ�� �Ϻ������� �˻�
// TRUE:��ȿ, FALSE:������ ���� ����
//
//////////////////////////////////////////////////////////////////////////
BOOL CCharacterMapper::IsShiftJISText( LPCSTR cszJpnText )
{
	BOOL bRetVal = TRUE;

	if(FALSE == bMapInitialized)
	{
		InitializeShiftJISMap();
		bMapInitialized = TRUE;
	}

	int i=0;
	while(cszJpnText[i])
	{
		WORD chCode = 0;

		// ���� �ڵ� �����
		if((BYTE)cszJpnText[i] >= 0x80 && cszJpnText[i+1] != '\0')
		{
			chCode = MAKEWORD((BYTE)cszJpnText[i+1], (BYTE)cszJpnText[i]);
			i++;
		}
		else
		{
			chCode = MAKEWORD((BYTE)cszJpnText[i], 0);
		}

		// �ڵ� �ʿ��� �˻�
		if(_SHIFT_JIS_MAP[chCode] == FALSE)
		{
			bRetVal = FALSE;
			break;
		}

		i++;
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// SAFE �ɼ��� ���� SHIFTJIS �ڵ� ��ȿ ���̺�
// 1: ��ȿ ����, 0: ������ ����
//
//////////////////////////////////////////////////////////////////////////
void CCharacterMapper::InitializeShiftJISMap()
{
	ZeroMemory(_SHIFT_JIS_MAP, 0x10000);
	
	_SHIFT_JIS_MAP[0x0A] = 1;
	_SHIFT_JIS_MAP[0x0D] = 1;
	memset(&_SHIFT_JIS_MAP[0x20], 1, 0x7E-0x20+1);

	memset(&_SHIFT_JIS_MAP[0x8140], 1, 0x81AC-0x8140+1);
	memset(&_SHIFT_JIS_MAP[0x81B8], 1, 0x81BF-0x81B8+1);
	memset(&_SHIFT_JIS_MAP[0x81C8], 1, 0x81CE-0x81C8+1);
	memset(&_SHIFT_JIS_MAP[0x81DA], 1, 0x81E8-0x81DA+1);
	memset(&_SHIFT_JIS_MAP[0x81F0], 1, 0x81F7-0x81F0+1);
	_SHIFT_JIS_MAP[0x91FC] = 1;
	
	memset(&_SHIFT_JIS_MAP[0x824F], 1, 0x8258-0x824F+1);
	memset(&_SHIFT_JIS_MAP[0x8260], 1, 0x8279-0x8260+1);
	memset(&_SHIFT_JIS_MAP[0x8281], 1, 0x829A-0x8281+1);
	memset(&_SHIFT_JIS_MAP[0x829F], 1, 0x82F1-0x829F+1);

	memset(&_SHIFT_JIS_MAP[0x8340], 1, 0x8396-0x8340+1);
	memset(&_SHIFT_JIS_MAP[0x839F], 1, 0x83B6-0x839F+1);
	memset(&_SHIFT_JIS_MAP[0x83BF], 1, 0x83D6-0x83BF+1);

	memset(&_SHIFT_JIS_MAP[0x8440], 1, 0x8460-0x8440+1);
	memset(&_SHIFT_JIS_MAP[0x8470], 1, 0x8491-0x8470+1);
	memset(&_SHIFT_JIS_MAP[0x849F], 1, 0x84BE-0x849F+1);

	memset(&_SHIFT_JIS_MAP[0x8740], 1, 0x875D-0x8740+1);
	memset(&_SHIFT_JIS_MAP[0x875F], 1, 0x8775-0x875F+1);
	memset(&_SHIFT_JIS_MAP[0x877E], 1, 0x879C-0x877E+1);

	memset(&_SHIFT_JIS_MAP[0x889F], 1, 0x88FC-0x889F+1);
	for(BYTE hiByte = 0x89; hiByte <= 0xE9; hiByte++)
	{
		WORD idxBegin = MAKEWORD(0x40, hiByte);
		WORD idxEnd = MAKEWORD(0xFC, hiByte);
		memset(&_SHIFT_JIS_MAP[idxBegin], 1, idxEnd-idxBegin+1);		
	}
	memset(&_SHIFT_JIS_MAP[0xEA40], 1, 0xEAA4-0xEA40+1);

	memset(&_SHIFT_JIS_MAP[0xED40], 1, 0xEDFC-0xED40+1);
	memset(&_SHIFT_JIS_MAP[0xEE40], 1, 0xEEFC-0xEE40+1);

}
//////////////////////////////////////////////////////////////////////////
//
// �ش� �ؽ�Ʈ ���ڰ� Encode ���ɼ��� �ִ��� Ȯ���ϴ� �Լ�
// TRUE : ���ڵ� �ʿ� FALSE : ���ڵ� ���ʿ�
//
//////////////////////////////////////////////////////////////////////////

BOOL CCharacterMapper::IsEncodedText(LPCSTR cszCode)
{
	// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
	// 95~3074, 3119~4535, 16632~18622, 19089~19450
	if ( 
		(0x88 <= (BYTE)cszCode[0] && (BYTE)cszCode[0] <= 0x9F) ||
		(0xE0 <= (BYTE)cszCode[0] && (BYTE)cszCode[0] <= 0xEE)
	)
		return TRUE;
	
	return FALSE;
}

/*
	* CCharacterMapper2 : ���� �����ϴ� EncodeKor ���� ���̺귯��

	1) ���� EncodeKor �� ������
		���� EncodeKor (CCharacterMapper) �� ��� KS X 1001 (KS C 5601)�� ��ȣ, �ѱ�,
		���� ��ü�� �Ϻ��� ���ڿ����� ���������ϴ� ������� �̷���� �Ϻ���� ȥ��ǵ���
		�Ǿ� �־� �ش� ���ڰ� �Ϻ��� ���ε� �ѱ�����, ���� �Ϻ��� �������� �� �浵�� ����.

	2) ���̵��
		- ������ �����Ǿ� ������ ������ 99%�� (Ŀ���ҵ� ���� ������ �ʴ� �̻�) ���ڰ�
		���ԵǾ� ���� ���� �� �ѱ��̹Ƿ� KS X 1001 �� ���� ������ �ʿ�ġ �ʴ�. Ȥ �ִ���
		Shift-JIS �� ���ڷ� ������ �� ���� ������.
		- ���� (������, 2-byte ����, �θ��� ��)�� ��� Shift-JIS �� �ּ� 95% ������
		���ڰ� �����Ѵ�.
		- �ϻ� ���Ǵ� �ѱ��� 99%�� KS X 1001 �� 2350�� �ѱۿ� ���Եǰ� ���� ������
		CP949 (Ȯ�� �ϼ��� �ѱ�)�� �����ϴ� ���ڰ� ���´�. �� ���ڴ� �� ���Ӵ� ���ƺ���
		100�� �̸����� ���� �� ������ �������� �ڵ带 �Ҵ��ؼ� ó���� �� ���� ������.
		- ���� ��κ��� ���ӿ����� IsLeadByte(Ex) ���� Windows API �Լ��� Leadbyte�� �����
		�����ϰ� üũ���� �ʰ� �׳� 0x81-9F�� 2����Ʈ, 0xE0-FC �� 2����Ʈ, �������� 1����Ʈ��
		ó���Ѵ�.
		- ������ ������ CP932 (Shift-JIS Ȯ����) ���� �� ������ �ִ�. �� ������
			(1) Leadbyte 0x85, 86, EB, EC, EF ���� 5 ���� (���� �Ҹ�)
			(2) Leadbyte 0xF0 ~ F9 ���� 10 ���� (�� �κ��� �ϸ� ���ڰ���. ����� ���ǰ����̴�)
		�̸�, �� ������ Trailbyte 0x40~7E, 0x80~FC ���� 188�� ������ ����� �� �ִ�.
		����, �ش� ������ Ǯ�� ����� ��� 15 x 188 = 2820 ������ CP932 �� ��ġ�� �ʴ�
		������ ��밡���ϴ�.
		- KS X 1001 �� �ѱ� (���� �� ���� ����)�� Leadbyte 0xB0 ~ C8 ���� 25�����̸�,
		�� ������ Trailbyte 0xA1 ~ FE ���� 94��, �� 25 x 94 = 2350���̴�.
		- ��̷ο� ���� CP932 �� 1 ������ 188�ڴ� KS X 1001�� 2���� 2 x 94 = 188 �� �����ϸ�,
		KS X 1001 �� 2 ���� ���ڴ� CP 932�� 1 ���� ���ڿ� �ٷ� ������ �� �ִ�.
	
	3) ����
		- KS X 1001 �� 25���� 2350�ڴ� CP932�� EB, EC, EF, F0~F9 �� 13������ 2:1�� �����Ѵ�.
		- ���� �� ���ڴ� (���� 1:1 ���ε��� �ʴ� ���ڰ� ���� ������) CP932�� ����ȯ ���Ѽ�
		�Ϻ��� ����Ѵ�. Ȥ��, ������ �ѱۿ��� ������ ��� ���� �߻����� �ʱ� ������ ����ó��
		��Ű�� �͵� ���� (�̰��� ���� ����)
		- CP949 Ȯ�� �ϼ��� �ѱ��� ����ó���� �Ͽ�, �������� CP932 �� 85, 86 ������ �Ҵ��Ѵ�.

	4) ����Ǵ� ����
		- �ѱ۰� �Ͼ ������ �����Ƿ� � ���ڿ��� ���ڵ��� �ѱ����� �����ȵ� �Ϻ�������
		����� �ʿ䰡 �������� (���� KoFilter �� �ʿ� ������ ���� �ִ�)
		- ��½� �ѱ۰� �Ϻ���, ���ڰ� ������ ����� ǥ�õ� ���̴�
		- �̿� ���� ���ʿ��� �����۾��̳� ���� ���Ͱ� ������� ������ �ӵ������ �����
		�� �� �ִ�

	5) ����Ǵ� ����
		- ������ ������ ��·�� ������� �ʴ� 5 ������ ������ ����ϴ� ���̱� ������
		���� ���ߴ� ������ ���� �� �ִ�.
		- IsLeadByte(Ex) ������ �����ϰ� MBCS üũ�� �ϴ� ��� ������ ���� �� �ִ�.
		- F0~F9 ������ 10 ������ ���� �̷��� ����� ���ܳ��� �����̹Ƿ� ���� ������ ������
		������ �� ������ ����ϴ� ���α׷��� ���� �� ������ (�Ϸʷ� �Ϻ� �޴����� ���
		�� ������ �̸�Ƽ���� �����Ͽ� ����ϰ� �ִ�) �� ��� ������ ���� �� �ִ�.

*/

WORD CCharacterMapper2::m_awCP932_Lead8586[2][188] = {NULL, };
CRITICAL_SECTION *CCharacterMapper2::m_pCS = NULL;

CCharacterMapper2::CCharacterMapper2()
{
	if (!m_pCS)
	{
		CRITICAL_SECTION *pCS = new CRITICAL_SECTION;
		if (InterlockedCompareExchange((LONG *)&m_pCS, (LONG)pCS, NULL) == NULL)
			InitializeCriticalSection(m_pCS);
		else
			delete pCS;
	}
}

void CCharacterMapper2::ClearStaticMembers()
{
	CRITICAL_SECTION *pCS = (CRITICAL_SECTION *)InterlockedExchange((LONG *)&m_pCS, NULL);
	if (pCS)
	{
		ZeroMemory(m_awCP932_Lead8586, sizeof(WORD)*2*188);
		DeleteCriticalSection(pCS);
		delete pCS;
	}
}

BOOL CCharacterMapper2::EncodeK2J(LPCSTR cszKorCode, LPSTR szJpnCode)
{
	if (!cszKorCode || !szJpnCode)
		return FALSE;

	if (cszKorCode[0] == '\0') 
		return FALSE;

	BOOL bRet = FALSE;
		
	BYTE achKor[3] = {0,}, achJpn[3] = {0,};
	achKor[0] = (BYTE) cszKorCode[0];
	achKor[1] = (BYTE) cszKorCode[1];

	if ( (0xB0 <= achKor[0]) && (achKor[0] <= 0xC8) && (0xA1 <= achKor[1]) && (achKor[1] <= 0xFE) )
	{
		// KS X 1001 �ѱ� ����

		// Leadbyte ����
		BYTE byTempLead = (achKor[0] - 0xB0)/2;	// B0-B1=0, B2-B3=1, ...

		if (byTempLead == 0)	// B0-B1
			achJpn[0] = 0xEB;
		else if (byTempLead == 1)	// B2-B3
			achJpn[0] = 0xEC;
		else	// B4-C8
			achJpn[0] = 0xEF + byTempLead - 2;	// B4-B5: 0xEF + 2 - 2 = 0xEF, B6-B7: 0xEF + 3 - 2 = 0xF0, ...

		// Trailbyte ����
		BYTE byTempTrail = achKor[1] - 0xA1;	// 0xA1-FE �� 0-93 ������ �ű��
		
		if (achKor[0] % 2)
			byTempTrail += 94;	// LeadByte �� Ȧ���� ��� ���� (94-187)�� ����

		byTempTrail += 0x40;	// CP932 �� ��ġ�� 0x40 ���ķ� �������� ��

		if (byTempTrail >= 0x7F)
			byTempTrail ++;		// 0x7F �� ������� �����Ƿ� +1 ������
		
		achJpn[1] = byTempTrail;	// �ϼ�!
		bRet = TRUE;
	}
	else if ( (0xA1 <= achKor[0]) && (achKor[0] <= 0xAF) && (0xA1 <= achKor[1]) && (achKor[1] <= 0xFE) )
	{
		// KS X 1001 ����, �Ͼ�, �θ����� ����

		WCHAR wchTemp[2] = {0, };

		try
		{
			/*
			// �Ͼ�� ����ȯ��Ų��
			if (!MyMultiByteToWideChar(949, 0, (LPCSTR)achKor, -1, wchTemp, 2))
				throw -1;
			if (!MyWideCharToMultiByte(932, 0, wchTemp, -1, (LPSTR)achJpn, 3, NULL, NULL))
				throw -2;
			bRet = TRUE;
			*/

			// ������ ����ȯ���� ó���Ϸ� �ߴµ� ������ �����̳� �θ����ڰ� ���� ���� �� ���� �����Ƿ�
			// �׳� ����ó�� ��Ų��. ���� ���߿� ����ó�� ������ Ȥ�ö� ��ģ�ٸ� ����ȯ�� ������ ����
			bRet = GetUserDefinedJpnCode(achKor, achJpn);
		}
		catch (LONG e)
		{
			// ������ ����ó���� �ϱ� ���� �޾����� ���� Debug.h �ֱ⵵ ���ϰ� �ؼ� �ϴ� ����
			UNREFERENCED_PARAMETER(e);
		}
	}
	else if ( (0xC9 <= achKor[0]) && (achKor[0] <= 0xFE) && (0xA1 <= achKor[1]) && (achKor[1] <= 0xFE) )
	{
		// KS X 1001 ���� ����
		bRet = GetUserDefinedJpnCode(achKor, achJpn);
	}
	else if ( (0x81 <= achKor[0]) && (achKor[0] <= 0xC6) && (0x41 <= achKor[1]) && (achKor[1] <= 0xFE) )
	{
		// CP 949 Ȯ�� �ѱ� ����
		bRet = GetUserDefinedJpnCode(achKor, achJpn);
	}

	if (bRet)
	{
		szJpnCode[0] = (char)achJpn[0];
		szJpnCode[1] = (char)achJpn[1];
	}

	return bRet;
}
BOOL CCharacterMapper2::DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode)
{
	if (!cszJpnCode || !szKorCode)
		return FALSE;

	if (cszJpnCode[0] == '\0')
		return FALSE;

	BOOL bRet = FALSE;

	BYTE achKor[3] = {0,}, achJpn[3] = {0,};
	achJpn[0] = (BYTE) cszJpnCode[0];
	achJpn[1] = (BYTE) cszJpnCode[1];

	if ( (0xEB == achJpn[0] || 0xEC == achJpn[0] || (0xEF <= achJpn[0] && achJpn[0] <= 0xF9)) &&
		(0x40 <= achJpn[1] && achJpn[1] <= 0xFE) )
	{
		// KS X 1001 �ѱ� ����

		// Leadbyte ����
		BYTE byTempLead = achJpn[0] - 0xEB;	// 0xEB = 0, 0xEC = 1
		if (achJpn[0] >= 0xEF)
			byTempLead -=2; // 0xEF = 0xEF-0xEB - 2 = 2, 0xF0 = 3, ...

		byTempLead = byTempLead * 2 + 0xB0; // 0 * 2 + 0xB0 = 0xB0, 1 * 2 + 0xB0 = 0xB2, 2 * 2 + 0xB0 = 0xB4, ...

		// Trailbyte ����
		BYTE byTempTrail = achJpn[1] - 0x40; // Trailbyte �� 0-188 ������ �ű��
		
		if (achJpn[1] > 0x7F)
			byTempTrail --;		// �Ⱦ��� 0x7F �� �ɷ��� ����

		if (byTempTrail >= 94)	// Trailbyte �� �Ĺݺ� (94-187) �̸�
		{
			byTempLead ++;		// Leadbyte �� Ȧ��
			byTempTrail -= 94;	// Trailbyte �� 94�� ���� 0-93 ������ ����
		}
		byTempTrail += 0xA1;	// A1-FE ������ ��������

		achKor[0] = byTempLead;
		achKor[1] = byTempTrail;
		bRet = TRUE;
	}
	else if ( (0x85 == achJpn[0] || 0x86 == achJpn[0]) && (0x40 <= achJpn[1] && achJpn[1] <= 0xFE) )
	{
		// ����� ���� ����
		bRet = GetUserDefinedKorCode(achJpn, achKor);
	}

	if (bRet)
	{
		szKorCode[0] = (char)achKor[0];
		szKorCode[1] = (char)achKor[1];
	}

	return bRet;
}

BOOL CCharacterMapper2::IsEncodedText(LPCSTR cszCode)
{
	BYTE byTest = cszCode[0];

	if ( 0x85 == byTest || 0x86 == byTest || 0xEB == byTest || 0xEC == byTest || (0xEF <= byTest && byTest <= 0xF9) )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CCharacterMapper2::StoreEncodedText(LPSTR __inout szText)
{
	int nLength, i;

	m_cStringArray.clear();

	string strResult;

	string strEncodedKorean;
	bool bIsKorean = false;

	nLength = lstrlenA(szText);

	for (i=0; i<nLength; i++)
	{
		if (IsEncodedText(szText+i))
		{
			// EncodeKor2 �ѱ�
			if (bIsKorean == false)
			{
				// ù��° �ѱ� ����
				bIsKorean = true;
			}
			// ���ڵ��� �ѱ��� �����Ѵ�
			strEncodedKorean+=szText[i];
			i++;
			if (i == nLength) break;
			strEncodedKorean+=szText[i];
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

					DecodeJ2K(szEncode, szDecode);

					strDecodedKorean += szDecode;
				}

				// ������ �� ��ũ
				m_cStringArray.push_back(strDecodedKorean);
				strResult+=ENCODE_MARKER_STRING;
				strEncodedKorean.clear();
			}
			if (IsDBCSLeadByteEx(932, (BYTE)szText[i]))
			{
				strResult+= szText[i];
				i++;
			}
			if (i == nLength) break;
			strResult+=szText[i];
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

			DecodeJ2K(szEncode, szDecode);

			strDecodedKorean += szDecode;
		}

		m_cStringArray.push_back(strDecodedKorean);
		strResult+=ENCODE_MARKER_STRING;
	}

	/*
	// ���ڵ��� �ѱ� �̿ܿ� ������ ���� ������ ���� ����
	nLength = strResult.size();
	for(i=0; i<nLength; i++)
	{
		if ((BYTE)strResult[i] >= 0x80)
			break;
	}
	if (i == nLength)
		return FALSE;
	*/

	lstrcpyA(szText, strResult.c_str());

	return TRUE;
}
BOOL CCharacterMapper2::RestoreDecodedText(LPSTR __inout szText)
{
	if (m_cStringArray.empty())
	{
		// ���ڵ� ġȯ�� ���� ����
		return TRUE;
	}

	int idxStringId = 0;
	int i, nLength = lstrlenA(szText);
	string strResult;

	for (i=0; i<nLength; i++)
	{
		if ( (nLength - i >= ENCODE_MARKER_LENGTH) && 
			(CompareStringA(LOCALE_NEUTRAL, NULL, szText+i, ENCODE_MARKER_LENGTH, ENCODE_MARKER_STRING, ENCODE_MARKER_LENGTH) == CSTR_EQUAL) )
		{
			// ���� ��Ŀ�� ã������ ġȯ
			strResult+=m_cStringArray[idxStringId];
			idxStringId++;
			i+=ENCODE_MARKER_LENGTH;
			if (i == nLength)
				break;
		}
		strResult+=szText[i];
	}
/*
	FILE *fp;
	fp=fopen("d:\\charmap.ko.txt", "a");
	fprintf(fp, "%s\n", strResult.c_str());
	fclose(fp);
*/
	lstrcpyA(szText, strResult.c_str());

	return TRUE;
}

BOOL CCharacterMapper2::GetUserDefinedJpnCode(const BYTE *cszKorCode, BYTE *szJpnCode)
{
	BOOL bRet = FALSE;
	BYTE byLead, byTrail;
	WORD wKorCode = *(WORD *)cszKorCode;

	EnterCriticalSection(m_pCS);

	for (byLead = 0; byLead < 2; byLead++)
	{
		for (byTrail = 0; byTrail < 188; byTrail ++)
		{
			if (m_awCP932_Lead8586[byLead][byTrail] == wKorCode)
			{
				bRet = TRUE;
				break;
			}
			else if (m_awCP932_Lead8586[byLead][byTrail] == NULL)
			{
				m_awCP932_Lead8586[byLead][byTrail] = wKorCode;
				bRet = TRUE;
				break;
			}
		}
		if (bRet)
			break;
	}
	LeaveCriticalSection(m_pCS);

	if (bRet)
	{
		byLead += 0x85;
		byTrail += 0x40;
		if (byTrail >= 0x7F)
			byTrail++;

		szJpnCode[0] = byLead;
		szJpnCode[1] = byTrail;
	}
	return bRet;

}

BOOL CCharacterMapper2::GetUserDefinedKorCode(const BYTE *cszJpnCode, BYTE *szKorCode)
{
	BYTE byLead, byTrail;
	WORD wKorCode;

	byLead = cszJpnCode[0] - 0x85;
	byTrail = cszJpnCode[1] - 0x40;
	if (cszJpnCode[1] > 0x7F)
		byTrail--;

	wKorCode = m_awCP932_Lead8586[byLead][byTrail];

	if (wKorCode)
	{
		*(WORD *)szKorCode = wKorCode;
		return TRUE;
	}

	return FALSE;
}