// BBSDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "BBSDlg.h"
#include "afxdialogex.h"

enum TIMER_ID {
	TIMER_ID_POPUP,
	TIMER_ID_CLEAR_STATUS,
	TIMER_ID_CLEAR_POST,
};

// CBBSDlg ダイアログ

IMPLEMENT_DYNAMIC(CBBSDlg, CDialog)

CBBSDlg::CBBSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBBSDlg::IDD, pParent)
	, click_(false)
	, show_(false)
	, icon_(NULL)
	, posting_(false)
{
	icon_ = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_WRITE),
		IMAGE_ICON,
		16, 16,
		LR_DEFAULTCOLOR);

	utl::createFont(font_.main);
	utl::createFont(font_.bold, true);
	utl::createFont(font_.edit);
	
	gl_.peca.setBBSCallback([this](){
		if (IsWindow(this->GetSafeHwnd()))
		{
			this->PostMessage(WM_COMMAND, IDM_UPDATE_CHANNEL_INFO_END);
		}
	});

	gl_.bbs.setPostCallback([this](bool result){
		if (IsWindow(this->GetSafeHwnd()))
		{
			if (result)
			{
				this->PostMessage(WM_COMMAND, IDM_UPDATE_POST_SUCCEEDED);
			}
			else
			{
				this->PostMessage(WM_COMMAND, IDM_UPDATE_POST_FAILED);
			}
		}
	});

	gl_.bbs.setUpdateCallback([this](long target){
		if (IsWindow(this->GetSafeHwnd()))
		{
			switch (target)
			{
			case bbs::UPDATE_COUNTER:		this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_COUNTER); break;
			case bbs::UPDATE_TITLE:			this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_TITLE); break;
			case bbs::UPDATE_STATUS:		this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_STATUS); break;
			case bbs::UPDATE_POST:			this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_POST); break;
			case bbs::UPDATE_DELAY_STATUS:	this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_DELAY_STATUS); break;
			case bbs::UPDATE_DELAY_POST:	this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_DELAY_POST); break;

			case bbs::UPDATE_ALL:
			default:
				this->PostMessage(WM_COMMAND, IDM_UPDATE_BBS_ALL); break;
			}
		}
	});
}

CBBSDlg::~CBBSDlg()
{
	if (icon_) { DestroyIcon(icon_); }
	if (font_.main.GetSafeHandle()) { font_.main.DeleteObject(); }
	if (font_.bold.GetSafeHandle()) { font_.bold.DeleteObject(); }
	if (font_.edit.GetSafeHandle()) { font_.edit.DeleteObject(); }

	gl_.peca.setBBSCallback([](){});
	gl_.bbs.setPostCallback([](bool){});
	gl_.bbs.setUpdateCallback([](long){});
}

void CBBSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_xcEdit);
	DDX_Control(pDX, IDC_BTN_WRITE, m_xcBtnWrite);
}

void CBBSDlg::setShow(bool show, bool edit)
{
	show_ = show;
	ShowWindow(show ? SW_SHOWNA : SW_HIDE);
	EnableWindow(show ? TRUE : FALSE);
	
	if (edit)
	{
		if (show && !posting_)
		{
			enableControl(true);
		}
		else
		{
			enableControl(false);
		}
		if (height_.edit > 0) { return; }
		height_.edit = getEditHeight();
	}
	else
	{
		enableControl(false);

		if (height_.edit <= 0) { return; }
		height_.edit = 0;
	}
}

void CBBSDlg::enableControl(bool enable)
{
	if (enable)
	{
		m_xcBtnWrite.EnableWindow(TRUE);

		if (!m_xcEdit.IsWindowEnabled())
		{
			m_xcEdit.EnableWindow(TRUE);
			m_xcEdit.SetSel(edit_.start, edit_.end);
		}

		SetFocusEdit();
		return;
	}

	if (m_xcEdit.IsWindowEnabled())
	{
		m_xcEdit.GetSel(edit_.start, edit_.end);
		m_xcEdit.EnableWindow(FALSE);
	}
	m_xcBtnWrite.EnableWindow(FALSE);

	SetFocus();
}

