#include "stdafx.h"
#include "MainDlg.h"

void CMainDlg::OnOption()
{
	EndMouseHideTimer();

	COptionDlg dlg;
	dlg.select_ = gl_.value.option.select.option;
	dlg.detail_.network.select_ = gl_.value.option.select.network;
	dlg.detail_.type.select_ = gl_.value.option.select.type;
	dlg.display_.information.select_ = gl_.value.option.select.information;
	dlg.display_.res.select_ = gl_.value.option.select.res;
	dlg.detail_.bbs.common.select_ = gl_.value.option.select.bbs.common;
	dlg.detail_.bbs.bar.select_ = gl_.value.option.select.bbs.bar;
	dlg.detail_.bbs.browser.select_ = gl_.value.option.select.bbs.browser;

	dlg.detail_.option = gl_.value.option.detail;
	dlg.detail_.bbs.option = gl_.value.option.bbs;
	dlg.operation_.option = gl_.value.option.operation;
	dlg.display_.option = gl_.value.option.display;

	//-----------------------------------------------------------------

	// キャンセルされたときのためにprocampを初期化する必要がある
	if (!gl_.value.video.procamp.init)
	{
		gl_.player.getProcAmp(gl_.value.video.procamp);
	}

	dlg.video_.adjust.value_ = gl_.value.video.procamp;
	dlg.video_.adjust.nominal_ = gl_.value.video.nominal;
	dlg.video_.adjust.resizer_ = gl_.value.video.resizer;

	//-----------------------------------------------------------------

	dlg.detail_.cfg.network = gl_.value.detail.network;
	dlg.detail_.cfg.type = gl_.value.detail.type;
	dlg.detail_.decoder.decoder_ = gl_.value.detail.decoder;
	dlg.detail_.decoder.video_ = gl_.value.detail.video;
	dlg.detail_.decoder.audio_ = gl_.value.detail.audio;
	dlg.detail_.cfg.browser = gl_.value.detail.browser;
	dlg.detail_.cfg.snapshot = gl_.value.detail.snapshot;
	dlg.detail_.cfg.start = gl_.value.detail.start;
	dlg.detail_.end.end_ = gl_.value.detail.end;
	dlg.detail_.bbs.cfg.board = gl_.value.detail.board;
	dlg.detail_.bbs.cfg.font = gl_.value.detail.font;
	dlg.detail_.bbs.cfg.popup = gl_.value.detail.popup;
	dlg.detail_.bbs.browser.dlg_.scroll.cfg_ = gl_.value.detail.scroll;
	dlg.detail_.bbs.browser.dlg_.skin.thread.splitter_ = gl_.value.detail.splitter;
	dlg.detail_.bbs.browser.dlg_.skin.thread.cfg_ = gl_.value.detail.skin.thread;
	dlg.detail_.bbs.browser.dlg_.skin.popup.cfg_ = gl_.value.detail.skin.popup;

	//-----------------------------------------------------------------

	dlg.operation_.shortcut.shortcut_ = gl_.value.operation.shortcut;
	dlg.operation_.cfg.mouse = gl_.value.operation.mouse;
	dlg.operation_.cfg.variation = gl_.value.operation.variation;
	dlg.operation_.cfg.window = gl_.value.operation.window;

	//-----------------------------------------------------------------

	dlg.display_.window.zoom_ = gl_.value.display.window.zoom;
	dlg.display_.window.size_ = gl_.value.display.window.size;
	dlg.display_.aspect.aspect_ = gl_.value.display.window.aspect;
	dlg.display_.datetime.datetime_ = gl_.value.display.datetime;
	dlg.display_.cfg.information = gl_.value.display.information;
	dlg.display_.status.status_ = gl_.value.display.status;
	dlg.display_.seek.seek_ = gl_.value.display.seek;
	dlg.display_.hint.hint_ = gl_.value.display.hint;
	dlg.display_.channel.channel_ = gl_.value.display.channel;
	dlg.display_.bbs.bbs_ = gl_.value.display.bbs;
	dlg.display_.res.res_ = gl_.value.display.res;

	dlg.display_.status.custom_ = gl_.value.display.custom.status;
	dlg.display_.seek.custom_ = gl_.value.display.custom.seek;
	dlg.display_.hint.custom_ = gl_.value.display.custom.hint;
	dlg.display_.channel.custom_ = gl_.value.display.custom.channel;
	dlg.display_.bbs.custom_ = gl_.value.display.custom.bbs;
	dlg.display_.res.custom_ = gl_.value.display.custom.res;

	int height = subpane_.getHeight();

	if (dlg.DoModal() == IDOK)
	{
		gl_.value.video.procamp = dlg.video_.adjust.value_;
		gl_.value.video.nominal = dlg.video_.adjust.nominal_;
		gl_.value.video.resizer = dlg.video_.adjust.resizer_;

		//-----------------------------------------------------------------

		gl_.value.detail.network = dlg.detail_.cfg.network;
		gl_.value.detail.type = dlg.detail_.cfg.type;
		gl_.value.detail.decoder = dlg.detail_.decoder.decoder_;
		gl_.value.detail.video = dlg.detail_.decoder.video_;
		gl_.value.detail.audio = dlg.detail_.decoder.audio_;
		gl_.value.detail.browser = dlg.detail_.cfg.browser;
		gl_.value.detail.snapshot = dlg.detail_.cfg.snapshot;
		gl_.value.detail.start = dlg.detail_.cfg.start;
		gl_.value.detail.end = dlg.detail_.end.end_;
		gl_.value.detail.board = dlg.detail_.bbs.cfg.board;
		gl_.value.detail.font = dlg.detail_.bbs.cfg.font;
		gl_.value.detail.popup = dlg.detail_.bbs.cfg.popup;
		gl_.value.detail.scroll = dlg.detail_.bbs.browser.dlg_.scroll.cfg_;
		gl_.value.detail.splitter = dlg.detail_.bbs.browser.dlg_.skin.thread.splitter_;
		gl_.value.detail.skin.thread = dlg.detail_.bbs.browser.dlg_.skin.thread.cfg_;
		gl_.value.detail.skin.popup = dlg.detail_.bbs.browser.dlg_.skin.popup.cfg_;

		gl_.player.setNetwork(gl_.value.detail.network);
		gl_.player.setType(gl_.value.detail.type);

		// フォント
		thread_.setFont(gl_.value.detail.font.thread.name, gl_.value.detail.font.thread.point);
		search_.setFont(gl_.value.detail.font.search.name, gl_.value.detail.font.search.point);
		CExtractDlg::setfont(gl_.value.detail.font.extract.name, gl_.value.detail.font.extract.point);
		
		// BBS
		LoadHtml();
		gl_.bbs.setConfig(gl_.value.detail.board);
		gl_.bbs.resizeHistory(false);
		gl_.bbs.resizeHistory(true);

		// メニューに履歴保存数適用
		thread_.createMenu();

		// スプリッタ設定
		layout_.SetSplit(gl_.value.detail.splitter.split);
		layout_.SetEdge(gl_.value.detail.splitter.edge);
		layout_.SetFace(gl_.value.detail.splitter.face);
		layout_.SetBorder(gl_.value.detail.splitter.border);
		layout_.RecalcLayout();
		layout_.RedrawWindow();

		dsp::DecoderIDSet decoderSet;
		for (auto it = gl_.value.detail.video.begin(); it != gl_.value.detail.video.end(); ++it)
		{
			if (it->enable)
			{
				decoderSet.video.id = it->id;
				decoderSet.video.name = it->name;
				break;
			}
		}

		for (auto it = gl_.value.detail.audio.begin(); it != gl_.value.detail.audio.end(); ++it)
		{
			if (it->enable)
			{
				decoderSet.audio.id = it->id;
				decoderSet.audio.name = it->name;
				break;
			}
		}

		gl_.player.setDecoderSet(decoderSet);

		//-----------------------------------------------------------------

		gl_.value.operation.shortcut = dlg.operation_.shortcut.shortcut_;
		gl_.value.operation.mouse = dlg.operation_.cfg.mouse;
		gl_.value.operation.variation = dlg.operation_.cfg.variation;
		gl_.value.operation.window = dlg.operation_.cfg.window;

		//-----------------------------------------------------------------

		gl_.value.display.window.zoom = dlg.display_.window.zoom_;
		gl_.value.display.window.size = dlg.display_.window.size_;
		gl_.value.display.window.aspect = dlg.display_.aspect.aspect_;
		gl_.value.display.datetime = dlg.display_.datetime.datetime_;
		gl_.value.display.information = dlg.display_.cfg.information;
		gl_.value.display.status = dlg.display_.status.status_;
		gl_.value.display.seek = dlg.display_.seek.seek_;
		gl_.value.display.hint = dlg.display_.hint.hint_;
		gl_.value.display.channel = dlg.display_.channel.channel_;
		gl_.value.display.bbs = dlg.display_.bbs.bbs_;
		gl_.value.display.res = dlg.display_.res.res_;

		//InitWindowSizeZoom();

		gl_.createChannelInfo();
		subpane_.SetConfig(gl_.value.display.status, gl_.value.display.channel);
		subpane_.SetConfig(gl_.value.display.seek, gl_.value.display.hint);
		subpane_.SetConfig(gl_.value.display.bbs, gl_.value.display.res);

		//RepositionWindow();

		//-----------------------------------------------------------------

		InitWindowSizeZoom();

		int h = subpane_.getHeight() - height;
		ExpandWindow(CRect(0, 0, 0, h));

		SizeHelper();

		sl_.detail = gl_.value.detail;
		sl_.display = gl_.value.display;
		sl_.operation = gl_.value.operation;
		sl_.save(gl_.current);
	}
	else
	{
		gl_.player.setProcAmp(gl_.value.video.procamp);
		gl_.player.setNominalRange(gl_.value.video.nominal);
		gl_.player.setResizer(gl_.value.video.resizer);
	}

	gl_.value.option.select.option = dlg.select_;
	gl_.value.option.select.network = dlg.detail_.network.select_;
	gl_.value.option.select.type = dlg.detail_.type.select_;
	gl_.value.option.select.information = dlg.display_.information.select_;
	gl_.value.option.select.res = dlg.display_.res.select_;
	gl_.value.option.select.bbs.common = dlg.detail_.bbs.common.select_;
	gl_.value.option.select.bbs.bar = dlg.detail_.bbs.bar.select_;
	gl_.value.option.select.bbs.browser = dlg.detail_.bbs.browser.select_;

	gl_.value.option.detail = dlg.detail_.option;
	gl_.value.option.bbs = dlg.detail_.bbs.option;
	gl_.value.option.operation = dlg.operation_.option;
	gl_.value.option.display = dlg.display_.option;

	gl_.value.display.custom.status = dlg.display_.status.custom_;
	gl_.value.display.custom.seek = dlg.display_.seek.custom_;
	gl_.value.display.custom.hint = dlg.display_.hint.custom_;
	gl_.value.display.custom.channel = dlg.display_.channel.custom_;
	gl_.value.display.custom.bbs = dlg.display_.bbs.custom_;
	gl_.value.display.custom.res = dlg.display_.res.custom_;

	StartMouseHideTimer();

	// フォーカスを返さないとレスポップアップが上手く表示されない
	if (gl_.value.window.show.edit && gl_.value.detail.board.focus)
	{
		subpane_.SetFocusBBSEdit(); // BBS popup
	}
	else
	{
		SetFocus(); // BBS popup
	}
}

