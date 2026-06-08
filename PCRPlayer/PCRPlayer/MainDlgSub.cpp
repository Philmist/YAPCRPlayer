#include "stdafx.h"
#include "MainDlg.h"

void CMainDlg::GetVideoRect(CRect& rc)
{
	if (layout_.IsMain())
	{
		layout_.GetMain()->GetClientRect(rc);

		SIZE aspect = {};
		gl_.getAspect(aspect);

		CRect dst;
		if (utl::getAspectRect(aspect, rc, dst))
		{
			rc = dst;
		}
	}

	//GetClientRect(&rc);

	//if (!IsExcludeSubpane())
	//{// サブペイン表示時
	//	rc.DeflateRect(0, 0, 0, subpane_.getHeight());
	//}

	//SIZE aspect = {};
	//gl_.getAspect(aspect);

	//CRect dst;
	//if (utl::getAspectRect(aspect, rc, dst))
	//{
	//	rc = dst;
	//}
}

void CMainDlg::SetTitleText()
{
	// 名前
	std::wstring text;
	if (gl_.latest.name.empty())
	{
		network::ChannelInfo info;
		if (gl_.peca.getInfo(info))
		{
			text = info.name;
		}
		else
		{
			text = utl::pathToFile(gl_.latest.path);
		}
	}
	else
	{
		text = gl_.latest.name;
	}

	// 表示サイズ
	bool v = false;
	CRect rc;
	if (!IsMinimized() && gl_.value.window.title.size)
	{
		GetVideoRect(rc);

		v = true;
	}

	// ソースサイズ
	bool s = false;
	SIZE size = {};
	if (gl_.value.window.title.source)
	{
		if (gl_.player.getNativeVideoSize(size))
		{
			s = true;
		}
	}

	CString title;
	if (v && s)
	{
		title.Format(L"%s (%dx%d/%dx%d)", text.c_str(), rc.Width(), rc.Height(), size.cx, size.cy);
	}
	else if (v)
	{
		title.Format(L"%s (%dx%d)", text.c_str(), rc.Width(), rc.Height());
	}
	else if (s)
	{
		title.Format(L"%s (%dx%d)", text.c_str(), size.cx, size.cy);
	}
	else
	{
		title = text.c_str();
	}

	if (title.IsEmpty())
	{
		title = L"PCRPlayer";
	}
	else if (gl_.value.window.title.player)
	{
		title += L" - PCRPlayer";
	}

	SetWindowText(title);
}

bool CMainDlg::RebuildFile()
{
	return OpenFile(gl_.latest.path, gl_.latest.name, gl_.latest.contact, false, true);
}

