// PopupBrowserDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include <exdispid.h> // DISPID_BEFORENAVIGATE2
#include "PCRPlayer.h"
#include "PopupBrowserDlg.h"
#include "ExtractDlg.h"
#include "DHtmlUtl.h"

enum TIMER_ID {
	TIMER_ID_POPUP,
};

#define TARGET_EXPAND 30
#define TARGET_MARGIN 8


// CPopupBrowserDlg ダイアログ

IMPLEMENT_DYNCREATE(CPopupBrowserDlg, CCustomDHtmlDialog)

CPopupBrowserDlg::CPopupBrowserDlg(CCustomDHtmlDialog* pParent /*=NULL*/, CCustomDHtmlDialog* pTarget /*=NULL*/)
	: CCustomDHtmlDialog(IDD_POPUP_BROWSER, /*IDR_HTML_POPUP_BROWSER,*/ pParent)
	, parent_(pParent)
	, target_(pTarget ? pTarget : pParent)
	, popup_(NULL)
	, counter_(0)
{
}

CPopupBrowserDlg::~CPopupBrowserDlg()
{
	gl_.callback.extract.pop(this);
}



void CPopupBrowserDlg::SizeHelper()
{
	if (::IsWindow(m_wndBrowser))
	{
		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(1, 1);
		m_wndBrowser.MoveWindow(rc);
	}
}

//=============================================================================

void CPopupBrowserDlg::onMouseMove()
{
	if (parent_ && !this_.hover)
	{
		CPoint pt;
		if (GetCursorPos(&pt))
		{
			CWnd* wnd = WindowFromPoint(pt);
			if (wnd->GetSafeHwnd() == parent_->GetSafeHwnd() ||
				wnd->GetSafeHwnd() == parent_->GetSafeBrowserHwnd())
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = wnd->GetSafeHwnd();
				tme.dwHoverTime = HOVER_DEFAULT;
				TrackMouseEvent(&tme);
				this_.hover = true;
			}
		}
	}

	if (this_.target.IsRectEmpty()) { return; }

	if (this_.cancel ||
		(popup_ && ::IsWindow(popup_->GetSafeHwnd()) && ::IsWindowVisible(popup_->GetSafeHwnd())))
	{
		return;
	}

	CPoint pt;
	if (GetCursorPos(&pt))
	{
		CRect rc(this_.target);
		if (!this_.readOnly && this_.margin)
		{
			if (this_.horizontal)
			{
				rc.InflateRect(TARGET_EXPAND, TARGET_MARGIN);
			}
			else
			{
				rc.InflateRect(TARGET_MARGIN, TARGET_EXPAND);
			}
		}
		if (rc.PtInRect(pt)) { return; }
	}
	
	this_.target.SetRectEmpty();
	showWindow(false);
}

bool CPopupBrowserDlg::onMouseLeave()
{
	this_.hover = false;
	CPoint pt;
	if (GetCursorPos(&pt))
	{
		CWnd* wnd = WindowFromPoint(pt);
		if (wnd)
		{
			if (parent_ && wnd->GetSafeHwnd() == parent_->GetSafeHwnd())
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = parent_->GetSafeHwnd();
				tme.dwHoverTime = HOVER_DEFAULT;
				TrackMouseEvent(&tme);
				this_.hover = true;
				return true;
			}
			else if (parent_ && wnd->GetSafeHwnd() == parent_->GetSafeBrowserHwnd())
			{
				// TrackMouseEventを設定するとスクロールが動かなくなる
				return true;
			}
			else if (wnd->GetSafeHwnd() == GetSafeHwnd() ||
				wnd->GetSafeHwnd() == GetSafeBrowserHwnd())
			{
				return true;
			}
		}
	}

	if (this_.cancel ||
		(popup_ && ::IsWindow(popup_->GetSafeHwnd()) && ::IsWindowVisible(popup_->GetSafeHwnd())))
	{
		return true;
	}

	showWindow(false);
	return false;
}