void CMainDlg::OnPlayerRebuild()
{
	RebuildFile();
}

void CMainDlg::OnPlayerTerminate()
{
	gl_.player.UnblockTerminate();
}

void CMainDlg::OnPeercastBump()
{
	gl_.peca.bump(gl_.value.detail.network.relay.manual.reconnect);
}


void CMainDlg::OnPeercastStop()
{
	gl_.peca.stop(gl_.value.detail.network.relay.manual.stop);
}

void CMainDlg::OnVolumeUp()
{
	gl_.value.audio.mute = false;
	gl_.player.setMute(false);
	gl_.value.audio.volume = min(gl_.value.audio.volume + gl_.value.operation.variation.volume.normal, 100);
	gl_.player.setVolume(gl_.value.audio.volume);
}

void CMainDlg::OnVolumeUpLow()
{
	gl_.value.audio.mute = false;
	gl_.player.setMute(false);
	gl_.value.audio.volume = min(gl_.value.audio.volume + gl_.value.operation.variation.volume.low, 100);
	gl_.player.setVolume(gl_.value.audio.volume);
}

void CMainDlg::OnVolumeUpHigh()
{
	gl_.value.audio.mute = false;
	gl_.player.setMute(false);
	gl_.value.audio.volume = min(gl_.value.audio.volume + gl_.value.operation.variation.volume.high, 100);
	gl_.player.setVolume(gl_.value.audio.volume);
}