bool CBBSDlg::enableIfControl(bool enable)
{
	if (enable && !posting_)
	{
		enableControl(true);
		return true;
	}
	enableControl(false);
	return false;
}

void CBBSDlg::setMessage(const std::wstring& message)
{
	if (::IsWindow(m_xcEdit))
	{
		m_xcEdit.ReplaceSel(message.c_str());
	}
}

void CBBSDlg::SetConfig(const sl::BBSConfig& bbs, const sl::ResConfig& res)
{
	bbs_ = bbs;
	font_.main.DeleteObject();
	font_.main.CreateFontIndirect(&bbs_.font);

	font_.bold.DeleteObject();
	LOGFONT copy = bbs_.font;
	copy.lfWeight = FW_BOLD;
	font_.bold.CreateFontIndirect(&copy);

	font_.edit.DeleteObject();
	font_.edit.CreateFontIndirect(&bbs_.edit.font);

	m_xcEdit.SetFont(&font_.edit);

	height_.bbs = utl::getFontHeight(GetSafeHwnd(), font_.main) + (bbs_.marginY * 2);
	height_.edit = gl_.value.window.show.edit ? getEditHeight() : 0;

	Invalidate(FALSE);

	res_.SetConfig(res, height_.bbs);
}

void CBBSDlg::SetFocusEdit()
{
	// GotoDlgCtrlを使うとテキストが全選択されるので元に戻す
	if (m_xcEdit.IsWindowEnabled())
	{
		int start, end;
		m_xcEdit.GetSel(start, end);
		GotoDlgCtrl(&m_xcEdit);
		m_xcEdit.SetSel(start, end);
	}
}

int CBBSDlg::getEditHeight()
{
	CFont* p = m_xcEdit.GetFont();
	if (p)
	{
		int edit = utl::getEditHeight(GetSafeHwnd(), *p);

		int count = m_xcEdit.GetLineCount();
		if (count <= 1)
		{
			return edit;//gl_.value.display.bbs.edit.height;
		}

		int font = utl::getFontHeight(GetSafeHwnd(), *p);
		return edit + (font * (count - 1));
		//return gl_.value.display.bbs.edit.height + (font * (count - 1));
	}
	return 0;
}

void CBBSDlg::changeEditHeight()
{
	int h = getEditHeight() - height_.edit;

	if (h == 0) { return; }

	height_.edit = getEditHeight();

	CWnd* sub = GetParent();
	if (!sub) { return; }

	CMainDlg* main = (CMainDlg*)sub->GetParent();
	if (!main) { return; }
	main->ExpandWindow(CRect(0, 0, 0, h));
}


void CBBSDlg::OnSizeHelper()
{
	CRect rc;
	GetClientRect(rc);

	if (IsWindow(m_xcEdit.GetSafeHwnd()))
	{
		m_xcEdit.SetWindowPos(NULL,
			0, rc.Height() - height_.edit,
			rc.Width() - BUTTON_WIDTH, height_.edit,
			SWP_NOZORDER|SWP_NOACTIVATE);
		m_xcEdit.RedrawWindow();
	}

	if (IsWindow(m_xcBtnWrite.GetSafeHwnd()))
	{
		m_xcBtnWrite.SetWindowPos(NULL,
			rc.Width() - BUTTON_WIDTH, rc.Height() - height_.edit,
			BUTTON_WIDTH, height_.edit,
			SWP_NOZORDER|SWP_NOACTIVATE);
		m_xcBtnWrite.RedrawWindow();
	}
}

void CBBSDlg::popup(bool show)
{
	if (show)
	{
		gl_.bbs.setBlock(false);

		res_.setPosition();
		res_.Invalidate(FALSE);
		res_.showWindow(true);
	}
	else
	{
		res_.showWindow(false);

		if (!gl_.value.window.show.browser)
		{
			gl_.bbs.setBlock(true);
		}
	}
}

