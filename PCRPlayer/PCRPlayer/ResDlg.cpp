// ResDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ResDlg.h"
//#include "afxdialogex.h"


// CResDlg ダイアログ

IMPLEMENT_DYNAMIC(CResDlg, CDialog)

CResDlg::CResDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResDlg::IDD, pParent)
	, target_(NULL)
	, align_(DT_CENTER)
	, height_(0)
{
	utl::createFont(font_.normal);
	utl::createFont(font_.bold.normal, true);
	utl::createFont(font_.underline, false, true);
	utl::createFont(font_.bold.underline, true, true);

	//gl_.bbs.setDatCallback([&](){
	//	if (IsWindow(this->GetSafeHwnd()))
	//	{
	//		this->PostMessage(WM_COMMAND, IDM_UPDATE_RES_END);
	//	}
	//});
	gl_.callback.dat.push(this);
}

CResDlg::~CResDlg()
{
	if (font_.normal.GetSafeHandle()) { font_.normal.DeleteObject(); }
	if (font_.bold.normal.GetSafeHandle()) { font_.bold.normal.DeleteObject(); }
	if (font_.underline.GetSafeHandle()) { font_.underline.DeleteObject(); }
	if (font_.bold.underline.GetSafeHandle()) { font_.bold.underline.DeleteObject(); }

	//gl_.bbs.setDatCallback([](){});
	gl_.callback.dat.pop(this);
}

void CResDlg::SetConfig(const sl::ResConfig& cfg, long height)
{
	res_ = cfg;
	height_ = height;

	font_.normal.DeleteObject();
	font_.normal.CreateFontIndirect(&res_.format.font);

	LOGFONT copy = {};
	font_.bold.normal.DeleteObject();
	copy = res_.format.font;
	copy.lfWeight = FW_BOLD;
	font_.bold.normal.CreateFontIndirect(&copy);

	font_.underline.DeleteObject();
	copy = res_.format.font;
	copy.lfUnderline = TRUE;
	font_.underline.CreateFontIndirect(&copy);

	font_.bold.underline.DeleteObject();
	copy = res_.format.font;
	copy.lfWeight = FW_BOLD;
	copy.lfUnderline = TRUE;
	font_.bold.underline.CreateFontIndirect(&copy);

	DestroyWindow();
	Create(IDD, GetDesktopWindow());

	if (!res_.back.opaque.enable)
	{
		SetLayeredWindowAttributes(0, res_.back.opaque.back, LWA_ALPHA);
	}

	OnUpdateResEnd();
}

void CResDlg::draw(CDC& dc, const CRect& org, bool mask, int text, int back)
{
	dc.SetTextColor(RGB(255, 255, 255));
	dc.SetBkColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);

	CPoint offset(res_.back.marginX, res_.back.marginY);

	if (mask)
	{
		dc.FillRect(org, &CBrush(RGB(back, back, back)));
		layout_.mask(dc, offset, RGB(text, text, text));
	}
	else
	{
		utl::drawFrameRect(dc, org, res_.back.color);
		if (res_.box.punct)
		{
			switch (res_.format.type)
			{
			default:
			case sl::ResConfig::FORMAT_NORMAL1:
			case sl::ResConfig::FORMAT_NORMAL2:
				layout_.drawBox(dc, offset, 0, 0);
				layout_.drawBox(dc, offset, 1, 1);
				layout_.drawText(dc, offset);
				break;
			case sl::ResConfig::FORMAT_SIMPLE1:
			case sl::ResConfig::FORMAT_SIMPLE2:
			case sl::ResConfig::FORMAT_COMPACT1:
			case sl::ResConfig::FORMAT_COMPACT2:
				layout_.drawBox(dc, offset, 0, 1);
				layout_.drawBox(dc, offset, 2, 2);
				layout_.drawText(dc, offset);
				break;
			}
		}
		else
		{
			layout_.drawBox(dc, offset);
			layout_.drawText(dc, offset);
		}
	}
}


void CResDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_COMMAND(IDM_UPDATE_RES_END, &CResDlg::OnUpdateResEnd)
	ON_COMMAND(IDM_UPDATE_SCROLL_END, &CResDlg::OnUpdateScrollEnd)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


// CResDlg メッセージ ハンドラー

