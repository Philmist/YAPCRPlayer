#include "stdafx.h"
#include "DHtmlUtl.h"

namespace utl {

bool loadHtml(const std::wstring& path, std::wstring& html)
{
	std::ifstream ifs(path, std::ios::in|std::ios::binary);
	if (ifs.is_open())
	{
		ifs.seekg (0, ifs.end);
		size_t length = (size_t)ifs.tellg();
		ifs.seekg (0, ifs.beg);

		if (length > 0)
		{
			std::vector<char> dat;
			dat.resize(length);
			ifs.read(dat.data(), dat.size());
			ifs.close();

			html = utl::toUTF16(std::string(dat.data(), dat.size()));
			return true;
		}
	}
	return false;
}




// count‚©‚ç‹t‡‚ÉƒfƒNƒŠƒƒ“ƒg‚µ‚È‚ª‚ç”­Œ¾”‚ğ‘‚«Š·‚¦‚é
// id L"id:" + ID
// count ”­Œ¾”
// total ”­Œ¾‘”
void refreshCounter(
	const CComPtr<IHTMLDocument2>& doc2, const std::wstring& id, int count, int total)
{
	if (!doc2) { return; }

	CComQIPtr<IHTMLDocument3> doc3(doc2);
	if (doc3)
	{
		CComPtr<IHTMLElementCollection> c;
		if (SUCCEEDED(doc3->getElementsByTagName(CComBSTR(L"span"), &c)) && c)
		{
			long len = 0;
			if (SUCCEEDED(c->get_length(&len)) && len > 0)
			{
				int countx = count;
				for (int i = len - 1; i >= 0; --i)
				{
					CComVariant vidx(i, VT_I4);
					CComVariant vid(0, VT_I4);
					CComPtr<IDispatch> disp;
					if (SUCCEEDED(c->item(vidx, vid, &disp)) && disp)
					{
						CComQIPtr<IHTMLElement> element(disp);
						if (element)
						{
							CComBSTR name;
							if (SUCCEEDED(element->get_className(&name)))
							{
								if (CString(name) == (L"id:" + id).c_str())
								{
									--countx;
									CString str;
									str.Format(
										L"[%d/%d]",
										countx,
										total
									);
									element->put_innerHTML(CComBSTR(str));
									if (countx <= 0) { break; }
								}
							}
						}
					}
				}
			}
		}
	}
}

//if (SUCCEEDED())
void refreshColor(
	const CComPtr<IHTMLDocument2>& doc2, const LinkMap& map, const bbs::SkinConfig& cfg)
{
	if (!doc2) { return; }

	CComPtr<IHTMLElementCollection> c;
	if (SUCCEEDED(doc2->get_links(&c)) && c)
	{
		long len = 0;
		if (SUCCEEDED(c->get_length(&len)))
		{
			for (int i = 0; i < len; ++i)
			{
				CComVariant vidx(i, VT_I4);
				CComVariant vid(0, VT_I4);
				CComPtr<IDispatch> disp;
				if (SUCCEEDED(c->item(vidx, vid, &disp)) && disp)
				{
					CComQIPtr<IHTMLElement> element(disp);
					if (element)
					{
						CComBSTR name;
						if (SUCCEEDED(element->get_className(&name)))
						{
							std::wstring id = CString(name).GetString();
							auto it = map.find(id);
							if (it != map.end())
							{
								if (it->second.id)
								{
									std::wstring color;

									if (it->second.total >= cfg.id.threshold)
									{
										color = toStr(cfg.id.color.third);
									}
									else if (it->second.total >= 2)
									{
										color = toStr(cfg.id.color.second);
									}
									else
									{
										color = toStr(cfg.id.color.first);
									}

									CComPtr<IHTMLStyle> style;
									if (SUCCEEDED(element->get_style(&style)) && style)
									{
										style->put_color(CComVariant(color.c_str()));
									}
								}
								else
								{
									std::wstring color;

									if (it->second.total >= cfg.number.threshold)
									{
										color = toStr(cfg.number.color.third);
									}
									else if (it->second.total >= 1)
									{
										color = toStr(cfg.number.color.second);
									}
									else
									{
										color = toStr(cfg.number.color.first);
									}

									CComPtr<IHTMLStyle> style;
									if (SUCCEEDED(element->get_style(&style)) && style)
									{
										style->put_color(CComVariant(color.c_str()));
									}
								}
							}
							else
							{
								if (id.find(L"id:") == 0)
								{
									CComPtr<IHTMLStyle> style;
									if (SUCCEEDED(element->get_style(&style)) && style)
									{
										std::wstring color = toStr(cfg.id.color.first);
										style->put_color(CComVariant(color.c_str()));
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void refreshColor(
	const CComPtr<IHTMLDocument2>& doc2, const std::vector<bbs::ResInfo>& dat, const bbs::SkinConfig& cfg)
{
	utl::LinkMap map;
	for (auto it = dat.begin(); it != dat.end(); ++it)
	{
		utl::Link link;
		link.id = false;
		link.total = (int)it->ref.size();
		map.insert(std::make_pair(L"ref:" + it->number, link));

		if (it->total > 0 && map.find(L"id:" + it->id) == map.end())
		{
			utl::Link link;
			link.id = true;
			link.total = it->total;
			map.insert(std::make_pair(L"id:" + it->id, link));
		}
	}

	if (!map.empty())
	{
		utl::refreshColor(doc2, map, cfg);
	}
}

std::wstring toStr(COLORREF color)
{
	return (
		boost::wformat(L"#%02x%02x%02x")
		% GetRValue(color)
		% GetGValue(color)
		% GetBValue(color)).str();
}

}; // namespace utl