bool CMainDlg::OpenFile(const std::wstring& path, const std::wstring& name, const std::wstring& contact, bool commandline, bool rebuild)
{
	// プレイヤー中断
	// ここで中断するとブロックされる
	// 中断はexecute()のスレッド内でする
	//gl_.player.terminate();


	// 初期設定
	dsp::BuildConfig cfg;
	cfg.audio = gl_.value.audio;
	cfg.video = gl_.value.video;
	cfg.network = gl_.value.detail.network;
	cfg.type = gl_.value.detail.type;
	cfg.decoder = gl_.value.detail.decoder;

	for (auto it = gl_.value.detail.video.begin(); it != gl_.value.detail.video.end(); ++it)
	{
		if (it->enable)
		{
			cfg.decoderSet.video.id = it->id;
			cfg.decoderSet.video.name = it->name;
			break;
		}
	}

	for (auto it = gl_.value.detail.audio.begin(); it != gl_.value.detail.audio.end(); ++it)
	{
		if (it->enable)
		{
			cfg.decoderSet.audio.id = it->id;
			cfg.decoderSet.audio.name = it->name;
			break;
		}
	}

	// フレームからメインウィンドウに通知
	CFrameDlg* frame = (CFrameDlg*)layout_.GetMain();
	if (!frame || !::IsWindow(frame->GetSafeHwnd())) { return false; }

	// playerメンバー変数初期化、pathからモード判定
	if (!gl_.player.init(frame->GetSafeHwnd(), WM_GRAPH_NOTIFY, path, cfg))
	{// 初期化失敗(中断中)
		return false;
	}

	gl_.peca.init(path);

	if (gl_.bbs.init(path))
	{// 既存のpathから変更された
		if (gl_.value.detail.board.contact && gl_.player.online())
		{
			if (commandline)
			{// コマンドラインのときは引数から読み込む
				gl_.bbs.execute(contact, gl_.value.detail.board, gl_.value.detail.board.movable.commandline);
			}
		}
	}

	// playerで初期化してから最新パス情報を変更
	if (!rebuild)
	{// 再構築で履歴を更新しない
		gl_.latest.path = path;
		gl_.latest.name = name;
		gl_.latest.contact = contact;
		gl_.latest.commandline = commandline;
		gl_.value.recent = gl_.latest;
	}

	SetTitleText();

	// 日時表示
	// ファイル名表示
	if (gl_.player.online())
	{
		subpane_.clearTime();
		subpane_.clearFilename();
	}
	else
	{
		switch (gl_.value.display.datetime.type)
		{
		case sl::DATETIME_TYPE_NONE:
			subpane_.clearTime();
			break;

		case sl::DATETIME_TYPE_FILETIME:
			subpane_.setTime(utl::getFileTime(path));
			break;

		case sl::DATETIME_TYPE_FILENAME:
		default:
			{
				utl::DateTime dt;
				dt.set(utl::pathToFile(path),
					gl_.value.display.datetime.exp,
					gl_.value.display.datetime.sub[0],
					gl_.value.display.datetime.sub[1],
					gl_.value.display.datetime.sub[2],
					gl_.value.display.datetime.sub[3],
					gl_.value.display.datetime.sub[4],
					gl_.value.display.datetime.sub[5]);

				subpane_.setTime(dt.get());
			}
			break;
		}

		subpane_.setFilename(utl::pathToFile(path).c_str());
	}

	subpane_.setNametext(name);


	// バー表示
	ApplySubpane();
	ShowBrowser(gl_.value.window.show.browser);

	gl_.player.execute();

	frame->Invalidate();
	return true;
}

bool CMainDlg::OpenNextFile(bool loop, bool prev)
{
	if (!gl_.player.online())
	{
		std::wstring next;
		if (utl::getNextFilePath(gl_.latest.path, next, loop, prev))
		{
			return OpenFile(next);
		}
	}
	return false;
}

