// DetailBBSBrowserThreadDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBrowserThreadDlg.h"
#include "afxdialogex.h"


// CDetailBBSBrowserThreadDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBrowserThreadDlg, CBasePropDlg)

CDetailBBSBrowserThreadDlg::CDetailBBSBrowserThreadDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSBrowserThreadDlg::~CDetailBBSBrowserThreadDlg()
{
}

void CDetailBBSBrowserThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSBrowserThreadDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSBrowserThreadDlg メッセージ ハンドラー

BOOL CDetailBBSBrowserThreadDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"レス");
	prop_.add(group);
	{
		auto number = prop_.number(
			L"新規表示レス数(0で無制限)", cfg_.def,
			L"スレッドを新しく読み込んだときに表示するレス数\r\nレス数が多いとき読み込みに時間がかかる場合少なく設定してください");
		group->AddSubItem(number);

		number = prop_.number(
			L"分割読み込みレス数(0で無制限)", cfg_.partial,
			L"スレッドをこのレス数ごとに分割して読み込みます\r\nレス数が多いときすべて一度に読み込むと読み込み終わるまで操作を受け付けなくなります");
		group->AddSubItem(number);
	}

	group = prop_.group(L"アンカー");
	prop_.add(group);
	{
		auto number = prop_.number(
			L"有効な範囲", cfg_.range,
			L"この値より広い範囲を持つアンカーを参照数の集計から除外します\r\n範囲が広すぎるアンカー(>>1-1000など)に対処する設定です");
		group->AddSubItem(number);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSBrowserThreadDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSBrowserThreadDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.def = tmp.def;
	cfg_.partial = tmp.partial;
	cfg_.range = tmp.range;

	prop_.set();
}