void CPopupBrowserDlg::hide()
{
	this_.cancel = false;
	showWindow(false);
	if (popup_) { popup_->hide(); }
}

BOOL CPopupBrowserDlg::popup(const CRect& area, const PopupBrowserConfig& cfg, bool margin)
{
	cfg_ = cfg;
	this_.margin = false;
	return preparePopup(area);
}

BOOL CPopupBrowserDlg::popup(IHTMLElement* pElement, const PopupBrowserConfig& cfg)
{
	return popup(NULL, pElement, cfg);
}

BOOL CPopupBrowserDlg::popup(CPopupBrowserDlg* parent, IHTMLElement* pElement, const PopupBrowserConfig& cfg)
{
	if (!parent_ || !pElement) { return FALSE; }

	CComBSTR name;
	if (!SUCCEEDED(pElement->get_className(&name))) { return FALSE; }

	cfg_ = cfg;
	this_.parent = parent;
	this_.extract = (LPCWSTR)CString(name);
	this_.margin = true;

	if (this_.parent)
	{
		std::wstring& extract = this_.parent->this_.extract;
		this_.readOnly = !extract.empty() && extract == this_.extract;
	}

	bbs::URIConvert cnv;
	bbs::URIConvert::Result res;
	if (cnv(this_.extract, res))
	{
		if (cfg_.anchor.enable && res.anchor.matched)
		{
			if (preparePopup(pElement))
			{
				gl_.bbs.extract(res.anchor.value, cfg_.anchor.limit, cfg_.anchor.head);
				return TRUE;
			}
		}
		else if (cfg_.id.enable && res.id.matched)
		{
			if (preparePopup(pElement))
			{
				gl_.bbs.extract(res.id.value, cfg_.id.limit, cfg_.id.head);
				return TRUE;
			}
		}
		else if (cfg_.ref.enable && res.ref.matched)
		{
			if (preparePopup(pElement))
			{
				gl_.bbs.extract(res.ref.value, cfg_.ref.limit, cfg_.ref.head);
				return TRUE;
			}
		}
		else if (res.http.matched)
		{
		}
	}
	return FALSE;
}

//=============================================================================

void CPopupBrowserDlg::createPopup()
{
	if (!popup_)
	{
		popup_ = new CPopupBrowserDlg(this, target_);
		popup_->counter_ = counter_ + 1;
	}
}

void CPopupBrowserDlg::destroyPopup()
{
	if (popup_)
	{
		if (::IsWindow(popup_->GetSafeHwnd()))
		{
			popup_->DestroyWindow();
		}
		delete popup_;
		popup_ = NULL;
	}
}


BOOL CPopupBrowserDlg::preparePopup(const CRect& area)
{
	gl_.bbs.getThreadURL(this_.url);

	this_.target = area;
	gl_.callback.extract.push(this);
	setShowWindow(true);

	if (!::IsWindow(GetSafeHwnd()))
	{
		Create(IDD_POPUP_BROWSER, GetDesktopWindow());
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE);
	}
	return TRUE;
}

BOOL CPopupBrowserDlg::preparePopup(IHTMLElement* pElement)
{
	if (!parent_ || !pElement) { return FALSE; }

	CRect area;
	if (parent_->GetBoundingClientRect(pElement, area))
	{
		HWND hwnd = parent_->GetSafeBrowserHwnd();
		::ClientToScreen(hwnd, &area.TopLeft());
		::ClientToScreen(hwnd, &area.BottomRight());
		return preparePopup(area);
	}
	return FALSE;
}

//=============================================================================

