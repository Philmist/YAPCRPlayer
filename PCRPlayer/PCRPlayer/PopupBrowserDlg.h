#pragma once

#ifdef _WIN32_WCE
#error "CCustomDHtmlDialog は Windows CE ではサポートされていません。"
#endif 

#include "CustomDHtmlDialog.h"
#include "BrowserHook.h"
#include "BrowserMenu.h"


#include <strsafe.h>

struct PopupBrowserConfig {
	struct Popup {
		Popup()
			: enable(true)
			, limit(1000)
			, head(true)
		{}
		bool enable; // ポップアップ表示
		int limit; // 表示レス数
		bool head; // 余りを先頭から削除して新しいレスを優先表示する
	};

	struct PopupAnchor : public Popup {
		PopupAnchor() { head = false; }
	};

	struct PopupID : public Popup {
		PopupID() { limit = 10; }
	};

	PopupBrowserConfig()
		: delay(50)
		, edge(true)
		, border(RGB(0, 0, 0))
	{}

	int delay; // ポップアップするまでの時間
	bool edge; // 3Dエッジ
	COLORREF border; // ボーダーカラー
	Popup ref; // レス番号ポップアップ
	PopupAnchor anchor; // アンカーポップアップ
	PopupID id; // IDポップアップ
};

// CPopupBrowserDlg ダイアログ

class CPopupBrowserDlg : public CCustomDHtmlDialog
{
	DECLARE_DYNCREATE(CPopupBrowserDlg)

public:
	CPopupBrowserDlg(CCustomDHtmlDialog* pParent = NULL, CCustomDHtmlDialog* pTarget = NULL);   // 標準コンストラクター
	virtual ~CPopupBrowserDlg();

	void SizeHelper();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_BROWSER, IDH = IDR_HTML_POPUP_BROWSER };
#endif

public:
	void setTarget(CCustomDHtmlDialog* dhtml) { target_ = dhtml; }

	void onMouseMove();
	bool onMouseLeave();

	void hide();

	BOOL popup(const CRect& area, const PopupBrowserConfig& cfg, bool margin = false);
	BOOL popup(IHTMLElement* pElement, const PopupBrowserConfig& cfg);

protected:
	BOOL popup(CPopupBrowserDlg* parent, IHTMLElement* pElement, const PopupBrowserConfig& cfg);

	void createPopup();
	void destroyPopup();
	BOOL preparePopup(const CRect& area);
	BOOL preparePopup(IHTMLElement* pElement);

protected:
	PopupBrowserConfig cfg_;
	CCustomDHtmlDialog* target_;
	CCustomDHtmlDialog* parent_;
	CPopupBrowserDlg* popup_;
	int counter_;

	struct This {
		This()
			: header(false)
			, complete(false)
			, callback(false)
			, parent(NULL)
			, margin(true)
			, horizontal(true)
			, show(false)
			, readOnly(false)
			, hover(false)
			, capture(false)
			, cancel(false)
		{}

		bool header;//新規header読み込み
		bool complete;//OnDocumentComplete完了
		bool callback;//extractCallback

		CPopupBrowserDlg* parent;
		std::wstring url;//スレッドURL
		std::wstring extract;//抽出対象
		CRect target;//ポップアップ位置
		bool margin;//ポップアップマージン
		bool horizontal;//ポップアップ位置水平伸長
		bool show;//ポップアップ表示
		DWORD time;//ポップアップ時間
		bool readOnly;//表示のみ
		bool hover;//TrackMouseEvent
		bool capture;//マウスキャプチャ
		bool cancel;//ポップアップ消去をキャンセル
	} this_;

	bool calcScrollSize(CSize& size);
	bool calcWindowSize(const CSize& scroll, CSize& size);
	void setPosition();
	void setShowWindow(bool show);
	void showWindow();
	void showWindow(bool show);
	void delayShowWindow();
	void scrollIntoView(int res);
	void capture(bool enable);
	void cancel(bool enable);
	void beginCapture();
	void endCapture();
	bool checkURL();

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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg HRESULT OnAnchorMouseClick(IHTMLElement *pElement);
	afx_msg HRESULT OnAnchorMouseOver(IHTMLElement *pElement);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextClose();
};
