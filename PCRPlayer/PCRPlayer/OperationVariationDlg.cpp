// OperationVariationDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "OperationVariationDlg.h"
#include "afxdialogex.h"


// COperationVariationDlg ダイアログ

IMPLEMENT_DYNAMIC(COperationVariationDlg, CBasePropDlg)

COperationVariationDlg::COperationVariationDlg(utl::PropSetting& setting, sl::Variation& variation)
	: CBasePropDlg(setting)
	, cfg_(variation)
{

}

COperationVariationDlg::~COperationVariationDlg()
{
}

void COperationVariationDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COperationVariationDlg, CBasePropDlg)
END_MESSAGE_MAP()


// COperationVariationDlg メッセージ ハンドラー

BOOL COperationVariationDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto bindValue = [this](CMFCPropertyGridProperty* group, sl::Variation::Value& value, int min, int max){
		auto number = prop_.number(L"通常", value.normal);
		number->EnableSpinControl(TRUE, min, max);
		group->AddSubItem(number);

		number = prop_.number(L"小(+Shift)", value.low);
		number->EnableSpinControl(TRUE, min, max);
		group->AddSubItem(number);

		number = prop_.number(L"大(+Ctrl)", value.high);
		number->EnableSpinControl(TRUE, min, max);
		group->AddSubItem(number);
	};

	auto group = prop_.group(L"音量変化量(%)");
	prop_.add(group);
	{
		bindValue(group, cfg_.volume, 0, 100);
	}

	group = prop_.group(L"シーク移動量(ミリ秒)");
	prop_.add(group);
	{
		bindValue(group, cfg_.seek, 0, 600000);
	}

	group = prop_.group(L"スレッドスクロール量(レス数)");
	prop_.add(group);
	{
		bindValue(group, cfg_.scroll, 0, 10000);
	}

	group = prop_.group(L"ウィンドウ拡大/縮小量(%)");
	prop_.add(group);
	{
		bindValue(group, cfg_.scale, 0, 100);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void COperationVariationDlg::OnGetProp()
{
	prop_.get();
}

void COperationVariationDlg::OnSetProp()
{
	cfg_ = sl::Variation();

	prop_.set();
}

