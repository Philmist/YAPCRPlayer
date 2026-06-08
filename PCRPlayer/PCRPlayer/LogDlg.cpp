// LogDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "LogDlg.h"
#include "afxdialogex.h"

enum TIMER_ID {
	TIMER_ID_UPDATE,
};

std::wstring formatTime(const SYSTEMTIME& time)
{
	CString str;
	str.Format(L"%02d:%02d:%02d.%03d", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	return std::wstring(str);
}

std::wstring formatType(int type)
{
	switch (type)
	{
	case utl::Log::LOGTYPE_GRAPH:		return L"Graph";
	case utl::Log::LOGTYPE_FILTER:		return L"Filter";
	case utl::Log::LOGTYPE_BBS:			return L"BBS";
	case utl::Log::LOGTYPE_PEERCAST:	return L"PeerCast";
	case utl::Log::LOGTYPE_NONE:		return L"None";
	}
	return L"None";
}

std::wstring CLogDlg::List::textImpl(value_type& value, int sub)
{
	switch (sub)
	{
	case 0: return formatTime(value.time);
	case 1: return formatType(value.type);
	case 2: return value.desc;
	}
	return std::wstring();
}


// CLogDlg ダイアログ

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)

CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
	, list_(log_)
	, m_xcList(list_)
{

}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_xcEdit);
	DDX_Control(pDX, IDC_LIST, m_xcList);
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CLogDlg::OnLvnItemchangedList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CLogDlg::OnNMRClickList)
	ON_COMMAND(IDM_LOG_GRAPH, &CLogDlg::OnLogGraph)
	ON_COMMAND(IDM_LOG_FILTER, &CLogDlg::OnLogFilter)
	ON_COMMAND(IDM_LOG_BBS, &CLogDlg::OnLogBbs)
	ON_COMMAND(IDM_LOG_PEERCAST, &CLogDlg::OnLogPeerCast)
	ON_COMMAND(IDM_LOG_NONE, &CLogDlg::OnLogNone)
	ON_COMMAND(IDM_LOG_ALL_COPY, &CLogDlg::OnLogAllCopy)
	ON_COMMAND(IDM_LOG_COPY, &CLogDlg::OnLogCopy)
	ON_COMMAND(IDM_LOG_CLEAR, &CLogDlg::OnLogClear)
END_MESSAGE_MAP()



void CLogDlg::CreateMenu()
{
	if (menu_.GetSafeHmenu()) { menu_.DestroyMenu(); }

	menu_.LoadMenu(IDR_LOGMENU);
	SetMenu(&menu_);

	CheckMenuItem(IDM_LOG_GRAPH, gl_.value.dialog.log.graph);
	CheckMenuItem(IDM_LOG_FILTER, gl_.value.dialog.log.filter);
	CheckMenuItem(IDM_LOG_BBS, gl_.value.dialog.log.bbs);
	CheckMenuItem(IDM_LOG_PEERCAST, gl_.value.dialog.log.peercast);
	CheckMenuItem(IDM_LOG_NONE, gl_.value.dialog.log.none);
}

