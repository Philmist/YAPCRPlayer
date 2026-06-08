// DetailBBSBarPostDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailBBSBarPostDlg.h"
#include "afxdialogex.h"


// CDetailBBSBarPostDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailBBSBarPostDlg, CBasePropDlg)

CDetailBBSBarPostDlg::CDetailBBSBarPostDlg(utl::PropSetting& setting, sl::BoardConfig& board)
	: CBasePropDlg(setting)
	, cfg_(board)
{

}

CDetailBBSBarPostDlg::~CDetailBBSBarPostDlg()
{
}

void CDetailBBSBarPostDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailBBSBarPostDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailBBSBarPostDlg メッセージ ハンドラー

BOOL CDetailBBSBarPostDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"操作");
	prop_.add(group);
	{
		auto combo = prop_.combo(L"書き込みショートカットキー", cfg_.shortcut);
		combo->AddComboValue(L"Shift + Enter", sl::BoardConfig::EDIT_SHORTCUT_SHIFT_ENTER);
		combo->AddComboValue(L"Ctrl + Enter", sl::BoardConfig::EDIT_SHORTCUT_CTRL_ENTER);
		combo->AddComboValue(L"Alt + Enter", sl::BoardConfig::EDIT_SHORTCUT_ALT_ENTER);
		group->AddSubItem(combo);
	}

	group = prop_.group(L"書き込み成功時");
	prop_.add(group);
	{
		auto check = prop_.check(L"スレッドを更新する", cfg_.write);
		group->AddSubItem(check);

		check = prop_.check(L"書き込み欄を非表示にする", cfg_.hide);
		group->AddSubItem(check);

		check = prop_.check(L"書き込みログを作成する", cfg_.log,
			L"書き込んだ内容を保存するログファイルを作成します\r\nログファイルがすでにある場合はログの最後に追記します");
		group->AddSubItem(check);

		auto file = prop_.file(
			L"書き込みログファイルパス", cfg_.path,
			L"", OFN_HIDEREADONLY, L"すべてのファイル (*.*)|*.*||");
		group->AddSubItem(file);

		check->AddLink(file);
	}

	group = prop_.group(L"フォーカス(内蔵BBSブラウザにフォーカスがある場合は無効)");
	prop_.add(group);
	{
		auto check = prop_.check(L"書き込み欄表示中は書き込み欄にフォーカスを固定する", cfg_.focus);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailBBSBarPostDlg::OnGetProp()
{
	prop_.get();
}

void CDetailBBSBarPostDlg::OnSetProp()
{
	auto tmp = sl::BoardConfig();
	cfg_.shortcut = tmp.shortcut;
	cfg_.write = tmp.write;
	cfg_.hide = tmp.hide;
	cfg_.log = tmp.log;
	cfg_.path = tmp.path;
	cfg_.focus = tmp.focus;

	prop_.set();
}