void CMainDlg::OnVolumeDown()
{
	gl_.value.audio.mute = false;
	gl_.player.setMute(false);
	gl_.value.audio.volume = max(gl_.value.audio.volume - gl_.value.operation.variation.volume.normal, 0);
	gl_.player.setVolume(gl_.value.audio.volume);
}

void CMainDlg::OnVolumeDownLow()
{
	gl_.value.audio.mute = false;
	gl_.player.setMute(false);
	gl_.value.audio.volume = max(gl_.value.audio.volume - gl_.value.operation.variation.volume.low, 0);
	gl_.player.setVolume(gl_.value.audio.volume);
}

void CMainDlg::OnVolumeDownHigh()
{
	gl_.value.audio.mute = false;
	gl_.player.setMute(false);
	gl_.value.audio.volume = max(gl_.value.audio.volume - gl_.value.operation.variation.volume.high, 0);
	gl_.player.setVolume(gl_.value.audio.volume);
}

//----------------------------------------------------------------

void CMainDlg::OnBalanceCenter()
{
	gl_.value.audio.balance = 0;
	gl_.player.setBalance(gl_.value.audio.balance);
}

void CMainDlg::OnBalanceLeft()
{
	gl_.value.audio.balance = -100;
	gl_.player.setBalance(gl_.value.audio.balance);
}

void CMainDlg::OnBalanceRight()
{
	gl_.value.audio.balance = 100;
	gl_.player.setBalance(gl_.value.audio.balance);
}

//----------------------------------------------------------------

void CMainDlg::OnSeekForward()
{
	LONGLONG cur = 0, dur = 0;
	if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
	{
		gl_.player.seek(min(cur + (10000i64 * gl_.value.operation.variation.seek.normal), dur));
	}
}

void CMainDlg::OnSeekForwardLow()
{
	LONGLONG cur = 0, dur = 0;
	if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
	{
		gl_.player.seek(min(cur + (10000i64 * gl_.value.operation.variation.seek.low), dur));
	}
}

void CMainDlg::OnSeekForwardHigh()
{
	LONGLONG cur = 0, dur = 0;
	if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
	{
		gl_.player.seek(min(cur + (10000i64 * gl_.value.operation.variation.seek.high), dur));
	}
}

void CMainDlg::OnSeekBack()
{
	LONGLONG cur = 0, dur = 0;
	if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
	{
		gl_.player.seek(max(cur - (10000i64 * gl_.value.operation.variation.seek.normal), 0));
	}
}

void CMainDlg::OnSeekBackLow()
{
	LONGLONG cur = 0, dur = 0;
	if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
	{
		gl_.player.seek(max(cur - (10000i64 * gl_.value.operation.variation.seek.low), 0));
	}
}

void CMainDlg::OnSeekBackHigh()
{
	LONGLONG cur = 0, dur = 0;
	if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
	{
		gl_.player.seek(max(cur - (10000i64 * gl_.value.operation.variation.seek.high), 0));
	}
}


void CMainDlg::OnThreadNext()
{
	gl_.bbs.scroll(gl_.value.operation.variation.scroll.normal);
}

void CMainDlg::OnThreadNextLow()
{
	gl_.bbs.scroll(gl_.value.operation.variation.scroll.low);
}

void CMainDlg::OnThreadNextHigh()
{
	gl_.bbs.scroll(gl_.value.operation.variation.scroll.high);
}

void CMainDlg::OnThreadPrev()
{
	gl_.bbs.scroll(-gl_.value.operation.variation.scroll.normal);
}

void CMainDlg::OnThreadPrevLow()
{
	gl_.bbs.scroll(-gl_.value.operation.variation.scroll.low);
}

void CMainDlg::OnThreadPrevHigh()
{
	gl_.bbs.scroll(-gl_.value.operation.variation.scroll.high);
}


void CMainDlg::OnScaleUp()
{
	WindowScale(gl_.value.operation.variation.scale.normal);
}

void CMainDlg::OnScaleUpLow()
{
	WindowScale(gl_.value.operation.variation.scale.low);
}

void CMainDlg::OnScaleUpHigh()
{
	WindowScale(gl_.value.operation.variation.scale.high);
}

void CMainDlg::OnScaleDown()
{
	WindowScale(-gl_.value.operation.variation.scale.normal);
}

void CMainDlg::OnScaleDownLow()
{
	WindowScale(-gl_.value.operation.variation.scale.low);
}

void CMainDlg::OnScaleDownHigh()
{
	WindowScale(-gl_.value.operation.variation.scale.high);
}


void CMainDlg::OnMute()
{
	gl_.player.setMute(utl::toggle(gl_.value.audio.mute));
}

void CMainDlg::OnMinimizeMute()
{
	ShowWindow(SW_MINIMIZE);
	gl_.player.setMute(true);
	minimize_ = sl::MOUSE_MCLICK_MINIMIZE_MUTE;
}

void CMainDlg::OnPause()
{
	gl_.player.togglePause();

	// スリープ抑止
	if (gl_.player.isRunning() && gl_.value.window.stop)
	{
		StartSleepStopTimer();
	}
	else
	{
		EndSleepStopTimer();
	}
}

void CMainDlg::OnMinimizePause()
{
	ShowWindow(SW_MINIMIZE);
	running_ = gl_.player.isRunning();
	if (running_) { gl_.player.pause(); }
	minimize_ = sl::MOUSE_MCLICK_MINIMIZE_PAUSE;
}

void CMainDlg::OnThreadReset()
{
	gl_.bbs.scroll(INT_MAX);
}

void CMainDlg::OnThreadReload()
{
	if (gl_.bbs.isReload())
	{
		gl_.bbs.interrupt();
	}
	else
	{
		gl_.bbs.reload();
	}
}

void CMainDlg::OnThreadRefresh()
{
	if (gl_.value.detail.scroll.refresh)
	{
		CBrowserDlg* dlg = (CBrowserDlg*)layout_.GetSub();
		if (dlg && IsWindow(dlg->GetSafeHwnd()))
		{
			dlg->startScroll();
		}
	}
	gl_.bbs.refresh();
}



