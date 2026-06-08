// ThreadDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ThreadDlg.h"
#include "afxdialogex.h"


// CThreadDlg ダイアログ

IMPLEMENT_DYNAMIC(CThreadDlg, CDialog)

CThreadDlg::CThreadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadDlg::IDD, pParent)
	, current_(0)
	, order_(true)
	, focus_(false)
{
	gl_.bbs.setSubjectCallback([this](bool start){
		if (IsWindow(this->GetSafeHwnd()))
		{
			if (start)
			{
				this->PostMessage(WM_COMMAND, IDM_UPDATE_THREAD_START);
			}
			else
			{
				this->PostMessage(WM_COMMAND, IDM_UPDATE_THREAD_END);
			}
		}
	});
}

CThreadDlg::~CThreadDlg()
{
	gl_.bbs.setSubjectCallback([](bool){});
}

void CThreadDlg::setFont(const std::wstring& name, int point)
{
	if (::IsWindow(GetSafeHwnd()) &&
		::IsWindow(m_xcEdit) &&
		::IsWindow(m_xcBtnUpdate) &&
		::IsWindow(m_xcList) &&
		::IsWindow(status_))
	{
		if (font_.GetSafeHandle()) { font_.DeleteObject(); }
		font_.CreatePointFont(point * 10, name.c_str());

		m_xcEdit.SetFont(&font_);
		m_xcBtnUpdate.SetFont(&font_);
		m_xcList.SetFont(&font_);
		status_.SetFont(&font_);

		this_.height = utl::getEditHeight(GetSafeHwnd(), font_);
		SizeHelper();
	}
}

void CThreadDlg::createMenu()
{
	if (::IsWindow(GetSafeHwnd()))
	{
		CreateMenu();
	}
}

//INT_PTR CThreadDlg::DoModal()
//{
//	CDialogTemplate dlt;
//
//	if (!dlt.Load(MAKEINTRESOURCE(IDD)))
//	{
//		return CDialog::DoModal();
//	}
//
//	//SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 
//
//	NONCLIENTMETRICS ncm;
//	ncm.cbSize = sizeof(NONCLIENTMETRICS);
//	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 
//	0);
//
//	//CFont font;
//	//font.CreateFontIndirect(&ncm.lfCaptionFont);
//	//m_Font.CreateFontIndirect(&ncm.lfCaptionFont);
//
//	//dlt.SetFont(ncm.lfCaptionFont.lfFaceName, utl::heightToPoint(abs(ncm.lfCaptionFont.lfHeight)));
//	dlt.SetFont(L"MS UI Gothic", 9);
//
//	LPSTR pdata = (LPSTR)GlobalLock(dlt.m_hTemplate);
//	m_lpszTemplateName = NULL;
//	InitModalIndirect(pdata);
//	int nResult = CDialog::DoModal();
//	GlobalUnlock(dlt.m_hTemplate);
//
//	return nResult;
//}


void CThreadDlg::GetContactURL(std::wstring& url)
{
	if (gl_.peca.getContactURL(url)) { return; }
	url = gl_.latest.contact;
}

