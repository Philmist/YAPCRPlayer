// SeekDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "SeekDlg.h"
//#include "afxdialogex.h"


enum TIMER_ID {
	TIMER_ID_UPDATE,
};

// CSeekDlg ダイアログ

IMPLEMENT_DYNAMIC(CSeekDlg, CDialog)

CSeekDlg::CSeekDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSeekDlg::IDD, pParent)
	, height_(16)
	, show_(false)
	, capture_(false)
{
	utl::createFont(font_);
}

CSeekDlg::~CSeekDlg()
{
	if (font_.GetSafeHandle()) { font_.DeleteObject(); }
}



LONGLONG CSeekDlg::getCursorTime()
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	CRect rc;
	GetClientRect(&rc);

	return getCursorTime(rc, pt);
}

LONGLONG CSeekDlg::getCursorTime(const RECT& rc, const POINT& pt)
{
	LONG right = (rc.right - 2); // 両端から-1
	if (right > 0)
	{
		LONGLONG dur = 0;
		gl_.player.getDuration(dur);

		if (pt.x >= rc.left && pt.x <= right)
		{
			double per = static_cast<double>(pt.x) / static_cast<double>(right);
			LONGLONG sel = static_cast<LONGLONG>(static_cast<double>(dur) * per);
			
			return sel;
		}
		else if (pt.x < rc.left)
		{
			return 0;
		}
		else if (pt.x > right)
		{
			return dur;
		}
	}
	return 0;
}

void CSeekDlg::SetConfig(const sl::SeekConfig& seek, const sl::HintConfig& hint)
{
	seek_ = seek;
	font_.DeleteObject();
	font_.CreateFontIndirect(&seek_.font);

	height_ = utl::getFontHeight(GetSafeHwnd(), font_) + (seek_.marginY * 2);

	Invalidate(FALSE);

	hint_.SetConfig(hint);
}


void CSeekDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSeekDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CSeekDlg メッセージ ハンドラー


