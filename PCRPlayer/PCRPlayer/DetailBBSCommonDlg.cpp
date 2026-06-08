// DetailBBSCommonDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSCommonDlg.h"

#include "afxdialogex.h"


// CDetailBBSCommonDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSCommonDlg, CDialog)

//CDetailBBSCommonDlg::CDetailBBSCommonDlg(CWnd* pParent /*=NULL*/)
CDetailBBSCommonDlg::CDetailBBSCommonDlg(sl::OptionConfing::BBS& option, sl::BoardConfig& board, sl::FontConfig& font)
	: CDialog(CDetailBBSCommonDlg::IDD, NULL)
	, select_(0)
	, dlg_(option, board, font)
{

}

CDetailBBSCommonDlg::~CDetailBBSCommonDlg()
{
}

void CDetailBBSCommonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_xcTab);
}


BEGIN_MESSAGE_MAP(CDetailBBSCommonDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDetailBBSCommonDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CDetailBBSCommonDlg メッセージ ハンドラー


BOOL CDetailBBSCommonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	auto bind = [this](const std::wstring& text, CDialog* dlg, int idd, int index){
		m_xcTab.InsertItem(index, text.c_str());
		Item item = { dlg, idd, index };
		vec_.push_back(item);
	};

	bind(L"全般", &dlg_.general, IDD_BASE_PROP, 0);
	bind(L"ネットワーク", &dlg_.network, IDD_BASE_PROP, 1);
	bind(L"スレッド", &dlg_.thread, IDD_BASE_PROP, 2);
	bind(L"フォント", &dlg_.font, IDD_BASE_PROP, 3);
	bind(L"URL置換", &dlg_.url, CReplaceDlg::IDD, 4);
	bind(L"レス本文置換", &dlg_.message, CReplaceDlg::IDD, 5);

	m_xcTab.GetWindowRect(&rect_);
	m_xcTab.AdjustRect(FALSE, &rect_);
	m_xcTab.ScreenToClient(&rect_);

	m_xcTab.SetCurSel(select_);
	OnTcnSelchangeTab(0, 0);
	m_xcTab.ModifyStyle(0, WS_CLIPCHILDREN);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDetailBBSCommonDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return FALSE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CDetailBBSCommonDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int sel = m_xcTab.GetCurSel();
	if (sel < 0) { if (pResult) { *pResult = 0; } }

	for (auto it = vec_.begin(); it != vec_.end(); ++it)
	{
		if (it->dlg)
		{
			if (it->index == sel)
			{
				if (!::IsWindow(it->dlg->GetSafeHwnd()))
				{
					it->dlg->Create(it->idd, this);
					it->dlg->SetParent(&m_xcTab);
					it->dlg->MoveWindow(rect_);
				}
				it->dlg->ShowWindow(SW_SHOW);
				select_ = sel;
			}
			else
			{
				if (::IsWindow(it->dlg->GetSafeHwnd()))
				{
					it->dlg->ShowWindow(SW_HIDE);
				}
			}
		}
	}

	if (pResult) { *pResult = 0; }
}