void CMainDlg::ExpandWindow(const CRect& rect)
{
	if (gl_.screen.full || gl_.screen.maximize)
	{
		CRect rc(gl_.screen.restore.rcNormalPosition);
		rc.InflateRect(rect);
		gl_.screen.restore.rcNormalPosition = rc;
	}
	else if (IsZoomed())
	{
		WINDOWPLACEMENT wp;
		ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp))
		{
			CRect rc(wp.rcNormalPosition);
			rc.InflateRect(rect);
			gl_.screen.restore.rcNormalPosition = rc;
			SetWindowPlacement(&wp);
		}
	}
	else
	{
		CRect rc;
		GetWindowRect(&rc);
		rc.InflateRect(rect);
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
	}

	SizeHelper();
	RedrawWindow(0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CMainDlg::ShowSubpane(bool bbs, bool seek, bool status)
{
	if (!gl_.player.online())
	{
		if (::IsWindow(thread_.GetSafeHwnd()))
		{
			thread_.DestroyWindow();
		}
	}

	if (bbs)
	{
		subpane_.showBBS(gl_.value.window.show.bbs ? gl_.player.online() : false, gl_.value.window.show.edit);
	}

	if (seek)
	{
		subpane_.showSeek(gl_.value.window.show.seek ? !gl_.player.online() : false);
	}

	if (status)
	{
		subpane_.showStatus(gl_.value.window.show.status);
	}
	subpane_.SizeHelper();
}

void CMainDlg::ApplySubpane(bool bbs, bool seek, bool status)
{
	int h = subpane_.getHeight();

	ShowSubpane(bbs, seek, status);

	ExpandWindow(CRect(0, 0, 0, subpane_.getHeight() - h));
}

void CMainDlg::ShowBrowser(bool show)
{
	if (gl_.player.online())
	{
		if (show)
		{
			if (layout_.GetSub())
			{
				layout_.ShowSub(TRUE);
				layout_.RecalcLayout();
				layout_.RedrawAll();
				gl_.bbs.setBlock(false);
			}
			else
			{
				layout_.CreateSub(new CBrowserDlg, IDD_BROWSER);
				CBrowserDlg* browser = (CBrowserDlg*)layout_.GetSub();
				if (browser)
				{
					// ブラウザ更新コールバック
					browser->setUpdateCallback([this](){
						HWND hwnd = this->subpane_.getBBS();
						if (::IsWindow(hwnd))
						{
							::PostMessage(hwnd, WM_COMMAND, IDM_UPDATE_BBS_BROWSER, 0);
						}
					});

					gl_.bbs.reset();
					layout_.ShowSub(TRUE);
					layout_.RecalcLayout();
					layout_.RedrawAll();
					gl_.bbs.setBlock(false);
				}
			}
		}
		else
		{
			if (layout_.IsVisibleSub())
			{
				if (::IsWindow(search_))
				{
					search_.DestroyWindow();
				}

				//CExtractDlg::destroy();

				CBrowserDlg* browser = (CBrowserDlg*)layout_.GetSub();
				if (browser)
				{
					browser->hidePopup();
				}

				if (gl_.value.window.show.edit && gl_.value.detail.board.focus)
				{
					SetFocusBBSEdit(); // BBS popup
				}
				else
				{
					SetFocus(); // BBS popup
				}

				layout_.ShowSub(FALSE);
				layout_.RecalcLayout();
				layout_.RedrawAll();
				gl_.bbs.setBlock(true);
			}
		}
	}
	else
	{
		if (layout_.GetSub())
		{
			if (::IsWindow(search_))
			{
				search_.DestroyWindow();
			}

			CExtractDlg::destroy();

			SetFocus();

			CWnd* wnd = layout_.DetachSub();
			if (wnd)
			{
				if (::IsWindow(wnd->GetSafeHwnd()))
				{
					wnd->DestroyWindow();
				}
				delete wnd;
				layout_.RecalcLayout();
				layout_.RedrawAll();
				gl_.bbs.setBlock(true);
			}
		}
	}
}

void CMainDlg::ToggleShowBrowser(int side)
{
	if (gl_.value.window.show.browser &&
		gl_.value.window.show.side == side)
	{
		gl_.value.window.show.browser = false;
		ShowBrowser(false);
	}
	else
	{
		gl_.value.window.show.side = side;
		layout_.SetSide(side);

		if (gl_.value.window.show.browser)
		{
			layout_.RecalcLayout();
			layout_.RedrawAll();
		}
		else
		{
			gl_.value.window.show.browser = true;
			ShowBrowser(true);
		}
	}
}


// サブペインとビデオウィンドウの配置
void CMainDlg::SizeHelper()
{
	CRect rc;
	GetClientRect(&rc);

	// サブダイアログ配置
	if (::IsWindow(subpane_.GetSafeHwnd())/* && subpane_.IsWindowVisible()*/)
	{
		CRect crc = rc;
		if (subpane_.isPopup())
		{// ポップアップ時はスクリーン座標へ変換
			ClientToScreen(&crc);
		}

		int width = crc.Width();
		int height = subpane_.getHeight();
		int x = crc.left;
		int y = max(0, crc.bottom - height);

		subpane_.SetWindowPos(NULL, x, y, width, height, SWP_NOZORDER|SWP_NOACTIVATE);

		if (!IsExcludeSubpane())
		{// サブペイン表示時
			rc.bottom = max(0, rc.bottom - height);
		}
	}

	if (::IsWindow(layout_))
	{
		layout_.MoveWindow(rc, FALSE);
		CFrameDlg* frame = (CFrameDlg*)layout_.GetMain();
		if (frame && ::IsWindow(frame->GetSafeHwnd()))
		{
			frame->SizeHelper();
			frame->RedrawWindow();
		}
	}

	//CRect rc;
	// ビデオ描画
	//SIZE aspect = {};
	//gl_.getAspect(aspect);

	//RECT dst = {};
	//if (utl::getAspectRect(aspect, rc, dst))
	//{
	//	rc = dst;
	//}

	//gl_.player.setVideoPosition(rc);

	//Invalidate();
	SetTitleText();
}

void CMainDlg::SizingHelper(RECT& rect, bool ratio, bool inscribe, UINT side)
{
	CRect rc;
	rc = rect;

	DeflateWindowRect(rc);

	// クライアントサイズ
	long cx = rc.Width(), cy = rc.Height();

	// アスペクト比
	SIZE aspect = {};
	if (ratio)
	{
		gl_.getAspect(aspect);
	}
	long ax = aspect.cx, ay = aspect.cy;

	long tx, ty, sx, sy;

	int w, h;

	int min = 100;

	if (gl_.value.window.show.frame)
	{
		min = 0;
	}

	if (ax <= 0 || ay <= 0)
	{
		tx = sx = cx;
		ty = sy = cy;

		w = min;
		h = min;
	}
	else
	{
		// 一時アスペクト比
		tx = (long)utl::lround((double)cy * ((double)ax / (double)ay));
		ty = (long)utl::lround((double)cx * ((double)ay / (double)ax));

		// クライアントサイズに収める
		sx = ty > cy ? cx : tx;
		sy = ty > cy ? ty : cy;

		// 最小サイズを求める
		if (ax < ay)
		{
			w = min;
			h = (long)utl::lround((double)min * ((double)ay / (double)ax));
		}
		else
		{
			w = (long)utl::lround((double)min * ((double)ax / (double)ay));
			h = min;
		}

		if (inscribe)
		{
			if (ty > cy)
			{
				side = WMSZ_BOTTOM;
			}
			else
			{
				side = WMSZ_RIGHT;
			}
		}
	}

	// 動かす部分を変更する
	switch(side)
	{
	case WMSZ_TOP:			rc.right = rc.left + max(w, tx); rc.top = rc.bottom - max(h, cy); break;
	case WMSZ_BOTTOM:		rc.right = rc.left + max(w, tx); rc.bottom = rc.top + max(h, cy); break;
	case WMSZ_LEFT:			rc.left = rc.right - max(w, cx); rc.bottom = rc.top + max(h, ty); break;
	case WMSZ_RIGHT:		rc.right = rc.left + max(w, cx); rc.bottom = rc.top + max(h, ty); break;

	case WMSZ_BOTTOMLEFT:	rc.left = rc.right - max(w, sx); rc.bottom = rc.top + max(h, sy); break;
	case WMSZ_TOPRIGHT:		rc.right = rc.left + max(w, sx); rc.top = rc.bottom - max(h, sy); break;
	case WMSZ_TOPLEFT:		rc.left = rc.right - max(w, sx); rc.top = rc.bottom - max(h, sy); break;
	case WMSZ_BOTTOMRIGHT:	rc.right = rc.left + max(w, sx); rc.bottom = rc.top + max(h, sy); break;
	default:
		return;
	}


	InflateWindowRect(rc);

	rect = rc;
}

void CMainDlg::MouseMClickHelper(int type)
{
	if (!gl_.value.detail.board.focus)
	{
		SetFocus(); // BBS
	}

	switch (type)
	{
	case sl::MOUSE_MCLICK_NONE: break;
	case sl::MOUSE_MCLICK_MINIMIZE_MUTE: OnMinimizeMute(); break;
	case sl::MOUSE_MCLICK_PAUSE: OnPause(); break;
	case sl::MOUSE_MCLICK_MINIMIZE_PAUSE: OnMinimizePause(); break;
	case sl::MOUSE_MCLICK_THREAD_RESET: OnThreadReset(); break;
	case sl::MOUSE_MCLICK_THREAD_RELOAD: OnThreadReload(); break;
	case sl::MOUSE_MCLICK_BBS_EDIT: OnWindowEdit(); break;

	case sl::MOUSE_MCLICK_MUTE:
	default: OnMute(); break;
	}
}

void CMainDlg::MouseWheelHelper(bool up, int type, bool invert)
{
	// UP/DOWN反転
	if (!up)
	{
		utl::toggle(invert);
	}

	if (gl_.value.detail.board.scroll && type == sl::MOUSE_WHEEL_THREAD)
	{
		utl::toggle(invert);
	}

	switch (type)
	{
	case sl::MOUSE_WHEEL_NONE: break;

	case sl::MOUSE_WHEEL_SEEK:
		if (!invert)
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnSeekBackLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnSeekBackHigh();
			}
			else
			{
				OnSeekBack();
			}
		}
		else
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnSeekForwardLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnSeekForwardHigh();
			}
			else
			{
				OnSeekForward();
			}
		}
		break;

	case sl::MOUSE_WHEEL_THREAD:
		if (!invert)
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnThreadPrevLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnThreadPrevHigh();
			}
			else
			{
				OnThreadPrev();
			}
		}
		else
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnThreadNextLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnThreadNextHigh();
			}
			else
			{
				OnThreadNext();
			}
		}
		break;

	case sl::MOUSE_WHEEL_ZOOM:
		if (!invert)
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnScaleUpLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnScaleUpHigh();
			}
			else
			{
				OnScaleUp();
			}
		}
		else
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnScaleDownLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnScaleDownHigh();
			}
			else
			{
				OnScaleDown();
			}
		}
		break;

	case sl::MOUSE_WHEEL_VOLUME:
	default:
		if (!invert)
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnVolumeUpLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnVolumeUpHigh();
			}
			else
			{
				OnVolumeUp();
			}
		}
		else
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				OnVolumeDownLow();
			}
			else if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				OnVolumeDownHigh();
			}
			else
			{
				OnVolumeDown();
			}
		}
		break;
	}
}

