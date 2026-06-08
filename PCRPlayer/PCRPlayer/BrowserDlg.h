#pragma once

#include "CustomDHtmlDialog.h"
#include "PopupBrowserDlg.h"
#include "ExtractDlg.h"
#include "ThreadDlg.h"
#include "CustomEventHandler.h"
#include "BrowserMenu.h"

#include "BBSSkin.h"

#include <unordered_set>
#include <unordered_map>
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/functional.hpp>

#include <strsafe.h>

#ifdef _WIN32_WCE
#error "CDHtmlDialog は Windows CE ではサポートされていません。"
#endif 

// CBrowserDlg ダイアログ

class CBrowserDlg : public CCustomDHtmlDialog
{
	DECLARE_DYNCREATE(CBrowserDlg)

public:
	CBrowserDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CBrowserDlg();


// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BROWSER, IDH = IDR_HTML_BROWSER };
#endif

	void setUpdateCallback(const boost::function<void()>& callback) { update_ = callback; }
	bool getStatus(std::wstring& text);
	size_t getPos() { return this_.pos; }
	void startScroll();
	void stopScroll();
	void hidePopup();

protected:
	CPopupBrowserDlg popup_;
	boost::function<void()> update_;

	struct This {
		This()
			: header(false)
			, complete(false)
			, callback(false)
			, init(true)
			, reset(false)
			, scroll(false)
			, first(false)
			, pos(0)
			, time(0)
		{}

		bool header;//新規header読み込み
		bool complete;//OnDocumentComplete完了
		bool callback;//datCallback

		bool init;//新規読み込み
		std::wstring url;//スレッドURL
		bool reset;//スレッドURLが同じ読み直し
		bool scroll;//スクロール中
		bool first;//>>1を追加読み込み
		size_t pos;//読み込み位置
		DWORD time;
		std::vector<bbs::ResInfo> dat;
	} this_;

	void autoScroll(bool latest = true);
	bool createRes(int size = 0);
	void setEventHandler();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual STDMETHODIMP TranslateAccelerator(LPMSG lpMsg, const GUID * pguidCmdGroup, DWORD nCmdID);
	virtual STDMETHODIMP ShowContextMenu( DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCallbackDat();
	afx_msg HRESULT OnAnchorMouseClick(IHTMLElement *pElement);
	afx_msg HRESULT OnAnchorMouseOver(IHTMLElement *pElement);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
