// BrowserDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include <exdispid.h> // DISPID_BEFORENAVIGATE2
#include "PCRPlayer.h"
#include "BrowserDlg.h"
#include "DHtmlUtl.h"

enum TIMER_ID {
	TIMER_ID_DAT,
	TIMER_ID_SCROLL,
};


// CBrowserDlg ダイアログ

IMPLEMENT_DYNCREATE(CBrowserDlg, CCustomDHtmlDialog)

CBrowserDlg::CBrowserDlg(CWnd* pParent /*=NULL*/)
	: CCustomDHtmlDialog(IDD_BROWSER, /*IDR_HTML_BROWSER,*/ pParent)
	, popup_(this)
	, update_([](){})
{
	gl_.callback.dat.push(this);
}

CBrowserDlg::~CBrowserDlg()
{
	gl_.callback.dat.pop(this);
}

void CBrowserDlg::startScroll()
{
	SetTimer(TIMER_ID_SCROLL, gl_.value.detail.scroll.interval, NULL);
	this_.scroll = true;
}

void CBrowserDlg::stopScroll()
{
	KillTimer(TIMER_ID_SCROLL);
	this_.scroll = false;
}

void CBrowserDlg::hidePopup()
{
	if (::IsWindow(popup_))
	{
		popup_.hide();
	}
}

void CBrowserDlg::autoScroll(bool latest)
{
	if (this_.init)
	{// 新規読み込み
		if (this_.reset)
		{// 読み直し
			if (this_.scroll)
			{// スクロール状態復元
				startScroll();
			}
		}
		else if (gl_.value.detail.scroll.init)
		{// 新規読み込み時
			startScroll();
		}
	}
	else
	{
		if (this_.scroll || (latest && gl_.value.detail.scroll.latest))
		{// スクロールしていた / 新着レス取得時
			startScroll();
		}
	}
}

void CBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CCustomDHtmlDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBrowserDlg, CCustomDHtmlDialog)
	ON_WM_SIZE()
	ON_COMMAND(IDM_UPDATE_RES_END, &CBrowserDlg::OnCallbackDat)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CBrowserDlg)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONCLICK, _T("a"), OnAnchorMouseClick)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("a"), OnAnchorMouseOver)
END_DHTML_EVENT_MAP()

// CBrowserDlg メッセージ ハンドラー


BOOL CBrowserDlg::OnInitDialog()
{
	CCustomDHtmlDialog::OnInitDialog();

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}



void CBrowserDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CCustomDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

	if (pDisp != m_pBrowserApp) { return; }

	if (!this_.header)
	{// リソースの読み込み終了後にヘッダーを読み込む
		// ブラウザウィンドウフック
		utl::BrowserHook::Subclass(GetSafeBrowserHwnd(), GetSafeHwnd());

		bbs::BBSSkin skin(gl_.value.detail.skin.thread);
		std::wstring html = skin.header(gl_.html.header);
		LoadFromString(html);
		this_.header = true;
	}
	else
	{// 新規読み込みが終わってからDatを処理する
		this_.complete = true;
		if (this_.callback)
		{
			this_.callback = false;
			OnCallbackDat();
		}
	}
}


HRESULT CBrowserDlg::TranslateAccelerator(LPMSG lpMsg, const GUID * pguidCmdGroup, DWORD nCmdID)
{
	if (lpMsg)
	{
		switch (lpMsg->message)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (lpMsg->wParam == VK_F5)
			{// 更新
				if (gl_.value.detail.scroll.refresh)
				{
					startScroll();
				}
				gl_.bbs.refresh();
				return S_OK;
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				switch (lpMsg->wParam)
				{
				case L'F':// 検索
					AfxGetMainWnd()->SendMessage(WM_COMMAND, IDM_CONTEXT_SEARCH);
					return S_OK;
				case L'A':// すべて選択
					SelectAll();
					return S_OK;
				case L'C':// コピー
					CopySelectedText();
					return S_OK;
				}
			}
		
			CMainDlg *wnd = (CMainDlg *)AfxGetMainWnd();
			if (wnd)
			{
				wnd->PostMessage(lpMsg->message, lpMsg->wParam, lpMsg->lParam);
				return S_OK;
			}
		}
	}
	return S_FALSE;


	//if (lpMsg && lpMsg->message == WM_KEYDOWN)
	//{
	//	if (lpMsg->wParam == VK_F5)
	//	{// 更新ショートカットの処理を変更
	//		gl_.bbs.refresh();
	//		return S_OK;
	//	}
	//	else if (GetKeyState(VK_CONTROL) & 0x8000)
	//	{// 編集操作以外のCtrlショートカットを抑止
	//		if (lpMsg->wParam != L'A' &&
	//			lpMsg->wParam != L'Z' &&
	//			lpMsg->wParam != L'X' &&
	//			lpMsg->wParam != L'C' &&
	//			lpMsg->wParam != L'V')
	//		{
	//			return S_OK;
	//		}
	//	}
	//}
	//return S_FALSE;

	//return CCustomDHtmlDialog::TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
}