BOOL CResDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	size_.SetSize(0, 0);

	if (!res_.back.opaque.enable)
	{
		SetLayeredWindowAttributes(0, res_.back.opaque.back, LWA_ALPHA);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CResDlg::PreTranslateMessage(MSG* pMsg)
{
	// EnterとESCを押すと終了してしまうので処理をつぶす
    if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
        return FALSE;
    }
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CResDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CResDlg::OnPaint()
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
	CFont* oldFont = memDC.SelectObject(&font_.normal);
	////////////////////////////////////////////////////////////////////////////////////

	utl::drawFrameRect(memDC, &org, res_.back.color);

	memDC.SetTextColor(RGB(0, 0, 0));
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);

	CPoint offset(res_.back.marginX, res_.back.marginY);
	if (res_.box.punct)
	{
		switch (res_.format.type)
		{
		default:
		case sl::ResConfig::FORMAT_NORMAL1:
		case sl::ResConfig::FORMAT_NORMAL2:
			layout_.drawBox(memDC, offset, 0, 0);
			layout_.drawBox(memDC, offset, 1, 1);
			layout_.drawText(memDC, offset);
			break;
		case sl::ResConfig::FORMAT_SIMPLE1:
		case sl::ResConfig::FORMAT_SIMPLE2:
		case sl::ResConfig::FORMAT_COMPACT1:
		case sl::ResConfig::FORMAT_COMPACT2:
			layout_.drawBox(memDC, offset, 0, 1);
			layout_.drawBox(memDC, offset, 2, 2);
			layout_.drawText(memDC, offset);
			break;
		}
	}
	else
	{
		layout_.drawBox(memDC, offset);
		layout_.drawText(memDC, offset);
	}
	///////////////////////////////////////////////////////////////
	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}

void CResDlg::setAlign(DWORD align)
{
	align_ = align;
}

void CResDlg::setDat()
{
	std::vector<bbs::ResInfo> dat;
	gl_.bbs.getDat(dat);
	if (gl_.value.detail.board.order)
	{
		std::reverse(dat.begin(), dat.end());
	}
	setDat(dat);
}

