// DetailBBSBarDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBarDlg.h"

#include "afxdialogex.h"


// CDetailBBSBarDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBarDlg, CDialog)


//CDetailBBSBarDlg::CDetailBBSBarDlg(CWnd* pParent /*=NULL*/)
CDetailBBSBarDlg::CDetailBBSBarDlg(sl::OptionConfing::BBS& option, sl::BoardConfig& board)
	: CDialog(CDetailBBSBarDlg::IDD, NULL)
	, select_(0)
	, dlg_(option, board)
{

}

CDetailBBSBarDlg::~CDetailBBSBarDlg()
{
}

void CDetailBBSBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_xcTab);
}


BEGIN_MESSAGE_MAP(CDetailBBSBarDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDetailBBSBarDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CDetailBBSBarDlg メッセージ ハンドラー


BOOL CDetailBBSBarDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	auto bind = [this](const std::wstring& text, CDialog* dlg, int idd, int index){
		m_xcTab.InsertItem(index, text.c_str());
		Item item = { dlg, idd, index };
		vec_.push_back(item);
	};

	bind(L"スレッド", &dlg_.thread, IDD_BASE_PROP, 0);
	bind(L"ポップアップ", &dlg_.popup, IDD_BASE_PROP, 1);
	bind(L"書き込み欄", &dlg_.post, IDD_BASE_PROP, 2);

	m_xcTab.GetWindowRect(&rect_);
	m_xcTab.AdjustRect(FALSE, &rect_);
	m_xcTab.ScreenToClient(&rect_);

	m_xcTab.SetCurSel(select_);
	OnTcnSelchangeTab(0, 0);
	m_xcTab.ModifyStyle(0, WS_CLIPCHILDREN);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDetailBBSBarDlg::PreTranslateMessage(MSG* pMsg)
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


void CDetailBBSBarDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
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
