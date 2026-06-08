
// PCRPlayerDlg.h : ヘッダー ファイル
//

#pragma once

#include "PCRPlayer.h"
#include "LayoutCtrl.h"
#include "SubpaneDlg.h"
#include "FrameDlg.h"
#include "BoxDlg.h"
#include "SearchDlg.h"
#include "OpenFileDlg.h"
#include "VersionDlg.h"
#include "LogDlg.h"
#include "ThreadDlg.h"
#include "BrowserDlg.h"

#include "OptionDlg.h"


//#include "afxdialogex.h"

// メニューの映像拡大率の範囲
#define IDM_WINDOW_ZOOM_RANGE_BEGIN		50000
#define IDM_WINDOW_ZOOM_RANGE_END		50099

// メニューの映像サイズの範囲
#define IDM_WINDOW_SIZE_RANGE_BEGIN		50100
#define IDM_WINDOW_SIZE_RANGE_END		50199

// メニューの画面比率の範囲
#define IDM_WINDOW_ASPECT_RANGE_BEGIN	50200
#define IDM_WINDOW_ASPECT_RANGE_END		50299

// メニューのフィルタグラフの範囲
#define IDM_FILTER_GRAPH_RANGE_BEGIN	50300
#define IDM_FILTER_GRAPH_RANGE_END		50399

// BBSメニューのリンクの範囲
#define IDM_BBS_LINK_RANGE_BEGIN		50400
#define IDM_BBS_LINK_RANGE_END			50499

// チャンネル情報のリンクの範囲
#define IDM_STATUS_LINK_RANGE_BEGIN		50500
#define IDM_STATUS_LINK_RANGE_END		50599


// CMainDlg ダイアログ
class CMainDlg : public CDialog
{
// コンストラクション
public:
	CMainDlg(CWnd* pParent = NULL);	// 標準コンストラクター
	virtual ~CMainDlg();

// ダイアログ データ
	enum { IDD = IDD_MAIN };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	bool boot_; // 起動時の再生
	bool nccalcsize_; // WM_NCCALCSIZE初期化フラグ

	CLogDlg log_; // ログダイアログ
	CLayoutCtrl layout_; // メインレイアウト
	CSubpaneDlg subpane_; // サブダイアログ
	CBoxDlg box_; // ボックスダイアログ
	CThreadDlg thread_; // スレッド選択
	CSearchDlg search_; // 検索

	utl::WindowSnap windowsnap_;
	utl::SizeMove sizemove_;

	bool mouseCaptureR_; // 右クリックフラグ
	bool mouseClickWheek_; // クリック中ホイールフラグ
	utl::Gesture gesture_; // マウスジェスチャー

	int minimize_; // 最小化ミュート/一時停止用
	bool running_; // 最小化一時停止で使う

	std::vector<sl::WindowZoom> windowZoom_; // 有効な要素のみのコレクション
	std::vector<sl::WindowSize> windowSize_;

	bool stop_; // 終了時チャンネル切断

	std::vector<std::wstring> links_; // BBSメニューに表示したリンク

	utl::ProcessManager process_;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	

	DECLARE_MESSAGE_MAP()

public:
	void GetVideoRect(CRect& rc);
	void SetTitleText();
	void ExpandWindow(const CRect& rect);
	void ShowSubpane(bool bbs = true, bool seek = true, bool status = true);
	void ApplySubpane(bool bbs = true, bool seek = true, bool status = true);
	void ShowBrowser(bool show);
	void ToggleShowBrowser(int side);
	bool RebuildFile();
	bool OpenFile(const std::wstring& path, const std::wstring& name = std::wstring(), const std::wstring& contact = std::wstring(), bool commandline = false, bool rebuild = false);
	bool OpenNextFile(bool loop, bool prev);
	void SizeHelper();
	void SizingHelper(RECT& rect, bool ratio, bool inscribe, UINT side);
	void MouseMClickHelper(int type);
	void MouseWheelHelper(bool up, int type, bool invert);
	void CalcWindowRatio(RECT& rc, bool inscribe);
	void ApplyWindowRatio(bool enable);
	void InflateWindowRect(RECT& src);
	void DeflateWindowRect(RECT& src);
	void StoreVideoWindowRect(RECT& src);
	void RestoreVideoWindowRect(RECT& src);
	void InitWindowSizeZoom();
	bool IsExcludeSubpane();
	void RecreateSubpane(bool popup);
	void WindowZoom(int zoom);
	void WindowScale(int scale);
	void RelativeWindowScale(int scale);
	void AbsoluteWindowSize(int width, int height);
	void GetContactURL(std::wstring& url);
	void OpenWebBrowser(const std::wstring& url);
	void SetFocusBBSEdit();
	CSubpaneDlg* GetSubpane() { return &subpane_; }
	CLayoutCtrl* GetLayout() { return &layout_; }
	void LoadHtml();
	bool OpenProcess(const std::wstring& path, const std::wstring& command);

	afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void StartMouseHideTimer();
	void EndMouseHideTimer();
	void StartSleepStopTimer();
	void EndSleepStopTimer();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	////////////////////////////////////////////////////////////////
	afx_msg void OnOption();
	afx_msg void OnPlayerRebuild();
	afx_msg void OnPlayerTerminate();
	afx_msg void OnPeercastBump();
	afx_msg void OnPeercastStop();
	afx_msg void OnVolumeUp();
	afx_msg void OnVolumeUpLow();
	afx_msg void OnVolumeUpHigh();
	afx_msg void OnVolumeDown();
	afx_msg void OnVolumeDownLow();
	afx_msg void OnVolumeDownHigh();
	afx_msg void OnBalanceCenter();
	afx_msg void OnBalanceLeft();
	afx_msg void OnBalanceRight();
	afx_msg void OnSeekForward();
	afx_msg void OnSeekForwardLow();
	afx_msg void OnSeekForwardHigh();
	afx_msg void OnSeekBack();
	afx_msg void OnSeekBackLow();
	afx_msg void OnSeekBackHigh();
	afx_msg void OnThreadNext();
	afx_msg void OnThreadNextLow();
	afx_msg void OnThreadNextHigh();
	afx_msg void OnThreadPrev();
	afx_msg void OnThreadPrevLow();
	afx_msg void OnThreadPrevHigh();
	afx_msg void OnScaleUp();
	afx_msg void OnScaleUpLow();
	afx_msg void OnScaleUpHigh();
	afx_msg void OnScaleDown();
	afx_msg void OnScaleDownLow();
	afx_msg void OnScaleDownHigh();
	afx_msg void OnMute();
	afx_msg void OnMinimizeMute();
	afx_msg void OnPause();
	afx_msg void OnMinimizePause();
	afx_msg void OnThreadReset();
	afx_msg void OnThreadReload();
	afx_msg void OnThreadRefresh();
	afx_msg void OnSnapshotSave();
	afx_msg void OnSnapshotFolder();
	afx_msg void OnWindowZoom100();
	afx_msg void OnWindowZoomRange(UINT nID);
	afx_msg void OnWindowSizeRange(UINT nID);
	afx_msg void OnWindowAspectRange(UINT nID);
	afx_msg void OnWindowAspectDefault();
	afx_msg void OnWindowAspectNone();
	afx_msg void OnWindowTopmost();
	afx_msg void OnWindowScreenSnap();
	afx_msg void OnWindowWindowSnap();
	afx_msg void OnWindowFixedRatio();
	afx_msg void OnWindowPlayRatio();
	afx_msg void OnWindowHeightRatio();
	afx_msg void OnWindowInscribeRatio();
	afx_msg void OnWindowFrame();
	afx_msg void OnWindowTitle();
	afx_msg void OnWindowBbs();
	afx_msg void OnWindowSeek();
	afx_msg void OnWindowStatus();
	afx_msg void OnWindowEdit();
	afx_msg void OnWindowBrowser();
	afx_msg void OnWindowBrowserTop();
	afx_msg void OnWindowBrowserBottom();
	afx_msg void OnWindowBrowserLeft();
	afx_msg void OnWindowBrowserRight();
	afx_msg void OnWindowBrowserToggleTop();
	afx_msg void OnWindowBrowserToggleBottom();
	afx_msg void OnWindowBrowserToggleLeft();
	afx_msg void OnWindowBrowserToggleRight();
	afx_msg void OnSleepStop();
	afx_msg void OnQuit();
	afx_msg void OnQuitStop();
	afx_msg void OnFullscreen();
	afx_msg void OnMaximize();
	afx_msg void OnFilterGraphRange(UINT nID);
	afx_msg void OnPrevFile();
	afx_msg void OnNextFile();
	afx_msg void OnPrevFileLoop();
	afx_msg void OnNextFileLoop();
	afx_msg void OnPlayModeNormal();
	afx_msg void OnPlayModeRepeat();
	afx_msg void OnPlayModeFolder();
	afx_msg void OnPlayModeFolderLoop();
	afx_msg void OnProcessPriorityRealtime();
	afx_msg void OnProcessPriorityHigh();
	afx_msg void OnProcessPriorityAboveNormal();
	afx_msg void OnProcessPriorityNormal();
	afx_msg void OnProcessPriorityBelowNomal();
	afx_msg void OnProcessPriorityIdle();
	afx_msg void OnUpdateChannelInfo();
	afx_msg void OnChannelInfoName();
	afx_msg void OnChannelInfoType();
	afx_msg void OnChannelInfoGenre();
	afx_msg void OnChannelInfoComment();
	afx_msg void OnChannelInfoPlaying();
	afx_msg void OnChannelInfoRelay();
	afx_msg void OnBbsBrowser();
	afx_msg void OnWebBrowser();
	afx_msg void OnOpenfileDialog();
	afx_msg void OnOpenfileClipboard();
	afx_msg void OnClipboardPath();
	afx_msg void OnClipboardUrl();
	afx_msg void OnVersion();
	afx_msg void OnLog();
	afx_msg void OnFullscreenHideBars();
	afx_msg void OnFullscreenFloating();
	afx_msg void OnFullscreenMouse();
	afx_msg void OnFullscreenTopmost();
	afx_msg void OnMiximizeToFullscreen();
	afx_msg void OnMiximizeShowTaskbar();
	afx_msg void OnTitlebarShowPlayerName();
	afx_msg void OnTitlebarShowWindowSize();
	afx_msg void OnTitlebarShowSourceSize();
	afx_msg void OnStatusbarShowFps();
	afx_msg void OnStatusbarShowWindowSize();
	afx_msg void OnStatusbarTime();
	afx_msg void OnStatusbarPlay();
	afx_msg void OnSeekbarShowFilename();
	afx_msg void OnBbsbarShowCounter();
	afx_msg void OnStartWindowZoom100();
	afx_msg void OnThread();
	afx_msg void OnBbsMenu();
	afx_msg void OnBbsSage();
	afx_msg void OnBbsLinkRange(UINT nID);
	afx_msg void OnUpdateTitle();
	afx_msg void OnContextSearch();
	afx_msg void OnContextExtract();
	afx_msg void OnContextPost();
	afx_msg void OnContextBbsBrowser();
};