void CMainDlg::OnSnapshotSave()
{
	namespace fs = boost::filesystem;

	std::wstring folder = utl::toFullpath(gl_.current, gl_.value.detail.snapshot.folder);

	//DebugText(L"folder:%s\n", folder.c_str());

	DWORD err = ::SHCreateDirectory(NULL, folder.c_str());
	if (ERROR_ALREADY_EXISTS == err ||
		ERROR_FILE_EXISTS == err ||
		ERROR_SUCCESS == err)
	{}

	std::wstring filename = utl::createSnapshotFilename(gl_.value.detail.snapshot.format);

	//DebugText(L"snapshot:%s\n", (fs::path(folder) / filename).wstring().c_str());

	UINT width = 0, height = 0;
	if (gl_.value.detail.snapshot.display)
	{
		CRect rc;
		GetVideoRect(rc);
		width = rc.Width();
		height = rc.Height();
	}

	gl_.player.saveCurrentImage(
		(fs::path(folder) / filename).wstring(),
		gl_.value.detail.snapshot.format,
		gl_.value.detail.snapshot.jpegQuality,
		width,
		height,
		gl_.value.detail.snapshot.fit);
}

void CMainDlg::OnSnapshotFolder()
{
	if (gl_.screen.full && gl_.value.window.fullscreen.topmost) { return; }

	std::wstring folder = utl::toFullpath(gl_.current, gl_.value.detail.snapshot.folder);

	::ShellExecute(NULL, L"open", folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void CMainDlg::OnWindowZoom100()
{
	//if (IsZoomed() || gl_.screen.full || gl_.screen.maximize) { return; }

	// 各種全画面から復帰
	if (IsZoomed())
	{
		ShowWindow(SW_RESTORE);
		return;
	}

	if (gl_.screen.full)
	{
		OnFullscreen();
		return;
	}

	if (gl_.screen.maximize)
	{
		OnMaximize();
		return;
	}

	SIZE size = {};
	if (gl_.player.getNativeVideoSize(size))
	{
		CRect def;
		GetWindowRect(&def);

		CRect src(0, 0, size.cx, size.cy);
		CRect dst;

		SIZE aspect = {};
		gl_.getAspect(aspect);

		if (!utl::getAspectRect(aspect, src, dst))
		{
			dst = src;
		}

		CRect rc(0, 0, dst.Width(), dst.Height());
		InflateWindowRect(rc);

		// 100%サイズとウィンドウサイズが同じサイズなら
		if (def.Width() == rc.Width() && def.Height() == rc.Height())
		{
			int type = gl_.player.online() ?
				gl_.value.operation.mouse.online.ldblclick :
				gl_.value.operation.mouse.offline.ldblclick;

			switch (type)
			{
				case sl::MOUSE_LDBLCLICK_ZOOM100_FULLSCREEN: OnFullscreen(); return;
				case sl::MOUSE_LDBLCLICK_ZOOM100_MAXIMIZE: OnMaximize(); return;
			}
		}

		box_.showWindow(false);
		SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_NOZORDER);
	}
}


void CMainDlg::OnWindowZoomRange(UINT nID)
{
	if (IsZoomed() || gl_.screen.full || gl_.screen.maximize) { return; }
	int i = nID - IDM_WINDOW_ZOOM_RANGE_BEGIN;
	if (i < 0 || i >= (int)windowZoom_.size()) { return; }

	try {
		sl::WindowZoom x = windowZoom_.at(i);
		WindowZoom(x.zoom);
	} catch(...) {}

	//try {
	//	SIZE size = {};
	//	if (gl_.player.getNativeVideoSize(size))
	//	{
	//		CRect src(0, 0, size.cx, size.cy);
	//		CRect dst;

	//		SIZE aspect = {};
	//		gl_.getAspect(aspect);

	//		if (utl::getAspectRect(aspect, src, dst))
	//		{
	//			sl::WindowZoom x = windowZoom_.at(i);
	//			double zoom = (double)x.zoom / 100.0;

	//			CRect rc(0, 0, utl::lround((double)dst.Width() * zoom), utl::lround((double)dst.Height() * zoom));
	//			InflateWindowRect(rc);

	//			box_.showWindow(false);
	//			SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOMOVE|SWP_NOZORDER);
	//		}
	//	}
	//} catch(...) {}
}

void CMainDlg::OnWindowSizeRange(UINT nID)
{
	if (IsZoomed() || gl_.screen.full || gl_.screen.maximize) { return; }
	int i = nID - IDM_WINDOW_SIZE_RANGE_BEGIN;
	if (i < 0 || i >= (int)windowSize_.size()) { return; }

	try {
		sl::WindowSize x = windowSize_.at(i);
		AbsoluteWindowSize(x.width, x.height);
	} catch(...) {}
}

void CMainDlg::OnWindowAspectRange(UINT nID)
{
	int i = nID - IDM_WINDOW_ASPECT_RANGE_BEGIN;
	if (i < 0 || i >= (int)gl_.value.display.window.aspect.size()) { return; }

	gl_.value.window.aspect = sl::WINDOW_ASPECT_CUSTOM + i;
	ApplyWindowRatio(gl_.value.window.ratio.fixed);
}

void CMainDlg::OnWindowAspectDefault()
{
	gl_.value.window.aspect = sl::WINDOW_ASPECT_DEFAULT;
	ApplyWindowRatio(gl_.value.window.ratio.fixed);
}


void CMainDlg::OnWindowAspectNone()
{
	gl_.value.window.aspect = sl::WINDOW_ASPECT_NONE;
	ApplyWindowRatio(gl_.value.window.ratio.fixed);
}


void CMainDlg::OnWindowTopmost()
{
	const CWnd* wnd = utl::toggle(gl_.value.window.show.topmost) ? &wndTopMost : &wndNoTopMost;
	SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

void CMainDlg::OnWindowScreenSnap()
{
	utl::toggle(gl_.value.window.snap.screen);
}


void CMainDlg::OnWindowWindowSnap()
{
	utl::toggle(gl_.value.window.snap.window);
}


void CMainDlg::OnWindowFixedRatio()
{
	utl::toggle(gl_.value.window.ratio.fixed);
	ApplyWindowRatio(gl_.value.window.ratio.fixed);
}

void CMainDlg::OnWindowPlayRatio()
{
	utl::toggle(gl_.value.window.ratio.play);
}

void CMainDlg::OnWindowHeightRatio()
{
	utl::toggle(gl_.value.window.ratio.height);
}

void CMainDlg::OnWindowInscribeRatio()
{
	utl::toggle(gl_.value.window.ratio.inscribe);
}




void CMainDlg::OnWindowFrame()
{
	if (IsZoomed() || gl_.screen.full || gl_.screen.maximize) { return; }

	CRect rc;
	GetWindowRect(&rc);

	if (gl_.value.window.show.frame)
	{
		gl_.value.window.show.frame = false;
		if (gl_.value.window.show.title)
		{
			utl::deflateWindowRect(rc, true, true);
		}
		else
		{
			utl::deflateWindowRect(rc, false, true);
		}

		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

		//ModifyStyle(WS_CAPTION|WS_THICKFRAME, 0);
		ModifyStyle(0, WS_CAPTION|WS_THICKFRAME);
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
	}
	else
	{
		gl_.value.window.show.frame = true;
		if (gl_.value.window.show.title)
		{
			utl::inflateWindowRect(rc, true, true);
			SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

			ModifyStyle(0, WS_CAPTION|WS_THICKFRAME);
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
		}
		else
		{
			utl::inflateWindowRect(rc, false, true);
			SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

			ModifyStyle(WS_CAPTION, WS_THICKFRAME);
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
		}
	}
}


void CMainDlg::OnWindowTitle()
{
	// フルスクリーンとフレームが無いときは無効
	if (IsZoomed() || gl_.screen.full || gl_.screen.maximize || !gl_.value.window.show.frame)
	{
		return;
	}

	CRect rc;
	GetWindowRect(&rc);

	if (gl_.value.window.show.title)
	{
		//utl::deflateWindowRect(rc, true, false);
		//ModifyStyle(WS_CAPTION, 0);
		//SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_FRAMECHANGED);

		utl::deflateWindowRect(rc, true, false);
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

		ModifyStyle(WS_CAPTION, 0);
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

		gl_.value.window.show.title = false;
	}
	else
	{
		//utl::inflateWindowRect(rc, true, false);
		//ModifyStyle(0, WS_CAPTION);
		//SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_FRAMECHANGED);

		utl::inflateWindowRect(rc, true, false);
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

		ModifyStyle(0, WS_CAPTION);
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

		gl_.value.window.show.title = true;
	}
}

void CMainDlg::OnWindowStatus()
{
	utl::toggle(gl_.value.window.show.status);
	ApplySubpane(false, false, true);
}

void CMainDlg::OnWindowSeek()
{
	utl::toggle(gl_.value.window.show.seek);
	ApplySubpane(false, true, false);
}

void CMainDlg::OnWindowBbs()
{
	utl::toggle(gl_.value.window.show.bbs);
	ApplySubpane(true, false, false);
}

void CMainDlg::OnWindowEdit()
{
	utl::toggle(gl_.value.window.show.edit);
	ApplySubpane(true, false, false);
}


void CMainDlg::OnWindowBrowser()
{
	utl::toggle(gl_.value.window.show.browser);
	ShowBrowser(gl_.value.window.show.browser);
	
	//if (gl_.player.online())
	//{
	//	int fix = layout_.GetIdeal() + layout_.GetSplit();

	//	if (!gl_.value.window.show.browser)
	//	{
	//		fix = -fix;
	//	}

	//	switch (layout_.GetSide())
	//	{
	//	case CLayoutCtrl::SIDE_LEFT:	ExpandWindow(CRect(fix, 0, 0, 0)); break;
	//	case CLayoutCtrl::SIDE_TOP:		ExpandWindow(CRect(0, fix, 0, 0)); break;
	//	case CLayoutCtrl::SIDE_RIGHT:	ExpandWindow(CRect(0, 0, fix, 0)); break;
	//	case CLayoutCtrl::SIDE_BOTTOM:	ExpandWindow(CRect(0, 0, 0, fix)); break;
	//	}

	//	ShowBrowser(gl_.value.window.show.browser);
	//}
}

void CMainDlg::OnWindowBrowserTop()
{
	gl_.value.window.show.side = CLayoutCtrl::SIDE_TOP;
	layout_.SetSide(gl_.value.window.show.side);
	layout_.RecalcLayout();
	layout_.RedrawAll();
}

void CMainDlg::OnWindowBrowserBottom()
{
	gl_.value.window.show.side = CLayoutCtrl::SIDE_BOTTOM;
	layout_.SetSide(gl_.value.window.show.side);
	layout_.RecalcLayout();
	layout_.RedrawAll();
}

void CMainDlg::OnWindowBrowserLeft()
{
	gl_.value.window.show.side = CLayoutCtrl::SIDE_LEFT;
	layout_.SetSide(gl_.value.window.show.side);
	layout_.RecalcLayout();
	layout_.RedrawAll();
}

void CMainDlg::OnWindowBrowserRight()
{
	gl_.value.window.show.side = CLayoutCtrl::SIDE_RIGHT;
	layout_.SetSide(gl_.value.window.show.side);
	layout_.RecalcLayout();
	layout_.RedrawAll();
}


void CMainDlg::OnWindowBrowserToggleTop()
{
	ToggleShowBrowser(CLayoutCtrl::SIDE_TOP);
}

void CMainDlg::OnWindowBrowserToggleBottom()
{
	ToggleShowBrowser(CLayoutCtrl::SIDE_BOTTOM);
}

void CMainDlg::OnWindowBrowserToggleLeft()
{
	ToggleShowBrowser(CLayoutCtrl::SIDE_LEFT);
}

void CMainDlg::OnWindowBrowserToggleRight()
{
	ToggleShowBrowser(CLayoutCtrl::SIDE_RIGHT);
}



void CMainDlg::OnSleepStop()
{
	utl::toggle(gl_.value.window.stop);

	// スリープ抑止
	if (gl_.player.isRunning() && gl_.value.window.stop)
	{
		StartSleepStopTimer();
	}
	else
	{
		EndSleepStopTimer();
	}
}



void CMainDlg::OnQuit()
{
	SendMessage(WM_CLOSE);
}

void CMainDlg::OnQuitStop()
{
	stop_ = true;
	SendMessage(WM_CLOSE);
}



void CMainDlg::OnFullscreen()
{
	if (IsZoomed())
	{
		ShowWindow(SW_RESTORE);
		return;
	}

	if (gl_.screen.maximize)
	{
		OnMaximize();
		return;
	}


	if (gl_.screen.full)
	{
		gl_.screen.full = false;

		// サブペイン子ウィンドウ化
		if (subpane_.isPopup())
		{
			RecreateSubpane(false);

			if (!gl_.value.detail.board.focus)
			{// フローティングから復帰後にフォーカスがBBSバーに移らないように
				SetFocus(); // BBS
			}
		}
		else
		{
			subpane_.ShowWindow(SW_SHOWNA);
		}

		// ウィンドウ位置復元
		SetWindowPlacement(&gl_.screen.restore);

		// ウィンドウサイズとウィンドウスタイルを同時に変えない
		if (gl_.value.window.show.title && gl_.value.window.show.frame ||
			!gl_.value.window.show.frame)
		{
			ModifyStyle(0, WS_CAPTION|WS_THICKFRAME);
		}
		else
		{
			ModifyStyle(WS_CAPTION, WS_THICKFRAME);
		}

		// 最前面化解除とスタイル変更の適用
		const CWnd* wnd = gl_.value.window.show.topmost ? &wndTopMost : &wndNoTopMost;
		SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);

		SizeHelper();

		Invalidate();
		UpdateWindow();

		EndMouseHideTimer();
	}
	else
	{
		::ZeroMemory(&gl_.screen.restore, sizeof(WINDOWPLACEMENT));
		gl_.screen.restore.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&gl_.screen.restore))
		{
			// ウィンドウをデスクトップサイズに
			POINT pt;
			if (::GetCursorPos(&pt))
			{
				MONITORINFO mi;
				if (utl::getDesktopRectFromPoint(pt, mi))
				{
					gl_.screen.full = true;

					// サブペインポップアップ化
					if (gl_.value.window.fullscreen.hide && gl_.value.window.fullscreen.floating)
					{
						RecreateSubpane(true);
					}
					else if (!gl_.value.window.fullscreen.hide)
					{
						subpane_.ShowWindow(SW_SHOWNA);
					}
					else
					{
						subpane_.ShowWindow(SW_HIDE);
					}

					ModifyStyle(WS_CAPTION|WS_THICKFRAME, 0, 0);
					//ModifyStyle(0, WS_CAPTION|WS_THICKFRAME);
#ifdef _DEBUG
					// デバッグモードでブレークしたとき
					// 最前面全画面だと操作を一切受け付けなくなるため
					const CWnd* wnd = &wndNoTopMost;
#else
					const CWnd* wnd = gl_.value.window.fullscreen.topmost ? &wndTopMost : &wndNoTopMost;
#endif

					SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);

					SetWindowPos(NULL,
						mi.rcMonitor.left, mi.rcMonitor.top,
						mi.rcMonitor.right - mi.rcMonitor.left,
						mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER);



					SizeHelper();

					Invalidate();
					UpdateWindow();

					StartMouseHideTimer();
				}
			}
		}
	}
}


