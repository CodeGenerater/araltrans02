// INIParser.cpp: implementation of the CINIParser class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#include "../StdAfx.h"
#include "INIParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CINIParser::CINIParser()
	: m_bIsUnicode(FALSE)
{

}

CINIParser::~CINIParser()
{
	Clear();
}

BOOL CINIParser::Load(CString path)
{
	// ���Ͽ��� ������ �б�
	FILE* pf = _tfopen(path, _T("rb") );
	if(NULL==pf) return FALSE;
	
	fseek(pf, 0, SEEK_END);
	long lSize = ftell(pf);
	fseek(pf, 0, SEEK_SET);
	if( lSize < 0 )
	{
		fclose(pf);
		return FALSE;
	}
	
	BYTE* buf = new BYTE[lSize+2];
	fread(buf, lSize, 1, pf);
	fclose(pf);
	buf[lSize] = 0;
	buf[lSize+1] = 0;

	// INI �ҽ� ������
	CString strINISource = _T("");
	
	
	//	- ���ڵ� �Ǻ� -
	// 	00 00 FE FF		UTF-32, big-endian 
	// 	FF FE 00 00		UTF-32, little-endian 
	// 	FE FF			UTF-16, big-endian 
	// 	FF FE			UTF-16, little-endian 
	// 	EF BB BF		UTF-8 
	if( buf[0]==0xFF && buf[1]==0xFE ) m_bIsUnicode = TRUE;
	else m_bIsUnicode = FALSE;

#ifdef _UNICODE
	// ANSI�� �����ڵ�� ����Ʈ
	if( FALSE == m_bIsUnicode )
	{
		wchar_t* buf2 = new wchar_t[lSize];
		::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buf, -1, buf2, lSize);
		strINISource = buf2;
		delete [] buf2;
	}
	// �ƴϸ� �׳� ����
	else
	{
		strINISource = (LPCWSTR)(buf+2);
	}
#else
	// �����ڵ�� ANSI�� ����Ʈ
	if( m_bIsUnicode )
	{
		char* buf2 = new char[lSize];
		::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(buf+2), -1, buf2, lSize, NULL, NULL);
		strINISource = buf2;
		delete [] buf2;
	}
	// �ƴϸ� �׳� ����
	else
	{
		strINISource = (LPCSTR)buf;
	}
#endif

	// ���� ����
	delete [] buf;

	// �Ľ�
	return Parse( strINISource );
}

BOOL CINIParser::Parse(CString strINISource)
{
	Clear();

	CININode* pCurNode = NULL;
	
	TCHAR seps[] = _T("\r\n");
	TCHAR* token = _tcstok( (LPTSTR)(LPCTSTR)strINISource, seps );
	
	while( token != NULL )
	{
		CString strLine = token;		
		strLine.TrimLeft();
		strLine.TrimRight();
		
		if( !strLine.IsEmpty() && strLine[0]==_T(';') )
		{
			// �ּ�������
		}
		else if( !strLine.IsEmpty() && strLine[0]==_T('[') && strLine[strLine.GetLength()-1]==_T(']') )
		{
			CString strNodeName = strLine.Mid( 1, strLine.GetLength()-2 );
			strNodeName.TrimLeft();
			strNodeName.TrimRight();
			pCurNode = CreateNode( strNodeName );
		}
		else if( strLine.Find(_T('=')) > 0 && NULL != pCurNode)
		{
			int nIdx = strLine.Find(_T('='));

			CString strAttrName = strLine.Left(nIdx);
			strAttrName.TrimLeft();
			strAttrName.TrimRight();

			CString strAttrValue = strLine.Mid(nIdx+1);
			strAttrValue.TrimLeft();
			strAttrValue.TrimRight();

			pCurNode->SetAttribute(strAttrName, strAttrValue);
		}

		token = _tcstok( NULL, seps );
	}

	return TRUE;
}

CString CINIParser::ToString()
{
	// INI �ҽ� ������
	CString strINISource = _T("");

	POSITION pos = m_listININode.GetHeadPosition();

	while(pos!=NULL)
	{
		CININode* pNode = m_listININode.GetNext(pos);
		strINISource += pNode->ToString();
		strINISource += _T("\r\n");
	}

	return strINISource;
}

BOOL CINIParser::Save(CString path)
{
	// INI �ҽ� ������
	CString strINISource = ToString();
	int	len = strINISource.GetLength();

	// ���� ����
	FILE* pf = _tfopen(path, _T("wb"));
	if(NULL==pf) return FALSE;
		
#ifdef _UNICODE

	// ANSI�ڵ� �����̶��
	if( FALSE == m_bIsUnicode )
	{
		char* ansibuf = new char[len+10];
		::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)strINISource, -1, ansibuf, len+10, NULL, NULL );
		fwrite(ansibuf, len, 1, pf);
		delete [] ansibuf;
	}
	// �ƴϸ� �׳� ����
	else
	{
		WORD dwPrefix = 0xFEFF;
		fwrite(&dwPrefix, 2, 1, pf);
		fwrite((LPCWSTR)strINISource, len*2, 1, pf);
	}

#else

	// �����ڵ� �����̶��
	if( m_bIsUnicode )
	{
		wchar_t* widebuf = new wchar_t[len+10];
		widebuf[0] = 0xFEFF;
		::MultiByteToWideChar( CP_ACP, 0, (LPCSTR)strINISource, -1, widebuf+1, len+10 );
		len = wcslen(widebuf+1);
		fwrite(widebuf, 2 + len*2, 1, pf);
		delete [] widebuf;
	}
	// �ƴϸ� �׳� ����
	else
	{
		fwrite( (LPCSTR)strINISource, len, 1, pf);
	}

#endif

	fclose(pf);

	return TRUE;
}

void CINIParser::Clear()
{
	POSITION pos = m_listININode.GetHeadPosition();

	while(pos!=NULL)
	{
		CININode* pNode = m_listININode.GetNext(pos);
		delete pNode;
	}

	m_listININode.RemoveAll();
}

// ���ο� ��带 �����Ͽ� �ν��Ͻ� ������ ��ȯ
CININode* CINIParser::CreateNode(CString name)
{
	if( FindNode(name) ) return NULL;
	
	CININode* pNode = new CININode();
	
	pNode->SetName(name);
	
	m_listININode.AddTail(pNode);
	
	return pNode;
}

// ��� ã��
CININode* CINIParser::FindNode(CString name)
{
	CININode* pRetNode = NULL;
	
	POSITION pos = m_listININode.GetHeadPosition();

	while(pos!=NULL)
	{
		CININode* pNode = m_listININode.GetNext(pos);
		if( pNode->GetName() == name )
		{
			pRetNode = pNode;
			break;
		}
	}

	return pRetNode;
}

// ��� ����
void CINIParser::DeleteNode(CString name)
{
	POSITION pos = m_listININode.GetHeadPosition();

	while(pos!=NULL)
	{
		POSITION curpos = pos;
		CININode* pNode = m_listININode.GetNext(pos);
		if( pNode->GetName() == name )
		{
			delete pNode;
			m_listININode.RemoveAt(curpos);
			return;
		}
	}

}

CININode* CINIParser::GetNodeAt(int index)
{
	POSITION pos = m_listININode.FindIndex(index);
	if(pos==NULL) return NULL;
	return m_listININode.GetAt(pos);
}

int CINIParser::GetNodeCount()
{
	return m_listININode.GetCount();
}