bool CPopupBrowserDlg::calcScrollSize(CSize& size)
{
	CComQIPtr<IHTMLElement2> body;
	if (GetBodyElement(body))
	{
		// 最低限1x1のウィンドウを作ってスクロールサイズを求める
		// +2はボーダーライン
		// OnSizeが呼ばれてブラウザコントロールがサイズ変更される
		SetWindowPos(NULL, 0, 0, 1 + 2, 1 + 2, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		body->get_scrollWidth(&size.cx);
		body->get_scrollHeight(&size.cy);
		return true;
	}
	return false;
}

bool CPopupBrowserDlg::calcWindowSize(const CSize& scroll, CSize& size)
{
	CComQIPtr<IHTMLElement2> body;
	if (GetBodyElement(body))
	{
		// スクロールサイズを適用してスクロールバーの幅を計算する
		// +2はボーダーライン
		SetWindowPos(NULL, 0, 0, scroll.cx + 2, scroll.cy + 2, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

		long clientW = 0, clientH = 0;
		CComQIPtr<IHTMLElement2> root;
		if (GetDocumentElement(root))
		{
			root->get_clientWidth(&clientW);
			root->get_clientHeight(&clientH);

			if (clientW == 0 || clientH == 0)
			{
				body->get_clientWidth(&clientW);
				body->get_clientHeight(&clientH);
			}

			long width = 0, height = 0;
			if (m_pBrowserApp)
			{
				m_pBrowserApp->get_Width(&width);
				m_pBrowserApp->get_Height(&height);

				// ブラウザサイズ(width) - 表示領域(clientW) = スクロールバーサイズ
				size.cx = scroll.cx + max(width - clientW, 0) + 2;
				size.cy = scroll.cy + max(height - clientH, 0) + 2;
				return true;
			}
		}
	}
	return false;
}

void CPopupBrowserDlg::setPosition()
{
	if (this_.target.IsRectEmpty()) { return; }
	CRect target(this_.target);


	CSize scroll;
	if (calcScrollSize(scroll))
	{
		// マウスカーソル位置のモニタ情報取得
		MONITORINFO mi;
		CPoint pt;
		GetCursorPos(&pt);
		utl::getDesktopRectFromPoint(pt, mi);
		CRect rc(mi.rcWork);

		// モニタの大きさに合わせる
		if (rc.Width() - scroll.cx < 0)
		{
			scroll.cx = rc.Width() - 2;
			if (scroll.cx < 0) { scroll.cx = 0; }
		}
		if (rc.Height() - scroll.cy < 0)
		{
			scroll.cy = rc.Height() - 2;
			if (scroll.cy < 0) { scroll.cy = 0; }
		}

		CSize size;
		if (calcWindowSize(scroll, size))
		{
			// スクロールバーを付けたサイズをモニタの大きさに合わせる
			if (rc.Width() - size.cx < 0)
			{
				size.cx = rc.Width();
			}
			if (rc.Height() - size.cy < 0)
			{
				size.cy = rc.Height();
			}

			this_.horizontal = true;
			int ax = target.left - TARGET_EXPAND;
			int ay = target.top - size.cy;

			if (ax + size.cx > rc.right)
			{
				ax = rc.right - size.cx;
			}
			else if (ax < rc.left)
			{
				ax = rc.left;
			}

			if (ay < rc.top)
			{// 上にはみ出す
				if (((rc.bottom - target.bottom) - size.cy) > 0)
				{// 下にはみ出さない
					ay = target.bottom;
				}
				else
				{
					ay = rc.top;
					CRect arc(ax, ay, ax + size.cx, ay + size.cy);
					if (arc.PtInRect(target.TopLeft()) &&
						arc.PtInRect(target.BottomRight()))
					{// ポップアップ位置が隠れる
						this_.horizontal = false;
						if (((rc.right - target.right) - size.cx) > 0)
						{// 右にはみ出さない
							ax = target.right;
						}
						else
						{
							ax = rc.right - size.cx;

							//ax = target.left - size.cx;
							//if (ax < 0)
							//{// 左にはみ出す
							//	ax = rc.left;
							//}
						}
					}
				}
			}

			SetWindowPos(NULL, ax, ay, size.cx, size.cy, SWP_NOZORDER|SWP_NOACTIVATE);
		}
	}
}

void CPopupBrowserDlg::setShowWindow(bool show)
{
	this_.show = show;
	this_.time = GetTickCount();
}

void CPopupBrowserDlg::showWindow()
{
	showWindow(this_.show);
	this_.show = false;
}

void CPopupBrowserDlg::showWindow(bool show)
{
	if (!::IsWindow(GetSafeHwnd())) { return; }
	KillTimer(TIMER_ID_POPUP);

	if (show)
	{
		if (!IsWindowVisible())
		{
			SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
			ShowWindow(SW_SHOWNA);
			ChangeFocus();
		}
	}
	else
	{
		if (IsWindowVisible())
		{
			// wndBottomやwndTopだとちらつきが発生する
			SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
			ShowWindow(SW_HIDE);
			ChangeFocus();
		}
	}
}

void CPopupBrowserDlg::delayShowWindow()
{
	int delay = cfg_.delay;
	DWORD diff = GetTickCount() - this_.time;
	if (delay <= 0 || (int)diff >= delay)
	{
		showWindow();
	}
	else
	{
		showWindow(false);
		SetTimer(TIMER_ID_POPUP, (delay - diff), NULL);
	}
}

void CPopupBrowserDlg::scrollIntoView(int res)
{
	if (target_)
	{
		CString anchor;
		anchor.Format(L"res:%d", res);
		CComPtr<IHTMLDocument2> doc2;
		if (SUCCEEDED(target_->GetDHtmlDocument(&doc2)))
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
}


//=============================================================================

void CPopupBrowserDlg::capture(bool enable)
{
	CPopupBrowserDlg* parent = this;
	while (true)
	{
		if (!parent) { break; }
		parent->this_.capture = enable;
		parent = parent->this_.parent;
	}
}

void CPopupBrowserDlg::cancel(bool enable)
{
	CPopupBrowserDlg* parent = this;
	while (true)
	{
		if (!parent) { break; }

		parent->this_.cancel = enable;
		parent = parent->this_.parent;
	}
}

void CPopupBrowserDlg::beginCapture()
{
	CPoint pt;
	if (GetCursorPos(&pt))
	{
		if (IsTextArea(pt))
		{
			capture(true);
			cancel(true);
		}
	}
}

void CPopupBrowserDlg::endCapture()
{
	capture(false);
	cancel(false);
}

bool CPopupBrowserDlg::checkURL()
{
	std::wstring url;
	if (gl_.bbs.getThreadURL(url) && this_.url == url)
	{
		return true;
	}
	return false;
}

//=============================================================================


void CPopupBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CCustomDHtmlDialog::DoDataExchange(pDX);
}




BOOL CPopupBrowserDlg::OnInitDialog()
{
	CCustomDHtmlDialog::OnInitDialog();

	SizeHelper();
	return FALSE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

BEGIN_MESSAGE_MAP(CPopupBrowserDlg, CCustomDHtmlDialog)
	ON_COMMAND(IDM_UPDATE_EXTRACT_END, &CPopupBrowserDlg::OnCallbackExtract)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(IDM_CONTEXT_CLOSE, &CPopupBrowserDlg::OnContextClose)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CPopupBrowserDlg)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONCLICK, _T("a"), OnAnchorMouseClick)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("a"), OnAnchorMouseOver)