void CBBSDlg::delayPopup()
{
	if (gl_.value.detail.board.delay > 0)
	{
		SetTimer(TIMER_ID_POPUP, gl_.value.detail.board.delay, NULL);
	}
	else
	{
		popup(true);
	}
}


void CBBSDlg::postLog(const std::wstring& name, const std::wstring& mail, const std::wstring& msg)
{
	SYSTEMTIME st;
	::memset(&st, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&st);

	LPCWSTR weeks[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
	LPCWSTR week = (st.wDayOfWeek < 0 || st.wDayOfWeek > 6) ? L"" : weeks[st.wDayOfWeek];

	CString time;
	time.Format(L"%04d/%02d/%02d(%s)%02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, week, st.wHour, st.wMinute, st.wSecond);

	std::wstring bbs, thread, url;
	gl_.bbs.getBoardTitle(bbs);
	gl_.bbs.getThreadTitle(thread);
	gl_.bbs.getThreadURL(url);

	if (!bbs.empty())
	{
		bbs = L"【" + bbs + L"】";
	}

	std::wstring str;
	str += L"----------------------------------------\r\n";
	str += std::wstring(time.GetString()) + L"\r\n";
	str += bbs + thread + L"\r\n";
	str += url + L"\r\n";
	str += L"Name:" + name + L"\r\n";
	str += L"Mail:" + mail + L"\r\n";
	str += msg + L"\r\n";

	std::wstring fullpath = utl::toFullpath(gl_.current, gl_.value.detail.board.path);

	namespace fs = boost::filesystem;
	fs::path p(fullpath);
	std::wstring parent = p.parent_path().wstring();

	DWORD err = ::SHCreateDirectory(NULL, parent.c_str());
	if (ERROR_ALREADY_EXISTS == err ||
		ERROR_FILE_EXISTS == err ||
		ERROR_SUCCESS == err)
	{}

	FILE *file = nullptr;
	if (!::_wfopen_s(&file, fullpath.c_str(), L"ab"))
	{
		std::string cnv = utl::toUTF8(str);

		fwrite(cnv.c_str(), sizeof(char), cnv.size(), file);

		::fclose(file);
	}
}

BEGIN_MESSAGE_MAP(CBBSDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_COMMAND(IDM_UPDATE_CHANNEL_INFO_END, &CBBSDlg::OnUpdateChannelInfoEnd)
	ON_BN_CLICKED(IDC_BTN_WRITE, &CBBSDlg::OnBnClickedBtnWrite)
	ON_EN_CHANGE(IDC_EDIT, &CBBSDlg::OnEnChangeEdit)
	ON_COMMAND(IDM_UPDATE_POST_SUCCEEDED, &CBBSDlg::OnUpdatePostSucceeded)
	ON_COMMAND(IDM_UPDATE_POST_FAILED, &CBBSDlg::OnUpdatePostFailed)
	ON_COMMAND(IDM_UPDATE_BBS_ALL, &CBBSDlg::OnUpdateBbsAll)
	ON_COMMAND(IDM_UPDATE_BBS_COUNTER, &CBBSDlg::OnUpdateBbsCounter)
	ON_COMMAND(IDM_UPDATE_BBS_TITLE, &CBBSDlg::OnUpdateBbsTitle)
	ON_COMMAND(IDM_UPDATE_BBS_STATUS, &CBBSDlg::OnUpdateBbsStatus)
	ON_COMMAND(IDM_UPDATE_BBS_POST, &CBBSDlg::OnUpdateBbsPost)
	ON_COMMAND(IDM_UPDATE_BBS_DELAY_STATUS, &CBBSDlg::OnUpdateBbsDelayStatus)
	ON_COMMAND(IDM_UPDATE_BBS_DELAY_POST, &CBBSDlg::OnUpdateBbsDelayPost)
	ON_COMMAND(IDM_UPDATE_BBS_BROWSER, &CBBSDlg::OnUpdateBbsBrowser)
END_MESSAGE_MAP()


// CBBSDlg メッセージ ハンドラー


BOOL CBBSDlg::OnInitDialog()
{
	
	CDialog::OnInitDialog();

	// 初期表示時に見えないようにする
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOACTIVATE);

	m_xcEdit.SetWindowText(edit_.text);
	m_xcEdit.SetSel(edit_.start, edit_.end);

	m_xcBtnWrite.SetIcon(icon_);

	// レスポップアップ
	res_.Create(CResDlg::IDD, GetDesktopWindow());
	res_.setTarget(this);
	res_.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE);
	//hint_.MoveWindow(0, 0, 300, 18);
	res_.setAlign(DT_LEFT);
	res_.setDat();

	OnSizeHelper();

	if (timer_.status)
	{
		KillTimer(TIMER_ID_CLEAR_STATUS);
		SetTimer(TIMER_ID_CLEAR_STATUS, 3000, NULL);
	}

	if (timer_.post)
	{
		KillTimer(TIMER_ID_CLEAR_POST);
		SetTimer(TIMER_ID_CLEAR_POST, 3000, NULL);
	}

	m_xcEdit.SetFont(&font_.edit);
	m_xcEdit.EnableWindow(posting_ ? FALSE : TRUE);
	m_xcBtnWrite.EnableWindow(posting_ ? FALSE : TRUE);

	// エディットが全選択されるため FALSE を返す
	return FALSE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CBBSDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	// キーメッセージを親に渡す
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		const CWnd* wnd = GetFocus();
		if (wnd && wnd->GetSafeHwnd() == m_xcEdit.GetSafeHwnd())
		{// エディットにフォーカスがあるときは親にキーを渡さない
			DWORD key = static_cast<DWORD>(pMsg->wParam);
			if (key == VK_RETURN)
			{// 書き込み
				int vkey = VK_SHIFT;
				switch (gl_.value.detail.board.shortcut)
				{
				default:
				case sl::BoardConfig::EDIT_SHORTCUT_SHIFT_ENTER: vkey = VK_SHIFT; break;
				case sl::BoardConfig::EDIT_SHORTCUT_CTRL_ENTER: vkey = VK_CONTROL; break;
				case sl::BoardConfig::EDIT_SHORTCUT_ALT_ENTER: vkey = VK_MENU; break;
				}
				
				if (GetAsyncKeyState(vkey) & 0x8000)
				{
					OnBnClickedBtnWrite();
					return TRUE;
				}
			}
			else if (key == VK_ESCAPE)
			{// ダイアログが閉じないようにする
				return TRUE;
			}
			else if (key == VK_TAB)
			{// Tabキーだけ親に渡す
				return FALSE;
			}
			break;
		}
		return FALSE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CBBSDlg::OnDestroy()
{
	CDialog::OnDestroy();

	m_xcEdit.GetWindowText(edit_.text);

	if (m_xcEdit.IsWindowEnabled())
	{
		m_xcEdit.GetSel(edit_.start, edit_.end);
	}

	if (::IsWindow(res_))
	{
		res_.DestroyWindow();
	}
}


BOOL CBBSDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CBBSDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: ここにメッセージ ハンドラー コードを追加します。
	// 描画メッセージで CDialog::OnPaint() を呼び出さないでください。

	CRect org;
	GetClientRect(&org);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, org.Width(), org.Height());
	CBitmap* oldBmp = memDC.SelectObject(&bmp);
	CFont* oldFont = memDC.SelectObject(&font_.main);
	////////////////////////////////////////////////////////////////////////////////////

	CRect rc(org);

	rc.DeflateRect(0, 0, 0, height_.edit);

	utl::drawStatusBar(memDC, &rc, bbs_.color);

	rc.DeflateRect(bbs_.marginX, 0, bbs_.marginX, 0);

	memDC.SetTextColor(bbs_.text[0]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);

	auto drawText = [this, &memDC](const std::wstring text, CRect& rc, DWORD align, bool bold){
		CFont* old = NULL;
		if (bold) { old = memDC.SelectObject(&font_.bold); }
		memDC.DrawText(text.c_str(), -1, rc, DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|align);
		if (bold) { memDC.SelectObject(old); }
	};

	auto calcText = [this, &memDC](const std::wstring text, CRect& rc, DWORD align, bool bold){
		CFont* old = NULL;
		if (bold) { old = memDC.SelectObject(&font_.bold); }
		memDC.DrawText(text.c_str(), -1, rc, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|align);
		if (bold) { memDC.SelectObject(old); }
	};

	// カウンター
	CRect calc;
	if (gl_.value.window.bbs.counter)
	{
		calcText(L"0000", calc, DT_RIGHT, false);
		counter_.width = calc.Width();

		int count = 0;
		if (gl_.bbs.getCounter(count))
		{
			drawText((boost::wformat(L"%d") % count).str(), rc, DT_RIGHT, false);
		}
		else
		{
			drawText(L"---", rc, DT_RIGHT, false);
		}
	}
	else
	{
		counter_.width = 0;
	}
	
	std::wstring title;
	gl_.bbs.getTitle(title);

	std::wstring status;
	gl_.bbs.getStatus(status);
	if (!status.empty()) { status = L" " + status; }

	std::wstring post;
	gl_.bbs.getPost(post);
	if (!post.empty()) { post += L" "; }


	calcText(title, calc, DT_LEFT, bbs_.title.bold);
	int left = max(rc.left, (org.Width() - calc.Width()) / 2);
	int right = min(rc.right - counter_.width, (org.Width() + calc.Width()) / 2);
	right = max(rc.left, right);

	CRect rcCenter(rc);
	rcCenter.left = left;
	rcCenter.right = right;

	//-----------------------------------------------
	calcText(status, calc, DT_LEFT, false);
	left = max(rc.left, rc.right - counter_.width - calc.Width());
	right = max(rc.left, rc.right - counter_.width);

	CRect rcRight(rc);
	rcRight.left = left;
	rcRight.right = right;

	//-----------------------------------------------
	calcText(post, calc, DT_LEFT, false);
	left = rc.left;
	right = min(rc.left + calc.Width(), rcRight.left);

	CRect rcLeft(rc);
	rcLeft.left = left;
	rcLeft.right = right;

	//-----------------------------------------------
	rcCenter.left = max(rcCenter.left, rcLeft.right);
	rcCenter.right = rcRight.left;


	if (gl_.bbs.isStop())
	{
		COLORREF oldColor = memDC.GetTextColor();
		memDC.SetTextColor(bbs_.title.text[0]);
		drawText(title, rcCenter, DT_LEFT, bbs_.title.bold);
		memDC.SetTextColor(oldColor);
	}
	else
	{
		drawText(title, rcCenter, DT_LEFT, bbs_.title.bold);
	}

	drawText(status, rcRight, DT_LEFT, false);
	drawText(post, rcLeft, DT_LEFT, false);


	///////////////////////////////////////////////////////////////
	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}


void CBBSDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	OnSizeHelper();
	Invalidate(FALSE);
}


void CBBSDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_POPUP:
		{
			popup(true);
			KillTimer(TIMER_ID_POPUP);
			break;
		}
	case TIMER_ID_CLEAR_STATUS:
		{
			gl_.bbs.setStatus(L"");
			Invalidate(FALSE);
			KillTimer(TIMER_ID_CLEAR_STATUS);
			timer_.status = false;
			break;
		}
	case TIMER_ID_CLEAR_POST:
		{
			gl_.bbs.setPost(L"");
			Invalidate(FALSE);
			KillTimer(TIMER_ID_CLEAR_POST);
			timer_.post = false;
			break;
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CBBSDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// マウス監視開始(OnMouseLeave)
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = GetSafeHwnd();
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);
	
	CRect org;
	GetClientRect(org);

	CRect rc(org);
	rc.DeflateRect(bbs_.marginX, 0, bbs_.marginX, height_.edit);

	CRect crc(rc);
	crc.left = max(rc.left, rc.right - counter_.width);
	if (PtInRect(crc, point))
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
		delayPopup();
		return;
	}
	
	if (PtInRect(rc, point))
	{
		delayPopup();
		return;
	}

	if (res_.IsWindowVisible())
	{
		popup(false);
	}
	
	KillTimer(TIMER_ID_POPUP);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	
	CDialog::OnMouseMove(nFlags, point);
}


