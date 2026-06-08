// DetailBBSCommonNetworkDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSCommonNetworkDlg.h"
#include "afxdialogex.h"


// CDetailBBSCommonNetworkDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSCommonNetworkDlg, CBasePropDlg)

CDetailBBSCommonNetworkDlg::CDetailBBSCommonNetworkDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSCommonNetworkDlg::~CDetailBBSCommonNetworkDlg()
{
}

void CDetailBBSCommonNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSCommonNetworkDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSCommonNetworkDlg メッセージ ハンドラー

BOOL CDetailBBSCommonNetworkDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"HTTP");
	prop_.add(group);
	{
		auto string = prop_.string(L"UserAgent", cfg_.userAgent);
		group->AddSubItem(string);

		auto number = prop_.number(L"タイムアウト(ミリ秒)", cfg_.timeout);
		number->EnableSpinControl(TRUE, 0, 60000);
		group->AddSubItem(number);
	}

	group = prop_.group(L"Proxy");
	prop_.add(group);
	{
		auto string = prop_.string(cfg_.proxy.get.enable, L"読み込み用Proxy", cfg_.proxy.get.name);
		group->AddSubItem(string);

		string = prop_.string(cfg_.proxy.post.enable, L"書き込み用Proxy", cfg_.proxy.post.name);
		group->AddSubItem(string);

		string = prop_.string(L"有効にするホスト(正規表現)", cfg_.proxy.white);
		group->AddSubItem(string);

		string = prop_.string(L"無効にするホスト(正規表現)", cfg_.proxy.black);
		group->AddSubItem(string);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSCommonNetworkDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSCommonNetworkDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.userAgent = tmp.userAgent;
	cfg_.timeout = tmp.timeout;
	cfg_.proxy = tmp.proxy;

	prop_.set();
}