END_DHTML_EVENT_MAP()


// CPopupBrowserDlg メッセージ ハンドラー


void CPopupBrowserDlg::OnCallbackExtract()
{
	if (!this_.complete)
	{
		this_.callback = true;
		return;
	}

	if (this_.cancel || (popup_ && popup_->this_.cancel))
	{
		return;
	}

	std::vector<bbs::ResInfo> dat;
	if (!gl_.bbs.getExtract(dat)) { return; }

	CComPtr<IHTMLDocument2> doc2;
	if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { return; }

	std::wstring noname;
	gl_.bbs.getNoName(noname);
	bbs::BBSSkin skin(gl_.value.detail.skin.popup, noname);

	std::wstring html = skin.header(gl_.html.popupHeader) + L"\r\n";

	std::wstring res;
	for (auto it = dat.begin(); it != dat.end(); ++it)
	{
		res += skin.res(gl_.html.popupRes, *it) + L"\r\n";
	}

	doc2->writeln(utl::SafeArray(html + res));
	doc2->close();

	// リンク色の更新
	utl::refreshColor(doc2, dat, gl_.value.detail.skin.popup);

	setPosition();
	ScrollToBottom();
	delayShowWindow();
}



void CPopupBrowserDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CCustomDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

	if (pDisp != m_pBrowserApp) { return; }

	if (!this_.header)
	{// リソースの読み込み終了後にヘッダーを読み込む
		// ブラウザウィンドウフック
		utl::BrowserHook::Subclass(GetSafeBrowserHwnd(), GetSafeHwnd());

		bbs::BBSSkin skin(gl_.value.detail.skin.popup);
		std::wstring html = skin.header(gl_.html.popupHeader);
		LoadFromString(html);
		this_.header = true;
	}
	else
	{// 新規読み込みが終わってからDatを処理する
		this_.complete = true;
		if (this_.callback)
		{
			this_.callback = false;
			OnCallbackExtract();
		}
	}

}