// 比率固定ウィンドウの適用
void CMainDlg::CalcWindowRatio(RECT& rc, bool inscribe)
{
	SizingHelper(rc,
		true,
		inscribe,
		gl_.value.window.ratio.height ? WMSZ_BOTTOM : WMSZ_RIGHT);
}

void CMainDlg::ApplyWindowRatio(bool enable)
{
	if (IsZoomed() || gl_.screen.full || gl_.screen.maximize)
	{
		if (enable)
		{
			CalcWindowRatio(gl_.screen.restore.rcNormalPosition, gl_.value.window.ratio.inscribe);
		}
		SizeHelper();
	}
	else
	{
		box_.showWindow(false);

		if (enable)
		{
			CRect rc;
			GetWindowRect(&rc);
			CalcWindowRatio(rc, gl_.value.window.ratio.inscribe);
			SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
		SizeHelper();
	}
}


// ウィンドウ枠幅追加
void CMainDlg::InflateWindowRect(RECT& src)
{
	CRect rc(src);
	if (gl_.value.window.show.frame)
	{
		utl::inflateWindowRect(rc, gl_.value.window.show.title, true);
	}

	CSize size;
	layout_.GetMainOtherSize(size);
	rc.InflateRect(0, 0, size.cx, subpane_.getHeight() + size.cy);
	src = rc;
}


void CMainDlg::DeflateWindowRect(RECT& src)
{
	CRect rc(src);
	if (gl_.value.window.show.frame)
	{
		utl::deflateWindowRect(rc, gl_.value.window.show.title, true);
	}

	CSize size;
	layout_.GetMainOtherSize(size);
	rc.DeflateRect(0, 0, size.cx, subpane_.getHeight() + size.cy);
	src = rc;
}

void CMainDlg::StoreVideoWindowRect(RECT& src)
{
	// ウィンドウ枠がない状態を保存する
	if (gl_.screen.full || gl_.screen.maximize)
	{
		src = gl_.screen.restore.rcNormalPosition;
	}
	else
	{
		// GetWindowPlacementで最小化状態でもサイズを取得できる
		WINDOWPLACEMENT wp = {};
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp))
		{
			src = wp.rcNormalPosition;
		}
		else
		{
			GetWindowRect(&src);
		}
	}

	CRect rc(src);
	if (gl_.value.window.show.frame)
	{
		utl::deflateWindowRect(rc, gl_.value.window.show.title, true);
	}
	rc.DeflateRect(0, 0, 0, subpane_.getHeight());
	src = rc;
}

