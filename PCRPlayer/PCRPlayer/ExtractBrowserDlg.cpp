// ExtractBrowserDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ExtractDlg.h"
#include "ExtractBrowserDlg.h"

enum TIMER_ID {
	TIMER_ID_DAT,
	TIMER_ID_SEARCH_COLOR,
};


// CExtractBrowserDlg ダイアログ

IMPLEMENT_DYNCREATE(CExtractBrowserDlg, CCustomDHtmlDialog)

CExtractBrowserDlg::CExtractBrowserDlg(CWnd* pParent /*=NULL*/)
	: CCustomDHtmlDialog(IDD_EXTRACT_BROWSER, /*IDR_HTML_EXTRACTBROWSERDLG,*/ pParent)
	, popup_(this)
{

}

CExtractBrowserDlg::~CExtractBrowserDlg()
{
	gl_.callback.extract.pop(this);
}

bool CExtractBrowserDlg::replaceText(std::wstring& text, const std::wstring& header, const::std::wstring& replace)
{
	if (text.find(header) == 0)
	{
		text = replace + std::wstring(text.begin() + header.size(), text.end());
		return true;
	}
	return false;
}

void CExtractBrowserDlg::setPaneText()
{
	if (!this_.status || !::IsWindow(this_.status->GetSafeHwnd())) { return; }

	std::wstring word;
	if (!this_.search.empty())
	{
		word = L"キーワード:\"" + this_.search + L"\"";
	}
	else
	{
		std::wstring extract = this_.extract;
		if (replaceText(extract, L"anchor:", L">>"))
		{
			word = L"抽出アンカー:\"" + extract + L"\"";
		}
		else if (replaceText(extract, L"id:", L"ID:"))
		{
			word = L"抽出ID:\"" + extract + L"\"";
		}
		else if(replaceText(extract, L"ref:", L""))
		{
			word = L"参照レス番号:\"" + extract + L"\"";
		}
	}

	CString str;
	if (this_.count > 0)
	{
		str.Format(L"レス数:%d 抽出数:%d", this_.dat.size(), this_.count);
	}
	else
	{
		str.Format(L"レス数:%d", this_.dat.size());
	}

	if (word.empty())
	{
		this_.status->SetPaneText(0, str);
	}
	else
	{
		this_.status->SetPaneText(0, (str + L" " + word.c_str()));
	}
}

void CExtractBrowserDlg::setPaneText(bool read)
{
	if (!this_.status || !::IsWindow(this_.status->GetSafeHwnd())) { return; }

	std::wstring desc;
	int per = 0;
	if (read)
	{
		desc = L"読み込み中";
		int pos = (int)std::distance(this_.dat.begin(), this_.it);
		per = (int)this_.dat.size() == 0 ? 100 : ((pos * 100) / (int)this_.dat.size());
	}
	else
	{
		desc = L"着色中";
		per = this_.count == 0 ? 100 : ((this_.pos * 100) / this_.count);
	}

	if (per < 100 && per > 0)
	{
		CString str;
		str.Format(L"%s %d%%", desc.c_str(), per);
		this_.status->SetPaneText(0, str);
	}
	else
	{
		setPaneText();
	}
}

void CExtractBrowserDlg::setPaneTextNotify()
{
	if (!this_.status || !::IsWindow(this_.status->GetSafeHwnd())) { return; }
	this_.status->SetPaneText(0, L"抽出元のスレッドが読み込まれていないためアンカーの抽出やポップアップ等が機能しません");
}

bool CExtractBrowserDlg::pageExtract()
{
	if (!this_.dhtml) { return false; }

	std::set<int> set;
	this_.dhtml->SearchParent(this_.search, [&](const CComPtr<IHTMLElement>& parent){
		std::wstring header = L"res:";
		std::wstring id;
		if (this_.dhtml->GetParentID(parent, header, id))
		{
			if (id.size() > header.size())
			{
				++this_.count;
				int res = wcstol(id.c_str() + header.size(), NULL, 10);
				if (set.find(res) == set.end())
				{
					set.insert(res);
				}
			}
		}
	});

	if (!set.empty())
	{
		std::vector<int> extract(set.begin(), set.end());
		gl_.callback.extract.push(this);
		gl_.bbs.extract(extract);
		return true;
	}
	return false;
}


