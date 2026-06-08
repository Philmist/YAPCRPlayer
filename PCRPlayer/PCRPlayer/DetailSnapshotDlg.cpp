// DetailSnapshotDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailSnapshotDlg.h"
#include "afxdialogex.h"


// CDetailSnapshotDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailSnapshotDlg, CBasePropDlg)

CDetailSnapshotDlg::CDetailSnapshotDlg(utl::PropSetting& setting, sl::SnapshotConfig& snapshot)
	: CBasePropDlg(setting)
	, cfg_(snapshot)
{

}

CDetailSnapshotDlg::~CDetailSnapshotDlg()
{
}

void CDetailSnapshotDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailSnapshotDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailSnapshotDlg メッセージ ハンドラー

BOOL CDetailSnapshotDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"保存設定");
	prop_.add(group);
	{
		auto folder = prop_.folder(
			L"保存フォルダパス", cfg_.folder,
			L"スナップショット保存フォルダ");
		group->AddSubItem(folder);

		auto combo = prop_.combo(L"形式", cfg_.format);
		combo->AddComboValue(L"BMP", utl::GRAPHIC_FORMAT_BMP);
		combo->AddComboValue(L"JPEG", utl::GRAPHIC_FORMAT_JPG);
		combo->AddComboValue(L"PNG", utl::GRAPHIC_FORMAT_PNG);
		group->AddSubItem(combo);

		auto number = prop_.number(L"JPEG品質(0-100)", cfg_.jpegQuality);
		number->EnableSpinControl(TRUE, 0, 100);
		group->AddSubItem(number);
	}

	group = prop_.group(L"解像度");
	prop_.add(group);
	{
		auto check = prop_.check(L"表示解像度で保存する", cfg_.display);
		group->AddSubItem(check);

		check = prop_.check(L"元の解像度より縮小しない", cfg_.fit);
		group->AddSubItem(check);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailSnapshotDlg::OnGetProp()
{
	prop_.get();
}

void CDetailSnapshotDlg::OnSetProp()
{
	cfg_ = sl::SnapshotConfig();

	prop_.set();
}

