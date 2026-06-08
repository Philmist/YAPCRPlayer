// DetailTypeWMVDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailTypeWMVDlg.h"
#include "afxdialogex.h"


// CDetailTypeWMVDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailTypeWMVDlg, CBasePropDlg)

CDetailTypeWMVDlg::CDetailTypeWMVDlg(utl::PropSetting& setting, sl::TypeConfig::WMV& type)
	: CBasePropDlg(setting)
	, cfg_(type)
{

}

CDetailTypeWMVDlg::~CDetailTypeWMVDlg()
{
}

void CDetailTypeWMVDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailTypeWMVDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailTypeWMVDlg メッセージ ハンドラー

BOOL CDetailTypeWMVDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"自動再構築(オンライン)");
	prop_.add(group);
	{
		m_.packet.time = (int)cfg_.packet.time;
		auto number = prop_.number(cfg_.packet.enable, L"指定時間パケットを受け取らなかったら再構築(ミリ秒)", m_.packet.time);
		number->EnableSpinControl(TRUE, 1000, 600000);
		group->AddSubItem(number);

		m_.frame.time = (int)cfg_.frame.time;
		number = prop_.number(cfg_.frame.enable, L"指定時間フレームが欠落し続けたら再構築(ミリ秒)", m_.frame.time);
		number->EnableSpinControl(TRUE, 1000, 600000);
		group->AddSubItem(number);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailTypeWMVDlg::OnGetProp()
{
	prop_.get();

	cfg_.packet.time = (DWORD)m_.packet.time;
	cfg_.frame.time = (DWORD)m_.frame.time;
}

void CDetailTypeWMVDlg::OnSetProp()
{
	cfg_ = sl::TypeConfig::WMV();

	m_.packet.time = (int)cfg_.packet.time;
	m_.frame.time = (int)cfg_.frame.time;
	prop_.set();
}