void CMainDlg::OnMaximize()
{
	if (IsZoomed())
	{
		ShowWindow(SW_RESTORE);
		return;
	}

	if (gl_.screen.full)
	{
		OnFullscreen();
		return;
	}

	if (gl_.screen.maximize)
	{
		gl_.screen.maximize = false;

		subpane_.ShowWindow(SW_HIDE);

		// ウィンドウ位置復元
		SetWindowPlacement(&gl_.screen.restore);

		// ウィンドウサイズとウィンドウスタイルを同時に変えない
		if (gl_.value.window.show.title && gl_.value.window.show.frame ||
			!gl_.value.window.show.frame)
		{
			ModifyStyle(0, WS_CAPTION|WS_THICKFRAME);
		}
		else
		{
			ModifyStyle(WS_CAPTION, WS_THICKFRAME);
		}

		// スタイル変更の適用
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

		subpane_.ShowWindow(SW_SHOWNA);

		SizeHelper();

		Invalidate();
		UpdateWindow();
	}
	else
	{
		::ZeroMemory(&gl_.screen.restore, sizeof(WINDOWPLACEMENT));
		gl_.screen.restore.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&gl_.screen.restore))
		{
			// ウィンドウをデスクトップサイズに
			POINT pt;
			if (::GetCursorPos(&pt))
			{
				MONITORINFO mi;
				if (utl::getDesktopRectFromPoint(pt, mi))
				{
					gl_.screen.maximize = true;

					subpane_.ShowWindow(SW_HIDE);

					ModifyStyle(WS_CAPTION|WS_THICKFRAME, 0, 0);
					//ModifyStyle(0, WS_CAPTION|WS_THICKFRAME);

					CRect rc;
					if (gl_.value.window.maximize.taskbar)
					{
						rc = mi.rcWork;
					}
					else
					{
						rc = mi.rcMonitor;
					}

					SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

					SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

					subpane_.ShowWindow(SW_SHOWNA);

					SizeHelper();

					Invalidate();
					UpdateWindow();
				}
			}
		}
	}
}