void CMainDlg::RestoreVideoWindowRect(RECT& src)
{
	CRect rc(src);
	if (gl_.value.window.show.frame)
	{
		utl::inflateWindowRect(rc, gl_.value.window.show.title, true);
	}
	rc.InflateRect(0, 0, 0, subpane_.getHeight());
	src = rc;
}

void CMainDlg::InitWindowSizeZoom()
{
	windowZoom_.clear();
	for (auto it = gl_.value.display.window.zoom.begin(); it != gl_.value.display.window.zoom.end(); ++it)
	{
		if (it->enable)
		{
			windowZoom_.push_back(*it);
		}
	}

	windowSize_.clear();
	for (auto it = gl_.value.display.window.size.begin(); it != gl_.value.display.window.size.end(); ++it)
	{
		if (it->enable)
		{
			windowSize_.push_back(*it);
		}
	}
}

bool CMainDlg::IsExcludeSubpane()
{
	return gl_.screen.full && (subpane_.isPopup() || !subpane_.IsWindowVisible());
}

void CMainDlg::RecreateSubpane(bool popup)
{
	if (popup)
	{
		// サブペインポップアップ化
		subpane_.ShowWindow(SW_HIDE);
		subpane_.DestroyWindow();
		subpane_.Create(CSubpaneDlg::IDD_POPUP, this);

		ShowSubpane();
	}
	else
	{
		// サブペイン子ウィンドウ化
		subpane_.DestroyWindow();
		subpane_.Create(CSubpaneDlg::IDD, this);

		ShowSubpane();
		subpane_.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE);
		subpane_.ShowWindow(SW_SHOWNA);
	}
}

