#pragma once

#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

namespace bbs {

enum SKIN_NAME_FORMAT {
	SKIN_NAME_FORMAT_NORMAL,
	SKIN_NAME_FORMAT_NODEFAULT,
};


struct SkinConfig {
	struct Color {
		struct Set {
			Set()
				: enable(true)
				, color(RGB(255, 255, 255))
			{}

			Set(bool enable, COLORREF color)
				: enable(enable)
				, color(color)
			{}

			bool enable;
			COLORREF color;
		};

		Color(COLORREF back = RGB(240, 240, 240))
			: text(true, RGB(96, 96, 96))
			, message(true, RGB(0, 0, 0))
			, link(true, RGB(0, 0, 255))
			, back(true, back)
			, newly(true, RGB(224, 224, 255))
			, border(true, RGB(208, 208, 208))
			, scroll(false, RGB(240, 240, 240))
		{}

		Set text; // 文字
		Set message; // 本文
		Set link; // リンク
		Set back; // 背景
		Set newly; // 新着
		Set border; // ボーダー
		Set scroll; // スクロールバー
	};

	struct TextColor {
		TextColor()
			: text(RGB(0, 128, 0))
			, link(RGB(0, 0, 255))
			, sage(RGB(128, 0, 128))
		{}

		COLORREF text;
		COLORREF link;
		COLORREF sage;
	};

	struct ColorSet {
		ColorSet(COLORREF first, COLORREF second, COLORREF third)
			: first(first)
			, second(second)
			, third(third)
		{}
		COLORREF first;
		COLORREF second;
		COLORREF third;
	};

	struct NumberColor : public ColorSet {
		NumberColor()
			: ColorSet(RGB(0, 0, 255), RGB(128, 0, 128), RGB(255, 0, 0))
		{}
	};

	struct IDColor : public ColorSet {
		IDColor()
			: ColorSet(RGB(0, 0, 0), RGB(0, 0, 255), RGB(255, 0, 0))
		{}
	};

	struct Font {
		Font()
			: name(L"ＭＳ Ｐゴシック")
			, point(9)
			, pixel(12)
		{}

		void set(const LOGFONT& lf)
		{
			name = lf.lfFaceName;
			point = abs(utl::heightToPoint(lf.lfHeight));
			pixel = abs(lf.lfHeight);
		}

		void get(LOGFONT& lf)
		{
			CFont cf;
			cf.CreatePointFont(point * 10, name.c_str());

			memset(&lf, 0, sizeof(LOGFONT));
			cf.GetLogFont(&lf);
			cf.DeleteObject();
		}

		std::wstring name; // フォント名
		int point; // フォントポイント
		int pixel; // フォントピクセル
	};

	struct Name {
		Name(int format = SKIN_NAME_FORMAT_NORMAL)
			: format(format)
			, bold(true)
		{}
		int format; // 名前の書式
		bool bold; // 太字
		TextColor color;
	};

	struct Mail {
		TextColor color;
	};

	struct Time {
		TextColor color;
	};

	struct Number {
		Number()
			: threshold(3)
		{}

		int threshold; // リンクが多いレス番号のしきい値
		NumberColor color; // first:0回/second:1回以上/third:しきい値以上
	};

	struct ID {
		ID(bool enable = true)
			: enable(enable)
			, threshold(5)
		{}

		bool enable; // IDの横に発言回数を表示
		int threshold; // 発言が多いIDのしきい値
		IDColor color; // first:1回/second:2回以上/third:しきい値以上
	};

	SkinConfig(const std::wstring& skin, COLORREF back, int name, bool id)
		: skin(skin)
		, color(back)
		, name(name)
		, id(id)
	{}

	std::wstring skin;
	Font font;
	Color color;
	Name name;
	Mail mail;
	Time time;
	Number number;
	ID id;
};

class BBSSkin {
	SkinConfig cfg_;
	std::wstring noname_;
	utl::AllTagRemover removeAllTag_;
	utl::TagRemover removeBTag_;

	boost::xpressive::wsregex rx_;
	utl::HTMLEscape esc_;

public:
	BBSSkin(const SkinConfig& cfg, const std::wstring& noname = std::wstring());
	virtual ~BBSSkin();

public:

	boost::xpressive::wsregex compile() const
	{
		using namespace boost::xpressive;
		try {
			return as_xpr(L"<&") >> (s1 = -+(~(set = L'<'))) >> L"/>";
		} catch(...) {}
		return boost::xpressive::wsregex();
	}

	const boost::xpressive::wsregex& regex() const
	{
		using namespace boost::xpressive;
		const static boost::xpressive::wsregex rx = compile();
		return rx;
	}

	std::wstring header(std::wstring& html) const
	{
		using namespace boost::xpressive;

		try {
			return regex_replace(html, regex(), [&](const wsmatch& what){
				if (what[1].matched)
				{
					std::wstring text = what[1];
					if (tag(text))
					{
						return text;
					}
				}
				return what[0].str();
			});
		} catch (...) {}

		return std::wstring();
	}

	std::wstring res(std::wstring& html, const ResInfo& info) const
	{
		using namespace boost::xpressive;

		try {
			return regex_replace(html, regex(), [&](const wsmatch& what){
				if (what[1].matched)
				{
					std::wstring text = what[1];
					if (tag(text, info))
					{
						return text;
					}
				}
				return what[0].str();
			});
		} catch (...) {}

		return std::wstring();
	}

private:
	bool tag(std::wstring& text) const;
	bool tag(std::wstring& text, const ResInfo& info) const;
	
	std::wstring colorStr(COLORREF color) const;
	bool eraseNoname(const std::wstring& src, std::wstring& dst) const;
	bool eraseNoname(const ResInfo& info, std::wstring& dst) const;
	std::wstring nameBase(const ResInfo& info) const;
	std::wstring nameAnchor(const ResInfo& info, const std::wstring& name) const;
	std::wstring datetimeFormat(const std::wstring& datetime, bool second) const;
	std::wstring fontColor(const ResInfo& info, const bbs::SkinConfig::TextColor& color) const;
	std::wstring fontColor(const ResInfo& info, const bbs::SkinConfig::TextColor& color, const std::wstring& text) const;
	std::wstring idLink(const ResInfo& info) const;
	std::wstring idCounter(const ResInfo& info) const;
	std::wstring idFormat(const ResInfo& info, bool count) const;
};

} // namespace bbs
