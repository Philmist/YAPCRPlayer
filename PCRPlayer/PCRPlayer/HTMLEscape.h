#pragma once

#include <unordered_map>
#include <boost/xpressive/xpressive.hpp>

namespace utl {

class HTMLEscape {
	std::unordered_map<std::wstring, wchar_t> map_;
	boost::xpressive::mark_tag tag_;
	boost::xpressive::mark_tag letter_;
	boost::xpressive::mark_tag digit_;
	boost::xpressive::mark_tag hex_;

public:
	HTMLEscape()
		: tag_(1)
		, letter_(2)
		, digit_(3)
		, hex_(4)
	{
		map_[L"quot"] = L'\"';
		map_[L"amp"] = L'&';
		map_[L"lt"] = L'<';
		map_[L"gt"] = L'>';
		map_[L"nbsp"] = L' ';
	}

	std::wstring operator()(const std::wstring& str) const
	{
		try {
			return boost::xpressive::regex_replace(str, regex(), *this);
		} catch (...) {}
		return str;
	}

	void replace(std::wstring& str)
	{
		try {
			str = boost::xpressive::regex_replace(str, regex(), *this);
		} catch (...) {}
	}

	boost::xpressive::wsregex compile() const
	{	
		using namespace boost::xpressive;

		try {
			return (L'<' >> (tag_ = +~as_xpr(L'>')) >> L'>')
				| (L'&' >> (letter_ = +range(L'a', L'z')) >> !as_xpr(L';'))
				| (L"&#"  >> (digit_ = +_d) >> !as_xpr(L';'))
				| icase(L"&#x"  >> (hex_ = +set[_d | range(L'a', L'f')]) >> !as_xpr(L';'));
		} catch (...) {}
		return wsregex();
	}

	const boost::xpressive::wsregex& regex() const
	{	
		using namespace boost::xpressive;
		const static wsregex rx = compile();
		return rx;
	}

	std::wstring operator()(const boost::xpressive::wsmatch& what) const;
};

class NameSplitter {
public:
	void operator()(const std::wstring& name, std::vector<std::pair<std::wstring, bool> >& dst)
	{
		using namespace boost::xpressive;
		try {
			dst.clear();
			HTMLEscape esc;
			for (wsregex_iterator it(name.begin(), name.end(), regex()), end; it != end; ++it)
			{
				dst.push_back(std::make_pair(esc(it->str(1)), it->str(2).empty()));
			}
		} catch (...) {}
	}

	boost::xpressive::wsregex compile() const
	{	
		using namespace boost::xpressive;
		try {
			return (s1 = -+_) >> icase((s2 = L"<b>") | L"</b>" | eos);
		} catch (...) {}
		return wsregex();
	}

	const boost::xpressive::wsregex& regex() const
	{	
		using namespace boost::xpressive;
		const static wsregex rx = compile();
		return rx;
	}
};

class TagRemover {
	boost::xpressive::wsregex rx_;
public:
	TagRemover(const std::wstring& tag)
		: rx_(compile(tag))
	{}

	std::wstring operator()(const std::wstring& html) const
	{
		using namespace boost::xpressive;
		try {
			return regex_replace(html, rx_, L"");
		} catch (...) {}
		return html;
	}

	boost::xpressive::wsregex compile(const std::wstring& tag) const
	{	
		using namespace boost::xpressive;
		try {
			return as_xpr(L'<') >> !(-*_s >> as_xpr(L'/')) >> -*_s >> icase(tag) >> ~before(alpha) >> -*~as_xpr('>') >> as_xpr(L'>');
		} catch (...) {}
		return wsregex();
	}
};

class AllTagRemover {
	boost::xpressive::wsregex rx_;
public:
	AllTagRemover()
		: rx_(compile())
	{}

	std::wstring operator()(const std::wstring& html) const
	{
		using namespace boost::xpressive;
		try {
			return regex_replace(html, rx_, L"");
		} catch (...) {}
		return html;
	}

	boost::xpressive::wsregex compile() const
	{	
		using namespace boost::xpressive;
		try {
			return as_xpr(L'<') >> -*~as_xpr('>') >> as_xpr(L'>');
		} catch (...) {}
		return wsregex();
	}
};

class ClassNameRemover {
	std::wstring name_;
	boost::xpressive::wsregex rx_;

public:
	ClassNameRemover(const std::wstring& name)
		: name_(name)
		, rx_(compile(name))
	{}

	std::wstring operator()(const std::wstring& html)
	{
		using namespace boost::xpressive;
		try {
			return regex_replace(html, rx_, [this](const wsmatch& what)->std::wstring{
				if (what[2].matched)
				{
					std::wstring target = what[2];
					if (target == name_)
					{
						return what[1] + what[4] + what[4] + what[3];
					}
					else if (target.find(name_ + L"-") == 0)
					{
						std::wstring anew(target.begin() + (name_ + L"-").size(), target.end());
						return what[1] + what[4] + anew + what[4] + what[3];
					}
				}
				return what[0];
			});
		} catch (...) {}
		return html;
	}

	boost::xpressive::wsregex compile(const std::wstring& tag)
	{	
		using namespace boost::xpressive;
		try {
			return
				(s1 = (as_xpr(L'<') >> -*~as_xpr('>') >> icase(as_xpr(L"class")) >> -*_s >> as_xpr(L'=') >> -*_s)) >>
				((s4 = as_xpr(L'\"')) >> (s2 = -*~as_xpr(L'\"')) >> as_xpr(L'\"') | (s4 = as_xpr(L'\'')) >> (s2 = -*~as_xpr(L'\'')) >> as_xpr(L'\'')) >>
				(s3 = (-*~as_xpr('>') >> as_xpr(L'>')));
		} catch (...) {}
		return wsregex();
	}
};


} // namespace utl