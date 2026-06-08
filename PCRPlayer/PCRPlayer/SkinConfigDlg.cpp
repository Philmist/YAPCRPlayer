// LayoutSkinDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "SkinConfigDlg.h"
#include "afxdialogex.h"


// CSkinConfigDlg ダイアログ

IMPLEMENT_DYNAMIC(CSkinConfigDlg, CBasePropDlg)

CSkinConfigDlg::CSkinConfigDlg(utl::PropSetting& setting, const std::wstring& skin, COLORREF back, int name, bool id, bool splitter)
	: CBasePropDlg(setting)
	, cfg_(skin, back, name, id)
	, this_(skin, back, name, id, splitter)
{

}

CSkinConfigDlg::~CSkinConfigDlg()
{
}



void CSkinConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CBasePropDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSkinConfigDlg, CBasePropDlg)
	ON_BN_CLICKED(IDC_BTN_SAMPLE1, &CSkinConfigDlg::OnBnClickedBtnSample1)
	ON_BN_CLICKED(IDC_BTN_SAMPLE2, &CSkinConfigDlg::OnBnClickedBtnSample2)
END_MESSAGE_MAP()


// CSkinConfigDlg メッセージ ハンドラー


BOOL CSkinConfigDlg::OnInitDialog()
{
	CBasePropDlg::OnInitDialog();

	auto bindColor = [this](CMFCPropertyGridProperty* group, const std::wstring& name, bbs::SkinConfig::TextColor& value){
		auto color = prop_.color(
			L"メール欄なし", value.text, L"<&" + name + L"TextColor/>\r\nメール欄に文字列がないときの文字色");
		group->AddSubItem(color);

		color = prop_.color(
			L"メール欄あり", value.link, L"<&" + name + L"LinkColor/>\r\nメール欄に文字列があるときの文字色");
		group->AddSubItem(color);

		color = prop_.color(
			L"メール欄sage", value.sage, L"<&" + name + L"SageColor/>\r\nメール欄が \"sage\" のときの文字色");
		group->AddSubItem(color);
	};

	auto group = prop_.group(L"全般");
	prop_.add(group);
	{
		auto skin = prop_.string(L"スキン", cfg_.skin);
		for (auto it = this_.skins.begin(); it != this_.skins.end(); ++it)
		{
			skin->AddOption(it->c_str());
		}
		skin->AllowEdit(FALSE);
		group->AddSubItem(skin);

		cfg_.font.get(this_.lf);
		auto font = prop_.font(L"フォント", this_.lf,
			L"<&FontName/> フォント名\r\n<&FontPoint/><&FontPixel/> フォントポイント/ピクセル");
		group->AddSubItem(font);
	}

	group = prop_.group(L"配色");
	prop_.add(group);
	{
		auto desc = [](const std::wstring& name)->std::wstring {
				return L"<&" + name + L"Color/>\r\n"
				L"色の数値に置換します (例：#f0f0f0)\r\n"
				L"チェックを外すと \"transparent\" に置換されます";
		};

		auto color = prop_.color(cfg_.color.text.enable, L"文字", cfg_.color.text.color, desc(L"Text"));
		group->AddSubItem(color);

		color = prop_.color(cfg_.color.message.enable, L"本文", cfg_.color.message.color, desc(L"Message"));
		group->AddSubItem(color);

		color = prop_.color(cfg_.color.link.enable, L"リンク", cfg_.color.link.color, desc(L"Link"));
		group->AddSubItem(color);

		color = prop_.color(cfg_.color.back.enable, L"背景", cfg_.color.back.color, desc(L"Back"));
		group->AddSubItem(color);

		color = prop_.color(cfg_.color.newly.enable, L"新着", cfg_.color.newly.color, desc(L"New"));
		group->AddSubItem(color);

		color = prop_.color(cfg_.color.border.enable, L"ボーダー", cfg_.color.border.color, desc(L"Border"));
		group->AddSubItem(color);

		color = prop_.color(cfg_.color.scroll.enable, L"スクロールバー", cfg_.color.scroll.color, desc(L"Scroll"));
		group->AddSubItem(color);
	}

	if (this_.splitter)
	{
		auto group = prop_.group(L"スプリッタ");
		prop_.add(group);
		{
			auto number = prop_.number(L"幅", splitter_.split);
			number->EnableSpinControl(TRUE, 0, 100);
			group->AddSubItem(number);

			auto check = prop_.check(L"3Dエッジ", splitter_.edge);
			group->AddSubItem(check);

			auto color = prop_.color(L"表面", splitter_.face);
			group->AddSubItem(color);
			check->AddLink(color, TRUE);

			color = prop_.color(L"ボーダーカラー", splitter_.border);
			group->AddSubItem(color);
			check->AddLink(color, TRUE);
		}
	}

	group = prop_.group(L"名前");
	prop_.add(group);
	{
		auto combo = prop_.combo(L"書式", cfg_.name.format,
			L"名前の書式を設定します\r\n\"非デフォルト名のみ\"にすると掲示板のデフォルト名以外の名前やトリップ、キャップのみ表示してデフォルト名は表示しません");
		combo->AddComboValue(L"通常", bbs::SKIN_NAME_FORMAT_NORMAL);
		combo->AddComboValue(L"非デフォルト名のみ", bbs::SKIN_NAME_FORMAT_NODEFAULT);
		group->AddSubItem(combo);

		auto check = prop_.check(
			L"太字", cfg_.name.bold,
			L"名前を太字で表示します\r\n"
			L"トリップ、キャップ等は本物と区別をつけるため太字にはなりません\r\n"
			L"スキンで名前を太字にしている場合太字を無効にはできません"
		);
		group->AddSubItem(check);

		bindColor(group, L"Name", cfg_.name.color);
	}

	group = prop_.group(L"メール欄");
	prop_.add(group);
	{
		bindColor(group, L"Mail", cfg_.mail.color);
	}

	group = prop_.group(L"日時");
	prop_.add(group);
	{
		bindColor(group, L"Time", cfg_.time.color);
	}

	group = prop_.group(L"レス番号");
	prop_.add(group);
	{
		auto color = prop_.color(
			L"アンカー参照数0", cfg_.number.color.first);
		group->AddSubItem(color);

		color = prop_.color(
			L"アンカー参照数1以上", cfg_.number.color.second);
		group->AddSubItem(color);

		color = prop_.color(
			L"アンカー参照数しきい値以上", cfg_.number.color.third);
		group->AddSubItem(color);

		auto number = prop_.number(
			L"しきい値", cfg_.number.threshold);
		group->AddSubItem(number);
	}

	group = prop_.group(L"ID");
	prop_.add(group);
	{
		auto check = prop_.check(
			L"IDの横に発言回数を表示する", cfg_.id.enable);
		group->AddSubItem(check);

		auto color = prop_.color(
			L"発言数1", cfg_.id.color.first);
		group->AddSubItem(color);

		color = prop_.color(
			L"発言数2以上", cfg_.id.color.second);
		group->AddSubItem(color);

		color = prop_.color(
			L"発言数しきい値以上", cfg_.id.color.third);
		group->AddSubItem(color);

		auto number = prop_.number(
			L"しきい値", cfg_.id.threshold);
		group->AddSubItem(number);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CSkinConfigDlg::OnBnClickedBtnSample1()
{
	prop_.get();
	cfg_.font.set(this_.lf);

	splitter_.edge = false;
	splitter_.face = RGB(16, 16, 16);
	splitter_.border = RGB(32, 32, 32);

	cfg_.color.text.enable = true;
	cfg_.color.text.color = RGB(160, 160, 160);
	cfg_.color.message.enable = true;
	cfg_.color.message.color = RGB(255, 255, 255);
	cfg_.color.link.enable = true;
	cfg_.color.link.color = RGB(128, 128, 255);
	cfg_.color.back.enable = true;
	cfg_.color.back.color = RGB(16, 16, 16);
	cfg_.color.newly.enable = true;
	cfg_.color.newly.color = RGB(16, 16, 64);
	cfg_.color.border.enable = true;
	cfg_.color.border.color = RGB(32, 32, 32);
	cfg_.color.scroll.enable = true;
	cfg_.color.scroll.color = RGB(16, 16, 16);

	cfg_.name.color.text = RGB(128, 255, 128);
	cfg_.name.color.link = RGB(128, 128, 255);
	cfg_.name.color.sage = RGB(255, 128, 255);

	cfg_.mail.color.text = RGB(128, 255, 128);
	cfg_.mail.color.link = RGB(128, 128, 255);
	cfg_.mail.color.sage = RGB(255, 128, 255);

	cfg_.time.color.text = RGB(128, 255, 128);
	cfg_.time.color.link = RGB(128, 128, 255);
	cfg_.time.color.sage = RGB(255, 128, 255);

	cfg_.number.color.first = RGB(128, 128, 255);
	cfg_.number.color.second = RGB(255, 128, 255);
	cfg_.number.color.third = RGB(255, 128, 128);

	cfg_.id.color.first = RGB(192, 192, 192);
	cfg_.id.color.second = RGB(128, 128, 255);
	cfg_.id.color.third = RGB(255, 128, 128);

	prop_.set();

	m_xcProp.RedrawWindow();
}

void CSkinConfigDlg::OnBnClickedBtnSample2()
{
	auto tmp = bbs::SkinConfig(this_.skin, this_.back, this_.name, this_.id);
	auto splitter = sl::SplitterConfig();

	prop_.get();
	cfg_.font.set(this_.lf);

	splitter_.edge = false;
	splitter_.face = splitter.face;
	splitter_.border = splitter.border;

	cfg_.color = tmp.color;
	cfg_.name.color = tmp.name.color;
	cfg_.mail.color = tmp.mail.color;
	cfg_.time.color = tmp.time.color;
	cfg_.number.color = tmp.number.color;
	cfg_.id.color = tmp.id.color;

	prop_.set();

	m_xcProp.RedrawWindow();
}

void CSkinConfigDlg::OnGetProp()
{
	prop_.get();
	cfg_.font.set(this_.lf);
}

void CSkinConfigDlg::OnSetProp()
{
	cfg_ = bbs::SkinConfig(this_.skin, this_.back, this_.name, this_.id);
	splitter_ = sl::SplitterConfig();

	cfg_.font.get(this_.lf);
	prop_.set();
}

