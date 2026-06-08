#include "stdafx.h"
#include "MFCPropertyGridOperator.h"


CMFCPropertyGridOperator::CMFCPropertyGridOperator(CMFCPropertyGridCtrl& ctrl)
	: ctrl_(ctrl)
{
}

CMFCPropertyGridOperator::~CMFCPropertyGridOperator()
{
}

int CMFCPropertyGridOperator::add(CMFCPropertyGridProperty* prop, BOOL redraw, BOOL adjustLayout)
{
	return ctrl_.AddProperty(prop, redraw, adjustLayout);
}

CMFCPropertyGridProperty* CMFCPropertyGridOperator::group(const std::wstring& name, BOOL list)
{
	Item item = { Item::ITEMTYPE_GROUP, items_.size() + 1, NULL, NULL };
	items_.push_back(item);
	return new CMFCPropertyGridProperty(name.c_str(), 0UL, list);
}

CMFCPropertyGridProperty* CMFCPropertyGridOperator::string(
	const std::wstring& name, std::wstring& str, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_STRING, items_.size() + 1, NULL, &str };
	items_.push_back(item);
	return new CMFCPropertyGridProperty(name.c_str(), _variant_t(_bstr_t(str.c_str())), desc.c_str(), item.id);
}

CMFCPropertyGridProperty* CMFCPropertyGridOperator::string(
	bool& check, const std::wstring& name, std::wstring& str, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_CHECK_STRING, items_.size() + 1, &check, &str };
	items_.push_back(item);
	return new CMFCPropertyGridNameCheckProperty(check, name.c_str(), _variant_t(_bstr_t(str.c_str())), desc.c_str(), item.id);
}

CMFCPropertyGridFontProperty* CMFCPropertyGridOperator::font(
	const std::wstring& name, LOGFONT& lf, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_FONT, items_.size() + 1, NULL, &lf };
	items_.push_back(item);
	return new CMFCPropertyGridFontProperty(name.c_str(), lf, CF_SCREENFONTS, desc.c_str(), item.id);
}

CMFCPropertyGridColorPropertyEx* CMFCPropertyGridOperator::color(
	const std::wstring& name, COLORREF& color, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_COLOR, items_.size() + 1, NULL, &color };
	items_.push_back(item);
	return new CMFCPropertyGridColorPropertyEx(name.c_str(), color, NULL, desc.c_str(), item.id);
}

CMFCPropertyGridNameCheckColorPropertyEx* CMFCPropertyGridOperator::color(
	bool& check, const std::wstring& name, COLORREF& color, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_CHECK_COLOR, items_.size() + 1, &check, &color };
	items_.push_back(item);
	return new CMFCPropertyGridNameCheckColorPropertyEx(check, name.c_str(), color, NULL, desc.c_str(), item.id);
}

CMFCPropertyGridComboProperty* CMFCPropertyGridOperator::combo(
	const std::wstring& name, int& index, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_COMBO, items_.size() + 1, NULL, &index };
	items_.push_back(item);
	return new CMFCPropertyGridComboProperty(name.c_str(), index, desc.c_str(), item.id);
}

CMFCPropertyGridNumberProperty* CMFCPropertyGridOperator::number(
	const std::wstring& name, int& number, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_NUMBER, items_.size() + 1, NULL, &number };
	items_.push_back(item);
	return new CMFCPropertyGridNumberProperty(name.c_str(), number, desc.c_str(), item.id);
}

CMFCPropertyGridNameCheckNumberProperty* CMFCPropertyGridOperator::number(
	bool& check, const std::wstring& name, int& number, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_CHECK_NUMBER, items_.size() + 1, &check, &number };
	items_.push_back(item);
	return new CMFCPropertyGridNameCheckNumberProperty(check, name.c_str(), number, desc.c_str(), item.id);
}

CMFCPropertyGridCheckProperty* CMFCPropertyGridOperator::check(
	const std::wstring& name, bool& check, const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_CHECK, items_.size() + 1, NULL, &check };
	items_.push_back(item);
	return new CMFCPropertyGridCheckProperty(name.c_str(), check, desc.c_str(), item.id);
}

CMFCPropertyGridFileProperty* CMFCPropertyGridOperator::file(
	const std::wstring& name, std::wstring& file,
	const std::wstring& defext, DWORD flags, const std::wstring& filter,
	const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_STRING, items_.size() + 1, NULL, &file };
	items_.push_back(item);
	return new CMFCPropertyGridFileProperty(
		name.c_str(), TRUE, file.c_str(),
		defext.c_str(), flags, filter.c_str(),
		desc.c_str(), item.id);
}

CMFCPropertyGridFolderProperty* CMFCPropertyGridOperator::folder(
	const std::wstring& name, std::wstring& folder,
	const std::wstring& title, DWORD flags,
	const std::wstring& desc)
{
	Item item = { Item::ITEMTYPE_STRING, items_.size() + 1, NULL, &folder };
	items_.push_back(item);
	return new CMFCPropertyGridFolderProperty(
		name.c_str(), folder.c_str(),
		title.c_str(), flags,
		desc.c_str(), item.id);
}


