#include "stdafx.h"

namespace utl {

std::wstring HTMLEscape::operator()(const boost::xpressive::wsmatch& what) const
{
	using namespace boost::xpressive;

	std::wstring str;
	try {
		if (what[tag_].matched)
		{
			const static wsregex rx = icase(skip(_s)(as_xpr(L"br")));
			if (regex_match(what[tag_].str(), rx))
			{
				str += L"\r\n";
				return str;
			}
			//str += L"[" + what[tag_].str() + L"]";
			return str;
		}
		else if (what[letter_].matched)
		{
			auto it = map_.find(what[letter_].str());
			if (it != map_.end())
			{
				str.push_back(it->second);
				return str;
			}
		}
		else if (what[digit_].matched)
		{
			wchar_t c = (wchar_t)wcstol(what[digit_].str().c_str(), NULL, 10);

			str.push_back(c);
			return str;
		}
		else if (what[hex_].matched)
		{
			wchar_t c = (wchar_t)wcstol(what[hex_].str().c_str(), NULL, 16);

			str.push_back(c);
			return str;
		}
		str = what[0];
	} catch (...) {}
	return str;
}

} // namespace utl