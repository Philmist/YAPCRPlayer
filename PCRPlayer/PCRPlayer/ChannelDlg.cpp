// ChannelDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ChannelDlg.h"
//#include "afxdialogex.h"


enum TIMER_ID {
	TIMER_ID_GETINFO,
};

// CChannelDlg ダイアログ

IMPLEMENT_DYNAMIC(CChannelDlg, CDialog)

CChannelDlg::CChannelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChannelDlg::IDD, pParent)
	, align_(DT_LEFT)
	, modified_(0)
{
	utl::createFont(font_, true);

	gl_.peca.setChannelCallback([this](bool start){
		if (IsWindow(this->GetSafeHwnd()))
		{
			if (start)
			{
				this->PostMessage(WM_COMMAND, IDM_UPDATE_CHANNEL_INFO_START);
			}
			else
			{
				this->PostMessage(WM_COMMAND, IDM_UPDATE_CHANNEL_INFO_END);
			}
		}
	});
}

CChannelDlg::~CChannelDlg()
{
	if (font_.GetSafeHandle()) { font_.DeleteObject(); }

	gl_.peca.setChannelCallback([](bool){});
}

void CChannelDlg::SetConfig(const sl::ChannelConfig& cfg)
{
	cfg_ = cfg;
	font_.DeleteObject();
	font_.CreateFontIndirect(&cfg_.font);

	DestroyWindow();
	Create(IDD, GetDesktopWindow());

	if (!cfg_.opaque.enable)
	{
		SetLayeredWindowAttributes(0, cfg_.opaque.back, LWA_ALPHA);
	}

	OnUpdateChannelInfoEnd();
}

void CChannelDlg::draw(CDC& dc, const CRect& org, bool mask, int text, int back)
{
	COLORREF color = cfg_.text[0];
	if (mask)
	{
		color = RGB(text, text, text);
		dc.FillRect(org, &CBrush(RGB(back, back, back)));
	}
	else
	{
		utl::drawFrameRect(dc, &org, cfg_.color);
	}

	CRect rc(org);
	rc.DeflateRect(cfg_.marginX, cfg_.marginY);

	dc.SetTextColor(color);
	dc.SetBkColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(text_.c_str(), -1, &rc, DT_NOPREFIX | align_);
}

void CChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChannelDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_COMMAND(IDM_UPDATE_CHANNEL_INFO_START, &CChannelDlg::OnUpdateChannelInfoStart)
	ON_COMMAND(IDM_UPDATE_CHANNEL_INFO_END, &CChannelDlg::OnUpdateChannelInfoEnd)
END_MESSAGE_MAP()


// CChannelDlg メッセージ ハンドラー



BOOL CChannelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	size_.SetSize(0, 0);

	if (!cfg_.opaque.enable)
	{
		SetLayeredWindowAttributes(0, cfg_.opaque.back, LWA_ALPHA);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CChannelDlg::PreTranslateMessage(MSG* pMsg)
{
	// EnterとESCを押すと終了してしまうので処理をつぶす
    if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
        return FALSE;
    }
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CChannelDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CChannelDlg::OnPaint()
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
	////////////////////////////////////////////////////////////////////////////////////

	draw(memDC, org);

	///////////////////////////////////////////////////////////////
	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}

void CChannelDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_GETINFO:
		{
			showWindow(false);
			setPanel();

			CRect rc;
			GetWindowRect(rc);
			setPosition(rc.left, rc.bottom);
			Invalidate(FALSE);
			KillTimer(TIMER_ID_GETINFO);
			break;
		}
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


void CChannelDlg::setAlign(DWORD align)
{
	align_ = align;
}

void CChannelDlg::setPanel()
{
	std::wstring text;
	if (gl_.peca.getPanel(text))
	{
		setText(text);
	}
	else
	{
		setText(L"チャンネル情報未取得");
	}
}

void CChannelDlg::setText(const std::wstring& text)
{
	text_ = text;

	if (text.empty())
	{
		size_.SetSize(0, 0);
		return;
	}

	CClientDC dc(this);
	CRect rc;
	CFont* fontOld = dc.SelectObject(&font_);
	dc.DrawText(text.c_str(), -1, &rc, DT_CALCRECT|DT_NOPREFIX|align_);
	size_.cx = rc.Width() + (cfg_.marginX * 2);
	size_.cy = rc.Height() + (cfg_.marginY * 2);

	if (cfg_.opaque.enable)
	{
		CRect org(0, 0, size_.cx, size_.cy);

		utl::LayeredWindow layer(size_.cx, size_.cy, font_);

		if (layer)
		{
			draw(layer.getImageCDC(), org);
			draw(layer.getMaskCDC(), org, true, cfg_.opaque.text, cfg_.opaque.back);
			if (layer.create())
			{
				layer.update(GetSafeHwnd(), dc);
			}
		}
	}

	dc.SelectObject(fontOld);
}

void CChannelDlg::setPosition(long x, long y)
{
	// 設置位置のモニタ情報取得
	MONITORINFO mi;
	CPoint pt;
	GetCursorPos(&pt);
	utl::getDesktopRectFromPoint(pt, mi);
	CRect rc(mi.rcMonitor);

	int ax = x;
	int ay = y - size_.cy;

	if (ax < rc.left) { ax = rc.left; }
	else if (ax + size_.cx  > rc.right) { ax = rc.right - size_.cx; }

	// 移動
	SetWindowPos(NULL, ax, ay, size_.cx, size_.cy, SWP_NOZORDER|SWP_NOACTIVATE);
	//MoveWindow(ax, ay, size_.cx, size_.cy);
}

void CChannelDlg::showWindow(bool show)
{
	if (show)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		// wndBottomやwndTopだとちらつきが発生する
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_HIDE);
	}
}

void CChannelDlg::OnUpdateChannelInfoStart()
{
	modified_ = GetTickCount();
	showWindow(false);
	setText(L"チャンネル情報更新中");

	CRect rc;
	GetWindowRect(rc);
	setPosition(rc.left, rc.bottom);
	Invalidate(FALSE);
}


void CChannelDlg::OnUpdateChannelInfoEnd()
{
	gl_.createChannelInfo();

	int diff = (modified_ + 250) - GetTickCount();

	SetTimer(TIMER_ID_GETINFO, diff < 0 ? 0 : diff, NULL);
}
