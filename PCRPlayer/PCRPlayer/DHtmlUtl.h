#pragma once

//BEGIN_DHTML_EVENT_MAP(CBrowserDlg)
//	DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONMOUSEWHEEL, OnLinkMouseWheel)
//END_DHTML_EVENT_MAP()

#define DHTML_EVENT_TAG_ALL(dispid, memberFxn)\
DHTML_EVENT_TAG(dispid, _T("a"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("abbr"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("acronym"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("address"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("applet"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("area"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("b"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("base"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("basefont"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("bdo"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("big"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("blockquote"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("body"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("br"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("button"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("caption"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("center"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("cite"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("code"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("col"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("colgroup"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("dd"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("del"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("dir"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("div"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("dfn"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("dl"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("dt"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("em"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("fieldset"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("font"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("form"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("frame"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("frameset"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("h1"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("h2"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("h3"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("h4"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("head"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("hr"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("html"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("i"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("iframe"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("img"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("input"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("ins"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("isindex"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("kbd"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("label"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("legend"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("li"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("link"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("map"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("menu"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("meta"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("noframes"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("noscript"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("object"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("ol"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("optgroup"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("option"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("param"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("pre"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("q"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("s"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("samp"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("script"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("select"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("small"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("span"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("strike"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("strong"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("style"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("sub"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("sup"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("table"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("tbody"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("td"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("textarea"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("tfoot"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("th"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("thead"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("title"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("tr"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("tt"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("u"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("ul"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("var"), memberFxn)\
DHTML_EVENT_TAG(dispid, _T("xmp"), memberFxn)\


namespace utl {

class SafeArray {
	SAFEARRAY *array_;

public:
	SafeArray()
		: array_(NULL)
	{}

	SafeArray(const std::wstring& str)
		: array_(NULL)
	{
		assign(str);
	}

	SafeArray& operator=(const SafeArray& x)
	{
		clear();
		SafeArrayCopy(x.array_, &array_);
		return *this;
	}

	SafeArray& operator=(const std::wstring& str)
	{
		assign(str);
		return *this;
	}

	virtual ~SafeArray()
	{
		clear();
	}

	operator SAFEARRAY *()
	{
		return array_;
	}

	bool assign(const std::wstring& str)
	{
		clear();
		array_ = SafeArrayCreateVector(VT_VARIANT, 0, 1);
		if (array_)
		{
			VARIANT *p = NULL;
			if (SafeArrayAccessData(array_, (LPVOID*)&p) == S_OK)
			{
				BSTR bstr = SysAllocString(str.c_str());
				if (bstr)
				{
					p->vt = VT_BSTR;
					p->bstrVal = bstr;

					if (SafeArrayUnaccessData(array_) == S_OK)
					{
						return true;
					}
				}
			}
		}
		clear();
		return false;
	}

	void clear()
	{
		if (array_) { SafeArrayDestroyData(array_); array_ = NULL; }
	}
};


bool loadHtml(const std::wstring& path, std::wstring& html);


struct Link {
	Link()
		: id(false)
		, total(0)
	{}
	bool id;
	int total;
};
typedef std::unordered_map<std::wstring, Link> LinkMap;

void refreshCounter(
	const CComPtr<IHTMLDocument2>& doc2, const std::wstring& id, int count, int total);
void refreshColor(
	const CComPtr<IHTMLDocument2>& doc2, const LinkMap& map, const bbs::SkinConfig& cfg);
void refreshColor(
	const CComPtr<IHTMLDocument2>& doc2, const std::vector<bbs::ResInfo>& dat, const bbs::SkinConfig& cfg);
std::wstring toStr(COLORREF color);



}; // namespace utl
