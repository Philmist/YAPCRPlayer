
// PCRPlayerDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "MainDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainDlg ダイアログ


enum TIMER_ID {
	TIMER_ID_MOUSEHIDE,
	TIMER_ID_SLEEPSTOP,
};

typedef enum : int {
	TIMER_STATUS = 0,
	TIMER_BUFFERING,
} TIMER_TYPE;




CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
	, boot_(false)
	, nccalcsize_(true)
	, mouseCaptureR_(false)
	, mouseClickWheek_(false)
	, minimize_(-1)
	, running_(false)
	, stop_(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	gl_.peca.setTitleCallback([this](){
		if (IsWindow(this->GetSafeHwnd()))
		{
			this->PostMessage(WM_COMMAND, IDM_UPDATE_TITLE);
		}
	});
}

CMainDlg::~CMainDlg()
{
	gl_.peca.setTitleCallback([](){});
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_MESSAGE(WM_GRAPH_NOTIFY, &CMainDlg::OnGraphNotify)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_MOVE()
	ON_WM_EXITSIZEMOVE()
	ON_WM_DROPFILES()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_QUERYOPEN()
	ON_WM_SYSCOMMAND()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SIZING()
	ON_WM_MOVING()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_EXITSIZEMOVE()
	ON_WM_NCCALCSIZE()
	ON_COMMAND(IDM_OPTION, &CMainDlg::OnOption)
	ON_COMMAND(IDM_PLAYER_REBUILD, &CMainDlg::OnPlayerRebuild)
	ON_COMMAND(IDM_PLAYER_TERMINATE, &CMainDlg::OnPlayerTerminate)
	ON_COMMAND(IDM_PEERCAST_BUMP, &CMainDlg::OnPeercastBump)
	ON_COMMAND(IDM_PEERCAST_STOP, &CMainDlg::OnPeercastStop)
	ON_COMMAND(IDM_VOLUME_UP, &CMainDlg::OnVolumeUp)
	ON_COMMAND(IDM_VOLUME_UP_LOW, &CMainDlg::OnVolumeUpLow)
	ON_COMMAND(IDM_VOLUME_UP_HIGH, &CMainDlg::OnVolumeUpHigh)
	ON_COMMAND(IDM_VOLUME_DOWN, &CMainDlg::OnVolumeDown)
	ON_COMMAND(IDM_VOLUME_DOWN_LOW, &CMainDlg::OnVolumeDownLow)
	ON_COMMAND(IDM_VOLUME_DOWN_HIGH, &CMainDlg::OnVolumeDownHigh)
	ON_COMMAND(IDM_SEEK_FORWARD, &CMainDlg::OnSeekForward)
	ON_COMMAND(IDM_SEEK_FORWARD_LOW, &CMainDlg::OnSeekForwardLow)
	ON_COMMAND(IDM_SEEK_FORWARD_HIGH, &CMainDlg::OnSeekForwardHigh)
	ON_COMMAND(IDM_SEEK_BACK, &CMainDlg::OnSeekBack)
	ON_COMMAND(IDM_SEEK_BACK_LOW, &CMainDlg::OnSeekBackLow)
	ON_COMMAND(IDM_SEEK_BACK_HIGH, &CMainDlg::OnSeekBackHigh)
	ON_COMMAND(IDM_THREAD_NEXT, &CMainDlg::OnThreadNext)
	ON_COMMAND(IDM_THREAD_NEXT_LOW, &CMainDlg::OnThreadNextLow)
	ON_COMMAND(IDM_THREAD_NEXT_HIGH, &CMainDlg::OnThreadNextHigh)
	ON_COMMAND(IDM_THREAD_PREV, &CMainDlg::OnThreadPrev)
	ON_COMMAND(IDM_THREAD_PREV_LOW, &CMainDlg::OnThreadPrevLow)
	ON_COMMAND(IDM_THREAD_PREV_HIGH, &CMainDlg::OnThreadPrevHigh)
	ON_COMMAND(IDM_SCALE_UP, &CMainDlg::OnScaleUp)
	ON_COMMAND(IDM_SCALE_UP_LOW, &CMainDlg::OnScaleUpLow)
	ON_COMMAND(IDM_SCALE_UP_HIGH, &CMainDlg::OnScaleUpHigh)
	ON_COMMAND(IDM_SCALE_DOWN, &CMainDlg::OnScaleDown)
	ON_COMMAND(IDM_SCALE_DOWN_LOW, &CMainDlg::OnScaleDownLow)
	ON_COMMAND(IDM_SCALE_DOWN_HIGH, &CMainDlg::OnScaleDownHigh)
	ON_COMMAND(IDM_MUTE, &CMainDlg::OnMute)
	ON_COMMAND(IDM_MINIMIZE_MUTE, &CMainDlg::OnMinimizeMute)
	ON_COMMAND(IDM_PAUSE, &CMainDlg::OnPause)
	ON_COMMAND(IDM_MINIMIZE_PAUSE, &CMainDlg::OnMinimizePause)
	ON_COMMAND(IDM_THREAD_RESET, &CMainDlg::OnThreadReset)
	ON_COMMAND(IDM_THREAD_RELOAD, &CMainDlg::OnThreadReload)
	ON_COMMAND(IDM_THREAD_REFRESH, &CMainDlg::OnThreadRefresh)
	ON_COMMAND(IDM_SNAPSHOT_SAVE, &CMainDlg::OnSnapshotSave)
	ON_COMMAND(IDM_SNAPSHOT_FOLDER, &CMainDlg::OnSnapshotFolder)
	ON_COMMAND(IDM_WINDOW_ZOOM_100, &CMainDlg::OnWindowZoom100)
	ON_COMMAND_RANGE(IDM_WINDOW_ZOOM_RANGE_BEGIN, IDM_WINDOW_ZOOM_RANGE_END, &CMainDlg::OnWindowZoomRange)
	ON_COMMAND_RANGE(IDM_WINDOW_SIZE_RANGE_BEGIN, IDM_WINDOW_SIZE_RANGE_END, &CMainDlg::OnWindowSizeRange)
	ON_COMMAND_RANGE(IDM_WINDOW_ASPECT_RANGE_BEGIN, IDM_WINDOW_ASPECT_RANGE_END, &CMainDlg::OnWindowAspectRange)
	ON_COMMAND(IDM_WINDOW_ASPECT_DEFAULT, &CMainDlg::OnWindowAspectDefault)
	ON_COMMAND(IDM_WINDOW_ASPECT_NONE, &CMainDlg::OnWindowAspectNone)
	ON_COMMAND(IDM_WINDOW_TOPMOST, &CMainDlg::OnWindowTopmost)
	ON_COMMAND(IDM_WINDOW_SCREEN_SNAP, &CMainDlg::OnWindowScreenSnap)
	ON_COMMAND(IDM_WINDOW_WINDOW_SNAP, &CMainDlg::OnWindowWindowSnap)
	ON_COMMAND(IDM_WINDOW_FIXED_RATIO, &CMainDlg::OnWindowFixedRatio)
	ON_COMMAND(IDM_WINDOW_PLAY_RATIO, &CMainDlg::OnWindowPlayRatio)
	ON_COMMAND(IDM_WINDOW_HEIGHT_RATIO, &CMainDlg::OnWindowHeightRatio)
	ON_COMMAND(IDM_WINDOW_INSCRIBE_RATIO, &CMainDlg::OnWindowInscribeRatio)
	ON_COMMAND(IDM_WINDOW_FRAME, &CMainDlg::OnWindowFrame)
	ON_COMMAND(IDM_WINDOW_TITLE, &CMainDlg::OnWindowTitle)
	ON_COMMAND(IDM_WINDOW_STATUS, &CMainDlg::OnWindowStatus)
	ON_COMMAND(IDM_WINDOW_SEEK, &CMainDlg::OnWindowSeek)
	ON_COMMAND(IDM_WINDOW_BBS, &CMainDlg::OnWindowBbs)
	ON_COMMAND(IDM_WINDOW_EDIT, &CMainDlg::OnWindowEdit)
	ON_COMMAND(IDM_WINDOW_BROWSER, &CMainDlg::OnWindowBrowser)
	ON_COMMAND(IDM_WINDOW_BROWSER_TOP, &CMainDlg::OnWindowBrowserTop)
	ON_COMMAND(IDM_WINDOW_BROWSER_BOTTOM, &CMainDlg::OnWindowBrowserBottom)
	ON_COMMAND(IDM_WINDOW_BROWSER_LEFT, &CMainDlg::OnWindowBrowserLeft)
	ON_COMMAND(IDM_WINDOW_BROWSER_RIGHT, &CMainDlg::OnWindowBrowserRight)
	ON_COMMAND(IDM_WINDOW_BROWSER_TOGGLE_TOP, &CMainDlg::OnWindowBrowserToggleTop)
	ON_COMMAND(IDM_WINDOW_BROWSER_TOGGLE_BOTTOM, &CMainDlg::OnWindowBrowserToggleBottom)
	ON_COMMAND(IDM_WINDOW_BROWSER_TOGGLE_LEFT, &CMainDlg::OnWindowBrowserToggleLeft)
	ON_COMMAND(IDM_WINDOW_BROWSER_TOGGLE_RIGHT, &CMainDlg::OnWindowBrowserToggleRight)
	ON_COMMAND(IDM_SLEEP_STOP, &CMainDlg::OnSleepStop)
	ON_COMMAND(IDM_QUIT, &CMainDlg::OnQuit)
	ON_COMMAND(IDM_QUIT_STOP, &CMainDlg::OnQuitStop)
	ON_COMMAND(IDM_FULLSCREEN, &CMainDlg::OnFullscreen)
	ON_COMMAND(IDM_MAXIMIZE, &CMainDlg::OnMaximize)
	ON_COMMAND(IDM_BALANCE_CENTER, &CMainDlg::OnBalanceCenter)
	ON_COMMAND(IDM_BALANCE_LEFT, &CMainDlg::OnBalanceLeft)
	ON_COMMAND(IDM_BALANCE_RIGHT, &CMainDlg::OnBalanceRight)
	ON_COMMAND_RANGE(IDM_FILTER_GRAPH_RANGE_BEGIN, IDM_FILTER_GRAPH_RANGE_END, &CMainDlg::OnFilterGraphRange)
	ON_COMMAND(IDM_PREV_FILE, &CMainDlg::OnPrevFile)
	ON_COMMAND(IDM_NEXT_FILE, &CMainDlg::OnNextFile)
	ON_COMMAND(IDM_PREV_FILE_LOOP, &CMainDlg::OnPrevFileLoop)
	ON_COMMAND(IDM_NEXT_FILE_LOOP, &CMainDlg::OnNextFileLoop)
	ON_COMMAND(IDM_PLAY_MODE_NORMAL, &CMainDlg::OnPlayModeNormal)
	ON_COMMAND(IDM_PLAY_MODE_REPEAT, &CMainDlg::OnPlayModeRepeat)
	ON_COMMAND(IDM_PLAY_MODE_FOLDER, &CMainDlg::OnPlayModeFolder)
	ON_COMMAND(IDM_PLAY_MODE_FOLDER_LOOP, &CMainDlg::OnPlayModeFolderLoop)
	ON_COMMAND(IDM_PROCESS_PRIORITY_REALTIME, &CMainDlg::OnProcessPriorityRealtime)
	ON_COMMAND(IDM_PROCESS_PRIORITY_HIGH, &CMainDlg::OnProcessPriorityHigh)
	ON_COMMAND(IDM_PROCESS_PRIORITY_ABOVE_NORMAL, &CMainDlg::OnProcessPriorityAboveNormal)
	ON_COMMAND(IDM_PROCESS_PRIORITY_NORMAL, &CMainDlg::OnProcessPriorityNormal)
	ON_COMMAND(IDM_PROCESS_PRIORITY_BELOW_NOMAL, &CMainDlg::OnProcessPriorityBelowNomal)
	ON_COMMAND(IDM_PROCESS_PRIORITY_IDLE, &CMainDlg::OnProcessPriorityIdle)
	ON_COMMAND(IDM_UPDATE_CHANNEL_INFO, &CMainDlg::OnUpdateChannelInfo)
	ON_COMMAND(IDM_CHANNEL_INFO_NAME, &CMainDlg::OnChannelInfoName)
	ON_COMMAND(IDM_CHANNEL_INFO_TYPE, &CMainDlg::OnChannelInfoType)
	ON_COMMAND(IDM_CHANNEL_INFO_GENRE, &CMainDlg::OnChannelInfoGenre)
	ON_COMMAND(IDM_CHANNEL_INFO_COMMENT, &CMainDlg::OnChannelInfoComment)
	ON_COMMAND(IDM_CHANNEL_INFO_PLAYING, &CMainDlg::OnChannelInfoPlaying)
	ON_COMMAND(IDM_CHANNEL_INFO_RELAY, &CMainDlg::OnChannelInfoRelay)
	ON_COMMAND(IDM_BBS_BROWSER, &CMainDlg::OnBbsBrowser)
	ON_COMMAND(IDM_WEB_BROWSER, &CMainDlg::OnWebBrowser)
	ON_COMMAND(IDM_OPENFILE_DIALOG, &CMainDlg::OnOpenfileDialog)
	ON_COMMAND(IDM_OPENFILE_CLIPBOARD, &CMainDlg::OnOpenfileClipboard)
	ON_COMMAND(IDM_CLIPBOARD_PATH, &CMainDlg::OnClipboardPath)
	ON_COMMAND(IDM_CLIPBOARD_URL, &CMainDlg::OnClipboardUrl)
	ON_COMMAND(IDM_VERSION, &CMainDlg::OnVersion)
	ON_COMMAND(IDM_LOG, &CMainDlg::OnLog)
	ON_COMMAND(IDM_FULLSCREEN_HIDE_BARS, &CMainDlg::OnFullscreenHideBars)
	ON_COMMAND(IDM_FULLSCREEN_FLOATING, &CMainDlg::OnFullscreenFloating)
	ON_COMMAND(IDM_FULLSCREEN_MOUSE, &CMainDlg::OnFullscreenMouse)
	ON_COMMAND(IDM_FULLSCREEN_TOPMOST, &CMainDlg::OnFullscreenTopmost)
	ON_COMMAND(IDM_MIXIMIZE_TO_FULLSCREEN, &CMainDlg::OnMiximizeToFullscreen)
	ON_COMMAND(IDM_MIXIMIZE_SHOW_TASKBAR, &CMainDlg::OnMiximizeShowTaskbar)
	ON_COMMAND(IDM_TITLEBAR_SHOW_PLAYER_NAME, &CMainDlg::OnTitlebarShowPlayerName)
	ON_COMMAND(IDM_TITLEBAR_SHOW_WINDOW_SIZE, &CMainDlg::OnTitlebarShowWindowSize)
	ON_COMMAND(IDM_TITLEBAR_SHOW_SOURCE_SIZE, &CMainDlg::OnTitlebarShowSourceSize)
	ON_COMMAND(IDM_STATUSBAR_SHOW_FPS, &CMainDlg::OnStatusbarShowFps)
	ON_COMMAND(IDM_STATUSBAR_SHOW_WINDOW_SIZE, &CMainDlg::OnStatusbarShowWindowSize)
	ON_COMMAND(IDM_STATUSBAR_TIME, &CMainDlg::OnStatusbarTime)
	ON_COMMAND(IDM_STATUSBAR_PLAY, &CMainDlg::OnStatusbarPlay)
	ON_COMMAND(IDM_SEEKBAR_SHOW_FILENAME, &CMainDlg::OnSeekbarShowFilename)
	ON_COMMAND(IDM_BBSBAR_SHOW_COUNTER, &CMainDlg::OnBbsbarShowCounter)
	ON_COMMAND(IDM_START_WINDOW_ZOOM_100, &CMainDlg::OnStartWindowZoom100)
	ON_COMMAND(IDM_THREAD, &CMainDlg::OnThread)
	ON_COMMAND(IDM_BBS_MENU, &CMainDlg::OnBbsMenu)
	ON_COMMAND(IDM_BBS_SAGE, &CMainDlg::OnBbsSage)
	ON_COMMAND_RANGE(IDM_BBS_LINK_RANGE_BEGIN, IDM_BBS_LINK_RANGE_END, &CMainDlg::OnBbsLinkRange)
	ON_COMMAND(IDM_UPDATE_TITLE, &CMainDlg::OnUpdateTitle)
	ON_COMMAND(IDM_CONTEXT_SEARCH, &CMainDlg::OnContextSearch)
	ON_COMMAND(IDM_CONTEXT_EXTRACT, &CMainDlg::OnContextExtract)
	ON_COMMAND(IDM_CONTEXT_POST, &CMainDlg::OnContextPost)
	ON_COMMAND(IDM_CONTEXT_BBS_BROWSER, &CMainDlg::OnContextBbsBrowser)
END_MESSAGE_MAP()








// CMainDlg メッセージ ハンドラー

BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。

	////////////////////////////////////////
	// ドラッグ＆ドロップ許可
	utl::dragAcceptFiles(GetSafeHwnd());

	////////////////////////////////////////
	// 設定読み込み
	sl_.load(gl_.current);
	gl_.value = sl_;

	// モードレスダイアログ設定読み込み
	thread_.cfg_ = gl_.value.dialog.thread;
	search_.cfg_ = gl_.value.dialog.search;
	CExtractDlg::cfg_ = gl_.value.dialog.extract;

	// 履歴設定
	gl_.bbs.setHistory(false, gl_.value.detail.board.links);
	gl_.bbs.setHistory(true, gl_.value.detail.board.threads);

	// html読み込み
	LoadHtml();

	// ウィンドウサイズの有効な要素のみコピー
	InitWindowSizeZoom();

	// 優先度
	::SetPriorityClass(GetCurrentProcess(), gl_.value.window.getPriority());

	// レイアウト
	layout_.Create(this);
	layout_.CreateMain(new CFrameDlg, CFrameDlg::IDD);

	CFrameDlg* frame = (CFrameDlg*)layout_.GetMain();
	if (frame)
	{
		frame->setMainWnd(this);
	}
	layout_.SetSplit(gl_.value.detail.splitter.split);
	layout_.SetEdge(gl_.value.detail.splitter.edge ? TRUE : FALSE);
	layout_.SetFace(gl_.value.detail.splitter.face);
	layout_.SetBorder(gl_.value.detail.splitter.border);
	layout_.SetReverse(FALSE);
	layout_.SetSide(gl_.value.window.show.side);
	layout_.SetIdealX(gl_.value.window.show.idealX);
	layout_.SetIdealY(gl_.value.window.show.idealY);
	layout_.SetInvert(TRUE);
	layout_.RecalcLayout();

	// サブペイン
	subpane_.Create(CSubpaneDlg::IDD, this);

	subpane_.SetConfig(gl_.value.display.status, gl_.value.display.channel);
	subpane_.SetConfig(gl_.value.display.seek, gl_.value.display.hint);
	subpane_.SetConfig(gl_.value.display.bbs, gl_.value.display.res);

	ApplySubpane();
	subpane_.ShowWindow(SW_SHOWNA);

	// ボックス
	box_.Create(CBoxDlg::IDD, GetDesktopWindow());

	// コマンドライン引数
	sl::PathInfo info;
	if (utl::CommandLine::size() > 1)
	{
		info.path = utl::CommandLine::str(1);

		if (utl::CommandLine::size() > 2)
		{
			info.name = utl::CommandLine::str(2);
		}

		if (utl::CommandLine::size() > 3)
		{
			info.contact = utl::CommandLine::str(3);
			info.commandline = true;
		}
	}
	else
	{
		if (gl_.value.detail.start.recent)
		{
			info = gl_.value.recent;
		}
	}

	// プレイヤー起動
	if (!info.path.empty())
	{
		boot_ = true;
		OpenFile(info.path, info.name, info.contact, info.commandline);
	}

	////////////////////////////////////////
	// ウィンドウ表示
	// SetWindowPlacementは表示命令も含まれるのでここでウィンドウが表示される

	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.showCmd = SW_SHOWNORMAL;
	wp.rcNormalPosition = gl_.value.placement;

	RestoreVideoWindowRect(wp.rcNormalPosition);
	SetWindowPlacement(&wp);

	// ウィンドウ最前面化
	const CWnd* wnd = gl_.value.window.show.topmost ? &wndTopMost : &wndNoTopMost;
	SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	// WM_NCCALCSIZEを初回だけフレーム有で呼び出す
	// フレームを消したときのちらつきが無くなる
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	nccalcsize_ = false;

	// ウィンドウスタイル
	// フレームを消すとサイズ変更時に不具合がでるため最後
	if (gl_.value.window.show.frame)
	{
		if (!gl_.value.window.show.title)
		{
			ModifyStyle(WS_CAPTION, WS_THICKFRAME);
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}

	SetFocus(); // IMEDisable
	utl::IMEDisable();

	if (gl_.value.detail.board.focus)
	{// フォーカスを変更するときは FALSE を返す
		subpane_.SetFocusBBSEdit();
		return FALSE;
	}
	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_MOUSEWHEEL:
		{
			CPoint pt(GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam));
			CWnd *wnd = WindowFromPoint(pt);
			if (wnd)
			{
				bool online = gl_.player.online();
				int type = online ?
					gl_.value.operation.mouse.online.wheel :
					gl_.value.operation.mouse.offline.wheel;
				bool invert = online ?
					gl_.value.operation.mouse.online.invert :
					gl_.value.operation.mouse.offline.invert;

				HWND hwnd = wnd->GetSafeHwnd();
				if (hwnd == GetSafeHwnd() || hwnd == layout_.GetSafeMainHwnd() || hwnd == box_.GetSafeHwnd())
				{// 画面上
					if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
					{
						mouseClickWheek_ = true;
						subpane_.clearDesc();

						type = online ?
							gl_.value.operation.mouse.online.rwheel :
							gl_.value.operation.mouse.offline.rwheel;

						invert = online ?
							gl_.value.operation.mouse.online.rinvert :
							gl_.value.operation.mouse.offline.rinvert;

						//static int z = 0;

						//short zDelta = GET_WHEEL_DELTA_WPARAM(pMsg->wParam);
						//if (zDelta > 0)// UP
						//{
						//	//OnWindowZoomRange(IDM_WINDOW_ZOOM_RANGE_BEGIN + 2);
						//	z = 10;
						//}
						//else if (zDelta < 0)// DOWN
						//{
						//	//OnWindowZoomRange(IDM_WINDOW_ZOOM_RANGE_BEGIN + 1);
						//	z = -10;
						//}

						//RelativeWindowZoom(z);
						//return TRUE;
					}
				}
				else if ((hwnd == subpane_.getBBS() || hwnd == subpane_.getRes()))
				{// BBSバー/レスポップアップ
					if (!gl_.value.operation.mouse.bbs.transfer)
					{
						type = gl_.value.operation.mouse.bbs.wheel;
						invert = gl_.value.operation.mouse.bbs.invert;
					}
				}
				else if (hwnd == subpane_.getSeek())
				{// シークバー
					if (!gl_.value.operation.mouse.seek.transfer)
					{
						type = gl_.value.operation.mouse.seek.wheel;
						invert = gl_.value.operation.mouse.seek.invert;
					}
				}
				else if (hwnd == subpane_.getStatus())
				{// ステータスバー
					if (!gl_.value.operation.mouse.status.transfer)
					{
						type = gl_.value.operation.mouse.status.wheel;
						invert = gl_.value.operation.mouse.status.invert;
					}
				}
				else
				{
					CBrowserDlg* browser = (CBrowserDlg*)layout_.GetSub();
					if (browser)
					{
						if (hwnd == browser->GetSafeHwnd() ||
							hwnd == browser->GetSafeBrowserHwnd())
						{// 内蔵ブラウザ
							::SendMessage(browser->GetSafeBrowserHwnd(), pMsg->message, pMsg->wParam, pMsg->lParam);
							return TRUE;
						}
					}
					return FALSE;
				}

				short zDelta = GET_WHEEL_DELTA_WPARAM(pMsg->wParam);
				if (zDelta > 0)// UP
				{
					MouseWheelHelper(true, type, invert);
				}
				else if (zDelta < 0)// DOWN
				{
					MouseWheelHelper(false, type, invert);
				}
				return TRUE;
			}
			break;
		}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			DWORD key = static_cast<DWORD>(pMsg->wParam);
			DWORD shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? utl::MODIFY_CODE_SHIFT : 0;
			DWORD ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? utl::MODIFY_CODE_CTRL : 0;
			DWORD alt = (GetAsyncKeyState(VK_MENU) & 0x8000) ? utl::MODIFY_CODE_ALT : 0;
			DWORD mod = shift | ctrl | alt;

			auto it = gl_.value.operation.shortcut.find(key, mod);
			if (it != gl_.value.operation.shortcut.data.end())
			{
				SendMessage(WM_COMMAND, it->id);
				return TRUE;
			}
			break;
		}
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return FALSE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//CRect src;
		//GetClientRect(&src);

		//if (!IsExcludeSubpane())
		//{// サブペイン表示時
		//	src.bottom = max(0, src.bottom - subpane_.getHeight());
		//}

		CPaintDC dc(this);
