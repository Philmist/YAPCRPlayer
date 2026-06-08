// DetailBBSBarThreadDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBarThreadDlg.h"
#include "afxdialogex.h"


// CDetailBBSBarThreadDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBarThreadDlg, CBasePropDlg)

CDetailBBSBarThreadDlg::CDetailBBSBarThreadDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSBarThreadDlg::~CDetailBBSBarThreadDlg()
{
}

void CDetailBBSBarThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSBarThreadDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSBarThreadDlg メッセージ ハンドラー

BOOL CDetailBBSBarThreadDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"更新(内蔵BBSブラウザ表示中は無効)");
	prop_.add(group);
	{
		auto check = prop_.check(L"ポップアップを表示しているときだけ更新する", cfg_.block);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSBarThreadDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSBarThreadDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.block = tmp.block;

	prop_.set();
}
