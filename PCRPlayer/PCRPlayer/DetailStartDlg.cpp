// DetailStartDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailStartDlg.h"
#include "afxdialogex.h"


// CDetailStartDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailStartDlg, CBasePropDlg)

CDetailStartDlg::CDetailStartDlg(utl::PropSetting& setting, sl::StartConfig& start)
	: CBasePropDlg(setting)
	, cfg_(start)
{

}

CDetailStartDlg::~CDetailStartDlg()
{
}

void CDetailStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailStartDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailStartDlg メッセージ ハンドラー

BOOL CDetailStartDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"再生");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"最後に再生したファイル/URLを再生する", cfg_.recent);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailStartDlg::OnGetProp()
{
	prop_.get();
}

void CDetailStartDlg::OnSetProp()
{
	cfg_ = sl::StartConfig();

	prop_.set();
}