void CThreadDlg::CreateMenu()
{
	if (menu_.GetSafeHmenu()) { menu_.DestroyMenu(); }

	menu_.LoadMenu(IDR_THREADMENU);
	SetMenu(&menu_);

	auto loadLinks = [](CMenu* menu, const bbs::Links& links, DWORD id, DWORD range){
		int i = 0;
		for (auto it = links.begin(); it != links.end(); ++it)
		{
			std::wstring title = it->title;
			if (title.empty())
			{
				title = it->url;
			}

			utl::replaceAll(title, L"&", L"&&");

			CString str;
			str.Format(L"%s", title.c_str());

			menu->InsertMenu(
				id,
				MF_BYCOMMAND|MF_STRING,
				range + i,
				str);

			++i;
			if (i >= 100) { break; }
		}
		menu->DeleteMenu(id, MF_BYCOMMAND);
	};

	CMenu* menu = menu_.GetSubMenu(0);
	if (menu)
	{
		if (!cfg_.links.empty())
		{
			menu->InsertMenu(IDM_FAVORITE_MENU, MF_BYCOMMAND|MF_SEPARATOR);
		}

		loadLinks(menu, cfg_.links, IDM_FAVORITE_MENU, IDM_FAVORITE_RANGE_BEGIN);
	}

	menu = menu_.GetSubMenu(1);
	if (menu)
	{
		if (!cfg_.threads.empty())
		{
			menu->InsertMenu(IDM_FAVORITE_THREAD_MENU, MF_BYCOMMAND|MF_SEPARATOR);
		}

		loadLinks(menu, cfg_.threads, IDM_FAVORITE_THREAD_MENU, IDM_FAVORITE_THREAD_RANGE_BEGIN);
	}

	menu = menu_.GetSubMenu(2);
	if (menu)
	{
		bbs::Links links, threads;
		gl_.bbs.getHistory(false, links);
		gl_.bbs.getHistory(true, threads);

		loadLinks(menu, links, IDM_HISTORY_MENU, IDM_HISTORY_RANGE_BEGIN);
		loadLinks(menu, threads, IDM_HISTORY_THREAD_MENU, IDM_HISTORY_THREAD_RANGE_BEGIN);
	}

	menu = menu_.GetSubMenu(3);
	if (menu)
	{
		auto checkItem = [=](int id, bool check) {
			menu->CheckMenuItem(id, (check ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		};

		checkItem(IDM_THREAD_STOP, cfg_.stop);
		checkItem(IDM_THREAD_STRETCH, cfg_.stretch);
		checkItem(IDM_THREAD_DATETIME, cfg_.datetime);
		checkItem(IDM_THREAD_CLOSE, cfg_.close);
	}

	menu = menu_.GetSubMenu(4);
	if (menu)
	{
		std::wstring url;
		if (gl_.player.peercast())
		{
			GetContactURL(url);
		}

		if (url.empty())
		{
			menu->EnableMenuItem(IDM_THREAD_CONTACT_URL, MF_GRAYED);
			menu->EnableMenuItem(IDM_THREAD_BBS_BROWSER, MF_GRAYED);
			menu->EnableMenuItem(IDM_THREAD_WEB_BROWSER, MF_GRAYED);
			menu->EnableMenuItem(IDM_THREAD_CLIPBOARD_URL, MF_GRAYED);
		}
	}
}

void CThreadDlg::CheckMenuItem(int id, bool check)
{
	CMenu* menu = menu_.GetSubMenu(3);
	menu->CheckMenuItem(id, (check ? MF_CHECKED : MF_UNCHECKED)|MF_BYCOMMAND);
}

void CThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_xcEdit);
	DDX_Control(pDX, IDC_BTN_UPDATE, m_xcBtnUpdate);
	DDX_Control(pDX, IDC_LIST, m_xcList);
}

void CThreadDlg::SizeHelper()
{
	if (::IsWindow(m_xcEdit) &&
		::IsWindow(m_xcList) &&
		::IsWindow(status_))
	{
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

		CRect rc;
		GetClientRect(rc);

		CRect br;
		m_xcBtnUpdate.GetWindowRect(&br);

		m_xcEdit.MoveWindow(rc.left, rc.top, rc.Width() - br.Width(), this_.height);
		m_xcBtnUpdate.MoveWindow(rc.left + rc.Width() - br.Width(), rc.top, br.Width(), this_.height);

		CRect sr;
		status_.GetWindowRect(&sr);
		m_xcList.MoveWindow(rc.left, this_.height, rc.Width(), rc.Height() - this_.height - sr.Height());
	}
}