//		if (gl_.player.repaint(dc))
//		{
//			// 動画以外の場所を黒く塗りつぶす
//			SIZE aspect = {};
//			gl_.getAspect(aspect);
//
//			RECT dst = {};
//			if (utl::getAspectRect(aspect, src, dst))
//			{
//				CRgn r, r1, r2;
//				r.CreateRectRgn(0, 0, 1, 1);
//				r1.CreateRectRgnIndirect(&src);
//				r2.CreateRectRgnIndirect(&dst);
//				r.CombineRgn(&r1, &r2, RGN_DIFF);
//#ifdef _DEBUG
//				dc.FillRgn(&r, &CBrush(RGB(0, 255, 0)));
//#else
//				dc.FillRgn(&r, &CBrush(RGB(0, 0, 0)));
//#endif
//			}
//			return;
//		}
//
//		// 背景を黒く塗りつぶす
//		dc.FillRect(&src, &CBrush(RGB(0, 0, 0)));

		//CDialog::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMainDlg::OnSize(UINT nType, int cx, int cy)
{
	if (nType == SIZE_MAXIMIZED)
	{// Aeroスナップの最大化
		ShowWindow(SW_RESTORE);
		if (gl_.value.window.maximize.fullscreen)
		{
			OnFullscreen(); return;
		}
		else
		{
			OnMaximize(); return;
		}
	}
	//CDialog::OnSize(nType, cx, cy);
	
	SizeHelper();
	RedrawWindow(0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CMainDlg::OnClose()
{
	// BBSブラウザプロセス終了
	if (gl_.value.detail.browser.close)
	{
		process_.close();
	}

	// モードレスダイアログの終了と設定保存
	if (::IsWindow(thread_.GetSafeHwnd()))
	{
		thread_.DestroyWindow();
	}
	gl_.value.dialog.thread = thread_.cfg_;

	if (::IsWindow(search_.GetSafeHwnd()))
	{
		search_.DestroyWindow();
	}
	gl_.value.dialog.search = search_.cfg_;

	CExtractDlg::destroy();
	gl_.value.dialog.extract = CExtractDlg::cfg_;

	if (::IsWindow(log_.GetSafeHwnd()))
	{
		log_.DestroyWindow();
	}
	//gl_.value.dialog.log = log_.cfg_;

	// 履歴取得
	gl_.bbs.getHistory(false, gl_.value.detail.board.links);
	gl_.bbs.getHistory(true, gl_.value.detail.board.threads);

	StoreVideoWindowRect(gl_.value.placement);
	gl_.value.window.show.idealX = layout_.GetIdealX();
	gl_.value.window.show.idealY = layout_.GetIdealY();

	ShowWindow(SW_HIDE); // 閉じるを押してから停止まで時間がかかるため

	// 保存しないものの初期値を退避
	auto placement = sl_.placement;
	auto window = sl_.window;
	auto audio = sl_.audio;
	auto video = sl_.video;

	// すべてコピー
	sl_ = gl_.value;

	// 初期値に戻す
	if (!gl_.value.detail.end.pos && !gl_.value.detail.end.size)
	{
		sl_.placement = placement;
	}
	else if (gl_.value.detail.end.pos && !gl_.value.detail.end.size)
	{// 位置のみ
		CRect rc(placement);
		sl_.placement.left = gl_.value.placement.left;
		sl_.placement.top = gl_.value.placement.top;
		sl_.placement.right = gl_.value.placement.left + rc.Width();
		sl_.placement.bottom = gl_.value.placement.top + rc.Height();
	}
	else if (!gl_.value.detail.end.pos && gl_.value.detail.end.size)
	{// サイズのみ
		CRect rc(gl_.value.placement);
		sl_.placement.left = placement.left;
		sl_.placement.top = placement.top;
		sl_.placement.right = placement.left + rc.Width();
		sl_.placement.bottom = placement.top + rc.Height();
	}

	if (!gl_.value.detail.end.aspect)
	{
		sl_.window.aspect = window.aspect;
	}

	if (!gl_.value.detail.end.volume)
	{
		sl_.audio.volume = audio.volume;
	}

	if (!gl_.value.detail.end.balance)
	{
		sl_.audio.balance = audio.balance;
	}

	if (!gl_.value.detail.end.mute)
	{
		sl_.audio.mute = audio.mute;
	}

	// 映像調整は保存しない
	sl_.video = video;

	// 保存
	sl_.save(gl_.current);
	
	// フィルタグラフにはウィンドウハンドルが必要なため
	// ウィンドウがある時点で終了処理する必要がある
	gl_.player.terminate();

	// 終了時チャンネル切断
	// プレイヤーの接続を切ってから切断する
	if (stop_ || gl_.value.detail.network.stop)
	{
		network::PeerCast peca(gl_.latest.path);
		if (peca)
		{
			bool manual = stop_ && gl_.value.detail.network.relay.manual.stop;
			bool automatic = gl_.value.detail.network.stop && gl_.value.detail.network.relay.automatic.stop;
			peca.stop(manual || automatic);
		}
	}

	// BBS停止
	gl_.bbs.terminate();

	CDialog::OnClose();
}


void CMainDlg::OnDestroy()
{
	CDialog::OnDestroy();

	//
}


void CMainDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (mouseCaptureR_) { return; }

	if (!gl_.value.detail.board.focus)
	{
		SetFocus(); // BBS
	}

	//PostMessageにしないとIMEの変換が確定しない
	if (!IsZoomed() && !gl_.screen.full && !gl_.screen.maximize && !gl_.value.window.show.frame)
	{ // ウィンドウフレームが無いときはサイズ変更
		//CRect rc;
		//GetClientRect(rc);
		//rc.DeflateRect(0, 0, 0, subpane_.getHeight());

		if (layout_.IsMain())
		{
			CRect rc;
			layout_.GetMain()->GetClientRect(rc);
			layout_.GetMain()->ClientToScreen(rc);
			ScreenToClient(rc);

			int pos = utl::windowFramePos(rc, point, gl_.value.operation.window.frame);

			switch (pos)
			{
			case utl::FRAMEPOS_TOP:
				::SetCursor(LoadCursor(NULL, IDC_SIZENS));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_TOP,0);
				return;

			case utl::FRAMEPOS_BOTTOM:
				::SetCursor(LoadCursor(NULL, IDC_SIZENS));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_BOTTOM,0);
				return;

			case utl::FRAMEPOS_TOPLEFT:
				::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_TOPLEFT,0);
				return;

			case utl::FRAMEPOS_BOTTOMRIGHT:
				::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_BOTTOMRIGHT,0);
				return;

			case utl::FRAMEPOS_TOPRIGHT:
				::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_TOPRIGHT,0);
				return;

			case utl::FRAMEPOS_BOTTOMLEFT:
				::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_BOTTOMLEFT,0);
				return;

			case utl::FRAMEPOS_LEFT:
				::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_LEFT,0);
				return;

			case utl::FRAMEPOS_RIGHT:
				::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				PostMessage(WM_SYSCOMMAND, SC_SIZE|WMSZ_RIGHT,0);
				return;

			default:
				break;
			}
		}
	}

	if (!IsZoomed() && !gl_.screen.full && !gl_.screen.maximize)
	{// 画面ドラッグ移動
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
		return;
	}

	//CDialog::OnLButtonDown(nFlags, point);
}

void CMainDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonUp(nFlags, point);
}


void CMainDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (mouseCaptureR_) { return; }

	int type = gl_.player.online() ? gl_.value.operation.mouse.online.ldblclick : gl_.value.operation.mouse.offline.ldblclick;

	switch (type)
	{
	case sl::MOUSE_LDBLCLICK_NONE: break;
	case sl::MOUSE_LDBLCLICK_MAXIMIZE: OnMaximize(); break;
	case sl::MOUSE_LDBLCLICK_ZOOM100: OnWindowZoom100(); break;
	case sl::MOUSE_LDBLCLICK_ZOOM100_FULLSCREEN: OnWindowZoom100(); break;
	case sl::MOUSE_LDBLCLICK_ZOOM100_MAXIMIZE: OnWindowZoom100(); break;

	case sl::MOUSE_LDBLCLICK_FULLSCREEN:
	default: OnFullscreen(); break;
	}

	//CDialog::OnLButtonDblClk(nFlags, point);
}


BOOL CMainDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CMainDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);
}

void CMainDlg::OnDropFiles(HDROP hDropInfo)
{
	std::vector<std::wstring> files;
    if (utl::dragQueryFiles(hDropInfo, files))
	{
		OpenFile(files[0]);
	}

	CDialog::OnDropFiles(hDropInfo);
}

void CMainDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// マウスジェスチャ
	if (mouseCaptureR_)
	{
		if (mouseClickWheek_)
		{
			subpane_.clearDesc();
		}
		else
		{
			gesture_.move(point);

			std::vector<int> gesture;
			gesture_.get(gesture);

			if (!gesture.empty())
			{
				std::wstring command;
				auto it = gl_.value.operation.shortcut.find(gesture);
				if (it != gl_.value.operation.shortcut.data.end())
				{
					command = utl::getIDText(it->id);
					command = L" (" + command + L")";
				}

				if (command.empty())
				{
					subpane_.setDesc(utl::getGestureText(gesture));
				}
				else
				{
					subpane_.setDesc(utl::getGestureText(gesture) + command);
				}
			}
			else
			{
				subpane_.clearDesc();
			}
		}
		return;
	}

	// ボックスダイアログの表示
	// フルスクリーンまたはフレームがないとき
	if (gl_.screen.full || gl_.screen.maximize || !gl_.value.window.show.frame || !gl_.value.window.show.title)
	{
		//CRect rc;
		//GetClientRect(rc);
		//rc.DeflateRect(0, 0, 0, subpane_.getHeight());

		if (layout_.IsMain())
		{
			CRect rc;
			layout_.GetMain()->GetClientRect(rc);
			layout_.GetMain()->ClientToScreen(rc);
			ScreenToClient(rc);

			CRect brc(rc);
			brc.bottom = brc.top + 24;
			brc.left = brc.right - 72;

			if (brc.PtInRect(point))
			{
				ClientToScreen(brc);
				//box_.MoveWindow(brc.left, brc.top, brc.Width(), brc.Height());
				box_.SetWindowPos(NULL, brc.left, brc.top, brc.Width(), brc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
				box_.showWindow(true);
				EndMouseHideTimer();
				return;
			}
		}
	}

	// ボックスダイアログの非表示
	if (box_.IsWindowVisible())
	{
		box_.showWindow(false);
	}

	// サブペイン表示
	// フルスクリーン
	if (gl_.screen.full)
	{
		CRect rc;
		GetClientRect(&rc);
		int height = subpane_.getHeight();
		// サブペインの領域
		if (rc.bottom - height < 0) { rc.bottom = 0; }
		else { rc.top = rc.bottom - height; }

		if (gl_.value.window.fullscreen.hide)
		{
			if (!subpane_.IsWindowVisible() && rc.PtInRect(point))
			{
				subpane_.ShowWindow(SW_SHOWNA);

				if (gl_.value.detail.board.focus)
				{
					subpane_.SetFocusBBSEdit(); // BBS
				}

				SizeHelper();
				EndMouseHideTimer();
			}
			else if (subpane_.IsWindowVisible())
			{
				// フォーカスを移してから消去
				SetFocus();
				subpane_.ShowWindow(SW_HIDE);

				SizeHelper();
				StartMouseHideTimer();
			}
			else
			{
				StartMouseHideTimer();
			}
			return;
		}
		else
		{// サブペインを隠さない
			if (rc.PtInRect(point))
			{// サブペイン上ではカーソルを消さない
				EndMouseHideTimer();
			}
			else
			{
				StartMouseHideTimer();
			}
			return;
		}
	}
	
	// サイズ変更カーソルの表示
	// 非フルスクリーン＆フレームがないとき
	if (!IsZoomed() && !gl_.screen.full && !gl_.screen.maximize && !gl_.value.window.show.frame)
	{
		//CRect rc;
		//GetClientRect(rc);
		//rc.DeflateRect(0, 0, 0, subpane_.getHeight());

		if (layout_.IsMain())
		{
			CRect rc;
			layout_.GetMain()->GetClientRect(rc);
			layout_.GetMain()->ClientToScreen(rc);
			ScreenToClient(rc);

			int pos = utl::windowFramePos(rc, point, gl_.value.operation.window.frame);

			switch (pos)
			{
			case utl::FRAMEPOS_TOP:
			case utl::FRAMEPOS_BOTTOM:
				::SetCursor(LoadCursor(NULL, IDC_SIZENS));
				break;

			case utl::FRAMEPOS_TOPLEFT:
			case utl::FRAMEPOS_BOTTOMRIGHT:
				::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				break;

			case utl::FRAMEPOS_TOPRIGHT:
			case utl::FRAMEPOS_BOTTOMLEFT:
				::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
				break;

			case utl::FRAMEPOS_LEFT:
			case utl::FRAMEPOS_RIGHT:
				::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				break;

			default:
				break;
			}
			return;
		}
	}

	//CDialog::OnMouseMove(nFlags, point);
}