void CMainDlg::OnFilterGraphRange(UINT nID)
{
	int i = nID - IDM_FILTER_GRAPH_RANGE_BEGIN;

	gl_.player.showFilterProperty(i, GetSafeHwnd(), AfxGetApp()->m_hInstance, GetCurrentThreadId());
}


void CMainDlg::OnPrevFile()
{
	OpenNextFile(false, true);
}


void CMainDlg::OnNextFile()
{
	OpenNextFile(false, false);
}

void CMainDlg::OnPrevFileLoop()
{
	OpenNextFile(true, true);
}


void CMainDlg::OnNextFileLoop()
{
	OpenNextFile(true, false);
}



void CMainDlg::OnPlayModeNormal()
{
	gl_.value.play.mode = sl::PLAY_MODE_NORMAL;
}


void CMainDlg::OnPlayModeRepeat()
{
	gl_.value.play.mode = sl::PLAY_MODE_REPEAT;
}


void CMainDlg::OnPlayModeFolder()
{
	gl_.value.play.mode = sl::PLAY_MODE_FOLDER;
}


void CMainDlg::OnPlayModeFolderLoop()
{
	gl_.value.play.mode = sl::PLAY_MODE_FOLDER_LOOP;
}





void CMainDlg::OnProcessPriorityRealtime()
{
	gl_.value.window.priority = sl::PROCESS_PRIORITY_REALTIME;
	SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());
}


void CMainDlg::OnProcessPriorityHigh()
{
	gl_.value.window.priority = sl::PROCESS_PRIORITY_HIGH;
	SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());
}


void CMainDlg::OnProcessPriorityAboveNormal()
{
	gl_.value.window.priority = sl::PROCESS_PRIORITY_ABOVE_NORMAL;
	SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());
}


void CMainDlg::OnProcessPriorityNormal()
{
	gl_.value.window.priority = sl::PROCESS_PRIORITY_NORMAL;
	SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());
}


void CMainDlg::OnProcessPriorityBelowNomal()
{
	gl_.value.window.priority = sl::PROCESS_PRIORITY_BELOW_NOMAL;
	SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());
}


