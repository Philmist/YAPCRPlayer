// DetailNetworkPeercastDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailNetworkPeercastDlg.h"
#include "afxdialogex.h"


// CDetailNetworkPeercastDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailNetworkPeercastDlg, CBasePropDlg)

CDetailNetworkPeercastDlg::CDetailNetworkPeercastDlg(utl::PropSetting& setting, sl::NetworkConfig& network)
	: CBasePropDlg(setting)
	, cfg_(network)
{

}

CDetailNetworkPeercastDlg::~CDetailNetworkPeercastDlg()
{
}

void CDetailNetworkPeercastDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailNetworkPeercastDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailNetworkPeercastDlg メッセージ ハンドラー

BOOL CDetailNetworkPeercastDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"プレイヤー終了時");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"プレイヤー終了時にチャンネル切断(自動)", cfg_.stop);
		group->AddSubItem(check);
	}

	group = prop_.group(L"チャンネル接続");
	prop_.add(group);
	{
		auto number = prop_.number(L"リトライ数", cfg_.connect.count);
		number->EnableSpinControl(TRUE, 0, 100);
		group->AddSubItem(number);

		number = prop_.number(L"リトライ間隔(秒)", cfg_.connect.wait);
		number->EnableSpinControl(TRUE, 0, 60);
		group->AddSubItem(number);

		number = prop_.number(L"タイムアウト(ミリ秒)", cfg_.connect.timeout);
		number->EnableSpinControl(TRUE, 0, 60000);
		group->AddSubItem(number);

		auto check = prop_.check(
			L"リトライ終了時にチャンネル切断(自動)", cfg_.connect.stop);
		group->AddSubItem(check);
	}

	group = prop_.group(L"チャンネル再接続");
	prop_.add(group);
	{
		auto number = prop_.number(
			L"チャンネル再接続(自動)の連続実行を制限する時間(ミリ秒)", cfg_.reconnect.interval);
		number->EnableSpinControl(TRUE, 10000, 600000);
		group->AddSubItem(number);
	}

	group = prop_.group(L"リレー");
	prop_.add(group);
	{
		std::wstring desc = L"(手動)はメニューやショートカットキーを自分で操作することを指します\r\n"
			"また、ネットワークやタイプなどで(自動)と表記されている設定を有効にしない限り自動的にチャンネル再接続/切断することはありません";

		auto check = prop_.check(
			L"リレーがあるときチャンネル再接続(手動)を無効にする", cfg_.relay.manual.reconnect, desc);
		group->AddSubItem(check);

		check = prop_.check(
			L"リレーがあるときチャンネル切断(手動)を無効にする", cfg_.relay.manual.stop, desc);
		group->AddSubItem(check);

		check = prop_.check(
			L"リレーがあるときチャンネル再接続(自動)を無効にする", cfg_.relay.automatic.reconnect, desc);
		group->AddSubItem(check);

		check = prop_.check(
			L"リレーがあるときチャンネル切断(自動)を無効にする", cfg_.relay.automatic.stop, desc);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailNetworkPeercastDlg::OnGetProp()
{
	prop_.get();
}

void CDetailNetworkPeercastDlg::OnSetProp()
{
	auto tmp = sl::NetworkConfig();
	cfg_.stop = tmp.stop;
	cfg_.connect = tmp.connect;
	cfg_.reconnect = tmp.reconnect;
	cfg_.relay = tmp.relay;

	prop_.set();
}
