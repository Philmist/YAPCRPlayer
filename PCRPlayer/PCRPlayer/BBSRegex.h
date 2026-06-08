#pragma once

#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

namespace bbs {

struct Range {
	Range() : first(0), last(0) {}
	Range(int v) : first(v), last(v) {}
	Range(int first, int last) : first(first), last(last) {}
	int first;
	int last;
	int range() const { return abs(first - last); }
	bool within(int v) const { return v >= first && v <= last; }
};

class BBSRegex {
public:
	boost::xpressive::wsregex compile(const std::wstring& text, boost::xpressive::regex_constants::syntax_option_type flag = boost::xpressive::regex_constants::ECMAScript)
	{
		try {
			return boost::xpressive::wsregex::compile(text, flag);
		} catch(...) {}
		return boost::xpressive::wsregex();
	}

	const boost::xpressive::wsregex& scheme()
	{// URL
		const static boost::xpressive::wsregex rx = compile(L"[a-zA-Z0-9_]*:");
		return rx;
	}

	const boost::xpressive::wsregex& body()
	{// URL
		const static boost::xpressive::wsregex rx = compile(L"//[a-zA-Z0-9_#$@&!?~:;.,=%/*+-]+");
		return rx;
	}

	const boost::xpressive::wsregex& gt()
	{// anchor
		using namespace boost::xpressive;
		const static boost::xpressive::wsregex rx = as_xpr(L"&gt;") | L'„';
		return rx;
	}
	const boost::xpressive::wsregex& gt2()
	{// anchor
		const static boost::xpressive::wsregex rx = gt() >> !gt();
		return rx;
	}

	const boost::xpressive::wsregex& number()
	{// anchor
		using namespace boost::xpressive;
		const static boost::xpressive::wsregex rx = +(_d | range(L'‚O', L'‚X'));
		return rx;
	}

	const boost::xpressive::wsregex& hyphen()
	{// anchor
		using namespace boost::xpressive;
		const static boost::xpressive::wsregex rx = as_xpr(L'-') | L'|';
		return rx;
	}

	const boost::xpressive::wsregex& comma()
	{// anchor
		using namespace boost::xpressive;
		const static boost::xpressive::wsregex rx = as_xpr(L',') | L'C';
		return rx;
	}

	const boost::xpressive::wsregex& pair()
	{// anchor
		const static boost::xpressive::wsregex rx = number() >> !(+(hyphen() >> number()));
		return rx;
	}

	const boost::xpressive::wsregex& groupe()
	{// anchor
		const static boost::xpressive::wsregex rx = pair() >> !(+(comma() >> pair()));
		return rx;
	}

	const boost::xpressive::wsregex& head()
	{// id
		using namespace boost::xpressive;
		const static boost::xpressive::wsregex rx = as_xpr(L"ID:");
		return rx;
	}

	const boost::xpressive::wsregex& serial()
	{// id
		using namespace boost::xpressive;
		//const static boost::xpressive::wsregex rx = repeat<8>(set[alnum | L'/' | L'+' | L'-' | L'.']) >> !(set = L'O', L'Q', L'o', L'0');
		const static boost::xpressive::wsregex rx = repeat<8>(set[range(L'!', L'~')]) >> !(set = L'O', L'Q', L'o', L'0');
		return rx;
	}

	bool convert(const std::wstring& text, Range& range);
	wchar_t convert(wchar_t c);
	std::wstring convert(const std::wstring& text);
};



class BBSReplace {
	BBSRegex regex_;

	const boost::xpressive::wsregex& url()
	{
		const static boost::xpressive::wsregex rx = regex_.scheme() >> regex_.body();
		return rx;
	}

	const boost::xpressive::wsregex& anchor()
	{
		const static boost::xpressive::wsregex rx = regex_.gt2() >> regex_.groupe();
		return rx;
	}

	const boost::xpressive::wsregex& id()
	{
		const static boost::xpressive::wsregex rx = regex_.head() >> regex_.serial();
		return rx;
	}

	boost::xpressive::mark_tag url_;
	boost::xpressive::mark_tag anchor_;
	boost::xpressive::mark_tag id_;

	boost::xpressive::mark_tag scheme_;
	boost::xpressive::mark_tag body_;

	boost::xpressive::mark_tag gt_;
	boost::xpressive::mark_tag comma_;
	boost::xpressive::mark_tag pair_;

	boost::xpressive::mark_tag serial_;

public:
	BBSReplace()
		: url_(1)
		, anchor_(2)
		, id_(3)
		, scheme_(4)
		, body_(5)
		, gt_(6)
		, comma_(7)
		, pair_(8)
		, serial_(9)
	{}

	void operator()(const std::wstring& name, std::wstring& message, std::vector<std::wstring>& urls, std::vector<Range>& ranges);

private:
	boost::xpressive::wsregex init();
	const boost::xpressive::wsregex& regex();

	bool url(std::wstring& text, std::wstring& url);
	bool scheme(std::wstring& text);
	void anchor(std::wstring& text, std::vector<Range>& ranges);
	bool id(std::wstring& text, std::wstring& id);
};

class URIConvert {
	boost::xpressive::mark_tag anchor_;
	boost::xpressive::mark_tag id_;
	boost::xpressive::mark_tag ref_;
	boost::xpressive::mark_tag http_;

public:
	URIConvert()
		: anchor_(1)
		, id_(2)
		, ref_(3)
		, http_(4)
	{}

	struct Result {
		struct Base {
			Base() : matched(false) {}
			bool matched;
		};

		struct Anchor : public Base {
			Range value;
		} anchor;

		struct ID : public Base {
			std::wstring value;
		} id;

		struct Ref : public Base {
			Ref() : value(0) {}
			int value;
		} ref;

		struct Http : public Base {
			Http() {}
			std::wstring value;
		} http;
	};

	bool operator()(const std::wstring& text, Result& result);

private:
	BBSRegex regex_;
	struct Scheme {
		const boost::xpressive::wsregex& anchor()
		{
			using namespace boost::xpressive;
			const static boost::xpressive::wsregex rx = as_xpr(L"anchor:");
			return rx;
		}
		const boost::xpressive::wsregex& id()
		{
			using namespace boost::xpressive;
			const static boost::xpressive::wsregex rx = as_xpr(L"id:");
			return rx;
		}
		const boost::xpressive::wsregex& ref()
		{
			using namespace boost::xpressive;
			const static boost::xpressive::wsregex rx = as_xpr(L"ref:");
			return rx;
		}
		const boost::xpressive::wsregex& http()
		{
			using namespace boost::xpressive;
			const static boost::xpressive::wsregex rx = as_xpr(L"http:") | L"https:";
			return rx;
		}
	} scheme_;

	boost::xpressive::wsregex init();
	const boost::xpressive::wsregex& regex();
};

} // namespace bbs