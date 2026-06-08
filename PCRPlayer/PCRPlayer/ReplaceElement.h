#pragma once

#include <boost/xpressive/xpressive.hpp>

namespace utl {

struct ReplaceElement {
	ReplaceElement()
		: enable(true)
		, icase(false)
	{}

	ReplaceElement(
		bool enable,
		const std::wstring& desc,
		const std::wstring& regex,
		const std::wstring& format,
		bool icase)
		: enable(enable)
		, desc(desc)
		, regex(regex)
		, format(format)
		, icase(icase)
	{}

	bool enable;
	std::wstring desc;
	std::wstring regex;
	std::wstring format;
	bool icase;

	bool operator()(std::wstring& str) const
	{
		using namespace boost::xpressive;

		if (!enable) { return false; }

		// regex_constants::single_line
		// $が改行にマッチしなくなる
		wsregex::flag_type flag = regex_constants::single_line;
		if (icase)
		{
			flag = regex_constants::single_line | regex_constants::icase;
		}

		try {
			str = boost::xpressive::regex_replace(str, wsregex::compile(regex, flag), format);
			return true;
		}
		catch (...) {}
		return false;
	}

	boost::xpressive::wsregex compile() const
	{
		using namespace boost::xpressive;

		// regex_constants::single_line
		// $が改行にマッチしなくなる
		wsregex::flag_type flag = regex_constants::single_line;
		if (icase)
		{
			flag = regex_constants::single_line | regex_constants::icase;
		}

		try {
			return wsregex::compile(regex, flag);
		}
		catch (...) {}
		return wsregex();
	}
};

} //namespace utl
