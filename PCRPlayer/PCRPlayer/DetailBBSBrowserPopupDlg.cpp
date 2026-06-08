// DetailBBSBrowserPopupDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBrowserPopupDlg.h"
#include "afxdialogex.h"


// CDetailBBSBrowserPopupDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBrowserPopupDlg, CBasePropDlg)

CDetailBBSBrowserPopupDlg::CDetailBBSBrowserPopupDlg(utl::PropSetting& setting, PopupBrowserConfig& popup)
	: CBasePropDlg(setting)
	, cfg_(popup)
{

}

CDetailBBSBrowserPopupDlg::~CDetailBBSBrowserPopupDlg()
{
}

void CDetailBBSBrowserPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSBrowserPopupDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSBrowserPopupDlg メッセージ ハンドラー

BOOL CDetailBBSBrowserPopupDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	std::wstring enableName = L"ポップアップ表示";
	std::wstring limitName = L"表示レス数(0で無制限)";
	std::wstring limitDesc = L"ポップアップで表示するレス数の上限";
	std::wstring headName = L"新しいレスを優先表示する";
	std::wstring headDesc = L"上限を超えたとき古いレスから削除して新しいレスを残します";


	auto group = prop_.group(L"全般");
	prop_.add(group);
	{
		auto number = prop_.number(
			L"ポップアップするまでの時間(ミリ秒)", cfg_.delay, L"");
		number->EnableSpinControl(TRUE, 0, 60000);
		group->AddSubItem(number);
	}


	group = prop_.group(L"ボーダーライン");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"3Dエッジ", cfg_.edge);
		group->AddSubItem(check);

		auto color = prop_.color(
			L"ボーダーカラー", cfg_.border);
		group->AddSubItem(color);

		check->AddLink(color, TRUE);
	}

	group = prop_.group(L"レス番号");
	prop_.add(group);
	{
		auto check = prop_.check(
			enableName, cfg_.ref.enable,
			L"レス番号のリンクで参照元のレスをポップアップ表示します");
		group->AddSubItem(check);

		auto number = prop_.number(
			limitName, cfg_.ref.limit,
			limitDesc);
		group->AddSubItem(number);

		check = prop_.check(
			headName, cfg_.ref.head,
			headDesc);
		group->AddSubItem(check);
	}

	group = prop_.group(L"アンカー");
	prop_.add(group);
	{
		auto check = prop_.check(
			enableName, cfg_.anchor.enable,
			L"アンカーのリンクで参照先のレスをポップアップ表示します");
		group->AddSubItem(check);

		auto number = prop_.number(
			limitName, cfg_.anchor.limit,
			limitDesc);
		group->AddSubItem(number);

		check = prop_.check(
			headName, cfg_.anchor.head,
			headDesc);
		group->AddSubItem(check);
	}

	group = prop_.group(L"ID");
	prop_.add(group);
	{
		auto check = prop_.check(
			enableName, cfg_.id.enable,
			L"IDのリンクで同じIDのレスをポップアップ表示します");
		group->AddSubItem(check);

		auto number = prop_.number(
			limitName, cfg_.id.limit,
			limitDesc);
		group->AddSubItem(number);

		check = prop_.check(
			headName, cfg_.id.head,
			headDesc);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSBrowserPopupDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSBrowserPopupDlg::OnSetProp()
{
	cfg_ = PopupBrowserConfig();

	prop_.set();
}