void CMainDlg::OnProcessPriorityIdle()
{
	gl_.value.window.priority = sl::PROCESS_PRIORITY_IDLE;
	SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());
}


void CMainDlg::OnUpdateChannelInfo()
{
	gl_.peca.info();
}

void CMainDlg::OnChannelInfoName()
{
	utl::toggle(gl_.value.window.status.channel.name);
	gl_.createChannelInfo();
}

void CMainDlg::OnChannelInfoType()
{
	utl::toggle(gl_.value.window.status.channel.type);
	gl_.createChannelInfo();
}

void CMainDlg::OnChannelInfoGenre()
{
	utl::toggle(gl_.value.window.status.channel.genre);
	gl_.createChannelInfo();
}

void CMainDlg::OnChannelInfoComment()
{
	utl::toggle(gl_.value.window.status.channel.comment);
	gl_.createChannelInfo();
}

void CMainDlg::OnChannelInfoPlaying()
{
	utl::toggle(gl_.value.window.status.channel.playing);
	gl_.createChannelInfo();
}

void CMainDlg::OnChannelInfoRelay()
{
	utl::toggle(gl_.value.window.status.channel.relay);
	gl_.createChannelInfo();
}



void CMainDlg::OnBbsBrowser()
{
	if (gl_.player.peercast())
	{
		std::wstring url;
		GetContactURL(url);
		if (!url.empty())
		{
			OpenProcess(gl_.value.detail.browser.bbs, url);
		}
	}
}


void CMainDlg::OnWebBrowser()
{
	if (gl_.player.peercast())
	{
		std::wstring url;
		GetContactURL(url);
		OpenWebBrowser(url);
	}
}



void CMainDlg::OnOpenfileDialog()
{
	COpenFileDlg dlg;
	dlg.cfg_ = gl_.value.dialog.file;
	if (dlg.DoModal() == IDOK)
	{
		OpenFile(dlg.path_);
	}
	gl_.value.dialog.file = dlg.cfg_;
}


void CMainDlg::OnOpenfileClipboard()
{
	if (OpenClipboard())
	{
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle)
		{
			WCHAR* pszData = (WCHAR*)GlobalLock(handle);

			std::wstring path(pszData);

			GlobalUnlock(handle);

			OpenFile(path);
		}

		CloseClipboard();
	}
}


void CMainDlg::OnClipboardPath()
{
	if (OpenClipboard())
	{
		EmptyClipboard();

		size_t size = gl_.latest.path.size() + 1;
		HGLOBAL hg = GlobalAlloc(GHND|GMEM_SHARE, sizeof(WCHAR) * size);
		LPWSTR mem = (LPWSTR)GlobalLock(hg);
		
		wcsncpy_s(mem, size, gl_.latest.path.c_str(), _TRUNCATE);
		GlobalUnlock(hg);

		SetClipboardData(CF_UNICODETEXT, hg);

		CloseClipboard();
	}
}


void CMainDlg::OnClipboardUrl()
{
	if (gl_.player.peercast())
	{
		std::wstring url;
		GetContactURL(url);

		if (OpenClipboard())
		{
			EmptyClipboard();

			size_t size = url.size() + 1;
			HGLOBAL hg = GlobalAlloc(GHND|GMEM_SHARE, sizeof(WCHAR) * size);
			LPWSTR mem = (LPWSTR)GlobalLock(hg);

			wcsncpy_s(mem, size, url.c_str(), _TRUNCATE);
			GlobalUnlock(hg);

			SetClipboardData(CF_UNICODETEXT, hg);

			CloseClipboard();
		}
	}
}


void CMainDlg::OnVersion()
{
	CVersionDlg dlg;
	if (dlg.DoModal() == IDOK)
	{

	}
}

void CMainDlg::OnLog()
{
	if (!::IsWindow(log_.GetSafeHwnd()))
	{
		log_.Create(CLogDlg::IDD, this);
		log_.ShowWindow(SW_SHOWNORMAL);
	}
}


void CMainDlg::OnFullscreenHideBars()
{
	utl::toggle(gl_.value.window.fullscreen.hide);

	if (gl_.screen.full)
	{
		if (gl_.value.window.fullscreen.hide && gl_.value.window.fullscreen.floating)
		{
			RecreateSubpane(true);
		}
		else if (subpane_.isPopup())
		{
			RecreateSubpane(false);
		}

		subpane_.ShowWindow(gl_.value.window.fullscreen.hide ? SW_HIDE : SW_SHOWNA);
	}

	SizeHelper();
}

void CMainDlg::OnFullscreenFloating()
{
	utl::toggle(gl_.value.window.fullscreen.floating);

	if (gl_.screen.full)
	{
		if (gl_.value.window.fullscreen.hide && gl_.value.window.fullscreen.floating)
		{
			RecreateSubpane(true);
		}
		else if (subpane_.isPopup())
		{
			RecreateSubpane(false);
		}

		subpane_.ShowWindow(gl_.value.window.fullscreen.hide ? SW_HIDE : SW_SHOWNA);
	}

	SizeHelper();
}


void CMainDlg::OnFullscreenMouse()
{
	utl::toggle(gl_.value.window.fullscreen.mouse);

	EndMouseHideTimer();
}


