#pragma once

#include <boost/shared_ptr.hpp>
#include "ExtractBrowserDlg.h"

// CExtractDlg ダイアログ

class CExtractDlg : public CDialog
{
	DECLARE_DYNAMIC(CExtractDlg)

protected:
	static std::vector<boost::shared_ptr<CExtractDlg>> vec_;
	static void erase(CExtractDlg* p);

public:
	static CExtractDlg* make(CWnd* pParent = NULL);
	static void destroy();
	static void setfont(const std::wstring& name, int point);

//protected:
	CExtractDlg(CWnd* pParent = NULL);   // 標準コンストラクター

	static sl::DialogConfig::Extract cfg_;

public:
	virtual ~CExtractDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXTRACT };
#endif

	
	void setSearch(const std::wstring& search) { browser_.setSearch(search); };
	void setExtract(const std::wstring& extract) { browser_.setExtract(extract); };
	void setCustomDHtmlDialog(CCustomDHtmlDialog* dhtml) { browser_.setCustomDHtmlDialog(dhtml); }
	void setFont(const std::wstring& name, int point);

protected:
	void SizeHelper();

protected:
	CFont font_;
	CStatusBar status_;
	CExtractBrowserDlg browser_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	virtual void PostNcDestroy();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
