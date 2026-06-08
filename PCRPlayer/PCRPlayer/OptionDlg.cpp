// OptionDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "OptionDlg.h"
#include "afxdialogex.h"


// COptionDlg ダイアログ

IMPLEMENT_DYNAMIC(COptionDlg, CDialog)

COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent)
	, select_(0)
{

}

COptionDlg::~COptionDlg()
{
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_xcTree);
	DDX_Control(pDX, IDC_TITLE, m_xcTitle);
}

void COptionDlg::createItem(const Item& item)
{
	if (!::IsWindow(item.dlg->GetSafeHwnd()))
	{
		item.dlg->Create(item.idd, this);
		item.dlg->SetWindowPos(NULL, pos_.x, pos_.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	item.dlg->ShowWindow(SW_SHOW);
	setTitle(item.title);
}

void COptionDlg::setTitle(const std::wstring& title)
{
	SetWindowText((L"オプション - " + title).c_str());
	m_xcTitle.SetWindowText(title.c_str());
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &COptionDlg::OnTvnSelchangedTree)
END_MESSAGE_MAP()


// COptionDlg メッセージ ハンドラー


BOOL COptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	namespace fs = boost::filesystem;

	std::vector<std::wstring> skins;
	if (utl::getFolderPath(gl_.current + L"skin\\", skins))
	{
		for (auto it = skins.begin(); it != skins.end(); ++it)
		{
			*it = fs::path(*it).filename().wstring();
		}

		detail_.bbs.browser.dlg_.skin.thread.setSkins(skins);
		detail_.bbs.browser.dlg_.skin.popup.setSkins(skins);
	}

	//----------------------------------------------------------

	auto parent = [this](const std::wstring& text)->HTREEITEM {
		HTREEITEM item = m_xcTree.InsertItem(text.c_str());
		m_xcTree.SetItemState(item, TVIS_BOLD|TVIS_EXPANDED, TVIS_BOLD|TVIS_EXPANDED);
		return item;
	};

	auto bind = [this](HTREEITEM parent, const std::wstring& text, CDialog* dlg, int idd)->bool {
		if (dlg == NULL) { return false; }
		auto item = m_xcTree.InsertItem(text.c_str(), parent);

		Item tmp = { item, text, dlg, idd };
		items_.push_back(tmp);
		return true;
	};

	//----------------------------------------------------------

	HTREEITEM item = NULL;

	item = parent(L"映像設定");
	bind(item, L"映像調整",				&video_.adjust, CVideoAdjustDlg::IDD);

	item = parent(L"詳細設定");
	bind(item, L"ネットワーク",			&detail_.network, CDetailNetworkDlg::IDD);
	bind(item, L"タイプ",				&detail_.type, CDetailTypeDlg::IDD);
	bind(item, L"デコーダ",				&detail_.decoder, CDetailDecoderDlg::IDD);
	bind(item, L"ブラウザ",				&detail_.browser, IDD_BASE_PROP_WIDE);
	bind(item, L"スナップショット",		&detail_.snapshot, IDD_BASE_PROP_WIDE);
	bind(item, L"起動時",				&detail_.start, IDD_BASE_PROP_WIDE);
	bind(item, L"終了時",				&detail_.end, CDetailEndDlg::IDD);

	item = parent(L"BBS設定");
	bind(item, L"共通",					&detail_.bbs.common, CDetailBBSCommonDlg::IDD);
	bind(item, L"BBSバー",				&detail_.bbs.bar, CDetailBBSBarDlg::IDD);
	bind(item, L"内蔵BBSブラウザ",		&detail_.bbs.browser, CDetailBBSBrowserDlg::IDD);
	
	item = parent(L"操作設定");
	bind(item, L"ショートカット",		&operation_.shortcut, COperationShortcutDlg::IDD);
	bind(item, L"マウス",				&operation_.mouse, IDD_BASE_PROP_WIDE);
	bind(item, L"ホイール変化量",		&operation_.variation, IDD_BASE_PROP_WIDE);
	bind(item, L"ウィンドウ",			&operation_.window, IDD_BASE_PROP_WIDE);

	item = parent(L"表示設定");
	bind(item, L"倍率/サイズ",			&display_.window, CDisplayWindowDlg::IDD);
	bind(item, L"アスペクト比",			&display_.aspect, CDisplayAspectDlg::IDD);
	bind(item, L"日時",					&display_.datetime, CDisplayDatetimeDlg::IDD);
	bind(item, L"チャンネル情報",		&display_.information, CDisplayInformationDlg::IDD);

	item = parent(L"レイアウト");
	bind(item, L"ステータスバー",		&display_.status, CDisplayStatusDlg::IDD);
	bind(item, L"シークバー",			&display_.seek, CDisplaySeekDlg::IDD);
	bind(item, L"シークヒント",			&display_.hint, CDisplayHintDlg::IDD);
	bind(item, L"チャンネル情報",		&display_.channel, CDisplayChannelDlg::IDD);
	bind(item, L"BBSバー",				&display_.bbs, CDisplayBBSDlg::IDD);
	bind(item, L"レスポップアップ",		&display_.res, CDisplayResDlg::IDD);

	//----------------------------------------------------------

	int marginX = 16, marginY = 8;
	int x = 0, y = 0;

	CRect rc;
	m_xcTree.GetWindowRect(&rc);
	x = rc.Width();
	m_xcTitle.GetWindowRect(&rc);
	y = rc.Height();

	pos_.SetPoint(x + marginX, y + marginY);

	try {
		Item i = items_.at(select_);
		createItem(i);
		m_xcTree.SetItemState(i.item, TVIS_SELECTED, TVIS_SELECTED);
		m_xcTree.EnsureVisible(i.item);
	} catch (...) {}

	//	LOGFONT lf;
	//	GetFont()->GetLogFont(&lf);
	//	lf.lfWeight = FW_BOLD;
	//	font_.CreateFontIndirect(&lf);
	//	m_xcTitle.SetFont(&font_);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL COptionDlg::PreTranslateMessage(MSG* pMsg)
{
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH COptionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: ここで DC の属性を変更してください。
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		if (pWnd && *pWnd == m_xcTitle)
		{
			// 文字色
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			// 文字背景色
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
			// 背景色
			hbr = GetSysColorBrush(COLOR_HIGHLIGHT);
		}
	}
	// TODO: 既定値を使用したくない場合は別のブラシを返します。
	return hbr;
}

void COptionDlg::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM item = pNMTreeView->itemNew.hItem;

	HTREEITEM parent = m_xcTree.GetParentItem(item);

	if (parent)
	{
		int i = 0;
		for (auto it = items_.begin(); it != items_.end(); ++it)
		{
			if (it->dlg)
			{
				if (it->item == item)
				{
					createItem(*it);
					select_ = i;
				}
				else
				{
					if (::IsWindow(it->dlg->GetSafeHwnd()))
					{
						it->dlg->ShowWindow(SW_HIDE);
					}
				}
			}
			++i;
		}
	}

	if (pResult) { *pResult = 0; }
}