HRESULT CPopupBrowserDlg::TranslateAccelerator(LPMSG lpMsg, const GUID * pguidCmdGroup, DWORD nCmdID)
{
	if (lpMsg && lpMsg->message == WM_KEYDOWN)
	{
		if (lpMsg->wParam == VK_F5)
		{
			//gl_.bbs.refresh();
			return S_OK;
		}
		else if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (lpMsg->wParam != L'A' &&
				lpMsg->wParam != L'Z' &&
				lpMsg->wParam != L'X' &&
				lpMsg->wParam != L'C' &&
				lpMsg->wParam != L'V')
			{
				return S_OK;
			}
		}
	}
	return S_FALSE;

	//return CCustomDHtmlDialog::TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
}

HRESULT CPopupBrowserDlg::ShowContextMenu(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved)
{
	cancel(true);
	HRESULT hr = utl::ContextMenu::show(this, dwID, ppt, pdispReserved, 2, checkURL(), this_.url);
	cancel(false);
	PostMessage(WM_MOUSELEAVE);
	return hr;
	//return CCustomDHtmlDialog::ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
}


void CPopupBrowserDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!popup_ ||
		!::IsWindow(popup_->GetSafeHwnd()) ||
		!::IsWindowVisible(popup_->GetSafeHwnd()))
	{
		if (!this_.capture)
		{
			cancel(false);
		}
	}

	if (this_.readOnly)
	{
		if (parent_)
		{
			parent_->PostMessage(WM_MOUSEMOVE);
		}
		return;
	}

	createPopup();
	if (popup_)
	{
		popup_->onMouseMove();
	}

	CCustomDHtmlDialog::OnMouseMove(nFlags, point);
}


void CPopupBrowserDlg::OnMouseLeave()
{
	if (popup_)
	{
		if (popup_->onMouseLeave())
		{
			return;
		}
	}

	if (this_.cancel ||
		(popup_ && ::IsWindow(popup_->GetSafeHwnd()) && ::IsWindowVisible(popup_->GetSafeHwnd())))
	{
		return;
	}

	showWindow(false);

	destroyPopup();

	if (parent_)
	{
		// 呼び出し先で自分が削除されるのでSendMessageで渡さない
		parent_->PostMessage(WM_MOUSELEAVE);
	}

	CCustomDHtmlDialog::OnMouseLeave();
}

void CPopupBrowserDlg::OnSize(UINT nType, int cx, int cy)
{
	//CCustomDHtmlDialog::OnSize(nType, cx, cy);

	SizeHelper();
}


void CPopupBrowserDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: ここにメッセージ ハンドラー コードを追加します。
					   // 描画メッセージで CCustomDHtmlDialog::OnPaint() を呼び出さないでください。
	CRect rc;
	GetClientRect(rc);

	if (gl_.value.detail.popup.edge)
	{
		dc.DrawEdge(rc, BDR_RAISEDOUTER, BF_RECT);
	}
	else
	{
		CPen pen(PS_SOLID, 1, gl_.value.detail.popup.border);
		CPen* oldPen = dc.SelectObject(&pen);
		CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
		dc.Rectangle(rc);
		dc.SelectObject(oldPen);
		dc.SelectObject(oldBrush);
	}
}

void CPopupBrowserDlg::OnDestroy()
{
	CCustomDHtmlDialog::OnDestroy();

	utl::BrowserHook::Unsubclass(GetSafeBrowserHwnd());
	destroyPopup();
}


BOOL CPopupBrowserDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CCustomDHtmlDialog::OnMouseWheel(nFlags, zDelta, pt);
}

HRESULT CPopupBrowserDlg::OnAnchorMouseClick(IHTMLElement* pElement)
{
	if (!pElement) { return S_OK; }

	CComBSTR name;
	if (!SUCCEEDED(pElement->get_className(&name))) { return S_OK; }

	bbs::URIConvert cnv;
	bbs::URIConvert::Result res;
	if (cnv((LPCWSTR)CString(name), res))
	{
		if (res.anchor.matched)
		{
			if (checkURL())
			{
				scrollIntoView(res.anchor.value.first);
			}
		}
		else if (res.id.matched)
		{
		}
		else if (res.ref.matched)
		{
			if (checkURL())
			{
				scrollIntoView(res.ref.value);
			}
			//CMenu menu;
			//if (menu.LoadMenu(IDR_CLICKMENU))
			//{
			//	CMenu *sub = menu.GetSubMenu(1);
			//	int idm = 0;
			//	if (sub)
			//	{
			//		if (!checkURL())
			//		{
			//			sub->EnableMenuItem(IDM_CLICK_JUMP, MF_DISABLED|MF_BYCOMMAND);
			//		}

			//		CPoint pt;
			//		GetCursorPos(&pt);

			//		beginCapture();
			//		idm = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
			//		endCapture();
			//	}
			//	menu.DestroyMenu();

			//	switch (idm)
			//	{
			//	case IDM_CLICK_JUMP: scrollIntoView(res.ref.value); break;
			//	}
			//}
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


HRESULT CPopupBrowserDlg::OnAnchorMouseOver(IHTMLElement* pElement)
{
	if (this_.readOnly)
	{
		return S_OK;
	}

	if (!pElement) { return S_OK; }

	if (GetKeyState(VK_LBUTTON) & 0x8000 || GetKeyState(VK_RBUTTON) & 0x8000)
	{
		return S_OK;
	}

	if (popup_)
	{
		if (checkURL())
		{
			popup_->popup(this, pElement, cfg_);
		}
	}
	return S_OK;
}

void CPopupBrowserDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_POPUP:
		{
			showWindow();
			KillTimer(TIMER_ID_POPUP);
			break;
		}
	}
	CCustomDHtmlDialog::OnTimer(nIDEvent);
}



BOOL CPopupBrowserDlg::OnEraseBkgnd(CDC* pDC)
{
	//return TRUE;
	return CCustomDHtmlDialog::OnEraseBkgnd(pDC);
}


void CPopupBrowserDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	ChangeFocus();
	//beginCapture();
	//CCustomDHtmlDialog::OnLButtonDblClk(nFlags, point);
}


void CPopupBrowserDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	ChangeFocus();
	//beginCapture();
	//CCustomDHtmlDialog::OnLButtonDown(nFlags, point);
}


void CPopupBrowserDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	ChangeFocus();
	//endCapture();
	//CCustomDHtmlDialog::OnLButtonUp(nFlags, point);
}


void CPopupBrowserDlg::OnContextClose()
{
	hide();
}
