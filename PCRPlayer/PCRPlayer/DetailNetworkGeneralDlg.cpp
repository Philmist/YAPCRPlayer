// DetailNetworkGeneralDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailNetworkGeneralDlg.h"
#include "afxdialogex.h"


// CDetailNetworkGeneralDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailNetworkGeneralDlg, CBasePropDlg)

CDetailNetworkGeneralDlg::CDetailNetworkGeneralDlg(utl::PropSetting& setting, sl::NetworkConfig& network)
	: CBasePropDlg(setting)
	, cfg_(network)
{

}

CDetailNetworkGeneralDlg::~CDetailNetworkGeneralDlg()
{
}

void CDetailNetworkGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailNetworkGeneralDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailNetworkGeneralDlg メッセージ ハンドラー

BOOL CDetailNetworkGeneralDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"自動再構築");
	prop_.add(group);
	{
		auto number = prop_.number(L"待機時間", cfg_.rebuild.wait);
		number->EnableSpinControl(TRUE, 0, 600);
		group->AddSubItem(number);
	}

	group = prop_.group(L"URL Buffer Reader");
	prop_.add(group);
	{
		auto string = prop_.string(L"UserAgent", cfg_.reader.userAgent);
		group->AddSubItem(string);

		auto number = prop_.number(L"タイムアウト(ミリ秒)", cfg_.reader.timeout);
		number->EnableSpinControl(TRUE, 0, 60000);
		group->AddSubItem(number);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailNetworkGeneralDlg::OnGetProp()
{
	prop_.get();
}

void CDetailNetworkGeneralDlg::OnSetProp()
{
	auto tmp = sl::NetworkConfig();
	cfg_.rebuild = tmp.rebuild;
	cfg_.reader = tmp.reader;

	prop_.set();
}