void CThreadDlg::ResetList()
{
	subject_.clear();

	std::wstring key;
	gl_.bbs.getKey(key);

	int stop = 1000;
	gl_.bbs.getStop(stop);

	bool exist = false;
	int i = 0;
	int pos = 0;
	for (auto it = org_.begin(); it != org_.end(); ++it)
	{
		if (cfg_.stop || it->count < stop)
		{
			subject_.push_back(*it);

			if (!key.empty() && key == it->key)
			{
				pos = i;
				exist = true;
			}

			++i;
		}
	}

	m_xcList.SetItemCountEx(static_cast<int>(subject_.size()), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

	if (exist && pos >= 0 && pos < static_cast<int>(subject_.size()))
	{
		m_xcList.SetItemState(pos, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		m_xcList.EnsureVisible(pos, FALSE);
	}

	m_xcList.RedrawWindow();

	ChangeStatus();
}


void CThreadDlg::ChangeStatus()
{
	std::wstring noname;
	if (gl_.bbs.getNoName(noname) && !noname.empty())
	{
		noname = L" デフォルト名:\"" + noname + L"\"";
	}

	CString str;
	if (cfg_.stop)
	{
		str.Format(L"スレッド数:%u%s", subject_.size(), noname.c_str());
	}
	else
	{
		str.Format(L"スレッド数:%u/%u%s", subject_.size(), org_.size(), noname.c_str());
	}
	status_.SetPaneText(0, str);
}


BEGIN_MESSAGE_MAP(CThreadDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, &CThreadDlg::OnLvnGetdispinfoList)
	ON_NOTIFY(NM_CLICK, IDC_LIST, &CThreadDlg::OnNMClickList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, &CThreadDlg::OnLvnColumnclickList)
	ON_BN_CLICKED(IDC_BTN_UPDATE, &CThreadDlg::OnBnClickedBtnUpdate)
	ON_WM_DESTROY()
	ON_COMMAND(IDM_UPDATE_THREAD_START, &CThreadDlg::OnUpdateThreadStart)
	ON_COMMAND(IDM_UPDATE_THREAD_END, &CThreadDlg::OnUpdateThreadEnd)
	ON_COMMAND(IDM_FAVORITE_ADD, &CThreadDlg::OnFavoriteAdd)
	ON_COMMAND(IDM_FAVORITE_EDIT, &CThreadDlg::OnFavoriteEdit)
	ON_COMMAND_RANGE(IDM_FAVORITE_RANGE_BEGIN, IDM_FAVORITE_RANGE_END, &CThreadDlg::OnFavoriteRange)
	ON_COMMAND(IDM_FAVORITE_THREAD_ADD, &CThreadDlg::OnFavoriteThreadAdd)
	ON_COMMAND(IDM_FAVORITE_THREAD_EDIT, &CThreadDlg::OnFavoriteThreadEdit)
	ON_COMMAND_RANGE(IDM_FAVORITE_THREAD_RANGE_BEGIN, IDM_FAVORITE_THREAD_RANGE_END, &CThreadDlg::OnFavoriteThreadRange)
	ON_COMMAND(IDM_HISTORY_CLEAR, &CThreadDlg::OnHistoryClear)
	ON_COMMAND_RANGE(IDM_HISTORY_RANGE_BEGIN, IDM_HISTORY_RANGE_END, &CThreadDlg::OnHistoryRange)
	ON_COMMAND(IDM_HISTORY_THREAD_CLEAR, &CThreadDlg::OnHistoryThreadClear)
	ON_COMMAND_RANGE(IDM_HISTORY_THREAD_RANGE_BEGIN, IDM_HISTORY_THREAD_RANGE_END, &CThreadDlg::OnHistoryThreadRange)
	ON_COMMAND(IDM_THREAD_STOP, &CThreadDlg::OnThreadStop)
	ON_COMMAND(IDM_THREAD_STRETCH, &CThreadDlg::OnThreadStretch)
	ON_COMMAND(IDM_THREAD_DATETIME, &CThreadDlg::OnThreadDatetime)
	ON_COMMAND(IDM_THREAD_CLOSE, &CThreadDlg::OnThreadClose)
	ON_COMMAND(IDM_THREAD_CONTACT_URL, &CThreadDlg::OnThreadContactUrl)
	ON_COMMAND(IDM_THREAD_BBS_BROWSER, &CThreadDlg::OnThreadBbsBrowser)
	ON_COMMAND(IDM_THREAD_WEB_BROWSER, &CThreadDlg::OnThreadWebBrowser)
	ON_COMMAND(IDM_THREAD_CLIPBOARD_URL, &CThreadDlg::OnThreadClipboardUrl)
END_MESSAGE_MAP()


// CThreadDlg メッセージ ハンドラー


void CThreadDlg::OnOK()
{// モードレスダイアログ終了
	DestroyWindow();

	//CDialog::OnOK();
}


void CThreadDlg::OnCancel()
{// モードレスダイアログ終了
	DestroyWindow();

	//CDialog::OnCancel();
}


BOOL CThreadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// サイズ変更枠
	this->ModifyStyle(0, WS_THICKFRAME);

	// スタイル変更/サイズの適用
	SetWindowPos(NULL, 0, 0, cfg_.width, cfg_.height, SWP_NOZORDER|SWP_NOMOVE|SWP_FRAMECHANGED);

	// メニュー
	CreateMenu();

	// リスト
	m_xcList.SetExtendedStyle(m_xcList.GetExtendedStyle()|
		LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_DOUBLEBUFFER);

	m_xcList.InsertColumn(0, L"", LVCFMT_LEFT, cfg_.column[0]);
	m_xcList.InsertColumn(1, L"No.", LVCFMT_CENTER, cfg_.column[1]);
	m_xcList.InsertColumn(2, L"タイトル", LVCFMT_LEFT, cfg_.column[2]);
	m_xcList.InsertColumn(3, L"レス", LVCFMT_CENTER, cfg_.column[3]);
	m_xcList.InsertColumn(4, L"勢い", LVCFMT_CENTER, cfg_.column[4]);
	m_xcList.InsertColumn(5, L"since", LVCFMT_LEFT, cfg_.column[5]);
	m_xcList.SetStretchColumn(cfg_.stretch ? 2 : -1); // 伸長するカラム
	m_xcList.SetStripe(true);
	m_xcList.SetHeaderText(5, cfg_.datetime ? L"since" : L"days");


	// ステータスバー
	if (status_.CreateEx(this, SBARS_SIZEGRIP))
	{
		const static UINT ind[1] = {ID_SEPARATOR};
		if (status_.SetIndicators(ind, 1))
		{
			status_.SetPaneInfo(0, 0, SBPS_STRETCH, 0);
		}
	}

	setFont(gl_.value.detail.font.thread.name, gl_.value.detail.font.thread.point);

	CenterWindow(AfxGetMainWnd());

	OnUpdateThreadEnd();
	//gl_.bbs.subject();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CThreadDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		CWnd* wnd = GetFocus();
		if (wnd && wnd->GetSafeHwnd() == m_xcEdit.GetSafeHwnd())
		{
			OnBnClickedBtnUpdate();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CThreadDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}

void CThreadDlg::OnLvnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (!pDispInfo) { return; }
	int i = pDispInfo->item.iItem;

	if (pDispInfo->item.mask & LVIF_TEXT)
	{ 
		CString csLine;
		try {
			switch (pDispInfo->item.iSubItem)
			{
			case 1:
				{
					csLine = subject_.at(i).number.c_str();
					break;
				}
			case 2:
				{
					csLine = subject_.at(i).title.c_str();
					break;
				}
			case 3:
				{
					csLine.Format(L"%d", subject_.at(i).count);
					break;
				}
			case 4:
				{
					csLine.Format(L"%0.1f", subject_.at(i).speed);
					break;
				}
			case 5:
				{
					if (cfg_.datetime)
					{
						using namespace boost::posix_time;
						using namespace boost::date_time;

						time_t time = wcstol(subject_.at(i).key.c_str(), NULL, 10);
						ptime pt = c_local_adjustor<ptime>::utc_to_local(from_time_t(time));

						std::wstring date = utl::convertDate(pt, sl::DATETIME_DATE_FORMAT_NONE, sl::DATETIME_TIME_FORMAT_24);
						csLine.Format(L"%s", date.c_str());
					}
					else
					{
						using namespace boost::posix_time;
						using namespace boost::date_time;

						time_t time = wcstol(subject_.at(i).key.c_str(), NULL, 10);
						ptime pt = c_local_adjustor<ptime>::utc_to_local(from_time_t(time));
						ptime lt = boost::posix_time::second_clock::local_time();

						double t = static_cast<double>((lt - pt).total_seconds()) / 60.0 / 60.0 / 24.0;
						csLine.Format(L"%.2f", t);
					}
				}
			}
		} catch (...) {}

		wcsncpy_s(
			pDispInfo->item.pszText,
			pDispInfo->item.cchTextMax,
			csLine, _TRUNCATE);
	}

	if (pResult) { *pResult = 0; }
}

void CThreadDlg::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int i = (pNMItemActivate->iItem);

	if (i >= 0)
	{
		try {
			gl_.bbs.change(subject_.at(i).key);
			if (cfg_.close)
			{
				if (pResult) { *pResult = 0; }
				DestroyWindow();
				return;
			}
		} catch(...) {}
	}

	if (pResult) { *pResult = 0; }
}