void CResDlg::setDat(const std::vector<bbs::ResInfo>& dat)
{
	if (dat.empty())
	{
		size_.SetSize(0, 0);
		return;
	}

	CClientDC dc(this);
	CFont* fontOld = dc.SelectObject(&font_.normal);

	auto format = [](const std::wstring& datetime, bool second)->std::wstring{
		using namespace boost::xpressive;
		try {
			const static wsregex rx = wsregex::compile(L"([0-9]+:[0-9]+)(:[0-9]+)?");
			wsmatch what;
			if (regex_search(datetime, what, rx))
			{
				if (second)
				{
					return what[1].str() + what[2].str();
				}
				return what[1].str();
			}
		} catch (...) {}
		return datetime;
	};

	auto bold = [this](int index){ return res_.color.bold[index] ? &font_.bold.normal : NULL; };
	auto text = [this](int index){ return res_.color.text[index]; };
	auto link = [this](int index){ return res_.color.link[index]; };
	auto sage = [this](int index){ return res_.color.sage[index]; };

	std::wstring punct;
	switch(res_.format.punct)
	{
	case sl::ResConfig::PUNCT_NONE: break;
	case sl::ResConfig::PUNCT_SPACE: punct = L" "; break;
	case sl::ResConfig::PUNCT_COLON:
	default: punct = L" : "; break;
	}

	std::wstring noname;
	gl_.bbs.getNoName(noname);
	boost::xpressive::wsregex nonamerx;
	
	try {
		nonamerx =
			boost::xpressive::wsregex::compile(L"^\\s*") >>
			boost::xpressive::as_xpr(noname) >>
			boost::xpressive::wsregex::compile(L"\\s*$");
	} catch(...) {}

	auto erase = [&](bool enable, const std::wstring& str)->std::wstring{
		if (!enable) { return str; }

		using namespace boost::xpressive;

		try {
			return regex_replace(str, nonamerx, L"");
		}
		catch (...) {}
		return str;
	};


	int space = res_.format.space;
	int width = res_.format.width;
	bool ids = res_.id.enable;
	int threshold = res_.id.threshold;
	int marginX = res_.box.marginX;
	int marginY = res_.box.marginY;
	int punctX = res_.box.punct ? res_.box.marginX : 0;
	int punctY = res_.box.punct ? res_.box.marginY : 0;
	bool trim = false;
	
	boost::function<COLORREF(int)> color;
	boost::function<COLORREF(int)> idColor;

	layout_ = layout::GridLayout();
	for (auto it = dat.begin(); it != dat.end(); ++it)
	{
		if (it->mail.empty()) { color = text; }
		else if (it->mail == L"sage") { color = sage; }
		else { color = link; }
		std::array<COLORREF, 6>& boxColor =
			gl_.value.detail.board.newly && it->latest ? res_.box.latest : res_.box.color;

		switch (res_.format.type)
		{
		default:
		case sl::ResConfig::FORMAT_NORMAL1:
		case sl::ResConfig::FORMAT_NORMAL2:
			{
				layout::TextLayout head;
				layout::TextLayout line;
				layout::GridRow row;

				head.addX(dc, it->number, color(0), bold(0));
				head.addX(dc, punct, color(8), bold(8));

				bool enable = res_.format.type == sl::ResConfig::FORMAT_NORMAL2;
				if (it->split.empty())
				{
					std::wstring str = erase(enable, it->name);
					if (!str.empty())
					{
						line.addX(dc, str + L" ", color(1), bold(1));
					}
				}
				else
				{
					bool empty = true;
					for (auto jt = it->split.begin(); jt != it->split.end(); ++jt)
					{
						std::wstring str = erase(enable, jt->first);
						if (!str.empty())
						{
							empty = false;
							line.addX(dc, str, color(1), jt->second ? bold(1) : NULL);
						}
					}

					if (!empty)
					{
						line.addX(dc, L" ", color(1), bold(1));
					}
				}

				line.addX(dc, L"[", color(9), bold(9));
				line.addX(dc, it->mail, color(2), bold(2));
				line.addX(dc, L"] ", color(9), bold(9));
				line.addX(dc, it->datetime, color(3), bold(3));

				if (!it->id.empty())
				{
					if (it->total <= 1) { idColor = text; }
					else if (it->total >= threshold) { idColor = sage; }
					else { idColor = link; }

					line.addX(dc, L" ID:", idColor(10), bold(10));
					line.addX(dc, it->id, idColor(4), bold(4));

					if (ids && it->total > 1)
					{
						line.addX(dc, (boost::wformat(L" [%d/%d]") % it->count % it->total).str(), idColor(4), bold(4));
					}
				}

				line.add(dc, it->message, color(5), bold(5), space, width, trim);
				row.add(head, boxColor, CRect(marginX, marginY, punctX, marginY), layout::GRID_ALIGN_TOP_RIGHT);
				row.add(line, boxColor, CRect(punctX, marginY, marginX, marginY), layout::GRID_ALIGN_LEFT_TOP, res_.box.separate);
				layout_.add(row, res_.box.space);
				break;
			}
		case sl::ResConfig::FORMAT_SIMPLE1:
		case sl::ResConfig::FORMAT_SIMPLE2:
			{
				layout::TextLayout top;
				layout::TextLayout head;
				layout::TextLayout line;
				layout::GridRow row;
				bool enable = res_.format.type == sl::ResConfig::FORMAT_SIMPLE2;

				top.addX(dc, it->number + L" ", color(6), bold(6));
				head.addX(dc, format(it->datetime, res_.format.second), color(7), bold(7));

				if (!it->id.empty())
				{
					if (it->total <= 1) { idColor = text; }
					else if (it->total >= threshold) { idColor = sage; }
					else { idColor = link; }

					head.addX(dc, L" ID:", idColor(10), bold(10));
					head.addX(dc, it->id, idColor(4), bold(4));

					if (ids && it->total > 1)
					{
						head.addX(dc, (boost::wformat(L" [%d/%d]") % it->count % it->total).str(), idColor(4), bold(4));
					}
				}

				head.addX(dc, punct, color(8), bold(8));

				if (it->split.empty())
				{
					std::wstring str = erase(enable, it->name);
					if (!str.empty())
					{
						line.addX(dc, str, color(1), bold(1));
					}
				}
				else
				{
					for (auto jt = it->split.begin(); jt != it->split.end(); ++jt)
					{
						std::wstring str = erase(enable, jt->first);
						if (!str.empty())
						{
							line.addX(dc, str, color(1), jt->second ? bold(1) : NULL);
						}
					}
				}
				line.add(dc, it->message, color(5), bold(5), space, width, trim);

				row.add(top, boxColor, CRect(marginX, marginY, 0, marginY), layout::GRID_ALIGN_TOP_RIGHT);
				row.add(head, boxColor, CRect(0, marginY, punctX, marginY));
				row.add(line, boxColor, CRect(punctX, marginY, marginX, marginY), layout::GRID_ALIGN_LEFT_TOP, res_.box.separate);
				layout_.add(row, res_.box.space);
				break;
			}
		case sl::ResConfig::FORMAT_COMPACT1:
		case sl::ResConfig::FORMAT_COMPACT2:
			{
				layout::TextLayout top;
				layout::TextLayout head;
				layout::TextLayout line;
				layout::GridRow row;
				bool enable = res_.format.type == sl::ResConfig::FORMAT_COMPACT2;

				top.addX(dc, it->number + L" ", color(6), bold(6));
				head.addX(dc, format(it->datetime, res_.format.second), color(7), bold(7));
				head.addX(dc, punct, color(8), bold(8));

				if (it->split.empty())
				{
					std::wstring str = erase(enable, it->name);
					if (!str.empty())
					{
						line.addX(dc, str, color(1), bold(1));
					}
				}
				else
				{
					for (auto jt = it->split.begin(); jt != it->split.end(); ++jt)
					{
						std::wstring str = erase(enable, jt->first);
						if (!str.empty())
						{
							line.addX(dc, str, color(1), jt->second ? bold(1) : NULL);
						}
					}
				}
				line.add(dc, it->message, color(5), bold(5), space, width, trim);

				row.add(top, boxColor, CRect(marginX, marginY, 0, marginY), layout::GRID_ALIGN_TOP_RIGHT);
				row.add(head, boxColor, CRect(0, marginY, punctX, marginY));
				row.add(line, boxColor, CRect(punctX, marginY, marginX, marginY), layout::GRID_ALIGN_LEFT_TOP, res_.box.separate);
				layout_.add(row, res_.box.space);
				break;
			}
		}
	}

	CRect rc(layout_.outline());
	rc.InflateRect(res_.back.marginX, res_.back.marginY);
	size_.cx = rc.Width();
	size_.cy = rc.Height();

	if (res_.back.opaque.enable)
	{
		CRect org(0, 0, size_.cx, size_.cy);
		utl::LayeredWindow layer(size_.cx, size_.cy, font_.normal);

		if (layer)
		{
			draw(layer.getImageCDC(), org);
			draw(layer.getMaskCDC(), org, true, res_.back.opaque.text, res_.back.opaque.back);
			if (layer.create())
			{
				layer.update(GetSafeHwnd(), dc);
			}
		}
	}

	dc.SelectObject(fontOld);
}