bool CExtractBrowserDlg::checkURL()
{
	std::wstring url;
	if (gl_.bbs.getThreadURL(url) && this_.url == url)
	{
		return true;
	}
	return false;
}

void CExtractBrowserDlg::scrollIntoView(int res)
{
	CString anchor;
	anchor.Format(L"res:%d", res);
	CComPtr<IHTMLDocument2> doc2;
	if (SUCCEEDED(this_.dhtml->GetDHtmlDocument(&doc2)))
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


void CExtractBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CCustomDHtmlDialog::DoDataExchange(pDX);
}

BOOL CExtractBrowserDlg::OnInitDialog()
{
	CCustomDHtmlDialog::OnInitDialog();

	this_.count = 0;
	this_.pos = 0;
	gl_.bbs.getThreadURL(this_.url);

	std::wstring board, thread;
	if (gl_.bbs.isThread() && gl_.bbs.getBoardTitle(board) && gl_.bbs.getThreadTitle(thread))
	{
		GetParent()->SetWindowText((L"抽出 【" + board + L"】 " + thread).c_str());
	}
	setPaneText();
	
	DWORD time = GetTickCount();
	if (!this_.search.empty())
	{
		pageExtract();
		gl_.addLog(L"Extract time:%d", GetTickCount() - time);
	}
	else if (!this_.extract.empty())
	{
		gl_.callback.extract.push(this);
		gl_.bbs.extract(this_.extract, true);
	}
	else if (this_.dhtml)
	{
		CComPtr<IHTMLTxtRange> range;
		if (this_.dhtml->GetSelectedTxtRange(range))
		{// 選択範囲
			CComBSTR str;
			if (SUCCEEDED(range->get_text(&str)))
			{// 選択文字列
				this_.search = CString(str).GetString();
				if (!this_.search.empty())
				{
					pageExtract();
				}
			}
		}
		gl_.addLog(L"Extract time:%d", GetTickCount() - time);
	}

	ShowWindow(SW_SHOWNORMAL);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

BEGIN_MESSAGE_MAP(CExtractBrowserDlg, CCustomDHtmlDialog)
	ON_COMMAND(IDM_UPDATE_EXTRACT_END, &CExtractBrowserDlg::OnCallbackExtract)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CExtractBrowserDlg)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONCLICK, _T("a"), OnAnchorMouseClick)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER, _T("a"), OnAnchorMouseOver)
	DHTML_EVENT_TAG((DISPID)DISPID_HTMLELEMENTEVENTS_ONMOUSEOUT, _T("a"), OnAnchorMouseOut)
END_DHTML_EVENT_MAP()



// CExtractBrowserDlg メッセージ ハンドラー


void CExtractBrowserDlg::OnCallbackExtract()
{
	if (!this_.complete)
	{
		this_.callback = true;
		return;
	}

	KillTimer(TIMER_ID_DAT);
	KillTimer(TIMER_ID_SEARCH_COLOR);

	if (!gl_.bbs.getExtract(this_.dat)) { return; }
	this_.it = this_.dat.begin();

	CComPtr<IHTMLDocument2> doc2;
	if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { return; }

	std::wstring noname;
	gl_.bbs.getNoName(noname);
	bbs::BBSSkin skin(gl_.value.detail.skin.thread, noname);

	std::wstring html = skin.header(gl_.html.header) + L"\r\n";

	doc2->write(utl::SafeArray(html));

	SetTimer(TIMER_ID_DAT, 0, NULL);
}

void CExtractBrowserDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
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
			OnCallbackExtract();
		}
	}
}

HRESULT CExtractBrowserDlg::TranslateAccelerator(LPMSG lpMsg, const GUID * pguidCmdGroup, DWORD nCmdID)
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


HRESULT CExtractBrowserDlg::ShowContextMenu(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved)
{
	return utl::ContextMenu::show(this, dwID, ppt, pdispReserved, 1, checkURL(), this_.url);
	//return CCustomDHtmlDialog::ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
}

void CExtractBrowserDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	popup_.onMouseMove();

	CCustomDHtmlDialog::OnMouseMove(nFlags, point);
}


void CExtractBrowserDlg::OnMouseLeave()
{
	popup_.onMouseLeave();

	CCustomDHtmlDialog::OnMouseLeave();
}


void CExtractBrowserDlg::OnDestroy()
{
	CCustomDHtmlDialog::OnDestroy();

	if (::IsWindow(popup_))
	{
		popup_.DestroyWindow();
	}

	utl::BrowserHook::Unsubclass(GetSafeBrowserHwnd());
}

HRESULT CExtractBrowserDlg::OnAnchorMouseClick(IHTMLElement* pElement)
{
	if (!pElement) { return S_OK; }
	if (!this_.dhtml) { return S_OK; }

	CComBSTR name;
	if (!SUCCEEDED(pElement->get_className(&name))) { return S_OK; }

	//gl_.addLog(L"OnAnchorMouseUp:%s", CString(name).GetString());

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
			//		idm = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
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


HRESULT CExtractBrowserDlg::OnAnchorMouseOver(IHTMLElement* pElement)
{
	if (!pElement) { return S_OK; }

	if (GetKeyState(VK_LBUTTON) & 0x8000 || GetKeyState(VK_RBUTTON) & 0x8000)
	{
		return S_OK;
	}

	if (checkURL())
	{
		popup_.popup(pElement, gl_.value.detail.popup);
	}
	else
	{
		setPaneTextNotify();
		this_.notify = true;
	}

	return S_OK;
}

HRESULT CExtractBrowserDlg::OnAnchorMouseOut(IHTMLElement* pElement)
{
	if (!pElement) { return S_OK; }

	if (this_.notify)
	{
		this_.notify = false;
		setPaneText();
	}
	return S_OK;
}



void CExtractBrowserDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_DAT:
		{
			int partial = gl_.value.detail.board.partial;

			setPaneText(true);
			CComPtr<IHTMLDocument2> doc2;
			if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { break; }

			std::wstring noname;
			gl_.bbs.getNoName(noname);
			bbs::BBSSkin skin(gl_.value.detail.skin.thread, noname);
			std::wstring res;
			int count = 0;
			for (;this_.it != this_.dat.end(); ++this_.it)
			{
				res += skin.res(gl_.html.res, *this_.it) + L"\r\n";
				++count;
				if (partial > 0 && count >= partial) { break; }
			}
			doc2->write(utl::SafeArray(res));

			if (this_.it == this_.dat.end())
			{
				KillTimer(TIMER_ID_DAT);
				doc2->close();
				utl::refreshColor(doc2, this_.dat, gl_.value.detail.skin.thread);

				//::RedrawWindow(GetSafeBrowserHwnd(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

				SetTimer(TIMER_ID_SEARCH_COLOR, 0, NULL);
				setPaneText();
				break;
			}
			setPaneText(true);
			ChangeFocus();
			break;
		}
	case TIMER_ID_SEARCH_COLOR:
		{
			setPaneText(false);
			std::wstring color = L"color=#0000ff style=\"background-color:#ffff00\"";
			int count = HighLight(gl_.value.detail.board.partial, this_.begin, this_.end, this_.search, true, color);
			this_.pos += count;

			if (count == 0)
			{
				KillTimer(TIMER_ID_SEARCH_COLOR);
				this_.begin.Release();
				this_.end.Release();
				setPaneText();
				break;
			}
			setPaneText(false);
			break;
		}
	}

	CCustomDHtmlDialog::OnTimer(nIDEvent);
}


void CExtractBrowserDlg::OnSize(UINT nType, int cx, int cy)
{
	if (StoreScroll())
	{
		CCustomDHtmlDialog::OnSize(nType, cx, cy);
		RestoreScroll();
		return;
	}
	CCustomDHtmlDialog::OnSize(nType, cx, cy);
}


BOOL CExtractBrowserDlg::OnEraseBkgnd(CDC* pDC)
{
	::RedrawWindow(GetSafeBrowserHwnd(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	return TRUE;
	//return CCustomDHtmlDialog::OnEraseBkgnd(pDC);
}