HRESULT CBrowserDlg::ShowContextMenu(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved)
{
	std::wstring url;
	if (!gl_.bbs.getURL(url))
	{
		gl_.bbs.getSourceURL(url);
	}
	return utl::ContextMenu::show(this, dwID, ppt, pdispReserved, 0, true, url);
	//return CCustomDHtmlDialog::ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
}

void CBrowserDlg::OnSize(UINT nType, int cx, int cy)
{
	if (StoreScroll())
	{
		CCustomDHtmlDialog::OnSize(nType, cx, cy);
		RestoreScroll();
		return;
	}

	CCustomDHtmlDialog::OnSize(nType, cx, cy);
}


void CBrowserDlg::OnCallbackDat()
{
	if (!this_.complete)
	{
		this_.callback = true;
		return;
	}

	KillTimer(TIMER_ID_DAT);
	KillTimer(TIMER_ID_SCROLL);

	CComPtr<IHTMLDocument2> doc2;
	if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { return; }

	if (!gl_.bbs.isThread())
	{// 掲示板URL
		this_.pos = 0;
		doc2->close();
		bbs::BBSSkin skin(gl_.value.detail.skin.thread);
		std::wstring header = skin.header(gl_.html.header);
		doc2->writeln(utl::SafeArray(header));
		return;
	}

	this_.time = GetTickCount();

	std::vector<bbs::ResInfo> dat;
	bool init = true;
	if (!gl_.bbs.getDat(dat, init)) { return; }

	if (init)
	{// 新規読み込み
		StoreScroll();

		this_.reset = false;
		std::wstring url; 
		if (gl_.bbs.getThreadURL(url))
		{
			if (!this_.url.empty() && this_.url == url)
			{
				this_.reset = true;
			}
			else
			{
				this_.url = url;
			}
		}

		this_.pos = 0;
		doc2->close();
		bbs::BBSSkin skin(gl_.value.detail.skin.thread);
		std::wstring header = skin.header(gl_.html.header);
		doc2->writeln(utl::SafeArray(header));

		setEventHandler();

		int def = gl_.value.detail.board.def;
		if (def == 0 || (int)dat.size() - def <= 0)
		{
			this_.pos = 0;
			this_.first = false;
		}
		else
		{
			this_.pos = dat.size() - def;
			this_.first = true;
		}
	}
	else
	{
		// 残りのレスを読み込む
		createRes();

		StoreScroll();

		// 新着レスを通常レスに書き換え
		RemoveClassName(L"new");
		
	}

	this_.init = init;
	bool latest = false;
	if (this_.pos > 0 && dat.size() > this_.pos)
	{// 新着レスのIDで過去レスのID発言数を更新する
		latest = true;
		std::unordered_set<std::wstring> set;
		for (auto it = dat.begin() + this_.pos; it != dat.end(); ++it)
		{
			if (it->total > 1 && set.find(it->id) == set.end())
			{
				set.insert(it->id);
				utl::refreshCounter(doc2, it->id, it->count, it->total);
			}
		}
	}

	this_.dat.swap(dat);

	if (createRes(gl_.value.detail.board.partial))
	{
		SetTimer(TIMER_ID_DAT, 0, NULL);
	}
	else
	{
		autoScroll(latest);
		ChangeFocus();
	}
}

void CBrowserDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_DAT:
		{
			if (!createRes(gl_.value.detail.board.partial))
			{
				KillTimer(TIMER_ID_DAT);
				autoScroll();
				ChangeFocus();
			}
			break;
		}
	case TIMER_ID_SCROLL:
		{
			if (!ScrollToBottomBy(gl_.value.detail.scroll.delta))
			{
				if (gl_.value.detail.scroll.last)
				{
					stopScroll();
				}
			}
			break;
		}
	}
	CCustomDHtmlDialog::OnTimer(nIDEvent);
}

