#pragma once
#include "afxwin.h"


// COpenFileDlg ダイアログ

class COpenFileDlg : public CDialog
{
	DECLARE_DYNAMIC(COpenFileDlg)

public:
	COpenFileDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~COpenFileDlg();

// ダイアログ データ
	enum { IDD = IDD_OPENFILE };

	sl::DialogConfig::File cfg_;
	std::wstring path_;

protected:
	CStatusBar status_;

	void SizeHelper();
	void GetCalcRect(LPRECT pRect);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnPath();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CSimpleEdit m_xcEditPath;
	CButton m_xcBtnPath;
	CButton m_xcBtnOk;
	CButton m_xcBtnCancel;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
};
