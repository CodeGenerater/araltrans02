#pragma once

#include <list>

using namespace std;

#define MAX_TEXT_LENGTH 1024

enum {
	TRANSCOMMAND_NOP = 0,
	TRANSCOMMAND_PTRCHEAT = 1,
	TRANSCOMMAND_OVERWRITE = 2,
	TRANSCOMMAND_SOW = 3,
	TRANSCOMMAND_SMSTR = 4
};

class COptionNode;
class CTransScriptParser;

//////////////////////////////////////////////////////////////////////////
// CTransCommand : CTransCommand �⺻ Ŭ����
class CTransCommand
{
public:
	friend class CHookPoint;

protected:

	int		m_nTransMethod;
	BOOL	m_bTrans;
	BOOL	m_bUnicode;
	BOOL	m_bUnicode8;
	BOOL	m_bClipKor;
	BOOL	m_bClipJpn;
	BOOL	m_bRemoveSpace;
	BOOL	m_bTwoByte;
	BOOL	m_bOneByte;
	BOOL	m_bSafe;
	BOOL	m_bAddNull;
	BOOL	m_bAdjustLen;
	BOOL	m_bEndPos;
	BOOL	m_bLenEnd;
	BOOL	m_bTjsStr;
	BOOL	m_bKiriName;
	BOOL	m_bPtrBack;
	BOOL	m_bDuplicated;
	CString m_strLenPos;
	int		m_nLenPos;
	int		m_nBackUpSize;

	CString m_strArgScript;

public:
	void	*m_pArgText;
	void	*m_pPrevArgText;
	BYTE	m_pOrigTextBuf[MAX_TEXT_LENGTH*2];
	BYTE	m_byReserved[0x100];					// LEN()�� ���� ��������
	BYTE	m_pTransTextBuf[MAX_TEXT_LENGTH*2];
	BYTE	m_pBackUpBuf[MAX_TEXT_LENGTH*2 + 0x10];					// ������ ������ ���� ��������
	BYTE	m_pBackUpLength[0x10];					// LEN ������ ���� ��������

public:
	static CTransCommand *CreateTransCommand(CString &strArgScript, COptionNode *pNode);	// CTransCommand ������ static �Լ�

protected:
	CTransCommand() : 
		m_nTransMethod(TRANSCOMMAND_NOP), m_bTrans(FALSE), m_bUnicode(FALSE), m_bUnicode8(FALSE), m_bClipKor(FALSE), 
		m_bClipJpn(FALSE), m_bRemoveSpace(FALSE), m_bTwoByte(FALSE), m_bOneByte(FALSE), m_bSafe(FALSE), m_bAddNull(FALSE), m_bEndPos(FALSE),
		m_bAdjustLen(FALSE), m_bLenEnd(FALSE), m_bTjsStr(FALSE), m_nLenPos(0), m_bKiriName(FALSE), m_bPtrBack(FALSE), m_bDuplicated(FALSE), m_nBackUpSize(0),
		m_pArgText(NULL), m_pPrevArgText(NULL)
	{
		m_strArgScript.Empty();
		ZeroMemory(m_pOrigTextBuf, MAX_TEXT_LENGTH*2);
		ZeroMemory(m_byReserved, 0x100);
		//ZeroMemory(m_pTransTextBuf, MAX_TEXT_LENGTH*2);
		ZeroMemory(m_pBackUpBuf, MAX_TEXT_LENGTH*2 + 0x10);
		ZeroMemory(m_pBackUpLength, 0x10);
	}
	~CTransCommand() { RestoreBackup(); }

protected:
	// Set functions //
	void SetArgScript(LPCTSTR strScript){ m_strArgScript = strScript; };

public:
	//void SetDistFromEBP(int nDistFromESP){ m_nDistFromESP = nDistFromESP; };
	//void SetTransMethod(int nMethod){ m_nTransMethod = nMethod; };
/*	void SetUnicode(BOOL bValue){ m_bUnicode = bValue; };
	void SetClipKor(BOOL bClipKor){ m_bClipKor = bClipKor; };
	void SetClipJpn(BOOL bClipJpn){ m_bClipJpn = bClipJpn; };
	void SetRemoveSpace(BOOL bRemove){ m_bRemoveSpace = bRemove; };
	void SetTwoByte(BOOL bEnable){ m_bTwoByte = bEnable; };
	void SetSafe(BOOL bEnable){ m_bSafe = bEnable; };
*/
	// ���� �ɼ��� �����մϴ�.
	virtual void SetTransOption(CString strTransOption, COptionNode *pNode);
	// Ư�� �ɼ� (OVERWRITE(IGNORE) ��) �� �����մϴ�.
	virtual void SetSpecialOption(CString strSpecialOption, COptionNode *pNode) {}