void CMainDlg::OnFullscreenTopmost()
{
	utl::toggle(gl_.value.window.fullscreen.topmost);

	if (gl_.screen.full)
	{
		const CWnd* wnd = gl_.value.window.fullscreen.topmost ? &wndTopMost : &wndNoTopMost;
		SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
}



void CMainDlg::OnMiximizeToFullscreen()
{
	utl::toggle(gl_.value.window.maximize.fullscreen);
}


void CMainDlg::OnMiximizeShowTaskbar()
{
	utl::toggle(gl_.value.window.maximize.taskbar);
}


void CMainDlg::OnTitlebarShowPlayerName()
{
	utl::toggle(gl_.value.window.title.player);
	SetTitleText();
}


void CMainDlg::OnTitlebarShowWindowSize()
{
	utl::toggle(gl_.value.window.title.size);
	SetTitleText();
}


void CMainDlg::OnTitlebarShowSourceSize()
{
	utl::toggle(gl_.value.window.title.source);
	SetTitleText();
}

void CMainDlg::OnStatusbarShowFps()
{
	utl::toggle(gl_.value.window.status.fps);
	subpane_.invalidateStatus();
}

void CMainDlg::OnStatusbarShowWindowSize()
{
	utl::toggle(gl_.value.window.status.size);
	subpane_.invalidateStatus();
}

void CMainDlg::OnStatusbarTime()
{
	utl::toggle(gl_.value.window.status.time);
	subpane_.invalidateStatus();
}

void CMainDlg::OnStatusbarPlay()
{
	utl::toggle(gl_.value.window.status.play);
	subpane_.invalidateStatus();
}


void CMainDlg::OnSeekbarShowFilename()
{
	utl::toggle(gl_.value.window.seek.filename);
	subpane_.invalidateSeek();
}

void CMainDlg::OnBbsbarShowCounter()
{
	utl::toggle(gl_.value.window.bbs.counter);
	subpane_.invalidateBBS();
}


void CMainDlg::OnStartWindowZoom100()
{
	utl::toggle(gl_.value.window.zoom);
}

void CMainDlg::OnThread()
{
	if (!::IsWindow(thread_))
	{
		thread_.Create(IDD_THREAD, this);
		thread_.ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		thread_.SetForegroundWindow();
	}



	//EndMouseHideTimer();

	//CThreadDlg dlg;
	//dlg.cfg_ = gl_.value.dialog.thread;
	//if (dlg.DoModal() == IDOK)
	//{

	//}
	//gl_.value.dialog.thread = dlg.cfg_;

	////sl_.dialog.thread = gl_.value.dialog.thread;
	////sl_.save(gl_.current);

	//StartMouseHideTimer();
}



void CMainDlg::OnBbsMenu()
{
	EndMouseHideTimer();

	CMenu root;
	CMenu* menu = NULL;
	if (!root.LoadMenu(IDR_MAINMENU)) { return; }
	
	menu = root.GetSubMenu(2);
	if (menu == NULL) { return; }
	//---------------------------------------------------

	auto checkRadioItem = [=](int id){
		menu->CheckMenuRadioItem(id, id, id, MF_BYCOMMAND);
	};

	auto checkItem = [=](int id, bool check){
		if (check)
		{
			menu->CheckMenuItem(id, MF_CHECKED|MF_BYCOMMAND);
		}
	};

	checkItem(IDM_WINDOW_EDIT, gl_.value.window.show.edit);

	checkItem(IDM_WINDOW_BROWSER, gl_.value.window.show.browser);

	switch (gl_.value.window.show.side)
	{
	case CLayoutCtrl::SIDE_TOP:		checkRadioItem(IDM_WINDOW_BROWSER_TOP); break;
	case CLayoutCtrl::SIDE_BOTTOM:	checkRadioItem(IDM_WINDOW_BROWSER_BOTTOM); break;
	case CLayoutCtrl::SIDE_LEFT:	checkRadioItem(IDM_WINDOW_BROWSER_LEFT); break;
	case CLayoutCtrl::SIDE_RIGHT:	checkRadioItem(IDM_WINDOW_BROWSER_RIGHT); break;
	}
	checkItem(IDM_BBSBAR_SHOW_COUNTER, gl_.value.window.bbs.counter);
	checkItem(IDM_THREAD_RELOAD, gl_.bbs.isReload());
	checkItem(IDM_BBS_SAGE, gl_.value.dialog.bbs.sage);

	{// リンク
		if (gl_.bbs.getLinks(links_))
		{
			int i = 0;
			for (auto it = links_.begin(); it != links_.end(); ++it)
			{
				const int len = 256;
				CString str;
				str.Format(L"%s%s", it->substr(0, len).c_str(), it->size() > len ? L"..." : L"");

				menu->InsertMenu(
					IDM_BBS_LINK_MENU,
					MF_BYCOMMAND|MF_STRING,
					IDM_BBS_LINK_RANGE_BEGIN + i,
					str);
				i++;
				if (i >= 100) { break; }
			}

			if (!links_.empty())
			{
				menu->InsertMenu(IDM_BBS_LINK_MENU, MF_BYCOMMAND|MF_SEPARATOR);
			}
		}
		menu->DeleteMenu(IDM_BBS_LINK_MENU, MF_BYCOMMAND);
	}

	//---------------------------------------------------
	CPoint pt;
	GetCursorPos(&pt);
	menu->TrackPopupMenu(TPM_LEFTBUTTON, pt.x, pt.y, this);
	root.DestroyMenu();
}


void CMainDlg::OnBbsSage()
{
	utl::toggle(gl_.value.dialog.bbs.sage);
}


void CMainDlg::OnBbsLinkRange(UINT nID)
{
	int i = nID - IDM_BBS_LINK_RANGE_BEGIN;

	if (gl_.player.online())
	{
		try {
			OpenWebBrowser(links_.at(i));
		} catch (...) {}
	}
}

void CMainDlg::OnUpdateTitle()
{
	gl_.createChannelInfo();
	SetTitleText();
}


void CMainDlg::OnContextSearch()
{
	std::wstring search = utl::ContextMenu::pop();
	if (!search.empty())
	{
		search_.setSearch(search);
	}
	else
	{
		CBrowserDlg* dlg = (CBrowserDlg*)layout_.GetSub();
		CComPtr<IHTMLTxtRange> range;
		if (dlg && dlg->GetSelectedTxtRange(range))
		{
			CComBSTR str;
			if (SUCCEEDED(range->get_text(&str)))
			{
				search_.setSearch(std::wstring(CString(str).GetString()));
			}
		}
	}

	if (!::IsWindow(search_))
	{
		CBrowserDlg* browser = (CBrowserDlg*)layout_.GetSub();
		if (browser)
		{
			search_.setBrowserDlg(browser);
			search_.Create(IDD_SEARCH, this);
			search_.ShowWindow(SW_SHOWNORMAL);
		}
	}
	else
	{
		search_.SetForegroundWindow();
	}
}

void CMainDlg::OnContextExtract()
{
	CExtractDlg* dlg = CExtractDlg::make();
	if (dlg)
	{
		dlg->setSearch(utl::ContextMenu::pop());
		dlg->setCustomDHtmlDialog((CBrowserDlg*)layout_.GetSub());
		dlg->Create(IDD_EXTRACT, this);
		dlg->ShowWindow(SW_SHOWNORMAL);
	}
}

void CMainDlg::OnContextPost()
{
	gl_.value.window.show.edit = true;
	ApplySubpane(true, false, false);

	subpane_.setMessage(utl::ContextMenu::pop());
}

void CMainDlg::OnContextBbsBrowser()
{
	std::wstring url = utl::ContextMenu::pop();
	if (!url.empty())
	{
		OpenProcess(gl_.value.detail.browser.bbs, url);
	}
}

