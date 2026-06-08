#include "stdafx.h"

namespace bbs {

BBSSkin::BBSSkin(const SkinConfig& cfg, const std::wstring& noname)
	: cfg_(cfg)
	, noname_(noname)
	, removeBTag_(L"b")
{
	try {
		using namespace boost::xpressive;
		// 名無しの前後にスペースがあるとマッチが失敗する
		const static wsregex rx = wsregex::compile(L"^\\s*(.*?)\\s*$");
		wsmatch what;
		if (regex_search(noname, what, rx))
		{
			noname_ = what[1].str();
		}

		rx_ =
			wsregex::compile(L"^(\\s*|<[^>]*>)*") >>
			as_xpr(noname_) >>
			wsregex::compile(L"(\\s*|<[^>]*>)*$");
	} catch(...) {}
}

BBSSkin::~BBSSkin()
{
}

bool BBSSkin::tag(std::wstring& text) const
{
	auto icmp = [&](const wchar_t* tag){ return _wcsicmp(text.c_str(), tag) == 0; };

	auto colorcmp = [&](const std::wstring& tag, const bbs::SkinConfig::TextColor& value)->bool{
		if (icmp((tag + L"TextColor").c_str()))
		{
			text = colorStr(value.text);
			return true;
		}
		else if (icmp((tag + L"LinkColor").c_str()))
		{
			text = colorStr(value.link);
			return true;
		}
		else if (icmp((tag + L"SageColor").c_str()))
		{
			text = colorStr(value.sage);
			return true;
		}
		return false;
	};

	auto colorsetcmp = [&](const std::wstring& tag, const bbs::SkinConfig::Color::Set& value)->bool{
		if (icmp(tag.c_str()))
		{
			if (value.enable)
			{
				text = colorStr(value.color);
			}
			else
			{
				text = L"transparent";
			}
			return true;
		}
		return false;
	};


	if (icmp(L"FontName"))
	{
		text = cfg_.font.name;
		return true;
	}
	else if (icmp(L"FontPoint"))
	{
		text = (boost::wformat(L"%d") % cfg_.font.point).str();
		return true;
	}
	else if (icmp(L"FontPixel"))
	{
		text = (boost::wformat(L"%d") % cfg_.font.pixel).str();
		return true;
	}
	else if (colorcmp(L"Name", cfg_.name.color)) { return true; }
	else if (colorcmp(L"Mail", cfg_.mail.color)) { return true; }
	else if (colorcmp(L"Time", cfg_.time.color)) { return true; }
	else if (colorsetcmp(L"TextColor", cfg_.color.text)) { return true; }
	else if (colorsetcmp(L"MessageColor", cfg_.color.message)) { return true; }
	else if (colorsetcmp(L"LinkColor", cfg_.color.link)) { return true; }
	else if (colorsetcmp(L"BackColor", cfg_.color.back)) { return true; }
	else if (colorsetcmp(L"NewColor", cfg_.color.newly)) { return true; }
	else if (colorsetcmp(L"BorderColor", cfg_.color.border)) { return true; }
	else if (colorsetcmp(L"ScrollColor", cfg_.color.scroll)) { return true; }

	return false;
}

bool BBSSkin::tag(std::wstring& text, const ResInfo& info) const
{
	auto icmp = [&](const wchar_t* tag){ return _wcsicmp(text.c_str(), tag) == 0; };

	if (tag(text))
	{
		return true;
	}
	else if (icmp(L"Number"))
	{
		text = L"<a class=\"ref:" + info.number + L"\" href=\"\">" + info.number + L"</a>";
		return true;
	}
	else if (icmp(L"NumberText"))
	{
		text = info.number;
		return true;
	}
	else if (icmp(L"NumberAnchor"))
	{
		text = L"<a class=\"anchor:" + info.number + L"\" href=\"\">" + info.number + L"</a>";
		return true;
	}
	else if (icmp(L"Name"))
	{
		std::wstring name = nameBase(info);
		text = name.empty() ? name : fontColor(info, cfg_.name.color, name);
		return true;
	}
	else if (icmp(L"NameText"))
	{
		std::wstring name = info.name;
		eraseNoname(info, name);
		text = name.empty() ? name : removeAllTag_(name);
		return true;
	}
	else if (icmp(L"NameExist"))
	{
		std::wstring name = info.name;
		eraseNoname(info, name);
		text = name.empty() ? L"false" : L"true";
		return true;
	}
	else if (icmp(L"NameBR"))
	{
		std::wstring name = nameBase(info);
		text = name.empty() ? name : fontColor(info, cfg_.name.color, name) + L"<br>";
		return true;
	}
	else if (icmp(L"Mail"))
	{
		text = fontColor(info, cfg_.mail.color, info.mail);
		return true;
	}
	else if (icmp(L"MailText"))
	{
		text = info.mail;
		return true;
	}
	else if (icmp(L"MailExist"))
	{
		text = info.mail.empty() ? L"false" : L"true";
		return true;
	}
	else if (icmp(L"MailSage"))
	{
		text = info.mail == L"sage" ? L"true" : L"false";
		return true;
	}
	else if (icmp(L"MailAge"))
	{
		text = (!info.mail.empty() && info.mail != L"sage") ? L"true" : L"false";
		return true;
	}
	else if (icmp(L"Date"))
	{
		text = info.datetime;
		std::wstring id = idFormat(info, cfg_.id.enable);
		if (!id.empty())
		{
			text += L" " + id;
		}
		return true;
	}
	else if (icmp(L"DateTime"))
	{
		text = fontColor(info, cfg_.time.color, info.datetime);
		return true;
	}
	else if (icmp(L"DateTimeText"))
	{
		text = info.datetime;
		return true;
	}
	else if (icmp(L"Time"))
	{
		text = fontColor(info, cfg_.time.color, datetimeFormat(info.datetime, false));
		return true;
	}
	else if (icmp(L"TimeText"))
	{
		text = datetimeFormat(info.datetime, false);
		return true;
	}
	else if (icmp(L"TimeSecond"))
	{
		text = fontColor(info, cfg_.time.color, datetimeFormat(info.datetime, true));
		return true;
	}
	else if (icmp(L"TimeSecondText"))
	{
		text = datetimeFormat(info.datetime, true);
		return true;
	}
	else if (icmp(L"ID"))
	{
		text = idFormat(info, cfg_.id.enable);
		return true;
	}
	else if (icmp(L"IDText"))
	{
		text = info.id;
		return true;
	}
	else if (icmp(L"IDExist"))
	{
		text = info.id.empty() ? L"false" : L"true";
		return true;
	}
	else if (icmp(L"IDLink"))
	{
		text = idLink(info);
		return true;
	}
	else if (icmp(L"IDCounter"))
	{
		text = idCounter(info);
		return true;
	}
	else if (icmp(L"IDNumber"))
	{
		if (info.identifier != 0)
		{
			text = L"<a class=\"id:" + info.id + L"\" href=\"\">";
			text += (boost::wformat(L"%d") % info.identifier).str();
			text += L"</a>";
		}
		else
		{
			text = info.id;
		}
		return true;
	}
	else if (icmp(L"IDNumberText"))
	{
		if (info.identifier != 0)
		{
			text = (boost::wformat(L"%d") % info.identifier).str();
		}
		else
		{
			text = info.id;
		}
		return true;
	}
	else if (icmp(L"Message"))
	{
		text = info.message;
		return true;
	}
	return false;
}


std::wstring BBSSkin::colorStr(COLORREF color) const
{
	return (
		boost::wformat(L"#%02x%02x%02x")
		% GetRValue(color)
		% GetGValue(color)
		% GetBValue(color)).str();
}

bool BBSSkin::eraseNoname(const std::wstring& src, std::wstring& dst) const
{
	using namespace boost::xpressive;
	bool result = false;
	std::wstring out;
	try {
		const static wsregex rx = (s1 = -+_) >> (s2 = icase(as_xpr(L"<b>") | L"</b>" | eos));
		for (wsregex_iterator it(src.begin(), src.end(), rx), end; it != end; ++it)
		{
			wsmatch what;
			std::wstring split = esc_(it->str(1));
			if (regex_search(split, what, rx_))
			{
				result = true;
			}
			else
			{
				out += it->str(1);
			}
			out += it->str(2);
		}
		dst = out;
		return result;
	} catch (...) {}
	return result;
}

bool BBSSkin::eraseNoname(const ResInfo& info, std::wstring& dst) const
{
	return cfg_.name.format == SKIN_NAME_FORMAT_NODEFAULT && eraseNoname(info.name, dst);
}

std::wstring BBSSkin::nameBase(const ResInfo& info) const
{
	std::wstring name = info.name;
	if (!eraseNoname(info, name))
	{
		name = nameAnchor(info, name);
	}

	if (!name.empty())
	{
		name = cfg_.name.bold ? L"<b>" + name + L"</b>" : removeBTag_(name);
	}
	return name;
}

std::wstring BBSSkin::nameAnchor(const ResInfo& info, const std::wstring& name) const
{
	try {
		BBSRegex regex;
		using namespace boost::xpressive;
		const static wsregex rx = bos >> *_s >> (s1 = +range(L'0', L'9')) >> *_s >> eos;
		wsmatch what;
		if (regex_search(name, what, rx))
		{
			std::wstring p = what.str(1);
			Range r;
			if (regex.convert(p, r))
			{
				return L"<a class=\"anchor:" + p + L"\" href=\"\" style=\"color:" + fontColor(info, cfg_.name.color) + L";\">" + name + L"</a>";
			}
		}
	} catch (...) {}
	return name;
}

std::wstring BBSSkin::datetimeFormat(const std::wstring& datetime, bool second) const
{
	using namespace boost::xpressive;
	try {
		const static wsregex rx = wsregex::compile(L"([0-9]+:[0-9]+)(:[0-9]+)?");
		wsmatch what;
		if (regex_search(datetime, what, rx))
		{
			if (second)
			{
				return what[1].str() + what[2].str();
			}
			return what[1].str();
		}
	} catch (...) {}
	return datetime;
}

std::wstring BBSSkin::fontColor(const ResInfo& info, const bbs::SkinConfig::TextColor& color) const
{
	if (info.mail == L"sage")
	{
		return colorStr(color.sage);
	}
	else if (!info.mail.empty())
	{
		return colorStr(color.link);
	}
	return colorStr(color.text);
}

std::wstring BBSSkin::fontColor(const ResInfo& info, const bbs::SkinConfig::TextColor& color, const std::wstring& text) const
{
	if (text.empty()) { return std::wstring(); }
	return L"<font color=\"" + fontColor(info, color) + L"\">" + text + L"</font>";
}

std::wstring BBSSkin::idLink(const ResInfo& info) const
{
	if (info.id.empty()) { return std::wstring(); }
	if (info.count > 0)
	{
		return L"<a class=\"id:" + info.id + L"\" href=\"\">ID:</a>";
	}
	return L"ID:";
}

std::wstring BBSSkin::idCounter(const ResInfo& info) const
{
	if (info.count > 0)
	{
		std::wstring count;
		if (info.total > 1)
		{
			count = (
				boost::wformat(L"[%d/%d]")
				% info.count
				% info.total
				).str();
		}
		return L"<span class=\"id:" + info.id + L"\">" + count + L"</span>";
	}
	return std::wstring();
}

std::wstring BBSSkin::idFormat(const ResInfo& info, bool count) const
{
	std::wstring id;
	if (!info.id.empty())
	{
		if (info.count > 0)
		{
			id = idLink(info) + info.id;

			if (count)
			{
				id += L" " + idCounter(info);
			}
		}
		else
		{
			id = L"ID:" + info.id;
		}
	}
	return id;
}

} // namespace bbs