	// Get functions //

	//int	 GetDistFromEBP(){ return m_nDistFromESP; };
	CString	GetArgScript(){ return m_strArgScript; };
	int  GetTransMethod(){ return m_nTransMethod; };
	BOOL GetTrans(){ return m_bTrans; };
	BOOL GetUnicode(){ return m_bUnicode; };
	BOOL GetUnicode8(){ return m_bUnicode8; };
	BOOL GetClipKor(){ return m_bClipKor; };
	BOOL GetClipJpn(){ return m_bClipJpn; };
	BOOL GetRemoveSpace(){ return m_bRemoveSpace; };
	BOOL GetTwoByte(){ return m_bTwoByte; };
	BOOL GetOneByte(){ return m_bOneByte; };
	BOOL GetSafe(){ return m_bSafe; };
	BOOL GetAddNull(){ return m_bAddNull; };
	BOOL GetEndPos(){ return m_bEndPos; };
	BOOL GetLenEnd(){ return m_bLenEnd; };
	BOOL GetKiriName(){ return m_bKiriName; };
	BOOL GetPtrBack(){ return m_bPtrBack; };
	BOOL GetDuplicated(){ return m_bDuplicated; };

	// Execute functions //

	// (��� ������ �ʿ��� ���) ������� ������ �����մϴ�.
	virtual void RestoreBackup() {}

	// ���� ���ڿ��� ��ġ�� �����ɴϴ�. �߸��� ���ڿ��� ��� NULL.
	virtual void *GetTextPoint(CTransScriptParser *pParser, void *pBackUp, BOOL bPtrBack, void *pBackUpLength);

	// �޾ƿ� ���ڿ��� ���� �� �ִ� ���ڿ����� Ȯ���մϴ�.
	virtual BOOL IsValidTextPoint(void *pArgText, long nSize = 1024*1024*1024);

	// Ȥ�� ���� ���ڿ��� �ٽ� �����Ϸ����� Ȯ���մϴ�.
	virtual BOOL IsDuplicateText(void *pArgText);

	// ���ڿ� ���̸� ������ ���ڿ��� ���̷� �����մϴ�.
	virtual BOOL ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr=NULL, void *pBackUp=NULL);

	// ���ڿ� ���� ����� ������ �մϴ�.
	virtual void BackUpLength(void *pBackUp);
	virtual void SetUpLength(void *pBackUp, int *pnLen, int nBytes, int nTempLen);

	// ������ ����� ������ �մϴ�.
	virtual bool BackUpTextPoint(void *pBackUp, int pMovSize = 1);
	virtual void SetUpTextPoint(void *pBackUp, void *pArgText, void *ppArgText, int pMovSize, int nType = 1);

	// ������ ���ڿ��� �� ���α׷��� �����մϴ�.
	virtual BOOL ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp) { return TRUE; }

};

//////////////////////////////////////////////////////////////////////////
// CTransCommandNOP : NOP�� CTransCommand Ȯ�� Ŭ����
class CTransCommandNOP : public CTransCommand {
public:
	friend class CTransCommand;

protected:
	CTransCommandNOP() : CTransCommand()
		{ m_nTransMethod = TRANSCOMMAND_NOP; }
	~CTransCommandNOP() {}

public:

};

//////////////////////////////////////////////////////////////////////////
// CTransCommandPTRCHEAT : PTRCHEAT ����� ���� CTransCommand Ȯ�� Ŭ����
class CTransCommandPTRCHEAT : public CTransCommand {
public:
	friend class CTransCommand;

protected:
	CTransCommandPTRCHEAT() : CTransCommand()
		{ m_nTransMethod = TRANSCOMMAND_PTRCHEAT; }
	~CTransCommandPTRCHEAT() {}
public:
	virtual BOOL ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp);

	virtual BOOL ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp);

};

