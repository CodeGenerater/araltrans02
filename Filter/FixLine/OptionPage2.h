#pragma once
#include "Dlg_AddFunc.h"
#include "tstring.h"
#include <vector>

using namespace std;

// COptionPage2 ��ȭ �����Դϴ�.

class COptionPage2 : public CPropertyPage
{
	DECLARE_DYNAMIC(COptionPage2)

public:
	COptionPage2();
	virtual ~COptionPage2();
	virtual void OnOK();
	afx_msg void OnBnClickedTab2Button3();

	CListCtrl m_ctrFuncList;

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_Page2 };

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

private:
	CDlg_AddFunc	m_cAddFunc;
	
public:
	vector<vector<FLStringElement2>> m_FuncPool;  //���� ��������
	CStringArray m_FuncPool2;  //���� �ϼ���
	CArray<int> m_FuncPool3;  //�Լ� Ư�� ����
	CArray<bool> m_FuncPool4; //Trim Ư�� ����
	
public:
	afx_msg void OnBnClickedTab2Button5();
public:
	afx_msg void OnBnClickedTab2Button4();
	int m_nSelect;
public:
	int m_nItemN;
public:
	afx_msg void OnNMClickTab2Funclist(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkTab2Funclist(NMHDR *pNMHDR, LRESULT *pResult);
public:
	bool m_bOK;
};