void CLogDlg::CheckMenuItem(int id, bool check)
{
	CMenu* menu = menu_.GetSubMenu(0);
	menu->CheckMenuItem(id, (check ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
}


// CLogDlg メッセージ ハンドラー

void CLogDlg::InitControls()
{
	int index = m_xcList.GetIndex();

	if (!list_.get(index, [this](const utl::Log& log) {
		m_xcEdit.SetWindowText(log.detail.c_str());
	}))
	{
		m_xcEdit.SetWindowText(L"");
	}
}

void CLogDlg::SizeHelper()
{
	if (::IsWindow(status_.GetSafeHwnd()))
	{
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}

	if (::IsWindow(status_.GetSafeHwnd()) &&
		::IsWindow(m_xcList.GetSafeHwnd()) &&
		::IsWindow(m_xcEdit.GetSafeHwnd()))
	{
		CRect rc;
		GetClientRect(&rc);

		CRect sr;
		status_.GetWindowRect(&sr);
		int height = 100;

		{
			CRect r(rc);
			r.bottom = max(r.bottom - height - sr.Height(), 0);
			m_xcList.MoveWindow(r);
		}

		{
			CRect r(rc);
			r.top = max(r.bottom - height - sr.Height(), 0);
			r.bottom = max(rc.Height() - sr.Height(), 0);
			m_xcEdit.MoveWindow(r);
		}
	}
}


BOOL CLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ステータスバー
	if (!::IsWindow(status_.GetSafeHwnd()))
	{
		status_.CreateEx(this, SBARS_SIZEGRIP);
	}

	if (::IsWindow(status_.GetSafeHwnd()))
	{
		const UINT ind[1] = {ID_SEPARATOR};
		status_.SetIndicators(ind, 1);
		status_.SetPaneInfo(0, 0, SBPS_STRETCH, 0);

		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}

	m_xcList.InsertColumn(0, L"時刻", LVCFMT_LEFT, 80);
	m_xcList.InsertColumn(1, L"タイプ", LVCFMT_LEFT, 60);
	m_xcList.InsertColumn(2, L"概要", LVCFMT_LEFT, 100);
	m_xcList.SetStretchColumn(2);

	// サイズ変更枠
	this->ModifyStyle(0, WS_THICKFRAME);

	// スタイル変更の適用
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

	//CMenu* pSysMenu = GetSystemMenu(FALSE);
	//if (pSysMenu != NULL)
	//{
	//	pSysMenu->InsertMenu(0, MF_BYPOSITION|MF_STRING, SC_SIZE, L"サイズ変更(&S)");
	//}


	// メニュー
	CreateMenu();

	SizeHelper();

	SetTimer(TIMER_ID_UPDATE, 100, NULL); // 更新用タイマー

	m_xcList.SelectLast();

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CLogDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。

	return CDialog::PreTranslateMessage(pMsg);
}

void CLogDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (menu_.GetSafeHmenu()) { menu_.DestroyMenu(); }
}

// モードレス ダイアログ ボックスの OK のボタンを実装する場合は、
// OnOK のメソッドをオーバーライドし、その中の DestroyWindow を呼び出す必要があります。
void CLogDlg::OnOK()
{
	DestroyWindow();
}


void CLogDlg::OnCancel()
{
	DestroyWindow();
}



HBRUSH CLogDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_EDIT)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		return static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	}
	return hbr;
}


void CLogDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}


void CLogDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_ID_UPDATE:
		{
			std::deque<utl::Log> log;
			if (gl_.getLog(log, [&](const utl::Log& l)->bool {
				switch (l.type)
				{
				case utl::Log::LOGTYPE_GRAPH:		return gl_.value.dialog.log.graph;
				case utl::Log::LOGTYPE_FILTER:		return gl_.value.dialog.log.filter;
				case utl::Log::LOGTYPE_BBS:			return gl_.value.dialog.log.bbs;
				case utl::Log::LOGTYPE_PEERCAST:	return gl_.value.dialog.log.peercast;
				case utl::Log::LOGTYPE_NONE:		return gl_.value.dialog.log.none;
				}
				return false;
			}))
			{
				BOOL select = m_xcList.IsSelectLast();
				list_.insert(log, LOGLIMIT);
				m_xcList.Apply();
				if (select)
				{
					m_xcList.SelectLast();
				}
				m_xcList.RedrawWindow();
				InitControls();
			}

			if (::IsWindow(status_.GetSafeHwnd()))
			{
				utl::StatusLog status;
				gl_.getStatus(status);
				if (status.valid)
				{
					CString str;
					str.Format(L"Buffer %lldms / Audio %lldms (%d) / Video %lldms (%d)",
						status.buffer / 10000i64,
						status.audio.total / 10000i64, status.audio.count,
						status.video.total / 10000i64, status.video.count);
					status_.SetPaneText(0, str);
				}
				else
				{
					status_.SetPaneText(0, L"");
				}
			}
		}
		break;
	}

	CDialog::OnTimer(nIDEvent);
}



void CLogDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	InitControls();

	if (pResult) { *pResult = 0; }
}


void CLogDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CMenu root;
	CMenu* menu = NULL;
	if (!root.LoadMenu(IDR_LOGMENU)) { return; }

	menu = root.GetSubMenu(1);
	if (menu == NULL) { return; }

	//---------------------------------------------------

	CPoint pt;
	GetCursorPos(&pt);
	menu->TrackPopupMenu(TPM_LEFTBUTTON, pt.x, pt.y, this);
	root.DestroyMenu();

	if (pResult) { *pResult = 0; }
}


void CLogDlg::OnLogGraph()
{
	CheckMenuItem(IDM_LOG_GRAPH, utl::toggle(gl_.value.dialog.log.graph));
}

void CLogDlg::OnLogFilter()
{
	CheckMenuItem(IDM_LOG_FILTER, utl::toggle(gl_.value.dialog.log.filter));
}

void CLogDlg::OnLogBbs()
{
	CheckMenuItem(IDM_LOG_BBS, utl::toggle(gl_.value.dialog.log.bbs));
}

void CLogDlg::OnLogPeerCast()
{
	CheckMenuItem(IDM_LOG_PEERCAST, utl::toggle(gl_.value.dialog.log.peercast));
}

void CLogDlg::OnLogNone()
{
	CheckMenuItem(IDM_LOG_NONE, utl::toggle(gl_.value.dialog.log.none));
}

void CLogDlg::OnLogAllCopy()
{
	std::wstring text;

	list_.foreach([&](const utl::Log& log){
		if (!text.empty())
		{
			text += L"\r\n";
		}

		std::wstring time = formatTime(log.time);
		text += time;
		text += L" : ";
		text += log.desc;

		if (!log.detail.empty())
		{
			utl::getLines(log.detail, false, [&](const std::wstring& detail) {
				text += L"\r\n" + time + L" : >\t" + detail;
			});
		}
	});

	if (OpenClipboard())
	{
		EmptyClipboard();

		size_t size = text.size() + 1;
		HGLOBAL hg = GlobalAlloc(GHND | GMEM_SHARE, sizeof(WCHAR) * size);
		LPWSTR mem = (LPWSTR)GlobalLock(hg);

		wcsncpy_s(mem, size, text.c_str(), _TRUNCATE);
		GlobalUnlock(hg);

		SetClipboardData(CF_UNICODETEXT, hg);

		CloseClipboard();
	}
}


void CLogDlg::OnLogCopy()
{
	std::wstring text;

	int index = m_xcList.GetIndex();
	list_.get(index, [&](const utl::Log& log){
		std::wstring time = formatTime(log.time);
		text += time;
		text += L" : ";
		text += log.desc;

		if (!log.detail.empty())
		{
			utl::getLines(log.detail, false, [&](const std::wstring& detail){
				text += L"\r\n" + time + L" : >\t" + detail;
			});
		}
	});

	if (OpenClipboard())
	{
		EmptyClipboard();

		size_t size = text.size() + 1;
		HGLOBAL hg = GlobalAlloc(GHND | GMEM_SHARE, sizeof(WCHAR) * size);
		LPWSTR mem = (LPWSTR)GlobalLock(hg);

		wcsncpy_s(mem, size, text.c_str(), _TRUNCATE);
		GlobalUnlock(hg);

		SetClipboardData(CF_UNICODETEXT, hg);

		CloseClipboard();
	}
}


void CLogDlg::OnLogClear()
{
	log_.clear();
	m_xcList.Init();
	m_xcList.RedrawWindow();
	InitControls();
}




