// DisplayInformationGeneralDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayInformationGeneralDlg.h"
#include "afxdialogex.h"


// CDisplayInformationGeneralDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayInformationGeneralDlg, CBasePropDlg)

CDisplayInformationGeneralDlg::CDisplayInformationGeneralDlg(utl::PropSetting& setting, sl::InformationConfig& information)
	: CBasePropDlg(setting)
	, cfg_(information)
{

}

CDisplayInformationGeneralDlg::~CDisplayInformationGeneralDlg()
{
}

void CDisplayInformationGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDisplayInformationGeneralDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDisplayInformationGeneralDlg メッセージ ハンドラー

BOOL CDisplayInformationGeneralDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto bindInfo = [&](CMFCPropertyGridProperty* group, sl::InformationConfig::Channel& value){
		auto check = prop_.check(L"チャンネル名", value.name);
		group->AddSubItem(check);
		check = prop_.check(L"タイプ", value.type);
		group->AddSubItem(check);
		check = prop_.check(L"ジャンル", value.genre);
		group->AddSubItem(check);
		check = prop_.check(L"詳細", value.desc);
		group->AddSubItem(check);
		check = prop_.check(L"コメント", value.comment);
		group->AddSubItem(check);
		check = prop_.check(L"アーティスト", value.artist);
		group->AddSubItem(check);
		check = prop_.check(L"タイトル", value.title);
		group->AddSubItem(check);
		check = prop_.check(L"アルバム", value.album);
		group->AddSubItem(check);
	};

	auto group = prop_.group(L"チャンネル情報置換(ステータスバー)");
	prop_.add(group);
	{
		bindInfo(group, cfg_.status);
	}

	group = prop_.group(L"チャンネル情報置換(ポップアップ)");
	prop_.add(group);
	{
		bindInfo(group, cfg_.panel);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDisplayInformationGeneralDlg::OnGetProp()
{
	prop_.get();
}

void CDisplayInformationGeneralDlg::OnSetProp()
{
	auto tmp = network::InformationControl();
	cfg_.status = tmp.status;
	cfg_.panel = tmp.panel;
	prop_.set();
}