void CMainDlg::WindowZoom(int zoom)
{
	SIZE size = {};
	if (gl_.player.getNativeVideoSize(size))
	{
		CRect src(0, 0, size.cx, size.cy);
		CRect dst;

		SIZE aspect = {};
		gl_.getAspect(aspect);

		if (!utl::getAspectRect(aspect, src, dst))
		{
			dst = src;
		}

		double dzoom = (double)zoom / 100.0;

		CRect rc(0, 0, utl::lround((double)dst.Width() * dzoom), utl::lround((double)dst.Height() * dzoom));
		InflateWindowRect(rc);

		box_.showWindow(false);
		SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_NOZORDER);
	}
}

void CMainDlg::WindowScale(int scale)
{
	if (IsZoomed() || gl_.screen.full || gl_.screen.maximize) { return; }
	RelativeWindowScale(scale);
}

void CMainDlg::RelativeWindowScale(int scale)
{
	SIZE size = {};
	if (!gl_.player.getNativeVideoSize(size))
	{
		CRect rc;
		GetVideoRect(rc);
		size.cx = rc.Width();
		size.cy = rc.Height();
	}

	double dscale = (double)scale / 100.0;
	CRect src(0, 0, utl::lround((double)size.cx * dscale), utl::lround((double)size.cy * dscale));

	CRect video;
	GetVideoRect(video);

	CRect dst(0, 0, video.Width() + src.Width(), video.Height() + src.Height());
	AbsoluteWindowSize(dst.Width(), dst.Height());
}

