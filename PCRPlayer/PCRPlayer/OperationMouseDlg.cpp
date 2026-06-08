// OperationMouseDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "OperationMouseDlg.h"
#include "afxdialogex.h"


// COperationMouseDlg ダイアログ

IMPLEMENT_DYNAMIC(COperationMouseDlg, CBasePropDlg)

COperationMouseDlg::COperationMouseDlg(utl::PropSetting& setting, sl::MouseOperation& mouse)
	: CBasePropDlg(setting)
	, cfg_(mouse)
{

}

COperationMouseDlg::~COperationMouseDlg()
{
}

void COperationMouseDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COperationMouseDlg, CBasePropDlg)
END_MESSAGE_MAP()


// COperationMouseDlg メッセージ ハンドラー

BOOL COperationMouseDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto bindWheel = [this](const std::wstring& name, int& value)->CMFCPropertyGridProperty*{
		auto prop = prop_.combo(name, value);
		prop->AddComboValue(L"なし", sl::MOUSE_WHEEL::MOUSE_WHEEL_NONE);
		prop->AddComboValue(L"音量変更", sl::MOUSE_WHEEL::MOUSE_WHEEL_VOLUME);
		prop->AddComboValue(L"シーク", sl::MOUSE_WHEEL::MOUSE_WHEEL_SEEK);
		prop->AddComboValue(L"スレッドスクロール", sl::MOUSE_WHEEL::MOUSE_WHEEL_THREAD);
		prop->AddComboValue(L"ウィンドウ拡大/縮小", sl::MOUSE_WHEEL::MOUSE_WHEEL_ZOOM);
		return prop;
	};

	auto bindInvert = [this](const std::wstring& name, bool& value)->CMFCPropertyGridProperty*{
		auto prop = prop_.check(name, value);
		return prop;
	};
	
	auto bindMClick = [this](int& value)->CMFCPropertyGridProperty*{
		auto prop = prop_.combo(L"ミドルクリック", value);
		prop->AddComboValue(L"なし", sl::MOUSE_MCLICK::MOUSE_MCLICK_NONE);
		prop->AddComboValue(L"ミュート", sl::MOUSE_MCLICK::MOUSE_MCLICK_MUTE);
		prop->AddComboValue(L"最小化ミュート", sl::MOUSE_MCLICK::MOUSE_MCLICK_MINIMIZE_MUTE);
		prop->AddComboValue(L"一時停止", sl::MOUSE_MCLICK::MOUSE_MCLICK_PAUSE);
		prop->AddComboValue(L"最小化一時停止", sl::MOUSE_MCLICK::MOUSE_MCLICK_MINIMIZE_PAUSE);
		prop->AddComboValue(L"スレッドスクロールリセット", sl::MOUSE_MCLICK::MOUSE_MCLICK_THREAD_RESET);
		prop->AddComboValue(L"スレッド自動更新", sl::MOUSE_MCLICK::MOUSE_MCLICK_THREAD_RELOAD);
		prop->AddComboValue(L"BBS書き込み欄表示", sl::MOUSE_MCLICK::MOUSE_MCLICK_BBS_EDIT);
		return prop;
	};

	auto bindLDBLClick = [this](int& value)->CMFCPropertyGridProperty*{
		auto prop = prop_.combo(L"左ダブルクリック", value);
		prop->AddComboValue(L"なし", sl::MOUSE_LDBLCLICK::MOUSE_LDBLCLICK_NONE);
		prop->AddComboValue(L"全画面化", sl::MOUSE_LDBLCLICK::MOUSE_LDBLCLICK_FULLSCREEN);
		prop->AddComboValue(L"最大化", sl::MOUSE_LDBLCLICK::MOUSE_LDBLCLICK_MAXIMIZE);
		prop->AddComboValue(L"倍率100%", sl::MOUSE_LDBLCLICK::MOUSE_LDBLCLICK_ZOOM100);
		prop->AddComboValue(L"100%/全画面化", sl::MOUSE_LDBLCLICK::MOUSE_LDBLCLICK_ZOOM100_FULLSCREEN);
		prop->AddComboValue(L"100%/最大化", sl::MOUSE_LDBLCLICK::MOUSE_LDBLCLICK_ZOOM100_MAXIMIZE);
		return prop;
	};

	auto bindScreen = [&](CMFCPropertyGridProperty* group, sl::MouseOperation::Screen& value){
		group->AddSubItem(bindWheel(L"ホイール", value.wheel));
		group->AddSubItem(bindInvert(L"ホイール反転", value.invert));
		group->AddSubItem(bindWheel(L"右クリック+ホイール", value.rwheel));
		group->AddSubItem(bindInvert(L"右クリック+ホイール反転", value.rinvert));
		group->AddSubItem(bindMClick(value.mclick));
		group->AddSubItem(bindLDBLClick(value.ldblclick));
	};

	auto bindBar = [&](CMFCPropertyGridProperty* group, sl::MouseOperation::Bar& value){
		auto check = prop_.check(
			L"画面上と同じ操作", value.transfer);
		group->AddSubItem(check);

		auto wheel = bindWheel(L"ホイール", value.wheel);
		auto invert = bindInvert(L"ホイール反転", value.invert);
		auto mclick = bindMClick(value.mclick);

		group->AddSubItem(wheel);
		group->AddSubItem(invert);
		group->AddSubItem(mclick);

		check->AddLink(wheel, TRUE);
		check->AddLink(invert, TRUE);
		check->AddLink(mclick, TRUE);
	};

	auto group = prop_.group(L"画面上(オンライン)");
	prop_.add(group);
	{
		bindScreen(group, cfg_.online);
	}

	group = prop_.group(L"画面上(オフライン)");
	prop_.add(group);
	{
		bindScreen(group, cfg_.offline);
	}

	group = prop_.group(L"ステータスバー上");
	prop_.add(group);
	{
		bindBar(group, cfg_.status);
	}

	group = prop_.group(L"シークバー上");
	prop_.add(group);
	{
		bindBar(group, cfg_.seek);
	}

	group = prop_.group(L"BBSバー上");
	prop_.add(group);
	{
		bindBar(group, cfg_.bbs);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void COperationMouseDlg::OnGetProp()
{
	prop_.get();
}

void COperationMouseDlg::OnSetProp()
{
	cfg_ = sl::MouseOperation();

	prop_.set();
}

