// DumpDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "DumpText.h"
#include "DumpDlg.h"
#include "AboutDlg.h"
#include "afxdialogex.h"
#include "OpQueue.h"
#include "MyEdit.h"

#define  IDT_TIMER  WM_USER + 376

using namespace std;

// CDumpDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDumpDlg, CDialogEx)

CDumpDlg::CDumpDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDumpDlg::IDD, pParent)
{
	m_bInitialized=false;
}

CDumpDlg::~CDumpDlg()
{
}

void CDumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_cEdit);
}


BEGIN_MESSAGE_MAP(CDumpDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_COMMAND(ID_M_About, &CDumpDlg::OnMAbout)
	ON_COMMAND(ID_M_AlwaysTop, &CDumpDlg::OnMAlwaystop)
	ON_COMMAND(ID_M_Background, &CDumpDlg::OnMBackground)
	ON_COMMAND(ID_M_Clear, &CDumpDlg::OnMClear)
	ON_COMMAND(ID_M_FileSave, &CDumpDlg::OnMFilesave)
	ON_COMMAND(ID_M_Font, &CDumpDlg::OnMFont)
	ON_COMMAND(ID_M_JHex, &CDumpDlg::OnMJhex)
	ON_COMMAND(ID_M_JHexUni, &CDumpDlg::OnMJhexuni)
	ON_COMMAND(ID_M_JText, &CDumpDlg::OnMJtext)
	ON_COMMAND(ID_M_KHex, &CDumpDlg::OnMKhex)
	ON_COMMAND(ID_M_KHexUni, &CDumpDlg::OnMKhexuni)
	ON_COMMAND(ID_M_KText, &CDumpDlg::OnMKtext)
	ON_COMMAND(ID_M_LoadDefault, &CDumpDlg::OnMLoaddefault)
	ON_COMMAND(ID_M_NewLine, &CDumpDlg::OnMNewline)
	ON_COMMAND(ID_M_SaveDefault, &CDumpDlg::OnMSavedefault)
END_MESSAGE_MAP()


// CDumpDlg �޽��� ó�����Դϴ�.


BOOL CDumpDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	m_pMenu = GetMenu();

	m_strRegDefault = L"SOFTWARE\\AralGood\\DumpText Option";
	{
		wchar_t szFileName[2048]=L"";
		int nLen = ::GetModuleFileNameW(NULL,szFileName,2048);
		while(nLen>=0&&szFileName[nLen]!=L'\\'){
			nLen--;
		}
		m_strRegMy = m_strRegDefault;
		m_strRegMy += (szFileName+nLen);
	}

	m_cEdit.LimitText();
	
	m_fgcolor_default = m_cEdit.GetDC()->GetTextColor();
	m_bgcolor_default = m_cEdit.GetDC()->GetBkColor();
	m_cEdit.GetFont()->GetLogFont(&m_font_default);

	m_cEdit.SetReadOnly(TRUE);

	m_cEdit.SetBackColor(m_bgcolor_default);

	getOption(false);

	m_bInitialized=true;

	applyOption(DOPT_ALL);

	m_cEdit.EnableScrollBar(SB_VERT,ESB_ENABLE_BOTH);
	m_cEdit.ShowScrollBar(SB_VERT);

	m_pTimer = CDumpDlg::SetTimer(IDT_TIMER,100,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}



void CDumpDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if(nIDEvent==m_pTimer){
		//��ɾ� �۾�!
		wstring strBuffer;

		bool isExit=false;
		
		while(!isOpQueueEmpty()){
			strBuffer.clear();
			bool bAdded=false;

			OpData sOP= popOpQueue();
			if(sOP.type==0x0101){
				KillTimer(nIDEvent);//�ϴ� ��ġ�� ���� -_-
				isExit=true;
				break;
			}
			else if(sOP.type==0x0200){//����
				if(this->m_bShowJText){
					strBuffer+=sOP.data;
					strBuffer+=L"\r\n";
					bAdded=true;
				}
				if(this->m_bShowJHex){
					strBuffer+=this->getHexString(sOP.data,932);
					strBuffer+=L"\r\n";
					bAdded=true;
				}
				if(this->m_bShowJHexU){
					strBuffer+=this->getUniHexString(sOP.data);
					strBuffer+=L"\r\n";
					bAdded=true;
				}
			}
			else if(sOP.type==0x0210){//������
				if(this->m_bShowKText){
					strBuffer+=sOP.data;
					strBuffer+=L"\r\n";
					bAdded=true;
				}
				if(this->m_bShowKHex){
					strBuffer+=this->getHexString(sOP.data,949);
					strBuffer+=L"\r\n";
					bAdded=true;
				}
				if(this->m_bShowKHexU){
					strBuffer+=this->getUniHexString(sOP.data);
					strBuffer+=L"\r\n";
					bAdded=true;
				}
			}

			if(bAdded){
				if(this->m_bNewLine&&((sOP.type==0x0200&&!m_bShowKText&&!m_bShowKHex&&!m_bShowKHexU)||(sOP.type==0x0210))){
					strBuffer+=L"\r\n";
				}
				m_cEdit.addText(strBuffer);
			}
		}

		if(isExit){
			OpData offOP;
			offOP.type = 0x0102;

			pushOpQueue(offOP,false);
			pushOpQueue(offOP,false);

			this->OnOK();
		}
		else{
			m_cEdit.flushText();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CDumpDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if(m_bInitialized){
		m_cEdit.MoveWindow(0,0,cx,cy);

		m_nWidth=cx;
		m_nHeight=cy;

		setOption(false,DOPT_SIZE);
	}
	else{
		m_nWidth_default=cx;
		m_nHeight_default=cy;
	}
}

void CDumpDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if(m_bInitialized){
		m_nTop=y;
		m_nLeft=x;

		setOption(false,DOPT_SIZE);
	}
	else{
		m_nTop_default=y;
		m_nLeft_default=x;
	}
}

void CDumpDlg::OnMAbout()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CAboutDlg cDlg;
	cDlg.DoModal();
}