int CResDlg::calc(const CRect& rc, long y)
{
	int ay = y - size_.cy;
	if (gl_.value.detail.board.under)
	{
		ay = y + height_;
	}

	if (gl_.value.detail.board.align)
	{
		if (gl_.value.detail.board.order)
		{
			if (ay + size_.cy > rc.bottom) { ay = rc.bottom - size_.cy; }
			if (ay < rc.top) { ay = rc.top; }
		}
		else
		{
			if (ay < rc.top) { ay = rc.top; }
			if (ay + size_.cy > rc.bottom) { ay = rc.bottom - size_.cy; }
		}
	}
	return ay;
}

void CResDlg::setPosition(long x, long y)
{
	// 設置位置のモニタ情報取得
	MONITORINFO mi;
	CPoint pt(x, y);
	utl::getDesktopRectFromPoint(pt, mi);
	CRect rc(mi.rcWork);

	// モニターからはみ出さない位置を計算
	int ax = x - (size_.cx / 2), ay = calc(rc, y);

	if (x - (size_.cx / 2) < rc.left) { ax = rc.left; }// 左
	else if (x + (size_.cx / 2) > rc.right) { ax = max(rc.left, rc.right - size_.cx); }// 右

	// 移動
	//MoveWindow(ax, ay, size_.cx, size_.cy);
	SetWindowPos(NULL, ax, ay, size_.cx, size_.cy, SWP_NOZORDER|SWP_NOACTIVATE);
}

void CResDlg::setPositionLeft(long x, long y)
{
	// 設置位置のモニタ情報取得
	MONITORINFO mi;
	CPoint pt(x, y);
	utl::getDesktopRectFromPoint(pt, mi);
	CRect rc(mi.rcWork);

	// モニターからはみ出さない位置を計算
	int ax = x - size_.cx, ay = calc(rc, y);

	if (x - size_.cx < rc.left) { ax = rc.left; }// 左
	else if (x > rc.right) { ax = max(rc.left, rc.right - size_.cx); }// 右

	// 移動
	//MoveWindow(ax, ay, size_.cx, size_.cy);
	SetWindowPos(NULL, ax, ay, size_.cx, size_.cy, SWP_NOZORDER|SWP_NOACTIVATE);
}

