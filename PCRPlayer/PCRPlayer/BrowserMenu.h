#pragma once

#include "CustomDHtmlDialog.h"

namespace utl {

class ContextMenu {
	ContextMenu(){}
	virtual ~ContextMenu(){}

	static std::wstring text_;

public:
	static void clear();
	static void put(const std::wstring& text);
	static std::wstring pop();
	static HRESULT show(CCustomDHtmlDialog* dhtml, DWORD menuID, POINT* pt, IDispatch* disp, int submenu, bool check, const std::wstring& url);
};

}// namespace utl