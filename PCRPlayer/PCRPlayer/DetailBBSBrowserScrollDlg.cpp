// DetailBBSBrowserScrollDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBrowserScrollDlg.h"
#include "afxdialogex.h"


// CDetailBBSBrowserScrollDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBrowserScrollDlg, CBasePropDlg)

CDetailBBSBrowserScrollDlg::CDetailBBSBrowserScrollDlg(utl::PropSetting& setting)
	: CBasePropDlg(setting)
{

}

CDetailBBSBrowserScrollDlg::~CDetailBBSBrowserScrollDlg()
{
}

void CDetailBBSBrowserScrollDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSBrowserScrollDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSBrowserScrollDlg メッセージ ハンドラー

BOOL CDetailBBSBrowserScrollDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"オートスクロール");
	prop_.add(group);
	{
		auto check = prop_.check(L"新規読み込み時に開始", cfg_.init,
			L"スレッドを新しく読み込んだときにオートスクロールを開始します");
		group->AddSubItem(check);

		check = prop_.check(L"新着レス取得時に開始", cfg_.latest,
			L"新着レスを取得したときにオートスクロールを開始します");
		group->AddSubItem(check);

		check = prop_.check(L"手動更新時に開始", cfg_.refresh,
			L"手動更新したときにオートスクロールを開始します");
		group->AddSubItem(check);

		check = prop_.check(L"手動スクロールで停止", cfg_.manual,
			L"手動でスクロールしたときオートスクロールを停止します");
		group->AddSubItem(check);

		check = prop_.check(L"末尾で停止", cfg_.last,
			L"スクロールが末尾に到達したときオートスクロールを停止します");
		group->AddSubItem(check);

		auto number = prop_.number(L"スクロール量", cfg_.delta,
			L"一度にスクロールする量");
		number->EnableSpinControl(TRUE, 1, INT_MAX);
		group->AddSubItem(number);

		number = prop_.number(L"スクロール間隔(ミリ秒)", cfg_.interval,
			L"スクロールする時間間隔");
		group->AddSubItem(number);
	}

	group = prop_.group(L"スクロール位置");
	prop_.add(group);
	{
		auto check = prop_.check(L"新規読み込み時に末尾へジャンプ", cfg_.position.init,
			L"スレッドを新しく読み込んだときに末尾へジャンプします");
		group->AddSubItem(check);

		check = prop_.check(L"新着レス取得時に末尾を維持", cfg_.position.latest,
			L"新着レスを取得したときに直前のスクロール位置が末尾だった場合は末尾へジャンプします");
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSBrowserScrollDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSBrowserScrollDlg::OnSetProp()
{
	cfg_ = sl::ScrollConfig();

	prop_.set();
}