void CResDlg::setPositionRight(long x, long y)
{
	// 設置位置のモニタ情報取得
	MONITORINFO mi;
	CPoint pt(x, y);
	utl::getDesktopRectFromPoint(pt, mi);
	CRect rc(mi.rcWork);

	// モニターからはみ出さない位置を計算
	int ax = x, ay = calc(rc, y);

	if (x < rc.left) { ax = rc.left; }// 左
	else if (x + size_.cx > rc.right) { ax = max(rc.left, rc.right - size_.cx); }// 右

	// 移動
	//MoveWindow(ax, ay, size_.cx, size_.cy);
	SetWindowPos(NULL, ax, ay, size_.cx, size_.cy, SWP_NOZORDER|SWP_NOACTIVATE);
}

void CResDlg::setPosition()
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	CRect r;
	target_->GetWindowRect(&r);

	CPoint p;
	GetCursorPos(&p);

	switch (gl_.value.detail.board.position)
	{
	case sl::BoardConfig::POPUP_POSITION_LEFT:
		{
			setPositionLeft(p.x, r.top);
			break;
		}
	case sl::BoardConfig::POPUP_POSITION_RIGHT:
		{
			setPositionRight(p.x, r.top);
			break;
		}
	case sl::BoardConfig::POPUP_POSITION_BOTH:
		{
			if (p.x < r.left + (r.Width() / 2))
			{
				setPositionLeft(p.x, r.top);
			}
			else
			{
				setPositionRight(p.x, r.top);
			}
			break;
		}
	case sl::BoardConfig::POPUP_POSITION_CENTER:
	default:
		{
			setPosition(p.x, r.top);
			break;
		}
	}
}

void CResDlg::showWindow(bool show)
{
	if (show)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_SHOWNA);
	}
	else
	{
		// wndBottomやwndTopだとちらつきが発生する
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_HIDE);
	}
}

void CResDlg::OnUpdateResEnd()
{
	setDat();
	setPosition();
	Invalidate(FALSE);
}

void CResDlg::OnUpdateScrollEnd()
{
	OnUpdateResEnd();
}


void CResDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// マウス監視開始(OnMouseLeave)
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = GetSafeHwnd();
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);

	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	ClientToScreen(&point);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&point);
	reinterpret_cast<CBBSDlg*>(target_)->OnMouseMove(nFlags, point);

	//CDialog::OnMouseMove(nFlags, point);
}

void CResDlg::OnMouseLeave()
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	reinterpret_cast<CBBSDlg*>(target_)->OnMouseLeave();

	CDialog::OnMouseLeave();
}



void CResDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	ClientToScreen(&point);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&point);
	reinterpret_cast<CBBSDlg*>(target_)->OnLButtonDown(nFlags, point);

	//CDialog::OnLButtonDown(nFlags, point);
}


void CResDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	ClientToScreen(&point);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&point);
	reinterpret_cast<CBBSDlg*>(target_)->OnLButtonDblClk(nFlags, point);

	//CDialog::OnLButtonDblClk(nFlags, point);
}


void CResDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	ClientToScreen(&point);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&point);
	reinterpret_cast<CBBSDlg*>(target_)->OnRButtonUp(nFlags, point);

	//CDialog::OnRButtonUp(nFlags, point);
}


void CResDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	ClientToScreen(&point);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&point);
	reinterpret_cast<CBBSDlg*>(target_)->OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDown(nFlags, point);
}


void CResDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	if (!target_) { return; }
	if (!target_->GetSafeHwnd()) { return; }

	ClientToScreen(&point);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&point);
	reinterpret_cast<CBBSDlg*>(target_)->OnMButtonDblClk(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}


BOOL CResDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!target_) { return FALSE; }
	if (!target_->GetSafeHwnd()) { return FALSE; }

	ClientToScreen(&pt);
	reinterpret_cast<CBBSDlg*>(target_)->ScreenToClient(&pt);
	reinterpret_cast<CMainDlg*>(target_)->SendMessage(
		WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));

	return FALSE;
	//return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}