void CMainDlg::StartMouseHideTimer()
{
	if (gl_.screen.full && gl_.value.window.fullscreen.mouse)
	{
		SetTimer(TIMER_ID_MOUSEHIDE, 1000, NULL);
		while (ShowCursor(TRUE) < 0); // 表示されるまでループ
	}
}

void CMainDlg::EndMouseHideTimer()
{
	KillTimer(TIMER_ID_MOUSEHIDE);
	while (ShowCursor(TRUE) < 0); // 表示されるまでループ
}

void CMainDlg::StartSleepStopTimer()
{
	// カウンターリセット
	SetThreadExecutionState(ES_DISPLAY_REQUIRED);

	INPUT input[1];
	ZeroMemory(input, sizeof(INPUT));
	input[0].type = INPUT_MOUSE;
	input[0].mi.dwFlags = MOUSEEVENTF_MOVE;
	input[0].mi.dx = 0;
	input[0].mi.dy = 0;
	SendInput(1, input, sizeof(INPUT));

	SetTimer(TIMER_ID_SLEEPSTOP, 30000, NULL);
	//DebugText(L"StartSleepStopTimer\n");
}

void CMainDlg::EndSleepStopTimer()
{
	KillTimer(TIMER_ID_SLEEPSTOP);
	//DebugText(L"EndSleepStopTimer\n");
}

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_MOUSEHIDE:
	{// 全画面時のマウス消去
	 // カーソル下がメインウィンドウの時のみ非表示にする
		CPoint pt;
		if (GetCursorPos(&pt))
		{
			CWnd* wnd = WindowFromPoint(pt);
			if (wnd)
			{
				if (wnd->GetSafeHwnd() == GetSafeHwnd())
				{
					while(ShowCursor(FALSE) >= 0); // カーソルの非表示
				}
				else
				{
					CFrameDlg* frame = (CFrameDlg*)layout_.GetMain();
					if (frame &&
						::IsWindow(frame->GetSafeHwnd()) &&
						wnd->GetSafeHwnd() == frame->GetSafeHwnd())
					{
						while(ShowCursor(FALSE) >= 0); // カーソルの非表示
					}
				}
			}
		}
		KillTimer(TIMER_ID_MOUSEHIDE);
		break;
	}

	case TIMER_ID_SLEEPSTOP:
		{// スリープ抑止
			// カウンターリセット
			SetThreadExecutionState(ES_DISPLAY_REQUIRED);

			INPUT input[1];
			ZeroMemory(input, sizeof(INPUT));
			input[0].type = INPUT_MOUSE;
			input[0].mi.dwFlags = MOUSEEVENTF_MOVE;
			input[0].mi.dx = 0;
			input[0].mi.dy = 0;
			SendInput(1, input, sizeof(INPUT));
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void CMainDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	int type = gl_.player.online() ? gl_.value.operation.mouse.online.mclick : gl_.value.operation.mouse.offline.mclick;

	MouseMClickHelper(type);

	CDialog::OnMButtonDown(nFlags, point);
}

void CMainDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}

void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMainDlg::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnSysKeyDown(nChar, nRepCnt, nFlags);
}




void CMainDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// 画面移動中は無効
	if (sizemove_) { return; }

	if (!gl_.value.detail.board.focus)
	{
		SetFocus(); // BBS
	}

	gesture_.start(gl_.value.operation.shortcut.movement, point);

	mouseCaptureR_ = true;
	SetCapture();

	EndMouseHideTimer();

	CDialog::OnRButtonDown(nFlags, point);
}



void CMainDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// 画面移動中は無効
	if (sizemove_) { return; }

	if (mouseCaptureR_)
	{
		mouseCaptureR_ = false;
		ReleaseCapture();
		subpane_.clearDesc();

		if (mouseClickWheek_)
		{
			mouseClickWheek_ = false;
			return;
		}

		if (!gesture_.empty())
		{
			std::vector<int> gesture;
			gesture_.get(gesture);
			auto it = gl_.value.operation.shortcut.find(gesture);
			if (it != gl_.value.operation.shortcut.data.end())
			{
				SendMessage(WM_COMMAND, it->id);
				return;
			}
			return;
		}
	}

	EndMouseHideTimer();

	CMenu root;
	CMenu* menu = NULL;
	if (!root.LoadMenu(IDR_MAINMENU)) { return; }
	
	menu = root.GetSubMenu(0);
	if (menu == NULL) { return; }

	auto checkRadioItem = [=](int id){
		menu->CheckMenuRadioItem(id, id, id, MF_BYCOMMAND);
	};

	auto checkItem = [=](int id, bool check){
		if (check)
		{
			menu->CheckMenuItem(id, MF_CHECKED|MF_BYCOMMAND);
		}
	};

	{// ウィンドウ倍率
		int i = 0;
		for (auto it = windowZoom_.begin(); it != windowZoom_.end(); ++it)
		{
			CString str;
			str.Format(L"%d%%", it->zoom);

			menu->InsertMenu(
				IDM_WINDOW_ZOOM_MENU,
				MF_BYCOMMAND|MF_STRING,
				IDM_WINDOW_ZOOM_RANGE_BEGIN + i,
				str);
			i++;
			if (i >= 100) { break; }
		}
		menu->DeleteMenu(IDM_WINDOW_ZOOM_MENU, MF_BYCOMMAND);
	}

	{// ウィンドウサイズ
		int i = 0;
		for (auto it = windowSize_.begin(); it != windowSize_.end(); ++it)
		{
			CString str;
			str.Format(L"%dx%d", it->width, it->height);

			menu->InsertMenu(
				IDM_WINDOW_SIZE_MENU,
				MF_BYCOMMAND|MF_STRING,
				IDM_WINDOW_SIZE_RANGE_BEGIN + i,
				str);
			i++;
			if (i >= 100) { break; }
		}
		menu->DeleteMenu(IDM_WINDOW_SIZE_MENU, MF_BYCOMMAND);
	}

	{// アスペクト比

		int index = -1;
		if  (gl_.value.window.aspect == sl::WINDOW_ASPECT_DEFAULT)
		{
			checkRadioItem(IDM_WINDOW_ASPECT_DEFAULT);
		}
		else if (gl_.value.window.aspect == sl::WINDOW_ASPECT_NONE)
		{
			checkRadioItem(IDM_WINDOW_ASPECT_NONE);
		}
		else
		{
			index = gl_.value.window.aspect - sl::WINDOW_ASPECT_CUSTOM;
		}


		int i = 0;
		if (gl_.value.display.window.aspect.size() > 0)
		{
			menu->InsertMenu(IDM_WINDOW_ASPECT_MENU, MF_SEPARATOR);
		}
		for (auto it = gl_.value.display.window.aspect.begin(); it != gl_.value.display.window.aspect.end(); ++it)
		{
			CString str;
			str.Format(L"%d:%d", it->ax, it->ay);

			menu->InsertMenu(
				IDM_WINDOW_ASPECT_MENU,
				MF_BYCOMMAND|MF_STRING,
				IDM_WINDOW_ASPECT_RANGE_BEGIN + i,
				str);

			if (i == index)
			{
				checkRadioItem(IDM_WINDOW_ASPECT_RANGE_BEGIN + i);
			}
			i++;
			if (i >= 100) { break; }
		}
		menu->DeleteMenu(IDM_WINDOW_ASPECT_MENU, MF_BYCOMMAND);
	}

	{// 表示
		checkItem(IDM_WINDOW_TOPMOST, gl_.value.window.show.topmost);
		checkItem(IDM_WINDOW_FRAME, gl_.value.window.show.frame);
		checkItem(IDM_WINDOW_TITLE, gl_.value.window.show.title);
		checkItem(IDM_WINDOW_STATUS, gl_.value.window.show.status);
		checkItem(IDM_WINDOW_SEEK, gl_.value.window.show.seek);
		checkItem(IDM_WINDOW_BBS, gl_.value.window.show.bbs);

		checkItem(IDM_WINDOW_EDIT, gl_.value.window.show.edit);

		checkItem(IDM_WINDOW_BROWSER, gl_.value.window.show.browser);

		switch (gl_.value.window.show.side)
		{
		case CLayoutCtrl::SIDE_TOP:		checkRadioItem(IDM_WINDOW_BROWSER_TOP); break;
		case CLayoutCtrl::SIDE_BOTTOM:	checkRadioItem(IDM_WINDOW_BROWSER_BOTTOM); break;
		case CLayoutCtrl::SIDE_LEFT:	checkRadioItem(IDM_WINDOW_BROWSER_LEFT); break;
		case CLayoutCtrl::SIDE_RIGHT:	checkRadioItem(IDM_WINDOW_BROWSER_RIGHT); break;
		}

		checkItem(IDM_TITLEBAR_SHOW_PLAYER_NAME, gl_.value.window.title.player);
		checkItem(IDM_TITLEBAR_SHOW_WINDOW_SIZE, gl_.value.window.title.size);
		checkItem(IDM_TITLEBAR_SHOW_SOURCE_SIZE, gl_.value.window.title.source);
		checkItem(IDM_STATUSBAR_SHOW_FPS, gl_.value.window.status.fps);
		checkItem(IDM_STATUSBAR_SHOW_WINDOW_SIZE, gl_.value.window.status.size);
		checkItem(IDM_STATUSBAR_TIME, gl_.value.window.status.time);
		checkItem(IDM_STATUSBAR_PLAY, gl_.value.window.status.play);
		checkItem(IDM_SEEKBAR_SHOW_FILENAME, gl_.value.window.seek.filename);
		checkItem(IDM_BBSBAR_SHOW_COUNTER, gl_.value.window.bbs.counter);

		if (!gl_.value.window.show.frame)
		{// フレームがない時タイトルバーは無効
			menu->EnableMenuItem(IDM_WINDOW_TITLE, MF_GRAYED|MF_BYCOMMAND);
		}

		if (IsZoomed() || gl_.screen.full || gl_.screen.maximize)
		{// 全画面/最大化時フレーム/タイトルバーは無効
			menu->EnableMenuItem(IDM_WINDOW_FRAME, MF_GRAYED|MF_BYCOMMAND);
			menu->EnableMenuItem(IDM_WINDOW_TITLE, MF_GRAYED|MF_BYCOMMAND);
		}
	}

	{// ウィンドウ
		checkItem(IDM_WINDOW_FIXED_RATIO, gl_.value.window.ratio.fixed);
		checkItem(IDM_WINDOW_PLAY_RATIO, gl_.value.window.ratio.play);
		checkItem(IDM_WINDOW_HEIGHT_RATIO, gl_.value.window.ratio.height);
		checkItem(IDM_WINDOW_INSCRIBE_RATIO, gl_.value.window.ratio.inscribe);

		checkItem(IDM_WINDOW_SCREEN_SNAP, gl_.value.window.snap.screen);
		checkItem(IDM_WINDOW_WINDOW_SNAP, gl_.value.window.snap.window);

		checkItem(IDM_START_WINDOW_ZOOM_100, gl_.value.window.zoom);

		checkItem(IDM_SLEEP_STOP, gl_.value.window.stop);

		checkItem(IDM_FULLSCREEN_HIDE_BARS, gl_.value.window.fullscreen.hide);
		checkItem(IDM_FULLSCREEN_FLOATING, gl_.value.window.fullscreen.floating);
		checkItem(IDM_FULLSCREEN_MOUSE, gl_.value.window.fullscreen.mouse);
		checkItem(IDM_FULLSCREEN_TOPMOST, gl_.value.window.fullscreen.topmost);

		checkItem(IDM_MIXIMIZE_TO_FULLSCREEN, gl_.value.window.maximize.fullscreen);
		checkItem(IDM_MIXIMIZE_SHOW_TASKBAR, gl_.value.window.maximize.taskbar);
	}

	{// チャンネル情報表示
		checkItem(IDM_CHANNEL_INFO_NAME, gl_.value.window.status.channel.name);
		checkItem(IDM_CHANNEL_INFO_TYPE, gl_.value.window.status.channel.type);
		checkItem(IDM_CHANNEL_INFO_GENRE, gl_.value.window.status.channel.genre);
		checkItem(IDM_CHANNEL_INFO_COMMENT, gl_.value.window.status.channel.comment);
		checkItem(IDM_CHANNEL_INFO_PLAYING, gl_.value.window.status.channel.playing);
		checkItem(IDM_CHANNEL_INFO_RELAY, gl_.value.window.status.channel.relay);
	}

	{// 音量
		checkItem(IDM_MUTE, gl_.value.audio.mute);

		if (gl_.value.audio.balance == 0)
		{
			checkRadioItem(IDM_BALANCE_CENTER);
		}
		else if (gl_.value.audio.balance == -100)
		{
			checkRadioItem(IDM_BALANCE_LEFT);
		}
		else if (gl_.value.audio.balance == 100)
		{
			checkRadioItem(IDM_BALANCE_RIGHT);
		}
	}

	{// スナップショット
		if (gl_.screen.full && gl_.value.window.fullscreen.topmost)
		{// 全画面化最前面時スナップショット保存フォルダを開くは無効
			menu->EnableMenuItem(IDM_SNAPSHOT_FOLDER, MF_GRAYED|MF_BYCOMMAND);
		}
	}

	{// 実行優先度
		switch (gl_.value.window.priority)
		{
		case sl::PROCESS_PRIORITY_REALTIME:		checkRadioItem(IDM_PROCESS_PRIORITY_REALTIME); break;
		case sl::PROCESS_PRIORITY_HIGH:			checkRadioItem(IDM_PROCESS_PRIORITY_HIGH); break;
		case sl::PROCESS_PRIORITY_ABOVE_NORMAL:	checkRadioItem(IDM_PROCESS_PRIORITY_ABOVE_NORMAL); break;
		case sl::PROCESS_PRIORITY_NORMAL:		checkRadioItem(IDM_PROCESS_PRIORITY_NORMAL); break;
		case sl::PROCESS_PRIORITY_BELOW_NOMAL:	checkRadioItem(IDM_PROCESS_PRIORITY_BELOW_NOMAL); break;
		case sl::PROCESS_PRIORITY_IDLE:			checkRadioItem(IDM_PROCESS_PRIORITY_IDLE); break;
		default: break;
		}
	}

	{// オフライン再生
		switch (gl_.value.play.mode)
		{
		case sl::PLAY_MODE_REPEAT:		checkRadioItem(IDM_PLAY_MODE_REPEAT); break;
		case sl::PLAY_MODE_FOLDER:		checkRadioItem(IDM_PLAY_MODE_FOLDER); break;
		case sl::PLAY_MODE_FOLDER_LOOP:	checkRadioItem(IDM_PLAY_MODE_FOLDER_LOOP); break;
		case sl::PLAY_MODE_NORMAL:		checkRadioItem(IDM_PLAY_MODE_NORMAL); break;
		default: break;
		}
	}


	{// フィルタグラフ
		std::vector<dsp::FilterProperty> prop;
		if (gl_.player.getFilterProperty(prop))
		{
			int i = 0;
			for (auto it = prop.begin(); it != prop.end(); ++it)
			{
				CString str;
				str.Format(L"%s", it->name.c_str());

				menu->InsertMenu(
					IDM_FILTER_GRAPH_MENU,
					MF_BYCOMMAND|MF_STRING,
					IDM_FILTER_GRAPH_RANGE_BEGIN + i,
					str);

				if (!it->enable)
				{
					menu->EnableMenuItem(IDM_FILTER_GRAPH_RANGE_BEGIN + i, MF_GRAYED|MF_BYCOMMAND);
				}

				i++;
				if (i >= 100) { break; }
			}
		}
		menu->DeleteMenu(IDM_FILTER_GRAPH_MENU, MF_BYCOMMAND);
	}

	{
		if (gl_.player.peercast())
		{// コンタクトURLの有無
			std::wstring url;
			GetContactURL(url);
			if (url.empty())
			{
				menu->EnableMenuItem(IDM_BBS_BROWSER, MF_GRAYED);
				menu->EnableMenuItem(IDM_WEB_BROWSER, MF_GRAYED);
				menu->EnableMenuItem(IDM_CLIPBOARD_URL, MF_GRAYED);
			}
		}
		else
		{// PeerCastモードメニュー削除
			for (int i = 0; i < 8; ++i)
			{
				menu->DeleteMenu(6, MF_BYPOSITION);
			}

			menu->DeleteMenu(18, MF_BYPOSITION);
		}
	}



	////////////////////////////////////////////////////
	CPoint pt;
	GetCursorPos(&pt);
	menu->TrackPopupMenu(TPM_LEFTBUTTON, pt.x, pt.y, this);
	root.DestroyMenu();

	CDialog::OnRButtonUp(nFlags, point);
}