void CMFCPropertyGridOperator::get()
{
	for (auto it = items_.begin(); it != items_.end(); ++it)
	{
		auto p = ctrl_.FindItemByData(it->id);
		if (!p) { continue; }

		switch (it->type)
		{
		case Item::ITEMTYPE_STRING:
		{
			_variant_t v = ((CMFCPropertyGridProperty*)p)->GetValue();
			if (v.vt == VT_BSTR)
			{
				*((std::wstring*)it->ptr) = CString(v.bstrVal).GetString();
			}
			break;
		}
		case Item::ITEMTYPE_CHECK_STRING:
		{
			*it->check = ((CMFCPropertyGridNameCheckProperty*)p)->GetCheck();

			_variant_t v = ((CMFCPropertyGridProperty*)p)->GetValue();
			if (v.vt == VT_BSTR)
			{
				*((std::wstring*)it->ptr) = CString(v.bstrVal).GetString();
			}
			break;
		}
		case Item::ITEMTYPE_FONT:
		{
			LPLOGFONT font = ((CMFCPropertyGridFontProperty*)p)->GetLogFont();
			*((LPLOGFONT)it->ptr) = *font;
			break;
		}
		case Item::ITEMTYPE_COLOR:
		{
			COLORREF color = ((CMFCPropertyGridColorPropertyEx*)p)->GetColor();
			*((COLORREF*)it->ptr) = color;
			break;
		}
		case Item::ITEMTYPE_CHECK_COLOR:
		{
			*it->check = ((CMFCPropertyGridNameCheckColorPropertyEx*)p)->GetCheck();

			COLORREF color = ((CMFCPropertyGridNameCheckColorPropertyEx*)p)->GetColor();
			*((COLORREF*)it->ptr) = color;
			break;
		}
		case Item::ITEMTYPE_COMBO:
		case Item::ITEMTYPE_NUMBER:
		{
			_variant_t v = ((CMFCPropertyGridProperty*)p)->GetValue();
			if (v.vt == VT_INT)
			{
				*((int*)it->ptr) = v.intVal;
			}
			break;
		}
		case Item::ITEMTYPE_CHECK_NUMBER:
		{
			*it->check = ((CMFCPropertyGridNameCheckNumberProperty*)p)->GetCheck();

			_variant_t v = ((CMFCPropertyGridProperty*)p)->GetValue();
			if (v.vt == VT_INT)
			{
				*((int*)it->ptr) = v.intVal;
			}
			break;
		}
		case Item::ITEMTYPE_CHECK:
		{
			_variant_t v = ((CMFCPropertyGridProperty*)p)->GetValue();
			if (v.vt == VT_BOOL)
			{
				*((bool*)it->ptr) = v.boolVal ? true : false;
			}
			break;
		}
		}
	}
}

void CMFCPropertyGridOperator::set()
{
	for (auto it = items_.begin(); it != items_.end(); ++it)
	{
		auto p = ctrl_.FindItemByData(it->id);
		if (!p) { continue; }

		switch (it->type)
		{
		case Item::ITEMTYPE_STRING:
		{
			((CMFCPropertyGridProperty*)p)->SetValue(_variant_t(_bstr_t(((std::wstring*)it->ptr)->c_str())));
			break;
		}
		case Item::ITEMTYPE_CHECK_STRING:
		{
			((CMFCPropertyGridNameCheckProperty*)p)->SetCheck(*it->check);
			((CMFCPropertyGridProperty*)p)->SetValue(_variant_t(_bstr_t(((std::wstring*)it->ptr)->c_str())));
			break;
		}
		case Item::ITEMTYPE_FONT:
		{
			LPLOGFONT font = ((CMFCPropertyGridFontProperty*)p)->GetLogFont();
			*font = *((LPLOGFONT)it->ptr);
			break;
		}
		case Item::ITEMTYPE_COLOR:
		{
			((CMFCPropertyGridColorPropertyEx*)p)->SetColor(*((COLORREF*)it->ptr));
			break;
		}
		case Item::ITEMTYPE_CHECK_COLOR:
		{
			((CMFCPropertyGridNameCheckColorPropertyEx*)p)->SetCheck(*it->check);
			((CMFCPropertyGridNameCheckColorPropertyEx*)p)->SetColor(*((COLORREF*)it->ptr));
			break;
		}
		case Item::ITEMTYPE_COMBO:
		case Item::ITEMTYPE_NUMBER:
		{
			((CMFCPropertyGridProperty*)p)->SetValue(_variant_t(*((int*)it->ptr)));
			break;
		}
		case Item::ITEMTYPE_CHECK_NUMBER:
		{
			((CMFCPropertyGridNameCheckNumberProperty*)p)->SetCheck(*it->check);
			((CMFCPropertyGridProperty*)p)->SetValue(_variant_t(*((int*)it->ptr)));
			break;
		}
		case Item::ITEMTYPE_CHECK:
		{
			((CMFCPropertyGridProperty*)p)->SetValue(_variant_t(*((bool*)it->ptr)));
			((CMFCPropertyGridCheckProperty*)p)->EnableLinks();
			break;
		}
		}
	}
}

void CMFCPropertyGridOperator::clear()
{
	items_.clear();
}


