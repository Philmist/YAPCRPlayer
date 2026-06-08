// StatusDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "StatusDlg.h"
//#include "afxdialogex.h"


enum TIMER_ID {
	TIMER_ID_UPDATE,
	TIMER_ID_SLIDER,
};

// CStatusDlg ダイアログ

IMPLEMENT_DYNAMIC(CStatusDlg, CDialog)

CStatusDlg::CStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatusDlg::IDD, pParent)
	, height_(16)
	, click_(false)
	, capture_(false)
	, show_(false)
{
	utl::createFont(font_);
}

CStatusDlg::~CStatusDlg()
{
	if (font_.GetSafeHandle()) { font_.DeleteObject(); }
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


void CStatusDlg::SetConfig(const sl::StatusConfig& status, const sl::ChannelConfig& channel)
{
	layout_.reset();

	status_ = status;
	font_.DeleteObject();
	font_.CreateFontIndirect(&status_.font);

	height_ = utl::getFontHeight(GetSafeHwnd(), font_) + (status_.marginY * 2);

	Invalidate(FALSE);

	channel_.SetConfig(channel);
}

void CStatusDlg::setDesc(const std::wstring& desc)
{
	desc_ = desc;
}

void CStatusDlg::clearDesc()
{
	desc_.clear();
}

BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CStatusDlg メッセージ ハンドラー


BOOL CStatusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 初期表示時に見えないようにする
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOACTIVATE);

	// ヒントダイアログ
	channel_.Create(CChannelDlg::IDD, GetDesktopWindow());
	channel_.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE);
	channel_.setAlign(DT_LEFT);
	channel_.setPanel(); // フローティング表示の復帰時に必要

	// 書式初期化
	layout_.init(gl_.value.format, [this](CRect& rc){
		reinterpret_cast<CSubpaneDlg*>(GetParent())->getVideoRect(rc);
	});

	SetTimer(TIMER_ID_UPDATE, 100, NULL); // 更新用タイマー

	SetFocus(); // IMEDisable
	utl::IMEDisable();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CStatusDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return FALSE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CStatusDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}

void CStatusDlg::OnPaint()
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
	memDC.SetTextColor(status_.text[0]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);
	
	// 背景
	utl::drawStatusBar(memDC, &org, status_.color);

	CRect rc(org);
	// マージン
	rc.DeflateRect(status_.marginX, 0);

	layout_.draw(memDC, rc, desc_, nametext_);

	///////////////////////////////////////////////////////////////
	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}



void CStatusDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_UPDATE:
		{// 更新
			if (!IsWindowVisible()) { break; }
			Invalidate(FALSE);
			break;
		}
	case TIMER_ID_SLIDER:
		{// スライダー遅延消去
			layout_.slider.enable = false;
			Invalidate(FALSE);
			KillTimer(TIMER_ID_SLIDER);
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}


void CStatusDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	Invalidate(FALSE);
}

void CStatusDlg::SetMouseHover()
{
	// マウス監視開始(OnMouseLeave)
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = GetSafeHwnd();
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);
}

void CStatusDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (capture_)
	{// スライダー操作
		// スライダー幅計算
		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(status_.marginX, 0);
		rc.left = rc.right - (layout_.volume.width + layout_.slider.width);
		rc.left += utl::StatusLayout::SLIDER_MARGIN;
		rc.right = rc.right - (layout_.volume.width + utl::StatusLayout::SLIDER_MARGIN);
		int width = rc.Width();

		// ノブの位置
		int pos = point.x - rc.left;
		pos = max(pos, 0);
		pos = min(pos, width);

		if (width > 0)
		{// 音量適用/ミュート解除
			int vol = (int)(((double)pos / (double)width)*100.0);
			
			gl_.value.audio.mute = false;
			gl_.player.setMute(false);
			gl_.value.audio.volume = vol;
			gl_.player.setVolume(vol);

			Invalidate(FALSE);
			UpdateWindow();
		}
		return;
	}

	
	{// ボリューム&スライダー
		if (layout_.volume.rect.PtInRect(point) ||
			(layout_.slider.enable && layout_.slider.rect.PtInRect(point)))
		{
			if (layout_.volume.enable)
			{// スライダー表示
				KillTimer(TIMER_ID_SLIDER);// 遅延消去タイマーリセット
				layout_.slider.enable = true;
			}

			channel_.showWindow(false);
			SetCursor(LoadCursor(NULL, IDC_HAND));
			SetMouseHover();
			return;
		}
	}

	{// チャンネル情報
		if (gl_.player.peercast() && layout_.name.rect.PtInRect(point))
		{
			if (layout_.name.enable)
			{// ポップアップ表示
				CRect rc;
				GetClientRect(rc);
				ClientToScreen(&rc);

				channel_.setPosition(rc.left, rc.top);
				channel_.Invalidate(FALSE);
				channel_.showWindow(true);
			}
			else
			{
				channel_.showWindow(false);
			}
			SetCursor(LoadCursor(NULL, IDC_HAND));
			SetMouseHover();
			return;
		}
	}

	//if (!gl_.value.window.status.fixed)
	//{
	//	if (layout_.time.enable)
	//	{// タイム
	//		if (!gl_.player.online() && layout_.time.rect.PtInRect(point))
	//		{
	//			channel_.showWindow(false);
	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			SetMouseHover();
	//			return;
	//		}
	//	}

	//	if (layout_.size.enable)
	//	{// サイズ
	//		if (layout_.size.rect.PtInRect(point))
	//		{
	//			channel_.showWindow(false);
	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			SetMouseHover();
	//			return;
	//		}
	//	}

	//	if (layout_.bitrate.enable)
	//	{// bitrate
	//		if (gl_.player.peercast() && layout_.bitrate.rect.PtInRect(point))
	//		{
	//			channel_.showWindow(false);
	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			SetMouseHover();
	//			return;
	//		}
	//	}

	//	if (layout_.fps.enable)
	//	{// fps
	//		if (layout_.fps.rect.PtInRect(point))
	//		{
	//			channel_.showWindow(false);
	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			SetMouseHover();
	//			return;
	//		}
	//	}
	//}

	channel_.showWindow(false);
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	//CDialog::OnMouseMove(nFlags, point);
}


void CStatusDlg::OnMouseLeave()
{
	CPoint pt;
	if (GetCursorPos(&pt))
	{
		CWnd* wnd = WindowFromPoint(pt);
		if (wnd)
		{
			if (wnd->GetSafeHwnd() == GetSafeHwnd())
			{
				return;
			}
		}
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));

	if (layout_.slider.enable)
	{
		SetTimer(TIMER_ID_SLIDER, 500, NULL);// スライダー消去
	}
	
	channel_.showWindow(false);

	CDialog::OnMouseLeave();
}


void CStatusDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!gl_.value.detail.board.focus)
	{
		SetFocus(); // BBS
	}

	{// ボリュームミュート
		if (layout_.volume.rect.PtInRect(point))
		{
			utl::toggle(gl_.value.audio.mute);
			gl_.player.setMute(gl_.value.audio.mute);
			Invalidate(FALSE);
			UpdateWindow();

			SetCursor(LoadCursor(NULL, IDC_HAND));
			click_ = true;
			return;
		}
	}

	if (!capture_)
	{// ボリュームスライダー
		if (layout_.slider.enable && layout_.slider.rect.PtInRect(point))
		{
			SetCapture();
			capture_ = true;
			SetCursor(LoadCursor(NULL, IDC_HAND));
			OnMouseMove(nFlags, point); // 音量処理を委任
			return;
		}
	}

	{// 名前
		if (gl_.player.peercast() && layout_.name.rect.PtInRect(point))
		{// 名前上は常にハンドカーソル
			SetCursor(LoadCursor(NULL, IDC_HAND));
			click_ = true;
			return;
		}
	}


	//if (!gl_.value.window.status.fixed)
	//{
	//	if (layout_.time.enable)
	//	{// タイム
	//		if (!gl_.player.online() && layout_.time.rect.PtInRect(point))
	//		{
	//			if (layout_.time.type < 1) { layout_.time.type++; } else { layout_.time.type = 0; }
	//			gl_.value.format.time.type = layout_.time.type;
	//			Invalidate(FALSE);
	//			UpdateWindow();

	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			click_ = true;
	//			return;
	//		}
	//	}

	//	if (layout_.size.enable)
	//	{// サイズ
	//		if (layout_.size.rect.PtInRect(point))
	//		{
	//			if (layout_.size.type < 2) { layout_.size.type++; } else { layout_.size.type = 0; }
	//			gl_.value.format.size.type = layout_.size.type;
	//			Invalidate(FALSE);
	//			UpdateWindow();

	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			click_ = true;
	//			return;
	//		}
	//	}

	//	if (layout_.bitrate.enable)
	//	{// ビットレート
	//		if (gl_.player.peercast() && layout_.bitrate.rect.PtInRect(point))
	//		{
	//			if (layout_.bitrate.type < 3) { layout_.bitrate.type++; } else { layout_.bitrate.type = 0; }
	//			gl_.value.format.bitrate.type = layout_.bitrate.type;
	//			Invalidate(FALSE);
	//			UpdateWindow();

	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			click_ = true;
	//			return;
	//		}
	//	}

	//	if (layout_.fps.enable)
	//	{// fps
	//		if (layout_.fps.rect.PtInRect(point))
	//		{
	//			if (layout_.fps.type < 3) { layout_.fps.type++; } else { layout_.fps.type = 0; }
	//			gl_.value.format.fps.type = layout_.fps.type;
	//			Invalidate(FALSE);
	//			UpdateWindow();

	//			SetCursor(LoadCursor(NULL, IDC_HAND));
	//			click_ = true;
	//			return;
	//		}
	//	}
	//}

	// 親座標に変換
	ClientToScreen(&point);
	reinterpret_cast<CSubpaneDlg*>(GetParent())->ScreenToClient(&point);
	reinterpret_cast<CSubpaneDlg*>(GetParent())->OnLButtonDown(nFlags, point);

	//CDialog::OnLButtonDown(nFlags, point);
}



void CStatusDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (capture_)
	{
		ReleaseCapture();
		capture_ = false;
	}

	if (click_)
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
		click_ = false;
	}
	//CDialog::OnLButtonUp(nFlags, point);
}

void CStatusDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (gl_.player.peercast() && layout_.name.rect.PtInRect(point))
	{// ダブルクリックでチャンネル情報更新
		gl_.peca.info();
	}

	OnLButtonDown(nFlags, point);

	//CDialog::OnLButtonDblClk(nFlags, point);
}




void CStatusDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!gl_.value.detail.board.focus)
	{
		SetFocus(); // BBS
	}

	{// ボリューム/スライダー表示切り替え
		if (layout_.volume.rect.PtInRect(point) ||
			(layout_.slider.enable && layout_.slider.rect.PtInRect(point)))
		{
			KillTimer(TIMER_ID_SLIDER);// 遅延消去タイマーリセット

			gl_.value.format.volume.enable = utl::toggle(layout_.volume.enable);
			layout_.slider.enable = layout_.volume.enable;

			Invalidate(FALSE);
			UpdateWindow();
			if (layout_.slider.enable)
			{
				SetCursor(LoadCursor(NULL, IDC_HAND));
				click_ = true;
			}
			return;
		}
	}

	//{// 名前
	//	if (gl_.player.peercast() && layout_.name.rect.PtInRect(point))
	//	{
	//		gl_.value.format.name.enable = utl::toggle(layout_.name.enable);
	//		OnMouseMove(nFlags, point);

	//		// 名前上は常にハンドカーソル
	//		SetCursor(LoadCursor(NULL, IDC_HAND));
	//		click_ = true;
	//		return;
	//	}
	//}

	//if (!gl_.value.window.status.fixed)
	//{
		//{// タイム
		//	if (!gl_.player.online() && layout_.time.rect.PtInRect(point))
		//	{
		//		gl_.value.format.time.enable = utl::toggle(layout_.time.enable);

		//		Invalidate(FALSE);
		//		UpdateWindow();

		//		if (layout_.time.enable)
		//		{
		//			SetCursor(LoadCursor(NULL, IDC_HAND));
		//			click_ = true;
		//		}
		//		return;
		//	}
		//}

		//{// サイズ
		//	if (layout_.size.rect.PtInRect(point))
		//	{
		//		gl_.value.format.size.enable = utl::toggle(layout_.size.enable);

		//		Invalidate(FALSE);
		//		UpdateWindow();

		//		if (layout_.size.enable)
		//		{
		//			SetCursor(LoadCursor(NULL, IDC_HAND));
		//			click_ = true;
		//		}
		//		return;
		//	}
		//}

		//{// ビットレート
		//	if (gl_.player.peercast() && layout_.bitrate.rect.PtInRect(point))
		//	{
		//		gl_.value.format.bitrate.enable = utl::toggle(layout_.bitrate.enable);

		//		Invalidate(FALSE);
		//		UpdateWindow();

		//		if (layout_.bitrate.enable)
		//		{
		//			SetCursor(LoadCursor(NULL, IDC_HAND));
		//			click_ = true;
		//		}
		//		return;
		//	}
		//}
	
		//{// fps
		//	if (layout_.fps.rect.PtInRect(point))
		//	{
		//		gl_.value.format.fps.enable = utl::toggle(layout_.fps.enable);

		//		Invalidate(FALSE);
		//		UpdateWindow();

		//		if (layout_.fps.enable)
		//		{
		//			SetCursor(LoadCursor(NULL, IDC_HAND));
		//			click_ = true;
		//		}
		//		return;
		//	}
		//}
	//}

	//CDialog::OnRButtonDown(nFlags, point);
}

template <std::size_t N>
void CStatusDlg::TypeMenu(int number, int& type, int& gl_type, const std::array<TypeSet, N>& set)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_STATUSMENU)) { return; }

	auto checkItem = [&](CMenu *sub, int id, bool check){
		if (check)
		{
			sub->CheckMenuItem(id, MF_CHECKED|MF_BYCOMMAND);
		}
	};

	CMenu *sub = menu.GetSubMenu(number);

	if (sub)
	{
		auto checkItem = [&](CMenu *sub, int id, bool check){
			if (check)
			{
				sub->CheckMenuItem(id, MF_CHECKED|MF_BYCOMMAND);
			}
		};

		for (auto it = set.begin(); it != set.end(); ++it)
		{
			checkItem(sub, it->id, type == it->number);
		}

		CPoint pt;
		GetCursorPos(&pt);
		int idm = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			if (idm == it->id)
			{
				gl_type = type = it->number;
				Invalidate(FALSE);
				UpdateWindow();
				break;
			}
		}
	}
	return;
}

void CStatusDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	{// 名前
		if (gl_.player.peercast() && layout_.name.rect.PtInRect(point))
		{
			CMenu menu;
			if (!menu.LoadMenu(IDR_STATUSMENU)) { return; }

			CMenu *sub = menu.GetSubMenu(0);

			if (sub)
			{
				auto checkItem = [&](CMenu *sub, int id, bool check){
					if (check)
					{
						sub->CheckMenuItem(id, MF_CHECKED|MF_BYCOMMAND);
					}
				};

				checkItem(sub, IDM_STATUS_CHANNEL_ENABLE, layout_.name.enable);

				std::vector<std::wstring> links;
				if (gl_.peca.getLinks(links))
				{
					int i = 0;
					for (auto it = links.begin(); it != links.end(); ++it)
					{
						const int len = 256;
						CString str;
						str.Format(L"%s%s", it->substr(0, len).c_str(), it->size() > len ? L"..." : L"");

						sub->InsertMenu(
							IDM_STATUS_LINK_MENU,
							MF_BYCOMMAND|MF_STRING,
							IDM_STATUS_LINK_RANGE_BEGIN + i,
							str);
						i++;
						if (i >= 100) { break; }
					}

					if (!links.empty())
					{
						sub->InsertMenu(IDM_STATUS_LINK_MENU, MF_BYCOMMAND|MF_SEPARATOR);
					}
				}
				sub->DeleteMenu(IDM_STATUS_LINK_MENU, MF_BYCOMMAND);

				CPoint pt;
				GetCursorPos(&pt);
				int idm = sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, this);

				if (idm >= IDM_STATUS_LINK_RANGE_BEGIN && idm <= IDM_STATUS_LINK_RANGE_END)
				{
					int i = idm - IDM_STATUS_LINK_RANGE_BEGIN;
					try {
						utl::selectExecute(gl_.value.detail.browser.def, gl_.value.detail.browser.web, links.at(i));
					} catch (...) {}
				}
				else
				{
					switch (idm)
					{
					case IDM_STATUS_CHANNEL_ENABLE:
						{
							gl_.value.format.name.enable = utl::toggle(layout_.name.enable);
							OnMouseMove(nFlags, point);
						}
						break;
					}
				}
			}
			return;
		}
	}

	{// タイム
		if (!gl_.player.online() && layout_.time.rect.PtInRect(point))
		{
			std::array<TypeSet, 2> set = {{
					{IDM_STATUS_TIME_TYPE0, 0},
					{IDM_STATUS_TIME_TYPE1, 1},
				}};
			TypeMenu(1, layout_.time.type, gl_.value.format.time.type, set);
			return;
		}
	}

	{// サイズ
		if (layout_.size.rect.PtInRect(point))
		{
			std::array<TypeSet, 3> set = {{
					{IDM_STATUS_SIZE_TYPE0, 0},
					{IDM_STATUS_SIZE_TYPE1, 1},
					{IDM_STATUS_SIZE_TYPE2, 2},
				}};
			TypeMenu(2, layout_.size.type, gl_.value.format.size.type, set);
			return;
		}
	}

	{// ビットレート
		if (gl_.player.peercast() && layout_.bitrate.rect.PtInRect(point))
		{
			std::array<TypeSet, 6> set = {{
					{IDM_STATUS_BITRATE_TYPE0, 0},
					{IDM_STATUS_BITRATE_TYPE1, 1},
					{IDM_STATUS_BITRATE_TYPE2, 2},
					{IDM_STATUS_BITRATE_TYPE3, 3},
					{IDM_STATUS_BITRATE_TYPE4, 4},
					{IDM_STATUS_BITRATE_TYPE5, 5},
				}};
			TypeMenu(3, layout_.bitrate.type, gl_.value.format.bitrate.type, set);
			return;
		}
	}

	{// fps
		if (layout_.fps.rect.PtInRect(point))
		{
			std::array<TypeSet, 4> set = {{
					{IDM_STATUS_FPS_TYPE0, 0},
					{IDM_STATUS_FPS_TYPE1, 1},
					{IDM_STATUS_FPS_TYPE2, 2},
					{IDM_STATUS_FPS_TYPE3, 3},
				}};
			TypeMenu(4, layout_.fps.type, gl_.value.format.fps.type, set);
			return;
		}
	}


	if (click_)
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
		click_ = false;
	}
	//CDialog::OnRButtonUp(nFlags, point);
}


void CStatusDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	OnRButtonDown(nFlags, point);

	//CDialog::OnRButtonDblClk(nFlags, point);
}

void CStatusDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (gl_.value.operation.mouse.status.transfer)
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

	main->MouseMClickHelper(gl_.value.operation.mouse.status.mclick);

	//CDialog::OnMButtonDown(nFlags, point);
}

void CStatusDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}


void CStatusDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (::IsWindow(channel_))
	{
		channel_.DestroyWindow();
	}
}
