// SearchDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "SearchDlg.h"
#include "afxdialogex.h"

// CSearchDlg ダイアログ

IMPLEMENT_DYNAMIC(CSearchDlg, CDialog)

CSearchDlg::CSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SEARCH, pParent)
{
}

CSearchDlg::~CSearchDlg()
{
}

void CSearchDlg::setSearch(const std::wstring& search)
{
	this_.search = search;

	if (::IsWindow(m_xcEdit))
	{
		m_xcEdit.SetWindowText(CString(search.c_str()));
		m_xcEdit.SetSel(0, -1);
	}
}

void CSearchDlg::setFont(const std::wstring& name, int point)
{
	if (::IsWindow(GetSafeHwnd()) &&
		::IsWindow(m_xcEdit) &&
		::IsWindow(m_xcBtnExtract) &&
		::IsWindow(m_xcBtnPrev) &&
		::IsWindow(m_xcBtnNext) &&
		::IsWindow(status_))
	{
		if (font_.GetSafeHandle()) { font_.DeleteObject(); }
		font_.CreatePointFont(point * 10, name.c_str());

		m_xcEdit.SetFont(&font_);
		m_xcBtnExtract.SetFont(&font_);
		m_xcBtnPrev.SetFont(&font_);
		m_xcBtnNext.SetFont(&font_);
		status_.SetFont(&font_);

		this_.height = utl::getEditHeight(GetSafeHwnd(), font_);
		SizeHelper();
	}
}

void CSearchDlg::SizeHelper()
{
	if (::IsWindow(status_) &&
		::IsWindow(m_xcEdit) &&
		::IsWindow(m_xcBtnExtract) &&
		::IsWindow(m_xcBtnPrev) &&
		::IsWindow(m_xcBtnNext))
	{
		CSize status;
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
		status = status_.CalcFixedLayout(TRUE, TRUE);
		status_.Invalidate();

		// button margin : left, top;
		// margin : right, bottom
		CRect sp(1, 1, 3, 3);
		MapDialogRect(sp);

		CRect rc;
		GetClientRect(rc);

		CRect btn;
		m_xcBtnExtract.GetWindowRect(btn);

		CRect top(rc);
		top.DeflateRect(sp.right, 0);
		top.top = rc.top + sp.bottom;
		top.bottom = top.top + this_.height;

		CRect bottom(rc);
		bottom.DeflateRect(sp.right, 0);
		bottom.top = top.bottom + sp.bottom;
		bottom.bottom = bottom.top + btn.Height();

		CRect edit(top);
		m_xcEdit.MoveWindow(edit);

		CRect extract, prev, next;
		extract = bottom;
		extract.right = extract.left + btn.Width();

		next = bottom;
		next.left = next.right - btn.Width();

		prev = bottom;
		prev.right = next.left - sp.right;
		prev.left = prev.right - btn.Width();

		m_xcBtnExtract.MoveWindow(extract);
		m_xcBtnPrev.MoveWindow(prev);
		m_xcBtnNext.MoveWindow(next);

		RedrawWindow();
	}
}

void CSearchDlg::GetCalcRect(LPRECT pRect)
{
	if (pRect &&
		::IsWindow(status_)&&
		::IsWindow(m_xcEdit) &&
		::IsWindow(m_xcBtnExtract) &&
		::IsWindow(m_xcBtnPrev) &&
		::IsWindow(m_xcBtnNext))
	{
		CRect status;
		status_.GetWindowRect(status);

		// button margin : left, top;
		// margin : right, bottom
		CRect sp(1, 1, 3, 3);
		MapDialogRect(sp);

		CRect btn;
		m_xcBtnExtract.GetWindowRect(btn);

		int width = (btn.Width() * 3) + (sp.right * 4);
		int height = this_.height + btn.Height() + (sp.bottom * 3) + status.Height();

		CRect rc(0, 0, width, height);
		AdjustWindowRectEx(&rc, GetStyle(), FALSE, GetExStyle());

		*pRect = rc;
	}
}