void CThreadDlg::OnLvnColumnclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (!pNMLV) { return; }

	size_t i = pNMLV->iSubItem;

	bool order = false;

	if (i != current_)
	{
		order_ = true;
		current_ = (int)i;
	}

	order = order_;
	order_ = order_ ? false : true;
	
	switch(i)
	{
	case 1:
		std::stable_sort(subject_.begin(), subject_.end(),
			[&](const bbs::ThreadInfo& l, const bbs::ThreadInfo& r)->bool{
				int result = StrCmpLogicalW(l.number.c_str(), r.number.c_str());
				return order ? result < 0 : result > 0;
		});
		m_xcList.RedrawWindow();
		break;
	case 2:
		std::stable_sort(subject_.begin(), subject_.end(),
			[&](const bbs::ThreadInfo& l, const bbs::ThreadInfo& r)->bool{
				int result = StrCmpLogicalW(l.title.c_str(), r.title.c_str());
				return order ? result < 0 : result > 0;
		});
		m_xcList.RedrawWindow();
		break;
	case 3:
		std::stable_sort(subject_.begin(), subject_.end(),
			[&](const bbs::ThreadInfo& l, const bbs::ThreadInfo& r)->bool{
				return order ? l.count < r.count : l.count > r.count;
		});
		m_xcList.RedrawWindow();
		break;
	case 4:
		std::stable_sort(subject_.begin(), subject_.end(),
			[&](const bbs::ThreadInfo& l, const bbs::ThreadInfo& r)->bool{
				return order ? l.speed < r.speed : l.speed > r.speed;
		});
		m_xcList.RedrawWindow();
		break;
	case 5:
		std::stable_sort(subject_.begin(), subject_.end(),
			[&](const bbs::ThreadInfo& l, const bbs::ThreadInfo& r)->bool{
				int result = StrCmpLogicalW(l.key.c_str(), r.key.c_str());
				return order ? result < 0 : result > 0;
		});
		m_xcList.RedrawWindow();
		break;

	default:
		break;
	}

	if (pResult) { *pResult = 0; }
}