//////////////////////////////////////////////////////////////////////////
// CTransCommandOVERWRITE : OVERWRITE ����� ���� CTransCommand Ȯ�� Ŭ����
class CTransCommandOVERWRITE : public CTransCommand {
public:
	friend class CTransCommand;

protected:
	BOOL	m_bAllSameText;
	BOOL	m_bIgnoreBufLen;

protected:
	CTransCommandOVERWRITE() : CTransCommand(), m_bAllSameText(FALSE), m_bIgnoreBufLen(FALSE)
		{ m_nTransMethod = TRANSCOMMAND_OVERWRITE; }
	~CTransCommandOVERWRITE() {}

	// �������� �� ���α׷��� �����ϴ�.
	BOOL OverwriteTextBytes( LPVOID pDest, LPVOID pSrc, void *pBackUp );

	// ���� ���� ��� ������ ������ ã�� ���������� �����ϴ�.
	BOOL SearchTextA(UINT_PTR ptrBegin, LPCSTR cszText, list<LPVOID>* pTextList);
	BOOL SearchTextW(UINT_PTR ptrBegin, LPCWSTR cwszText, list<LPVOID>* pTextList);

public:
	BOOL GetAllSameText(){ return m_bAllSameText; };
	BOOL GetIgnoreBufLen(){ return m_bIgnoreBufLen; };

	virtual void SetTransOption(CString strTransOption, COptionNode *pNode);
	virtual void SetSpecialOption(CString strSpecialOption, COptionNode *pNode);

	virtual BOOL ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp);

};


//////////////////////////////////////////////////////////////////////////
// CTransCommandSOW : SOW ����� ���� CTransCommand Ȯ�� Ŭ����
class CTransCommandSOW : public CTransCommand {
public:
	friend class CTransCommand;

protected:
	BYTE *m_pBackupPoint;
	BYTE *m_pBackupBuffer;
	UINT m_nBackupSize;

	// LEN() ����
	int *m_pnBackupLenPoint;
	int m_nLenBytes;
	int m_nBackupLenPoint;
	bool m_bIsNullIncluded;

protected:
	CTransCommandSOW() : CTransCommand(), m_pBackupPoint(NULL), m_pBackupBuffer(NULL), m_nBackupSize(0),
						m_pnBackupLenPoint(NULL), m_nLenBytes(0), m_nBackupLenPoint(0), m_bIsNullIncluded(false)
		{ m_nTransMethod = TRANSCOMMAND_SOW; }
	~CTransCommandSOW() { RestoreBackup(); }

	// �� ����Ÿ�� ����ϰ� �������� �����ϴ�.
	void DoBackupAndOverwrite(const PBYTE pBackupPoint, UINT nBackupSize);

public:
	virtual void RestoreBackup();
	virtual BOOL ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp);
	virtual BOOL ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp);

};

//////////////////////////////////////////////////////////////////////////
// CTransCommandSMSTR : SMSTR ����� ���� CTransCommand Ȯ�� Ŭ����
class CTransCommandSMSTR : public CTransCommand {
public:
	friend class CTransCommand;

protected:
	BOOL	m_bIgnoreBufLen;
	BOOL	m_bMatchSize;
	BOOL	m_bForcePtr;
	BOOL	m_bNullchar;
	BOOL	m_bReverse;
	BOOL	m_bYuris;
	BOOL	m_bYurisP;
	BOOL	m_bWill;

	int m_YurisOffset;

protected:
	CTransCommandSMSTR() : 
		CTransCommand(), m_bIgnoreBufLen(FALSE), m_bMatchSize(FALSE), m_bForcePtr(FALSE), 
		m_bNullchar(FALSE), m_bReverse(FALSE), m_bYuris(FALSE), m_bYurisP(FALSE), m_bWill(FALSE)
		{ m_nTransMethod = TRANSCOMMAND_SMSTR;}
	~CTransCommandSMSTR() {}

	// �������� ���̿� ���� ����Ʈ ���ڿ��� �����ŵ�ϴ�.
	BOOL ApplySmartString( LPVOID pDest, LPVOID pSrc, void *pBackUp );

public:
	virtual void SetSpecialOption(CString strSpecialOption, COptionNode *pNode);

	virtual void *GetTextPoint(CTransScriptParser *pParser, void *pBackUp, BOOL bPtrBack, void *pBackUpLength);

	virtual BOOL ChangeLength(void *pOrigBasePtr, CTransScriptParser *pParser, void *pNewBasePtr, void *pBackUp);

	virtual BOOL ApplyTranslatedText(void *pArgText, PREGISTER_ENTRY pRegisters, CTransScriptParser *pParser, BOOL bIsDuplicated, void *pBackUp);

};