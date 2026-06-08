// DetailBBSCommonGeneralDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSCommonGeneralDlg.h"
#include "afxdialogex.h"


// CDetailBBSCommonFontDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSCommonFontDlg, CBasePropDlg)

CDetailBBSCommonFontDlg::CDetailBBSCommonFontDlg(utl::PropSetting& setting, sl::FontConfig& font)
	: CBasePropDlg(setting)
	, cfg_(font)
{

}

CDetailBBSCommonFontDlg::~CDetailBBSCommonFontDlg()
{
}

void CDetailBBSCommonFontDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSCommonFontDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSCommonFontDlg メッセージ ハンドラー

BOOL CDetailBBSCommonFontDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"フォント");
	prop_.add(group);
	{
		std::wstring desc = L"スレッド/ポップアップのフォントはスキン設定から変更できます";

		cfg_.thread.get(font_.thread);
		auto font = prop_.font(L"スレッド選択ウィンドウ", font_.thread, desc);
		group->AddSubItem(font);

		cfg_.search.get(font_.search);
		font = prop_.font(L"検索ウィンドウ", font_.search, desc);
		group->AddSubItem(font);

		cfg_.extract.get(font_.extract);
		font = prop_.font(L"抽出ウィンドウ", font_.extract, desc);
		group->AddSubItem(font);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSCommonFontDlg::OnGetProp()
{
	prop_.get();

	cfg_.thread.set(font_.thread);
	cfg_.search.set(font_.search);
	cfg_.extract.set(font_.extract);
}

void CDetailBBSCommonFontDlg::OnSetProp()
{
	cfg_ = sl::FontConfig();
	cfg_.thread.get(font_.thread);
	cfg_.search.get(font_.search);
	cfg_.extract.get(font_.extract);
	prop_.set();
}
