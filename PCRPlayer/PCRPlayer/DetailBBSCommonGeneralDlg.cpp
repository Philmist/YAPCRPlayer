// DetailBBSCommonGeneralDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSCommonGeneralDlg.h"
#include "afxdialogex.h"


// CDetailBBSCommonGeneralDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSCommonGeneralDlg, CBasePropDlg)

CDetailBBSCommonGeneralDlg::CDetailBBSCommonGeneralDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSCommonGeneralDlg::~CDetailBBSCommonGeneralDlg()
{
}

void CDetailBBSCommonGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSCommonGeneralDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSCommonGeneralDlg メッセージ ハンドラー

BOOL CDetailBBSCommonGeneralDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"全般");
	prop_.add(group);
	{
		auto string = prop_.string(L"したらば掲示板形式で読み込むホスト(正規表現)", cfg_.shitaraba);
		group->AddSubItem(string);
	}

	group = prop_.group(L"コンタクトURL(PeerCast)");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"コンタクトURLを読み込む", cfg_.contact,
			L"最初のチャンネル情報読み込み時にコンタクトURLを読み込みます");
		group->AddSubItem(check);

		check = prop_.check(
			L"コンタクトURLにURL変更を反映する", cfg_.change,
			L"スレッドの変更や自動移動で別のURLが読み込まれた時にメインメニューのコンタクトURLに変更を反映します\r\n"
			L"スレッド選択ダイアログのメニューから変更前のコンタクトURLを操作できます");
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSCommonGeneralDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSCommonGeneralDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.shitaraba = tmp.shitaraba;
	cfg_.contact = tmp.contact;
	cfg_.change = tmp.change;

	prop_.set();
}