void CMainDlg::AbsoluteWindowSize(int width, int height)
{
	CRect rc(0, 0, width, height);
	InflateWindowRect(rc);

	box_.showWindow(false);
	if (gl_.value.window.ratio.fixed)
	{
		CalcWindowRatio(rc, true);
	}
	SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
}

void CMainDlg::GetContactURL(std::wstring& url)
{
	if (gl_.value.detail.board.change)
	{
		if (gl_.bbs.getURL(url)) { return; }

		gl_.bbs.getSourceURL(url);
		if (!url.empty()) { return; }
	}

	if (gl_.peca.getContactURL(url)) { return; }
	url = gl_.latest.contact;
}

void CMainDlg::OpenWebBrowser(const std::wstring& url)
{
	utl::selectExecute(gl_.value.detail.browser.def, gl_.value.detail.browser.web, url);
}

void CMainDlg::SetFocusBBSEdit()
{
	subpane_.SetFocusBBSEdit();
}

void CMainDlg::LoadHtml()
{
	std::wstring thread = gl_.current + L"skin\\" + gl_.value.detail.skin.thread.skin;
	std::wstring popup = gl_.current + L"skin\\" + gl_.value.detail.skin.popup.skin;

	std::wstring result;

	if (utl::loadHtml(thread + L"\\Header.html", gl_.html.header) &&
		utl::loadHtml(thread + L"\\Res.html", gl_.html.res))
	{
		// 新着レスを通常レスへ変換
		gl_.html.newRes = gl_.html.res;
		utl::ClassNameRemover remover(L"new");
		gl_.html.res = remover(gl_.html.res);
	}
	else
	{
		result += L"スレッドスキンファイルの読み込みに失敗しました<br>"
			L"以下のファイルが必要です<br>Header.html<br>Res.html<br><br>";
	}

	if (utl::loadHtml(popup + L"\\PopupHeader.html", gl_.html.popupHeader) &&
		utl::loadHtml(popup + L"\\PopupRes.html", gl_.html.popupRes))
	{

	}
	else
	{
		result += L"ポップアップスキンファイルの読み込みに失敗しました<br>"
			L"以下のファイルが必要です<br>PopupHeader.html<br>PopupRes.html<br><br>";
	}

	if (!result.empty())
	{
		gl_.html.header =
			L"<html><head></head><body bgcolor=\"#f0f0f0\">"
			+ result +
			L"</body></head></html>";
		gl_.html.res.clear();
		gl_.html.newRes.clear();
		gl_.html.popupHeader.clear();
		gl_.html.popupRes.clear();
	}
}

bool CMainDlg::OpenProcess(const std::wstring& path, const std::wstring& command)
{
	return process_.open(path, command);
}