bool CSearchDlg::Search(bool next)
{
	if (!this_.browser) { return false; }

	CString text;
	m_xcEdit.GetWindowText(text);
	std::wstring search(text.GetString());
	if (search.empty())
	{
		status_.SetPaneText(0, L"0/0");
		return false;
	}

	std::wstring url;
	if (!gl_.bbs.getThreadURL(url)) { return false; }
	if (url.empty()) { return false; }

	size_t pos = this_.browser->getPos();

	if (search != this_.search || url != this_.url || pos != this_.pos)
	{// 検索環境が変わったらBookmarkMapを作り直す
		this_.search = search;//検索文字列
		this_.url = url;//スレッドURL
		this_.pos = pos;//ブラウザレス数
		if (!this_.browser->CreateBookmarkMap(search, this_.bookmark))
		{
			status_.SetPaneText(0, L"0/0");
			return false;
		}
	}

	CComBSTR character(L"character");
	CComBSTR textedit(L"textedit");

	CComPtr<IHTMLTxtRange> sel;
	if (!this_.browser->GetSelectedTxtRange(sel)) { return false; }

	long actual;
	if (next)
	{// 選択範囲の終わりから最後までの範囲
		if (!SUCCEEDED(sel->setEndPoint(CComBSTR(L"StartToEnd"), sel)) ||
			!SUCCEEDED(sel->moveEnd(textedit, 1, &actual)))
		{
			return false;
		}
	}
	else
	{// 最初から選択範囲の直前までの範囲
		if (!SUCCEEDED(sel->setEndPoint(CComBSTR(L"EndToStart"), sel)) ||
			!SUCCEEDED(sel->moveStart(textedit, -1, &actual)))
		{
			return false;
		}

		long ret = 0;
		if (SUCCEEDED(sel->compareEndPoints(CComBSTR(L"StartToEnd"), sel, &ret)) &&
			ret == 0)
		{// 始点と終点が同じ(空の範囲)なら全体の範囲に変更する
			if (!SUCCEEDED(sel->moveStart(textedit, -1, &actual)) ||
				!SUCCEEDED(sel->moveEnd(textedit, 1, &actual)))
			{
				return false;
			}
		}
	}

	CComPtr<IHTMLTxtRange> range;
	if (!this_.browser->CreateTxtRange(range)) { return false; }

	long count = 0, flag = 0;
	if (!next) { count = -1; flag = 0; }
	VARIANT_BOOL success = VARIANT_FALSE;
	if (!SUCCEEDED(sel->findText(CComBSTR(search.c_str()), count, flag, &success))) { return false; }
	if (!success)
	{// 見つからなかったらループのため全体の範囲で検索する
		if (!SUCCEEDED(sel->moveStart(textedit, -1, &actual)) ||
			!SUCCEEDED(sel->moveEnd(textedit, 1, &actual)))
		{
			return false;
		}

		if (!SUCCEEDED(sel->findText(CComBSTR(search.c_str()), count, flag, &success))) { return false; }
		if (!success) { return false; }
	}

	sel->scrollIntoView();
	sel->select();

	CComBSTR bookmark;
	if (SUCCEEDED(sel->getBookmark(&bookmark)))
	{// BookmarkMapの中から検索位置を探して表示する
		auto it = this_.bookmark.find(CString(bookmark).GetString());
		if (it == this_.bookmark.end())
		{
			if (!this_.browser->CreateBookmarkMap(search, this_.bookmark))
			{
				status_.SetPaneText(0, L"0/0");
				return false;
			}
			it = this_.bookmark.find(CString(bookmark).GetString());
		}

		if (it != this_.bookmark.end())
		{
			CString str;
			str.Format(L"%d/%u", it->second, this_.bookmark.size());
			status_.SetPaneText(0, str);
		}
		else
		{
			status_.SetPaneText(0, L"");
		}
	}
	return true;
}

void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_xcEdit);
	DDX_Control(pDX, IDC_BTN_EXTRACT, m_xcBtnExtract);
	DDX_Control(pDX, IDC_BTN_PREV, m_xcBtnPrev);
	DDX_Control(pDX, IDC_BTN_NEXT, m_xcBtnNext);
}


BEGIN_MESSAGE_MAP(CSearchDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_EXTRACT, &CSearchDlg::OnBnClickedBtnExtract)
	ON_BN_CLICKED(IDC_BTN_PREV, &CSearchDlg::OnBnClickedBtnPrev)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CSearchDlg::OnBnClickedBtnNext)
