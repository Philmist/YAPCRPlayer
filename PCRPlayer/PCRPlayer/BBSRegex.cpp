#include "stdafx.h"

namespace bbs {


bool BBSRegex::convert(const std::wstring& text, Range& range)
{
	using namespace boost::xpressive;
	try {
		std::set<int> set;
		const static wsregex rx = hyphen();
		for (wsregex_token_iterator it(text.begin(), text.end(), rx, -1), end; it != end; ++it)
		{
			set.insert(wcstol(convert(it->str()).c_str(), NULL, 10));
		}

		if (!set.empty())
		{
			range= Range(*set.begin(), *set.rbegin());
			return true;
		}
	} catch (...) {}
	return false;
}

wchar_t BBSRegex::convert(wchar_t c)
{
	const wchar_t half[] = L"0123456789";
	const wchar_t full[] = L"‚O‚P‚Q‚R‚S‚T‚U‚V‚W‚X";
	const int size = sizeof(full) / sizeof(full[0]);
	for (int i = 0; i < size; ++i)
	{
		if (c == full[i])
		{
			return half[i];
		}
	}
	return c;
}

std::wstring BBSRegex::convert(const std::wstring& text)
{
	std::wstring str;
	for (auto it = text.begin(); it != text.end(); ++it)
	{
		str += convert(*it);
	}
	return str;
}

void BBSReplace::operator()(const std::wstring& name, std::wstring& message, std::vector<std::wstring>& urls, std::vector<Range>& ranges)
{
	using namespace boost::xpressive;

	try {
		message = regex_replace(message, regex(), [&](const wsmatch& what){
			if (what[url_].matched)
			{
				std::wstring text = what[url_];
				std::wstring u;
				if (url(text, u)) { urls.push_back(u); };
				return text;
			}
			else if (what[anchor_].matched)
			{
				std::wstring text = what[anchor_];
				anchor(text, ranges);
				return text;
			}
			else if (what[id_].matched)
			{
				std::wstring text = what[id_];
				std::wstring u;
				id(text, u);
				return text;
			}
			return what[0].str();
		});

		const static wsregex rx = bos >> *_s >> (s1 = +range(L'0', L'9')) >> *_s >> eos;
		wsmatch what;
		if (regex_search(name, what, rx))
		{
			std::wstring p = what.str(1);
			Range r;
			if (regex_.convert(p, r))
			{
				ranges.push_back(r);
			}
		}
	} catch (...) {}
}


boost::xpressive::wsregex BBSReplace::init()
{
	using namespace boost::xpressive;
	try {
		return
			(url_ = (regex_.scheme() >> regex_.body())) |
			(anchor_ = (regex_.gt2() >> regex_.groupe())) |
			(id_ = (regex_.head() >> regex_.serial()));
	}
	catch(...) {}
	return wsregex();
}

const boost::xpressive::wsregex& BBSReplace::regex()
{
	using namespace boost::xpressive;
	const static wsregex rx = init();
	return rx;
}

bool BBSReplace::url(std::wstring& text, std::wstring& url)
{
	using namespace boost::xpressive;
	try {
		const static wsregex rx = (scheme_ = regex_.scheme()) >> (body_ = regex_.body());
		wsmatch what;
		if (regex_search(text, what, rx))
		{
			std::wstring s = what[scheme_];
			if (scheme(s))
			{
				url = s + what[body_];
				text = L"<a href=\"" + url + L"\">" + text + L"</a>";
				return true;
			}
		}
	} catch (...) {}
	return false;
}

bool BBSReplace::scheme(std::wstring& text)
{
	using namespace boost::xpressive;
	try {
		const static wsregex rx = wsregex::compile(L"(?:http|ttp|tp)(s)?:$");
		wsmatch what;
		if (regex_search(text, what, rx))
		{
			text = L"http" + what[1] + L":";
			return true;
		}
	} catch (...) {}
	return false;
}

void BBSReplace::anchor(std::wstring& text, std::vector<Range>& ranges)
{
	using namespace boost::xpressive;
	try {
		std::wstring str;
		const static wsregex rx = ((gt_ = regex_.gt2()) | (comma_ = regex_.comma()))  >> (pair_ = regex_.pair());
		for (wsregex_iterator it(text.begin(), text.end(), rx), end; it != end; ++it)
		{
			std::wstring p = (*it)[pair_];
			Range r;
			if (regex_.convert(p, r))
			{
				if ((*it)[gt_].matched)
				{
					str += L"<a class=\"anchor:" + p + L"\" href=\"\">" + (*it)[0] + L"</a>";
				}
				else if ((*it)[comma_].matched)
				{
					str += (*it)[comma_] + L"<a class=\"anchor:" + p + L"\" href=\"\">" + (*it)[pair_] + L"</a>";
				}
				ranges.push_back(r);
			}
			else
			{
				str += (*it)[0];
			}
		}
		text = str;
	} catch (...) {}
}

bool BBSReplace::id(std::wstring& text, std::wstring& id)
{
	using namespace boost::xpressive;
	try {
		const static wsregex rx = regex_.head() >> (serial_ = regex_.serial());
		wsmatch what;
		if (regex_search(text, what, rx))
		{
			std::wstring s = what[serial_];
			text = L"<a class=\"id:" + s + L"\" href=\"\">ID:</a>" + s;
			id = s;
			return true;
		}
	} catch (...) {}
	return false;
}

/////////////////////////////////////////////////////////////////////

bool URIConvert::operator()(const std::wstring& text, Result& result)
{
	using namespace boost::xpressive;
	try {
		wsmatch what;
		if (regex_search(text, what, regex()))
		{
			if (what[anchor_].matched)
			{
				std::wstring text = what[anchor_];
				result.anchor.matched = regex_.convert(text, result.anchor.value);
				return result.anchor.matched;
			}
			else if (what[id_].matched)
			{
				result.id.value = what[id_];
				result.id.matched = true;
				return result.id.matched;
			}
			else if (what[ref_].matched)
			{
				std::wstring text = what[ref_];
				result.ref.value = wcstol(regex_.convert(text).c_str(), NULL, 10);
				result.ref.matched = true;
				return result.ref.matched;
			}
			else if (what[http_].matched)
			{
				result.http.value = what[http_];
				result.http.matched = true;
				return result.http.matched;
			}
		}
	} catch (...) {}
	return false;
}

boost::xpressive::wsregex URIConvert::init()
{
	using namespace boost::xpressive;
	try {
		return
			scheme_.anchor() >> (anchor_ = +~_s) |
			scheme_.id() >> (id_ = +~_s) |
			scheme_.ref() >> (ref_ = +~_s) |
			(http_ = (scheme_.http() >> +~_s));

			//scheme_.anchor() >> (anchor_ = regex_.pair()) |
			//scheme_.id() >> (id_ = regex_.serial()) |
			//scheme_.ref() >> (ref_ = regex_.number()) |
			//(http_ = (scheme_.http() >> regex_.body()));
	}
	catch(...) {}
	return wsregex();
}

const boost::xpressive::wsregex& URIConvert::regex()
{
	using namespace boost::xpressive;
	const static wsregex rx = init();
	return rx;
}

}  // namespace bbs