afx_msg LRESULT CMainDlg::OnGraphNotify(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case dsp::GRAPH_NOTIFY_START:
		{
			if (boot_ && gl_.value.window.zoom)
			{// 起動時
				if (!IsZoomed() && !gl_.screen.full && !gl_.screen.maximize)
				{
					WindowZoom(100);
				}
			}

			boot_ = false;

			ApplyWindowRatio(gl_.value.window.ratio.play);
			layout_.RedrawAll();

			// スリープ抑止開始
			if (gl_.value.window.stop)
			{
				StartSleepStopTimer();
			}

			gl_.peca.info();
			break;
		}
	case dsp::GRAPH_NOTIFY_END:
		{
			boot_ = false;

			ApplyWindowRatio(gl_.value.window.ratio.play);
			layout_.RedrawAll();

			// スリープ抑止停止
			if (gl_.value.window.stop)
			{
				EndSleepStopTimer();
			}
			break;
		}
	case dsp::GRAPH_NOTIFY_RESTART:
		{
			boot_ = false;

			switch (gl_.value.play.mode)
			{
			case sl::PLAY_MODE_REPEAT:
				gl_.player.seek(0);
				break;

			case sl::PLAY_MODE_FOLDER:
				OnNextFile();
				break;

			case sl::PLAY_MODE_FOLDER_LOOP:
				OnNextFileLoop();
				break;

			case sl::PLAY_MODE_NORMAL:
			default:
				break;
			}
			break;
		}
	case dsp::GRAPH_NOTIFY_CONNECT:
		{
			gl_.peca.info();
			break;
		}
	case dsp::GRAPH_NOTIFY_RETRY_END:
		{
			if (gl_.value.detail.board.retry)
			{
				gl_.bbs.interrupt();
			}
			break;
		}
	default:
		break;
	}

	return 0;
}




/*
afx_msg LRESULT CMainDlg::OnGraphNotify(WPARAM wParam, LPARAM lParam)
{
	long code;
	LONG_PTR p1, p2;

	// イベントを全て取得
	while (gl_.player.getEvent(code, p1, p2))
	{
		DebugText(L"DS event code: 0x%08X(%d)\n", code, code);
		switch (code)
		{
		case EC_OPENING_FILE:
			{
				break;
			}
		case EC_BUFFERING_DATA:
			{
				if (p1)
				{
					DebugText(L"EC_BUFFERING_DATA TRUE\n");
					//subpane_.status_.setStatus(L"バッファ中");
				}
				else
				{
					DebugText(L"EC_BUFFERING_DATA FALSE\n");
					//subpane_.status_.setStatus(L"バッファ終了");
				}
				subpane_.invalidateSeek();
			}
		case EC_LOADSTATUS:
			{
				switch (p1)
				{
				case AM_LOADSTATUS_CLOSED: DebugText(L"AM_LOADSTATUS_CLOSED\n"); break;
				case AM_LOADSTATUS_CONNECTING: DebugText(L"AM_LOADSTATUS_CONNECTING\n"); break;
				case AM_LOADSTATUS_LOCATING: DebugText(L"AM_LOADSTATUS_LOCATING\n"); break;
				case AM_LOADSTATUS_OPEN: DebugText(L"AM_LOADSTATUS_OPEN\n"); break;
				case AM_LOADSTATUS_OPENING: DebugText(L"AM_LOADSTATUS_OPENING\n"); break;
				}
				break;
			}
		case EC_REPAINT:
			{
				CPaintDC dc(this);
				gl_.player.repaint(dc);
				DebugText(L"EC_REPAINT\n");
				break;
			}
		case EC_COMPLETE:
			{// 再生終了
				DebugText(L"EC_COMPLETE\n");
				//gl_.player.abort();
				gl_.player.execute();
				Invalidate();

				//gl_.player.stepToLastFrame();
				
				//LONGLONG dur = 0;
				//if (gl_.player.getDuration(dur))
				//{
				//	gl_.player.pause();
				//	gl_.player.seek(dur-3000000);
				//	LONGLONG st = 0;
				//	HRESULT hr = gl_.player.getSeek()->GetStopPosition(&st);
				//	controller_.setComplete(true);
				//}
				
				//Invalidate();
				//OnPaint();
				break;
			}
		case EC_STEP_COMPLETE:
			{
				DebugText(L"EC_COMPLETE\n");
				break;
			}
		case EC_ACTIVATE:
			{
				DebugText(L"EC_ACTIVATE\n");
				//controller_.Invalidate(FALSE);
				break;
			}
		case EC_VIDEO_SIZE_CHANGED:
			{
				DebugText(L"EC_VIDEO_SIZE_CHANGED\n");
				break;
			}
		default:
			{
				break;
			}
		}
	}
	return NOERROR;
}

*/