void CDumpDlg::OnMAlwaystop()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bAlwaysTop=!m_bAlwaysTop;

	applyOption(DOPT_TOP);
	setOption(false,DOPT_TOP);
}


void CDumpDlg::OnMBackground()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CColorDialog cDlg(this->m_bgcolor,CC_FULLOPEN);
	if(cDlg.DoModal()==IDOK){
		this->m_bgcolor = cDlg.GetColor();

		applyOption(DOPT_BG);
		setOption(false,DOPT_BG);

	}
}


void CDumpDlg::OnMClear()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_cEdit.clearText();
}


void CDumpDlg::OnMFilesave()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CFileDialog cDlg=CFileDialog(FALSE,L"txt",L"DumpText",OFN_OVERWRITEPROMPT,L"�ؽ�Ʈ����(*.txt)|*.txt|�������(*.*)|*.*||");
	if(cDlg.DoModal()==IDOK){
		int nLen = m_cEdit.GetWindowTextLengthW();
		vector<wchar_t> buffer;
		buffer.resize(nLen+2);
		buffer[0]=0xFEFF;
		m_cEdit.GetWindowTextW(buffer.data()+1,nLen+1);
		buffer.pop_back();

		CFile cFile;

		if(cFile.Open(cDlg.GetPathName().GetString(),CFile::modeCreate|CFile::shareExclusive|CFile::modeWrite)!=0){
			cFile.Write(buffer.data(),buffer.size()*sizeof(wchar_t));
			cFile.Close();
		}
		else{
			AfxMessageBox(L"������ �����ϴµ� �����߽��ϴ�");
		}
	}

	
}


