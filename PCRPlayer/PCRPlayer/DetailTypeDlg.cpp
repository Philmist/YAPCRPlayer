// DetailTypeDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailTypeDlg.h"

#include "afxdialogex.h"


// CDetailTypeDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailTypeDlg, CDialog)

//CDetailTypeDlg::CDetailTypeDlg(CWnd* pParent /*=NULL*/)
CDetailTypeDlg::CDetailTypeDlg(sl::OptionConfing::Detail::Type& option, sl::TypeConfig& type)
	: CDialog(CDetailTypeDlg::IDD, NULL)
	, select_(0)
	, dlg_(option, type)
{

}

CDetailTypeDlg::~CDetailTypeDlg()
{
}

void CDetailTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_xcTab);
}


BEGIN_MESSAGE_MAP(CDetailTypeDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDetailTypeDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CDetailTypeDlg メッセージ ハンドラー


BOOL CDetailTypeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	auto bind = [this](const std::wstring& text, CDialog* dlg, int idd, int index){
		m_xcTab.InsertItem(index, text.c_str());
		Item item = { dlg, idd, index };
		vec_.push_back(item);
	};

	bind(L"WMV", &dlg_.wmv, IDD_BASE_PROP, 0);
	bind(L"FLV", &dlg_.flv, IDD_BASE_PROP, 1);

	m_xcTab.GetWindowRect(&rect_);
	m_xcTab.AdjustRect(FALSE, &rect_);
	m_xcTab.ScreenToClient(&rect_);

	m_xcTab.SetCurSel(select_);
	OnTcnSelchangeTab(0, 0);
	m_xcTab.ModifyStyle(0, WS_CLIPCHILDREN);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDetailTypeDlg::PreTranslateMessage(MSG* pMsg)
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


void CDetailTypeDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
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
