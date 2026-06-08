// DetailBBSCommonThreadDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSCommonThreadDlg.h"
#include "afxdialogex.h"


// CDetailBBSCommonThreadDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSCommonThreadDlg, CBasePropDlg)

CDetailBBSCommonThreadDlg::CDetailBBSCommonThreadDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSCommonThreadDlg::~CDetailBBSCommonThreadDlg()
{
}

void CDetailBBSCommonThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSCommonThreadDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSCommonThreadDlg メッセージ ハンドラー

BOOL CDetailBBSCommonThreadDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"自動更新");
	prop_.add(group);
	{
		auto check = prop_.check(L"新規読み込み時に開始", cfg_.reload,
			L"スレッドを新しく読み込んだときに自動更新を開始します");
		group->AddSubItem(check);

		auto number = prop_.number(L"更新間隔(秒)", cfg_.interval);
		number->EnableSpinControl(TRUE, 5, 999);
		group->AddSubItem(number);

		check = prop_.check(L"リトライ終了時に自動更新停止(PeerCast)", cfg_.retry,
			L"チャンネル接続のリトライが終了したとき自動更新を停止します");
		group->AddSubItem(check);
	}

	group = prop_.group(L"スレッド移動条件");
	prop_.add(group);
	{
		std::wstring desc = L"現在のスレッドより新しく、一番勢いの速いスレッドへ移動";
		auto check = prop_.check(L"スレッドURLを設定したときレス数上限に達していたら", cfg_.thread, desc);
		group->AddSubItem(check);

		check = prop_.check(L"掲示板URLを設定したとき", cfg_.board, desc);
		group->AddSubItem(check);

		check = prop_.check(L"スレッド更新時にレス数上限に達したとき", cfg_.move, desc);
		group->AddSubItem(check);
	}

	group = prop_.group(L"スレッド移動する読み込み動作");
	prop_.add(group);
	{
		auto check = prop_.check(L"コンタクトURL", cfg_.movable.contact);
		group->AddSubItem(check);

		check = prop_.check(L"コマンドライン引数", cfg_.movable.commandline);
		group->AddSubItem(check);

		check = prop_.check(L"アドレスバーから入力", cfg_.movable.manual);
		group->AddSubItem(check);

		check = prop_.check(L"お気に入り掲示板", cfg_.movable.favorite.board);
		group->AddSubItem(check);

		check = prop_.check(L"お気に入りスレッド", cfg_.movable.favorite.thread);
		group->AddSubItem(check);

		check = prop_.check(L"掲示板履歴", cfg_.movable.history.board);
		group->AddSubItem(check);

		check = prop_.check(L"スレッド履歴", cfg_.movable.history.thread);
		group->AddSubItem(check);
	}

	group = prop_.group(L"新着レス");
	prop_.add(group);
	{
		auto check = prop_.check(L"新着表示をする", cfg_.newly);
		group->AddSubItem(check);

		check = prop_.check(L"新規読み込み時のレスを新着レスにする", cfg_.latest,
			L"スレッドを新しく読み込んだときにすべてのレスを新着レスにします");
		group->AddSubItem(check);
	}

	group = prop_.group(L"履歴");
	prop_.add(group);
	{
		auto number = prop_.number(L"掲示板履歴保存数", cfg_.history.board);
		number->EnableSpinControl(TRUE, 0, HISTORY_MAX);
		group->AddSubItem(number);

		number = prop_.number(L"スレッド履歴保存数", cfg_.history.thread);
		number->EnableSpinControl(TRUE, 0, HISTORY_MAX);
		group->AddSubItem(number);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSCommonThreadDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSCommonThreadDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.reload = tmp.reload;
	cfg_.interval = tmp.interval;
	cfg_.retry = tmp.retry;
	cfg_.thread = tmp.thread;
	cfg_.board = tmp.board;
	cfg_.move = tmp.move;
	cfg_.movable = tmp.movable;
	cfg_.newly = tmp.newly;
	cfg_.latest = tmp.latest;
	cfg_.history = tmp.history;

	prop_.set();
}