void CDumpDlg::OnMFont()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CFontDialog cDlg;
	cDlg.m_cf.Flags|=CF_NOVERTFONTS|CF_EFFECTS|CF_INITTOLOGFONTSTRUCT;
	cDlg.m_cf.lpLogFont=&m_font;
	cDlg.m_cf.rgbColors=m_fgcolor;

	if(cDlg.DoModal() == IDOK){
		m_fgcolor = cDlg.m_cf.rgbColors;

		applyOption(DOPT_FONT);
		setOption(false,DOPT_FONT);
	}
}

void CDumpDlg::OnMJhex()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bShowJHex=!m_bShowJHex;

	applyOption(DOPT_JPN);
	setOption(false,DOPT_JPN);
}

void CDumpDlg::OnMJhexuni()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bShowJHexU=!m_bShowJHexU;

	applyOption(DOPT_JPN);
	setOption(false,DOPT_JPN);
}

void CDumpDlg::OnMJtext()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bShowJText=!m_bShowJText;

	applyOption(DOPT_JPN);
	setOption(false,DOPT_JPN);
}

void CDumpDlg::OnMKhex()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bShowKHex=!m_bShowKHex;

	applyOption(DOPT_KOR);
	setOption(false,DOPT_KOR);
}


void CDumpDlg::OnMKhexuni()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bShowKHexU=!m_bShowKHexU;

	applyOption(DOPT_KOR);
	setOption(false,DOPT_KOR);
}

void CDumpDlg::OnMKtext()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bShowKText=!m_bShowKText;

	applyOption(DOPT_KOR);
	setOption(false,DOPT_KOR);
}

void CDumpDlg::OnMLoaddefault()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	getOption(true);
	setOption(false,DOPT_ALL);
	applyOption(DOPT_ALL);
}

void CDumpDlg::OnMNewline()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_bNewLine=!m_bNewLine;

	applyOption(DOPT_NEWLN);
	setOption(false,DOPT_NEWLN);
}

void CDumpDlg::OnMSavedefault()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	setOption(true,DOPT_ALL);
}

void CDumpDlg::CheckSimple(UINT Item,bool chk){
	if(chk){
		m_pMenu->CheckMenuItem(Item,MF_CHECKED);
	}
	else{
		m_pMenu->CheckMenuItem(Item,MF_UNCHECKED);
	}
}

