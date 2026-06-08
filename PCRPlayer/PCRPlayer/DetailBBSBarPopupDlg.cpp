// DetailBBSBarPopupDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBarPopupDlg.h"
#include "afxdialogex.h"


// CDetailBBSBarPopupDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBarPopupDlg, CBasePropDlg)

CDetailBBSBarPopupDlg::CDetailBBSBarPopupDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSBarPopupDlg::~CDetailBBSBarPopupDlg()
{
}

void CDetailBBSBarPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSBarPopupDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSBarPopupDlg メッセージ ハンドラー

BOOL CDetailBBSBarPopupDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"全般");
	prop_.add(group);
	{
		auto number = prop_.number(L"表示レス数", cfg_.res);
		number->EnableSpinControl(TRUE, 1, 100);
		group->AddSubItem(number);

		number = prop_.number(L"ポップアップするまでの時間", cfg_.delay);
		number->EnableSpinControl(TRUE, 0, 1000);
		group->AddSubItem(number);
	}

	group = prop_.group(L"表示位置");
	prop_.add(group);
	{
		auto combo = prop_.combo(L"ポップアップ位置", cfg_.position);
		combo->AddComboValue(L"カーソル中央に表示", sl::BoardConfig::POPUP_POSITION_CENTER);
		combo->AddComboValue(L"カーソル左に表示", sl::BoardConfig::POPUP_POSITION_LEFT);
		combo->AddComboValue(L"カーソル右に表示", sl::BoardConfig::POPUP_POSITION_RIGHT);
		combo->AddComboValue(L"ウィンドウ中央でカーソル左右を切り替え", sl::BoardConfig::POPUP_POSITION_BOTH);
		group->AddSubItem(combo);

		auto check = prop_.check(L"バーの下に配置する", cfg_.under);
		group->AddSubItem(check);

		check = prop_.check(L"モニタースクリーンからはみ出さないように表示する", cfg_.align,
			L"ポップアップがモニタースクリーンからはみ出さないように表示します\r\nポップアップがモニタースクリーンより大きい場合ははみ出ます");
		group->AddSubItem(check);
	}

	group = prop_.group(L"反転");
	prop_.add(group);
	{
		auto check = prop_.check(L"レス順序反転", cfg_.order);
		group->AddSubItem(check);

		check = prop_.check(L"スレッドスクロール反転", cfg_.scroll);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSBarPopupDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSBarPopupDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.res = tmp.res;
	cfg_.delay = tmp.delay;
	cfg_.position = tmp.position;
	cfg_.under = tmp.under;
	cfg_.align = tmp.align;
	cfg_.order = tmp.order;
	cfg_.scroll = tmp.scroll;

	prop_.set();
}