BOOL CSeekDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 初期表示時に見えないようにする
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOACTIVATE);

	// ヒントダイアログ
	hint_.Create(CHintDlg::IDD, GetDesktopWindow());
	hint_.SetWindowPos(NULL, 0, 0, 300, 18, SWP_NOZORDER|SWP_NOACTIVATE);
	//hint_.MoveWindow(0, 0, 300, 18);

	SetTimer(TIMER_ID_UPDATE, 100, NULL); // 更新用タイマー

	SetFocus(); // IMEDisable
	utl::IMEDisable();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CSeekDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return FALSE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CSeekDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CSeekDlg::OnPaint()
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
	CFont* oldFont = memDC.SelectObject(&font_);
	///////////////////////////////////////////////////////////////
	int pos = 0;
	bool time = false;
	LONGLONG dur = 0, cur = 0;
	if (gl_.player.getDuration(dur) && gl_.player.getCurrentPosition(cur))
	{
		// ステータスバーが非表示なら時間表示
		time = !gl_.value.window.show.status;

		CRect rc(org);
		
		LONG right = (rc.right - 1); // 右端-1 (pos + 1 左端+1)
		if (capture_)
		{
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			if (pt.x >= rc.left && pt.x <= right) { pos = pt.x; }
			else if (pt.x < rc.left) { pos = rc.left; }
			else if (pt.x > right) { pos = right; }
		}
		else if (dur > 0)
		{
			double per = static_cast<double>(cur) / static_cast<double>(dur);
			pos = static_cast<LONG>(static_cast<double>(right) * per);
		}

		utl::drawSeekBar(memDC, rc, pos + 1, seek_.knob, seek_.color);
	}
	else
	{
		utl::drawSeekBar(memDC, org, 0, seek_.knob, seek_.color);
	}
	///////////////////////////////////////////////////////////////
	memDC.SetTextColor(seek_.text[0]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);
	CRect calc;
	CRect rc(org);
	rc.DeflateRect(seek_.marginX, 0, seek_.marginX, 0);

	std::wstring text;


	if (time)
	{
		time = true;
		text = utl::convertTimes(cur, false) + L" / " + utl::convertTimes(dur, false);

		memDC.DrawText(text.c_str(), -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

		memDC.DrawText(text.c_str(), -1, &calc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_CALCRECT);
	}
	else if (datetime_.valid())
	{
		boost::posix_time::ptime dtime = datetime_.get(cur);
		text = utl::convertDate(dtime, gl_.value.display.datetime.dateFormat, gl_.value.display.datetime.timeFormat);

		memDC.DrawText(text.c_str(), -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

		memDC.DrawText(text.c_str(), -1, &calc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_CALCRECT);
	}


	CRect lrc(rc);
	bool filename = gl_.value.window.seek.filename && !filename_.empty();
	if (filename)
	{
		lrc.right = max(rc.left, rc.right - calc.Width());
		memDC.DrawText(filename_.c_str(), -1, &lrc, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
	}


	{//----------------------------------------------------------------------------------------
		CRect rc(org);
		rc.right = pos + 1;
		dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	}//----------------------------------------------------------------------------------------


	memDC.SetTextColor(seek_.text[1]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);

	if (time)
	{
		memDC.DrawText(text.c_str(), -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	}
	else if (datetime_.valid())
	{
		memDC.DrawText(text.c_str(), -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	}


	if (filename)
	{
		memDC.DrawText(filename_.c_str(), -1, &lrc, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
	}

	org.left = pos + 1;
	dc.BitBlt(org.left, org.top, org.Width(), org.Height(), &memDC, org.left, org.top, SRCCOPY);

	///////////////////////////////////////////////////////////////
	
	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}


void CSeekDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	Invalidate(FALSE);
}

void CSeekDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!gl_.value.detail.board.focus)
	{
		SetFocus(); // BBS
	}

	if (!capture_)
	{
		SetCapture();
		capture_ = true;
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return;
	}
	//CDialog::OnLButtonDown(nFlags, point);
}


void CSeekDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (capture_)
	{
		gl_.player.seek(getCursorTime());
		ReleaseCapture();
		capture_ = false;
	}

	CRect rect;
	GetClientRect(rect);

	if (!PtInRect(rect, point))
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	//CDialog::OnLButtonUp(nFlags, point);
}

void CSeekDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags, point);

	//CDialog::OnLButtonDblClk(nFlags, point);
}

void CSeekDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_UPDATE:
		{
			if (!IsWindowVisible()) { break; }
			Invalidate(FALSE);
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}




void CSeekDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	if (PtInRect(rect, point) || capture_)
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
		if (!capture_)
		{
			// マウス監視開始(OnMouseLeave)
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = GetSafeHwnd();
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);

			if (!gl_.value.detail.board.focus)
			{
				SetFocus(); // BBS
			}
		}

		ClientToScreen(rect);

		std::wstring text;
		LONGLONG cur = getCursorTime();

		if (datetime_.valid())
		{
			boost::posix_time::ptime time = datetime_.get(cur);
		
			text = utl::convertDate(time, gl_.value.display.datetime.dateFormat, gl_.value.display.datetime.timeFormat)
				+ L"\r\n" + utl::convertTimes(cur, false);
		}
		else
		{
			text = utl::convertTimes(cur, false);
		}

		long pos = 0;
		long right = rect.right - 1;
		if (rect.left + point.x < rect.left) { pos = rect.left; }
		else if (rect.left + point.x > right) { pos = right; }
		else { pos = rect.left + point.x; }

		hint_.setText(text);
		hint_.setPosition(pos, rect.top);
		hint_.Invalidate(FALSE);
		hint_.showWindow(true);
		Invalidate(FALSE);
		UpdateWindow();
	}
	else
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		hint_.showWindow(false);
	}

	CDialog::OnMouseMove(nFlags, point);
}


void CSeekDlg::OnMouseLeave()
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	hint_.showWindow(false);
	CDialog::OnMouseLeave();
}


void CSeekDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (gl_.value.operation.mouse.seek.transfer)
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

	main->MouseMClickHelper(gl_.value.operation.mouse.seek.mclick);

	//CDialog::OnMButtonDown(nFlags, point);
}

void CSeekDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}


void CSeekDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (::IsWindow(hint_))
	{
		hint_.DestroyWindow();
	}
}
