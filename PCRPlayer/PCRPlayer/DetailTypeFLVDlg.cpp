// DetailTypeFLVDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailTypeFLVDlg.h"
#include "afxdialogex.h"


// CDetailTypeFLVDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailTypeFLVDlg, CBasePropDlg)

CDetailTypeFLVDlg::CDetailTypeFLVDlg(utl::PropSetting& setting, sl::TypeConfig::FLV& type)
	: CBasePropDlg(setting)
	, cfg_(type)
{

}

CDetailTypeFLVDlg::~CDetailTypeFLVDlg()
{
}

void CDetailTypeFLVDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDetailTypeFLVDlg, CBasePropDlg)
END_MESSAGE_MAP()


// CDetailTypeFLVDlg メッセージ ハンドラー

BOOL CDetailTypeFLVDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto group = prop_.group(L"チャンネル再接続(PeerCast) 要プレイヤー再構築");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"ループを検出したらチャンネル再接続(自動)", cfg_.loop);
		group->AddSubItem(check);
	}

	group = prop_.group(L"キーフレーム(オンライン) 要プレイヤー再構築");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"キーフレームから再生する", cfg_.keyframe,
			L"バッファ最適化/遅延補正を実行したときにもキーフレームから再生します");
		group->AddSubItem(check);
	}

	group = prop_.group(L"再生時間表示(オンライン)");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"再生時間をタイムスタンプで表示する", cfg_.timestamp);
		group->AddSubItem(check);
	}

	group = prop_.group(L"バッファ(オンライン) 要プレイヤー再構築");
	prop_.add(group);
	{
		auto number = prop_.number(L"初期バッファ(ミリ秒)", cfg_.buffer,
			L"短くすると遅延が少なくなります\r\n"
			L"長くするとバッファ不足による再生の途切れを起こしにくくなります\r\n"
			L"再生が途切れたときは自動的にバッファを増やして停止を防ぎます");
		number->EnableSpinControl(TRUE, 0, 600000);
		group->AddSubItem(number);
	}

	group = prop_.group(L"バッファ最適化(オンライン) 要プレイヤー再構築");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"バッファを最適化する", cfg_.optimize,
			L"再生開始時に余分なバッファを消去して遅延を初期バッファ以下にします");
		group->AddSubItem(check);
	}

	group = prop_.group(L"遅延補正(オンライン) 要プレイヤー再構築");
	prop_.add(group);
	{
		std::wstring desc =
			L"定期的にバッファ最適化を実行して遅延を初期バッファ以下にします\r\n"
			L"バッファが上限値を超えた時に実行します\r\n"
			L"バッファ上限は初期バッファより長く設定する必要があります\r\n"
			L"補正間隔は再生開始からはじまりバッファ不足で待機時間がリセットされます";

		auto check = prop_.check(
			L"遅延補正を実行する", cfg_.delay, desc);
		group->AddSubItem(check);

		auto number = prop_.number(L"バッファ上限(ミリ秒)", cfg_.limit, desc);
		number->EnableSpinControl(TRUE, 0, 600000);
		group->AddSubItem(number);

		check->AddLink(number, FALSE);

		number = prop_.number(L"補正間隔(ミリ秒)", cfg_.interval, desc);
		number->EnableSpinControl(TRUE, 0, 600000);
		group->AddSubItem(number);

		check->AddLink(number, FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDetailTypeFLVDlg::OnGetProp()
{
	prop_.get();
}

void CDetailTypeFLVDlg::OnSetProp()
{
	cfg_ = sl::TypeConfig::FLV();

	prop_.set();
}