HRESULT CBrowserDlg::OnAnchorMouseClick(IHTMLElement* pElement)
{
	if (!pElement) { return S_OK; }

	CComBSTR name;
	if (!SUCCEEDED(pElement->get_className(&name))) { return S_OK; }

	//gl_.addLog(L"OnAnchorMouseUp:%s", CString(name).GetString());

	bbs::URIConvert cnv;
	bbs::URIConvert::Result res;
	if (cnv((LPCWSTR)CString(name), res))
	{
		if (res.anchor.matched)
		{
			CString anchor;
			anchor.Format(L"res:%d", res.anchor.value.first);
			CComPtr<IHTMLDocument2> doc2;
			if (SUCCEEDED(GetDHtmlDocument(&doc2)))
			{
				CComQIPtr<IHTMLDocument3> doc3(doc2);
				if (doc3)
				{
					CComPtr<IHTMLElement> element;
					if (SUCCEEDED(doc3->getElementById(CComBSTR(anchor), &element)) && element)
					{
						element->scrollIntoView(CComVariant(true));
					}
				}
			}
		}
		else if (res.id.matched)
		{
		}
		else if (res.ref.matched)
		{
		}
		else if (res.http.matched)
		{
		}
	}
	else
	{
		CComQIPtr<IHTMLAnchorElement> anchor(pElement);
		CComBSTR href;
		if (anchor && SUCCEEDED(anchor->get_href(&href)))
		{
			std::wstring url = CString(href).GetString();
			utl::selectExecute(gl_.value.detail.browser.def, gl_.value.detail.browser.web, url);
		}
	}
	return S_OK;
}

HRESULT CBrowserDlg::OnAnchorMouseOver(IHTMLElement* pElement)
{
	if (!pElement) { return S_OK; }

	if (GetKeyState(VK_LBUTTON) & 0x8000 || GetKeyState(VK_RBUTTON) & 0x8000)
	{
		return S_OK;
	}

	popup_.popup(pElement, gl_.value.detail.popup);

	return S_OK;
}

void CBrowserDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	popup_.onMouseMove();

	ClientToScreen(&point);
	(static_cast<CMainDlg*>(AfxGetMainWnd()))->ScreenToClient(&point);
	(static_cast<CMainDlg*>(AfxGetMainWnd()))->OnMouseMove(nFlags, point);
	//CCustomDHtmlDialog::OnMouseMove(nFlags, point);
}

void CBrowserDlg::OnMouseLeave()
{
	popup_.onMouseLeave();

	CCustomDHtmlDialog::OnMouseLeave();
}

void CBrowserDlg::OnDestroy()
{
	CCustomDHtmlDialog::OnDestroy();

	if (::IsWindow(popup_))
	{
		popup_.DestroyWindow();
	}

	utl::BrowserHook::Unsubclass(GetSafeBrowserHwnd());
}



void CBrowserDlg::setEventHandler()
{
	CComPtr<IHTMLDocument2> doc2;
	if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { return; }

	CComPtr<IHTMLWindow2> window;
	if (!SUCCEEDED(doc2->get_parentWindow(&window)) || !window) { return; }

	_variant_t onscroll(new CCustomEventHandler([this]()->bool{
		if (custom_.scroll)
		{
			custom_.scroll = false;
		}
		else if (this_.scroll && gl_.value.detail.scroll.manual)
		{
			stopScroll();
			//gl_.addLog(L"manual");
		}
		

		//CComPtr<IHTMLEventObj> obj;
		//if (SUCCEEDED(this->GetEvent(&obj)) && obj)
		//{
		//	//long p = 0;
		//	//obj->get_keyCode(&p);
		//	//if (p == )
		//	//CComQIPtr<IHTMLEventObj4> obj4(obj);
		//	//if (!obj4) { return true; }
		//	//long delta = 0;
		//	//obj4->get_wheelDelta(&delta);
		//}
		return true;
	}));
	window->put_onscroll(onscroll);
}




//CComVariant vZoom = 100;
//m_pBrowserApp->ExecWB(OLECMDID_OPTICAL_ZOOM, OLECMDEXECOPT_DODEFAULT, &vZoom, NULL);
//void CBrowserDlg::setEventHandler()
//{
//	CComPtr<IHTMLDocument2> doc2;
//	if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { return; }
//
//	CComQIPtr<IHTMLDocument5> doc5(doc2);
//	if (!doc5) { return; }
//
//	_variant_t mousewheel(new CCustomEventHandler([this]()->bool{
//		CComPtr<IHTMLEventObj> obj;
//		if (SUCCEEDED(this->GetEvent(&obj)) && obj)
//		{
//			long p = 0;
//			obj->get_keyCode(&p);
//			//if (p == )
//			//CComQIPtr<IHTMLEventObj4> obj4(obj);
//			//if (!obj4) { return true; }
//			//long delta = 0;
//			//obj4->get_wheelDelta(&delta);
//		}
//		return true;
//	}));
//
//	doc2->put_onkeydown(mousewheel);
//}



BOOL CBrowserDlg::OnEraseBkgnd(CDC* pDC)
{
	::RedrawWindow(GetSafeBrowserHwnd(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	return TRUE;
	//return CCustomDHtmlDialog::OnEraseBkgnd(pDC);
}