void CBBSDlg::OnMouseLeave()
{
	CPoint pt;
	if (GetCursorPos(&pt))
	{
		CWnd* wnd = WindowFromPoint(pt);
		if (wnd)
		{
			if (wnd->GetSafeHwnd() == res_.GetSafeHwnd())
			{
				return;
			}
		}
	}

	KillTimer(TIMER_ID_POPUP);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	popup(false);
	CDialog::OnMouseLeave();
}


void CBBSDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_xcEdit.IsWindowEnabled())
	{
		int start, end;
		m_xcEdit.GetSel(start, end);

		SetFocus();

		GotoDlgCtrl(&m_xcEdit);
		m_xcEdit.SetSel(start, end);
	}

	CRect org;
	GetClientRect(org);

	CRect rc(org);
	rc.DeflateRect(bbs_.marginX, 0, bbs_.marginX, height_.edit);

	CRect crc(rc);
	crc.left = max(rc.left, rc.right - counter_.width);
	if (PtInRect(crc, point))
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
		click_ = true;
		SendMessage(WM_COMMAND, IDM_THREAD_RELOAD);
		return;
	}

	if (PtInRect(rc, point))
	{
		KillTimer(TIMER_ID_POPUP);
		popup(false);
		SendMessage(WM_COMMAND, IDM_THREAD);
		return;
	}

	CDialog::OnLButtonDown(nFlags, point);
}


void CBBSDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (click_)
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
		click_ = false;
	}
	//CDialog::OnLButtonUp(nFlags, point);
}


void CBBSDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags, point);

	//CDialog::OnLButtonDblClk(nFlags, point);
}

void CBBSDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// フォーカスを移して変換を確定する
	// m_xcEditにフォーカスを戻す
	if (m_xcEdit.IsWindowEnabled())
	{
		int start, end;
		m_xcEdit.GetSel(start, end);

		SetFocus();

		GotoDlgCtrl(&m_xcEdit);
		m_xcEdit.SetSel(start, end);
	}

	CDialog::OnRButtonDown(nFlags, point);
}


void CBBSDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect org;
	GetClientRect(org);

	CRect rc(org);
	rc.bottom = rc.top + height_.bbs;

	if (PtInRect(rc, point))
	{
		KillTimer(TIMER_ID_POPUP);
		popup(false);
		SendMessage(WM_COMMAND, IDM_BBS_MENU);
		return;
	}

	CDialog::OnRButtonUp(nFlags, point);
}

void CBBSDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (gl_.value.operation.mouse.bbs.transfer)
	{// 画面上操作へ委譲
	 // 親座標に変換
		ClientToScreen(&point);
		reinterpret_cast<CSubpaneDlg*>(GetParent())->ScreenToClient(&point);
		reinterpret_cast<CSubpaneDlg*>(GetParent())->OnMButtonDown(nFlags, point);
		return;
	}

	CSubpaneDlg* sub = reinterpret_cast<CSubpaneDlg*>(GetParent());
	if (!sub) { return; }
	CMainDlg* main = reinterpret_cast<CMainDlg*>(sub->GetParent());
	if (!main) { return; }

	main->MouseMClickHelper(gl_.value.operation.mouse.bbs.mclick);

	//CDialog::OnMButtonDown(nFlags, point);
}

void CBBSDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}

void CBBSDlg::OnUpdateChannelInfoEnd()
{
	if (gl_.value.detail.board.contact)
	{
		std::wstring url;
		if (gl_.peca.getContactURL(url))
		{
			if (!gl_.latest.commandline || gl_.latest.contact != url)
			{// コンタクトURL引数がない、もしくは引数とチャンネル情報が違う
				gl_.bbs.execute(url, gl_.value.detail.board, gl_.value.detail.board.contact, true);
			}
		}
	}
}


void CBBSDlg::OnBnClickedBtnWrite()
{
	if (posting_) { return; }

	CString text;
	m_xcEdit.GetWindowText(text);

	std::wstring mail = gl_.value.dialog.bbs.sage ? L"sage" : L"";
	gl_.bbs.post(L"", mail, std::wstring(text));

	posting_ = true;

	enableIfControl(false);
}


void CBBSDlg::OnEnChangeEdit()
{
	if (!m_xcEdit.IsWindowEnabled()) { return; }

	changeEditHeight();
}



void CBBSDlg::OnUpdatePostSucceeded()
{
	if (gl_.value.detail.board.log)
	{
		CString msg;
		m_xcEdit.GetWindowText(msg);
		postLog(L"", gl_.value.dialog.bbs.sage ? L"sage" : L"", (LPCWSTR)msg);
	}

	m_xcEdit.SetWindowText(L"");
	changeEditHeight();

	posting_ = false; // 無効にしてからコントロールを有効に
	enableIfControl(true);

	if (gl_.value.detail.board.hide)
	{
		CWnd* sub = GetParent();
		if (!sub) { return; }

		CMainDlg* main = (CMainDlg*)sub->GetParent();
		if (!main) { return; }
		main->OnWindowEdit();
	}
}


void CBBSDlg::OnUpdatePostFailed()
{
	posting_ = false; // 無効にしてからコントロールを有効に
	enableIfControl(true);
}



void CBBSDlg::OnUpdateBbsAll()
{
	Invalidate(FALSE);
}

void CBBSDlg::OnUpdateBbsCounter()
{
	Invalidate(FALSE);
}

void CBBSDlg::OnUpdateBbsTitle()
{
	Invalidate(FALSE);
}

void CBBSDlg::OnUpdateBbsStatus()
{
	Invalidate(FALSE);
	KillTimer(TIMER_ID_CLEAR_STATUS);
	timer_.status = false;
}


void CBBSDlg::OnUpdateBbsPost()
{
	Invalidate(FALSE);
	KillTimer(TIMER_ID_CLEAR_POST);
	timer_.post = false;
}


void CBBSDlg::OnUpdateBbsDelayStatus()
{
	Invalidate(FALSE);
	KillTimer(TIMER_ID_CLEAR_STATUS);
	SetTimer(TIMER_ID_CLEAR_STATUS, 3000, NULL);
	timer_.status = true;
}


void CBBSDlg::OnUpdateBbsDelayPost()
{
	Invalidate(FALSE);
	KillTimer(TIMER_ID_CLEAR_POST);
	SetTimer(TIMER_ID_CLEAR_POST, 3000, NULL);
	timer_.post = true;
}

void CBBSDlg::OnUpdateBbsBrowser()
{
	CMainDlg* main = (CMainDlg*)AfxGetMainWnd();
	if (main)
	{
		CBrowserDlg* browser = (CBrowserDlg*)main->GetLayout()->GetSub();
		if (browser)
		{
			std::wstring text;
			if (browser->getStatus(text))
			{
				gl_.bbs.setStatus(text);
			}
			else
			{
				gl_.bbs.setStatus(L"");
			}
			Invalidate(FALSE);
			KillTimer(TIMER_ID_CLEAR_STATUS);
			timer_.status = true;
		}
	}
}
