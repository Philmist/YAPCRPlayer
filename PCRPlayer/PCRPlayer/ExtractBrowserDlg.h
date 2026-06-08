#pragma once

#ifdef _WIN32_WCE
#error "CCustomDHtmlDialog は Windows CE ではサポートされていません。"
#endif 

#include "CustomDHtmlDialog.h"
#include "BrowserHook.h"
#include "BrowserMenu.h"

// CExtractBrowserDlg ダイアログ

class CExtractBrowserDlg : public CCustomDHtmlDialog
{
	DECLARE_DYNCREATE(CExtractBrowserDlg)

public:
	CExtractBrowserDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CExtractBrowserDlg();
// オーバーライド

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXTRACT_BROWSER, IDH = IDR_HTML_EXTRACTBROWSERDLG };
#endif

	void setSearch(const std::wstring& search) { this_.search = search; }
	void setExtract(const std::wstring& extract) { this_.extract = extract; }
	void setCustomDHtmlDialog(CCustomDHtmlDialog* dhtml) { this_.dhtml = dhtml; popup_.setTarget(dhtml); }
	void setStatusBar(CStatusBar* status) { this_.status = status; }

protected:
	CPopupBrowserDlg popup_;
	struct This {
		This()
			: header(false)
			, complete(false)
			, callback(false)
			, dhtml(NULL)
			, status(NULL)
			, count(0)
			, pos(0)
			, notify(false)
		{}

		bool header;//新規header読み込み
		bool complete;//OnDocumentComplete完了
		bool callback;//extractCallback

		std::wstring search;//抽出文字列
		std::wstring extract;//抽出URI
		CCustomDHtmlDialog* dhtml;//抽出元
		CStatusBar* status;//親のステータスバー
		std::wstring url;//スレッドURL
		int count;//抽出件数
		std::vector<bbs::ResInfo> dat;//抽出本文
		std::vector<bbs::ResInfo>::iterator it;//抽出本文読み込み位置
		CComPtr<IMarkupPointer> begin;//着色用ポインター
		CComPtr<IMarkupPointer> end;//着色用ポインター
		int pos;//着色位置
		bool notify;//ステータス通知表示
	} this_;

	bool replaceText(std::wstring& text, const std::wstring& header, const::std::wstring& replace);
	void setPaneText();
	void setPaneText(bool read);
	void setPaneTextNotify();
	bool pageExtract();
	bool checkURL();
	void scrollIntoView(int res);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnCallbackExtract();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual STDMETHODIMP TranslateAccelerator(LPMSG lpMsg, const GUID * pguidCmdGroup, DWORD nCmdID);
	virtual STDMETHODIMP ShowContextMenu( DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnDestroy();
	afx_msg HRESULT OnAnchorMouseClick(IHTMLElement *pElement);
	afx_msg HRESULT OnAnchorMouseOver(IHTMLElement *pElement);
	afx_msg HRESULT OnAnchorMouseOut(IHTMLElement *pElement);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