void CThreadDlg::OnBnClickedBtnUpdate()
{
	CString text;
	m_xcEdit.GetWindowText(text);

	gl_.bbs.execute(std::wstring(text), gl_.value.detail.board, gl_.value.detail.board.movable.manual);
}


void CThreadDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CRect rc;
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(&wp))
	{
		rc = wp.rcNormalPosition;
	}
	else
	{
		GetWindowRect(rc);
	}

	cfg_.width = rc.Width();
	cfg_.height = rc.Height();

	for (int i = 0; i < (int)cfg_.column.size(); ++i)
	{
		cfg_.column[i] = m_xcList.GetColumnWidth(i);
	}

	if (menu_.GetSafeHmenu()) { menu_.DestroyMenu(); }
}




void CThreadDlg::OnUpdateThreadStart()
{
	status_.SetPaneText(0, L"スレッド一覧更新中");
}

void CThreadDlg::OnUpdateThreadEnd()
{
	std::wstring url;
	if (!gl_.bbs.getURL(url))
	{
		gl_.bbs.getSourceURL(url);
	}

	if (!url.empty())
	{
		m_xcEdit.SetWindowText(url.c_str());
		m_xcEdit.SetSel(0, -1);
	}

	std::wstring bbs;
	if (gl_.bbs.getBoardTitle(bbs))
	{
		std::wstring title = L"スレッド選択";
		if (!bbs.empty())
		{
			title += L" 【" + bbs + L"】";
		}

		std::wstring thread;
		if (gl_.bbs.getThreadTitle(thread) && !thread.empty())
		{
			title += L" " + thread;
		}

		SetWindowText(title.c_str());
	}
	else
	{
		SetWindowText(L"スレッド選択");
	}

	std::vector<bbs::ThreadInfo> info;
	if (gl_.bbs.getSubject(info))
	{
		org_ = info;
	}
	else
	{
		org_.clear();
	}
	ResetList();

	CreateMenu();
}




