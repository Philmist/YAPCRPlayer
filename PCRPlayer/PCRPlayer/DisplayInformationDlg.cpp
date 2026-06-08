// DisplayInformationDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayInformationDlg.h"

#include "afxdialogex.h"


// CDisplayInformationDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayInformationDlg, CDialog)

//CDisplayInformationDlg::CDisplayInformationDlg(CWnd* pParent /*=NULL*/)
CDisplayInformationDlg::CDisplayInformationDlg(sl::OptionConfing::Display::Information& option, sl::InformationConfig& information)
	: CDialog(CDisplayInformationDlg::IDD, NULL)
	, select_(0)
	, dlg_(option, information)
{

}

CDisplayInformationDlg::~CDisplayInformationDlg()
{
}

void CDisplayInformationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_xcTab);
}


BEGIN_MESSAGE_MAP(CDisplayInformationDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDisplayInformationDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CDisplayInformationDlg メッセージ ハンドラー


BOOL CDisplayInformationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	auto bind = [this](const std::wstring& text, CDialog* dlg, int idd, int index){
		m_xcTab.InsertItem(index, text.c_str());
		Item item = { dlg, idd, index };
		vec_.push_back(item);
	};

	bind(L"全般", &dlg_.general, IDD_BASE_PROP, 0);
	bind(L"チャンネル名", &dlg_.name, CReplaceDlg::IDD, 1);
	bind(L"タイプ", &dlg_.type, CReplaceDlg::IDD, 2);
	bind(L"ジャンル", &dlg_.genre, CReplaceDlg::IDD, 3);
	bind(L"詳細", &dlg_.desc, CReplaceDlg::IDD, 4);
	bind(L"コメント", &dlg_.comment, CReplaceDlg::IDD, 5);
	bind(L"アーティスト", &dlg_.artist, CReplaceDlg::IDD, 6);
	bind(L"タイトル", &dlg_.title, CReplaceDlg::IDD, 7);
	bind(L"アルバム", &dlg_.album, CReplaceDlg::IDD, 8);


	m_xcTab.GetWindowRect(&rect_);
	m_xcTab.AdjustRect(FALSE, &rect_);
	m_xcTab.ScreenToClient(&rect_);

	m_xcTab.SetCurSel(select_);
	OnTcnSelchangeTab(0, 0);
	m_xcTab.ModifyStyle(0, WS_CLIPCHILDREN);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayInformationDlg::PreTranslateMessage(MSG* pMsg)
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


void CDisplayInformationDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
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