void CDumpDlg::applyOption(DWORD dOpt){
	if(dOpt&DOPT_JPN){
		CheckSimple(ID_M_JText,m_bShowJText);
		CheckSimple(ID_M_JHex,m_bShowJHex);
		CheckSimple(ID_M_JHexUni,m_bShowJHexU);
	}

	if(dOpt&DOPT_KOR){
		CheckSimple(ID_M_KText,m_bShowKText);
		CheckSimple(ID_M_KHex,m_bShowKHex);
		CheckSimple(ID_M_KHexUni,m_bShowKHexU);
	}

	if(dOpt&DOPT_SIZE){
		m_cEdit.MoveWindow(0,0,m_nWidth,m_nHeight);
		this->SetWindowPos(NULL,m_nLeft,m_nTop,m_nWidth,m_nHeight,SWP_NOZORDER);
	}

	if(dOpt&DOPT_NEWLN){
		CheckSimple(ID_M_NewLine,m_bNewLine);
	}

	if(dOpt&DOPT_TOP){
		if(m_bAlwaysTop){
			this->SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		else{
			this->SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		CheckSimple(ID_M_AlwaysTop,m_bAlwaysTop);
	}

	if(dOpt&DOPT_FONT){
		m_cEdit.SetTextColor(m_fgcolor);

		m_cFont.DeleteObject();
		m_cFont.CreateFontIndirectW(&m_font);
		m_cEdit.SetFont(&m_cFont);
	}

	if(dOpt&DOPT_BG){
		m_cEdit.SetBackColor(m_bgcolor);
	}
}

void CDumpDlg::getOption(bool isDefault){
	DWORD data;
	wchar_t szData[2048];
	wchar_t * nextToken=NULL;

	wstring strRegPath;
	CRegKey cReg;
	if(!isDefault){
		strRegPath = m_strRegMy;

		if(cReg.Open(HKEY_CURRENT_USER,strRegPath.c_str(),KEY_READ) != ERROR_SUCCESS){
			getOption(true);
			setOption(false,DOPT_ALL);
			return;
		}
	}//end !isDefault
	else{
		strRegPath = m_strRegDefault;

		if(cReg.Open(HKEY_CURRENT_USER,strRegPath.c_str(),KEY_READ) != ERROR_SUCCESS){
			resetOption();
			getOption(true);
			return;
		}

	}//end isDefault

	//DOPT_JPN
	cReg.QueryDWORDValue(L"DOPT_JPN",data);
	m_bShowJText = (data&0x1)!=0;
	m_bShowJHex	 = (data&0x2)!=0;
	m_bShowJHexU = (data&0x4)!=0;

	//DOPT_KOR
	cReg.QueryDWORDValue(L"DOPT_KOR",data);
	m_bShowKText = (data&0x1)!=0;
	m_bShowKHex  = (data&0x2)!=0;
	m_bShowKHexU = (data&0x4)!=0;

	//DOPT_SIZE
	data=2048;
	nextToken=NULL;
	cReg.QueryStringValue(L"DOPT_SIZE",szData,&data);
	m_nTop    = _wtoi(wcstok_s(szData,L"|",&nextToken)) - 50;
	m_nLeft   = _wtoi(wcstok_s(NULL,L"|",&nextToken)) - 8;
	m_nWidth  = _wtoi(wcstok_s(NULL,L"|",&nextToken)) + 16;
	m_nHeight = _wtoi(wcstok_s(NULL,L"|",&nextToken)) + 58;

	//DOPT_NEWLN
	cReg.QueryDWORDValue(L"DOPT_NEWLN",data);
	m_bNewLine = data!=FALSE;

	//DOPT_TOP
	cReg.QueryDWORDValue(L"DOPT_TOP",data);
	m_bAlwaysTop = data!=FALSE;

	//DOPT_FONT
	data = sizeof(LOGFONT);
	cReg.QueryBinaryValue(L"DOPT_FONT_BASE",&this->m_font,&data);
	cReg.QueryDWORDValue(L"DOPT_FONT_COLOR",data);
	this->m_fgcolor = data;

	//DOPT_BG
	cReg.QueryDWORDValue(L"DOPT_BG",data);
	this->m_bgcolor = data;

	cReg.Close();
}

void CDumpDlg::setOption(bool isDefault,DWORD dOpt){
	DWORD data=0;
	wchar_t szData[2048]=L"";

	wstring strRegPath;
	CRegKey cReg;
	if(!isDefault){
		strRegPath = m_strRegMy;
	}
	else{
		strRegPath = m_strRegDefault;
	}

	if(cReg.Create(HKEY_CURRENT_USER,strRegPath.c_str()) != ERROR_SUCCESS){
		AfxMessageBox(L"�ɰ��� ����!\r\n������Ʈ�� �ʱ�ȭ�� �����߽��ϴ�.\r\n�����ڿ��� ���� �ʿ��մϴ�",MB_ICONSTOP);
		return;
	}

	//DOPT_JPN
	data=0;
	data|=(DWORD)m_bShowJText;
	data|=(DWORD)m_bShowJHex<<1;
	data|=(DWORD)m_bShowJHexU<<2;
	cReg.SetDWORDValue(L"DOPT_JPN",data);

	//DOPT_KOR
	data=0;
	data|=(DWORD)m_bShowKText;
	data|=(DWORD)m_bShowKHex<<1;
	data|=(DWORD)m_bShowKHexU<<2;
	cReg.SetDWORDValue(L"DOPT_KOR",data);

	//DOPT_SIZE
	data = swprintf_s(szData,2048,L"%d|%d|%d|%d",m_nTop,m_nLeft,m_nWidth,m_nHeight);
	cReg.SetStringValue(L"DOPT_SIZE",szData);

	//DOPT_NEWLN
	cReg.SetDWORDValue(L"DOPT_NEWLN",m_bNewLine);

	//DOPT_TOP
	cReg.SetDWORDValue(L"DOPT_TOP",m_bAlwaysTop);

	//DOPT_FONT
	cReg.SetBinaryValue(L"DOPT_FONT_BASE",&this->m_font,sizeof(LOGFONT));
	cReg.SetDWORDValue(L"DOPT_FONT_COLOR",this->m_fgcolor);

	//DOPT_BG
	cReg.SetDWORDValue(L"DOPT_BG",this->m_bgcolor);

	cReg.Close();
}

void CDumpDlg::resetOption(){
	DWORD data=0;
	wchar_t szData[2048]=L"";

	wstring strRegPath=m_strRegDefault;
	CRegKey cReg;

	if(cReg.Create(HKEY_CURRENT_USER,strRegPath.c_str()) != ERROR_SUCCESS){
		AfxMessageBox(L"�ɰ��� ����!\r\n������Ʈ�� �ʱ�ȭ�� �����߽��ϴ�.\r\n�����ڿ��� ���� �ʿ��մϴ�",MB_ICONSTOP);
		return;
	}

	//DOPT_JPN
	data=0;
	data|=1;
	data|=0<<1;
	data|=0<<2;
	cReg.SetDWORDValue(L"DOPT_JPN",data);

	//DOPT_KOR
	data=0;
	data|=0;
	data|=0<<1;
	data|=0<<2;
	cReg.SetDWORDValue(L"DOPT_KOR",data);

	//DOPT_SIZE
	data = swprintf_s(szData,2048,L"%d|%d|%d|%d",m_nTop_default,m_nLeft_default,m_nWidth_default,m_nHeight_default);
	//data = swprintf_s(szData,2048,L"%d|%d|%d|%d",200,200,200,200);
	cReg.SetStringValue(L"DOPT_SIZE",szData);

	//DOPT_NEWLN
	cReg.SetDWORDValue(L"DOPT_NEWLN",true);

	//DOPT_TOP
	cReg.SetDWORDValue(L"DOPT_TOP",false);

	//DOPT_FONT
	cReg.SetBinaryValue(L"DOPT_FONT_BASE",&this->m_font_default,sizeof(LOGFONT));
	cReg.SetDWORDValue(L"DOPT_FONT_COLOR",this->m_fgcolor_default);

	//DOPT_BG
	cReg.SetDWORDValue(L"DOPT_BG",this->m_bgcolor_default);

	cReg.Close();
}

wstring CDumpDlg::getHexString(wstring strData,int nLocale){
	wchar_t buff[6]=L"";

	wstring strRet;
	strRet.reserve(strData.length()*3);

	vector<char> strSSrc;
	strSSrc.resize(strData.size()*2+1);
	int nLen=MyWideCharToMultiByte(nLocale,NULL,strData.c_str(),-1,strSSrc.data(),strSSrc.size(),NULL,NULL);
	nLen--;

	for(int i=0;i<nLen;i++){
		if(IsDBCSLeadByteEx(nLocale,strSSrc[i])){
			swprintf_s(buff,6,L"%02X%02X ",(BYTE)strSSrc[i],(BYTE)strSSrc[i+1]);
			i++;
		}
		else{
			swprintf_s(buff,6,L"%02X ",(BYTE)strSSrc[i]);
		}
		strRet+=buff;
	}
	if(!strRet.empty())strRet.pop_back();

	return strRet;
}

wstring CDumpDlg::getUniHexString(wstring strData){
	wchar_t buff[6]=L"";

	wstring strRet;
	strRet.reserve(strData.length()*5);

	for(UINT i=0;i<strData.length();i++){
		swprintf_s(buff,6,L"%04X ",(unsigned short)strData[i]);
		strRet+=buff;
	}
	if(!strRet.empty())strRet.pop_back();

	return strRet;
}