void CThreadDlg::OnFavoriteAdd()
{
	std::wstring title, url;
	if (gl_.bbs.getBoardTitle(title) && gl_.bbs.getBoardURL(url))
	{
		bool exist = false;
		for (auto it = cfg_.links.begin(); it != cfg_.links.end(); ++it)
		{
			if (it->url == url)
			{
				exist = true;
				break;
			}
		}

		if (!exist)
		{
			bbs::Link link = { title, url };
			cfg_.links.push_back(link);

			CreateMenu();
		}
	}
}


void CThreadDlg::OnFavoriteEdit()
{
	CFavoriteDlg dlg;
	dlg.links_ = cfg_.links;
	if (dlg.DoModal() == IDOK)
	{
		cfg_.links = dlg.links_;
		CreateMenu();
	}
}

void CThreadDlg::OnFavoriteRange(UINT nID)
{
	int i = nID - IDM_FAVORITE_RANGE_BEGIN;
	if (i < 0 || i >= (int)cfg_.links.size()) { return; }

	try {
		bbs::Link link = cfg_.links.at(i);

		m_xcEdit.SetWindowText(link.url.c_str());
		gl_.bbs.execute(link.url, gl_.value.detail.board, gl_.value.detail.board.movable.favorite.board);
	} catch(...) {}
}

void CThreadDlg::OnFavoriteThreadAdd()
{
	std::wstring board, thread, url;
	if (gl_.bbs.getBoardTitle(board) &&
		gl_.bbs.getThreadTitle(thread) &&
		gl_.bbs.getThreadURL(url))
	{
		bool exist = false;
		for (auto it = cfg_.threads.begin(); it != cfg_.threads.end(); ++it)
		{
			if (it->url == url)
			{
				exist = true;
				break;
			}
		}

		if (!exist)
		{
			bbs::Link link = { L"【" + board + L"】 " + thread, url };
			cfg_.threads.push_back(link);

			CreateMenu();
		}
	}
}


void CThreadDlg::OnFavoriteThreadEdit()
{
	CFavoriteDlg dlg;
	dlg.links_ = cfg_.threads;
	if (dlg.DoModal() == IDOK)
	{
		cfg_.threads = dlg.links_;
		CreateMenu();
	}
}

void CThreadDlg::OnFavoriteThreadRange(UINT nID)
{
	int i = nID - IDM_FAVORITE_THREAD_RANGE_BEGIN;
	if (i < 0 || i >= (int)cfg_.threads.size()) { return; }

	try {
		bbs::Link link = cfg_.threads.at(i);

		m_xcEdit.SetWindowText(link.url.c_str());
		gl_.bbs.execute(link.url, gl_.value.detail.board, gl_.value.detail.board.movable.favorite.thread);
	} catch(...) {}
}

