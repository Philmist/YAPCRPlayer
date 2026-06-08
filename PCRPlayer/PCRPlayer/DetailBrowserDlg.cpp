// DetailBrowserDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBrowserDlg.h"
#include "afxdialogex.h"


// CDetailBrowserDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBrowserDlg, CBasePropDlg)

CDetailBrowserDlg::CDetailBrowserDlg(utl::PropSetting& setting, sl::BrowserConfig& browser)
	: CBasePropDlg(setting)
	, cfg_(browser)
{

}

CDetailBrowserDlg::~CDetailBrowserDlg()
{
}

void CDetailBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBrowserDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBrowserDlg メッセージ ハンドラー

BOOL CDetailBrowserDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"URLを開くWebブラウザ");
	prop_.add(group);
	{
		auto file = prop_.file(
			L"Webブラウザパス", cfg_.web,
			L"", OFN_HIDEREADONLY, L"Webブラウザ (*.exe)|*.exe|すべてのファイル (*.*)|*.*||");
		group->AddSubItem(file);

		auto check = prop_.check(
			L"既定のWebブラウザを使う", cfg_.def);
		group->AddSubItem(check);

		check->AddLink(file, TRUE);
	}

	group = prop_.group(L"BBSブラウザ");
	prop_.add(group);
	{
		auto file = prop_.file(
			L"BBSブラウザパス", cfg_.bbs,
			L"", OFN_HIDEREADONLY, L"BBSブラウザ (*.exe)|*.exe|すべてのファイル (*.*)|*.*||");
		group->AddSubItem(file);

		auto check = prop_.check(
			L"終了時にBBSブラウザを終了する", cfg_.close);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBrowserDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBrowserDlg::OnSetProp()
{
	cfg_ = sl::BrowserConfig();

	prop_.set();
}

