// OperationWindowDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "OperationWindowDlg.h"
#include "afxdialogex.h"


// COperationWindowDlg ダイアログ

IMPLEMENT_DYNAMIC(COperationWindowDlg, CBasePropDlg)

COperationWindowDlg::COperationWindowDlg(utl::PropSetting& setting, sl::WindowOperation& window)
	: CBasePropDlg(setting)
	, cfg_(window)
{

}

COperationWindowDlg::~COperationWindowDlg()
{
}

void COperationWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COperationWindowDlg, CBasePropDlg)
END_MESSAGE_MAP()


// COperationWindowDlg メッセージ ハンドラー

BOOL COperationWindowDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"サイズ変更枠(幅)");
	prop_.add(group);
	{
		auto number = prop_.number(L"フレーム非表示時", cfg_.frame);
		number->EnableSpinControl(TRUE, 0, INT_MAX);
		group->AddSubItem(number);
	}

	group = prop_.group(L"スクリーンスナップ/ウィンドウスナップ(距離)");
	prop_.add(group);
	{
		auto number = prop_.number(L"スクリーン", cfg_.snap.screen);
		number->EnableSpinControl(TRUE, 0, INT_MAX);
		group->AddSubItem(number);

		number = prop_.number(L"ウィンドウ", cfg_.snap.window);
		number->EnableSpinControl(TRUE, 0, INT_MAX);
		group->AddSubItem(number);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void COperationWindowDlg::OnGetProp()
{
	prop_.get();
}

void COperationWindowDlg::OnSetProp()
{
	cfg_ = sl::WindowOperation();

	prop_.set();
}