END_MESSAGE_MAP()


// CSearchDlg メッセージ ハンドラー


BOOL CSearchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ステータスバー
	if (status_.CreateEx(this, SBARS_SIZEGRIP))
	{
		const static UINT ind[1] = {ID_SEPARATOR};
		if (status_.SetIndicators(ind, 1))
		{
			status_.SetPaneInfo(0, 0, SBPS_STRETCH, 0);
		}
	}

	if (::IsWindow(status_))
	{
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}

	// 選択文字列設定
	if (this_.search.empty())
	{
		if (this_.browser)
		{
			CComPtr<IHTMLTxtRange> range;
			if (this_.browser->GetSelectedTxtRange(range))
			{
				CComBSTR str;
				if (SUCCEEDED(range->get_text(&str)))
				{
					m_xcEdit.SetWindowText(CString(str));
				}
			}
		}
	}
	else
	{
		m_xcEdit.SetWindowText(CString(this_.search.c_str()));
	}

	setFont(gl_.value.detail.font.search.name, gl_.value.detail.font.search.point);

	// サイズ変更枠
	ModifyStyle(WS_MAXIMIZEBOX, WS_THICKFRAME);

	// スタイル変更/サイズの適用
	CRect rc;
	GetCalcRect(&rc);
	SetWindowPos(NULL, 0, 0, cfg_.width, rc.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_FRAMECHANGED);

	CenterWindow(AfxGetMainWnd());

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CSearchDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		CWnd* wnd = GetFocus();
		if (wnd && wnd->GetSafeHwnd() == m_xcEdit.GetSafeHwnd())
		{
			//OnBnClickedBtnNext();
			OnBnClickedBtnExtract();
			DestroyWindow();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CSearchDlg::OnOK()
{
	DestroyWindow();
	//CDialog::OnOK();
}


void CSearchDlg::OnCancel()
{
	DestroyWindow();
	//CDialog::OnCancel();
}


void CSearchDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}

void CSearchDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	CRect rc;
	GetCalcRect(rc);

	switch (fwSide)
	{
	case WMSZ_BOTTOM:
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_RIGHT:
		{
			pRect->bottom = pRect->top + rc.Height();
			if ((pRect->right - pRect->left) < rc.Width())
			{
				pRect->right = pRect->left + rc.Width();
			}
			break;
		}
	case WMSZ_TOP:
	case WMSZ_TOPLEFT:
	case WMSZ_LEFT:
		{
			pRect->top = pRect->bottom - rc.Height();
			if ((pRect->right - pRect->left) < rc.Width())
			{
				pRect->left = pRect->right - rc.Width();
			}
			break;
		}
	case WMSZ_TOPRIGHT:
		{
			pRect->top = pRect->bottom - rc.Height();
			if ((pRect->right - pRect->left) < rc.Width())
			{
				pRect->right = pRect->left + rc.Width();
			}
			break;
		}
	case WMSZ_BOTTOMLEFT:
		{
			pRect->bottom = pRect->top + rc.Height();
			if ((pRect->right - pRect->left) < rc.Width())
			{
				pRect->left = pRect->right - rc.Width();
			}
			break;
		}
	default:
		break;
	}
}

void CSearchDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CRect rc;
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(&wp))
	{
		rc = wp.rcNormalPosition;
	}
	else
	{
		GetWindowRect(rc);
	}

	cfg_.width = rc.Width();
	cfg_.height = rc.Height();

	this_ = This();
}

void CSearchDlg::OnBnClickedBtnExtract()
{
	CString text;
	m_xcEdit.GetWindowText(text);
	std::wstring search = text.GetString();

	if (!search.empty() && this_.browser)
	{
		CExtractDlg* dlg = CExtractDlg::make();
		if (!dlg) { return; }
		dlg->setSearch(search);
		dlg->setCustomDHtmlDialog(this_.browser);
		dlg->Create(IDD_EXTRACT, AfxGetMainWnd());
		dlg->ShowWindow(SW_SHOWNORMAL);
	}
}

void CSearchDlg::OnBnClickedBtnPrev()
{
	Search(false);
}


void CSearchDlg::OnBnClickedBtnNext()
{
	Search(true);
}




