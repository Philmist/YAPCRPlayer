#pragma once

#include "afxwin.h"

#include "BrowserDlg.h"
#include "CustomEdit.h"

// CSearchDlg ダイアログ

class CSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CSearchDlg)

public:
	CSearchDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CSearchDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SEARCH };
#endif

	sl::DialogConfig::Search cfg_;

	void setBrowserDlg(CBrowserDlg* browser) { this_.browser = browser; }
	void setSearch(const std::wstring& search);
	void setFont(const std::wstring& name, int point);

protected:
	void SizeHelper();
	void GetCalcRect(LPRECT pRect);
	bool Search(bool next);

protected:
	CFont font_;
	CStatusBar status_;

	struct This {
		This()
			: height(21)
			, browser(NULL)
			, pos(0)
		{}

		int height;
		CBrowserDlg* browser;
		CCustomDHtmlDialog::BookmarkMap bookmark;
		std::wstring search;//検索文字列
		std::wstring url;//スレッドURL
		size_t pos;//ブラウザレス数
	} this_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnExtract();
	afx_msg void OnBnClickedBtnPrev();
	afx_msg void OnBnClickedBtnNext();

	CCustomEdit m_xcEdit;
	CButton m_xcBtnExtract;
	CButton m_xcBtnPrev;
	CButton m_xcBtnNext;
};