void CThreadDlg::OnHistoryClear()
{
	if (AfxMessageBox(L"掲示板履歴をクリアしますか？", MB_YESNO) == IDYES)
	{
		gl_.bbs.clearHistory(false);
		CreateMenu();
	}
}

void CThreadDlg::OnHistoryRange(UINT nID)
{
	bbs::Links links;
	gl_.bbs.getHistory(false, links);

	int i = nID - IDM_HISTORY_RANGE_BEGIN;
	if (i < 0 || i >= (int)links.size()) { return; }

	try {
		bbs::Link link = links.at(i);

		m_xcEdit.SetWindowText(link.url.c_str());
		gl_.bbs.execute(link.url, gl_.value.detail.board, gl_.value.detail.board.movable.history.board);
	} catch(...) {}
}

void CThreadDlg::OnHistoryThreadClear()
{
	if (AfxMessageBox(L"スレッド履歴をクリアしますか？", MB_YESNO) == IDYES)
	{
		gl_.bbs.clearHistory(true);//
		CreateMenu();
	}
}

void CThreadDlg::OnHistoryThreadRange(UINT nID)
{
	bbs::Links links;
	gl_.bbs.getHistory(true, links);//

	int i = nID - IDM_HISTORY_THREAD_RANGE_BEGIN;
	if (i < 0 || i >= (int)links.size()) { return; }

	try {
		bbs::Link link = links.at(i);

		m_xcEdit.SetWindowText(link.url.c_str());
		gl_.bbs.execute(link.url, gl_.value.detail.board, gl_.value.detail.board.movable.history.thread);
	} catch(...) {}
}

void CThreadDlg::OnThreadStop()
{
	CheckMenuItem(IDM_THREAD_STOP, utl::toggle(cfg_.stop));

	ResetList();
	order_ = true;
	current_ = 0;
}

void CThreadDlg::OnThreadStretch()
{
	CheckMenuItem(IDM_THREAD_STRETCH, utl::toggle(cfg_.stretch));
	
	m_xcList.SetStretchColumn(cfg_.stretch ? 2 : -1);
	m_xcList.SizeHelper();
}

void CThreadDlg::OnThreadDatetime()
{
	CheckMenuItem(IDM_THREAD_DATETIME, utl::toggle(cfg_.datetime));

	m_xcList.SetHeaderText(5, cfg_.datetime ? L"since" : L"days");
	m_xcList.RedrawWindow();
}

void CThreadDlg::OnThreadClose()
{
	CheckMenuItem(IDM_THREAD_CLOSE, utl::toggle(cfg_.close));
}



void CThreadDlg::OnThreadContactUrl()
{
	if (gl_.player.peercast())
	{
		std::wstring url;
		GetContactURL(url);

		m_xcEdit.SetWindowText(url.c_str());
		gl_.bbs.execute(url, gl_.value.detail.board, gl_.value.detail.board.movable.contact);
	}
}


void CThreadDlg::OnThreadBbsBrowser()
{
	CMainDlg* wnd = (CMainDlg*)AfxGetMainWnd();
	if (gl_.player.peercast())
	{
		std::wstring url;
		GetContactURL(url);

		if (!url.empty())
		{
			std::wstring path = gl_.value.detail.browser.bbs;
			CMainDlg* wnd =(CMainDlg*) AfxGetMainWnd();
			if (wnd)
			{
				wnd->OpenProcess(path, url);
			}
			else
			{
				utl::processExecute(path, url);
			}
		}
	}
}


void CThreadDlg::OnThreadWebBrowser()
{
	if (gl_.player.peercast())
	{
		std::wstring url;
		GetContactURL(url);
		utl::selectExecute(gl_.value.detail.browser.def, gl_.value.detail.browser.web, url);
	}
}


void CThreadDlg::OnThreadClipboardUrl()
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