// 最小化復帰
BOOL CMainDlg::OnQueryOpen()
{
	switch(minimize_)
	{
	case sl::MOUSE_MCLICK_MINIMIZE_MUTE:
		gl_.player.setMute(gl_.value.audio.mute);
		break;

	case sl::MOUSE_MCLICK_MINIMIZE_PAUSE:
		if (running_) { gl_.player.run(); }
		break;
	}
	minimize_ = -1;
	return CDialog::OnQueryOpen();
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID)
	{
	case SC_MAXIMIZE:
		if (gl_.value.window.maximize.fullscreen)
		{
			OnFullscreen(); return;
		}
		else
		{
			OnMaximize(); return;
		}
		break;
	}

	CDialog::OnSysCommand(nID, lParam);
}

void CMainDlg::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
    if (nHitTest == HTCAPTION)
    {// ダブルクリックされたのはタイトルバー
        if (IsZoomed())
        {// 最大化中
            SendMessage(WM_SYSCOMMAND, SC_RESTORE);
        }
        else
        {// 最大化されてない
            SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
        }
        return;
    }

	CDialog::OnNcLButtonDblClk(nHitTest, point);
}

void CMainDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	//CRect rc;
	//if (sizemove_.getRect(rc))
	//{
	//	lpwndpos->x = rc.left;
	//	lpwndpos->y = rc.top;
	//	lpwndpos->cx = rc.Width();
	//	lpwndpos->cy = rc.Height();
	//}
	CDialog::OnWindowPosChanging(lpwndpos);
}

void CMainDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	// サイズ変更しないならreturn;
	if (gl_.screen.full) { return; }

	if (pRect)
	{
		SizingHelper(*pRect, gl_.value.window.ratio.fixed, false, fwSide);
		sizemove_.setRect(pRect);
	}
}

void CMainDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialog::OnMoving(fwSide, pRect);
	if (pRect)
	{
		windowsnap_.OnMoving(GetSafeHwnd(), pRect,
			gl_.value.window.snap.screen, gl_.value.window.snap.window, gl_.value.operation.window.snap);
		sizemove_.setRect(pRect);
	}
}

void CMainDlg::OnEnterSizeMove()
{
	sizemove_.OnEnterSizeMove();
	windowsnap_.OnEnterSizeMove(GetSafeHwnd());
	CDialog::OnEnterSizeMove();
}

void CMainDlg::OnExitSizeMove()
{
	sizemove_.OnExitSizeMove();
	CDialog::OnExitSizeMove();
}

void CMainDlg::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	if (!nccalcsize_)
	{
		if (!gl_.value.window.show.frame)
		{
			RedrawWindow(0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
			return;
		}
		//if (IsZoomed() || gl_.screen.full || gl_.screen.maximize) { return; }
	}
	CDialog::OnNcCalcSize(bCalcValidRects, lpncsp);
}

