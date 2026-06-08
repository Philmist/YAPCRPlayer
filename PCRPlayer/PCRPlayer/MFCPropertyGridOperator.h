#pragma once

#include "afxpropertygridctrl.h"

#include "MFCPropertyGridNameCheckProperty.h"
#include "MFCPropertyGridNumberProperty.h"
#include "MFCPropertyGridNameCheckNumberProperty.h"
#include "MFCPropertyGridColorPropertyEx.h"
#include "MFCPropertyGridNameCheckColorPropertyEx.h"
#include "MFCPropertyGridComboProperty.h"
#include "MFCPropertyGridCheckProperty.h"
#include "MFCPropertyGridFolderProperty.h"

class CMFCPropertyGridOperator {
	CMFCPropertyGridCtrl& ctrl_;
	struct Item {
		enum ITEMTYPE {
			ITEMTYPE_ANY,
			ITEMTYPE_GROUP,
			ITEMTYPE_STRING,
			ITEMTYPE_CHECK_STRING,
			ITEMTYPE_FONT,
			ITEMTYPE_COMBO,
			ITEMTYPE_COLOR,
			ITEMTYPE_CHECK_COLOR,
			ITEMTYPE_NUMBER,
			ITEMTYPE_CHECK_NUMBER,
			ITEMTYPE_CHECK,
		};

		int type;
		DWORD_PTR id;
		bool* check;
		void* ptr;
	};
	std::vector<Item> items_;

public:
	CMFCPropertyGridOperator(CMFCPropertyGridCtrl& ctrl);
	virtual ~CMFCPropertyGridOperator();

	CMFCPropertyGridProperty* group(const std::wstring& name, BOOL list = FALSE);
	CMFCPropertyGridProperty* string(const std::wstring& name, std::wstring& str, const std::wstring& desc = std::wstring());
	CMFCPropertyGridProperty* string(bool& check, const std::wstring& name, std::wstring& str, const std::wstring& desc = std::wstring());
	CMFCPropertyGridFontProperty* font(const std::wstring& name, LOGFONT& lf, const std::wstring& desc = std::wstring());
	CMFCPropertyGridColorPropertyEx* color(const std::wstring& name, COLORREF& color, const std::wstring& desc = std::wstring());
	CMFCPropertyGridNameCheckColorPropertyEx* color(bool& check, const std::wstring& name, COLORREF& color, const std::wstring& desc = std::wstring());
	CMFCPropertyGridComboProperty* combo(const std::wstring& name, int& index, const std::wstring& desc = std::wstring());
	CMFCPropertyGridNumberProperty* number(const std::wstring& name, int& number, const std::wstring& desc = std::wstring());
	CMFCPropertyGridNameCheckNumberProperty* number(bool& check, const std::wstring& name, int& number, const std::wstring& desc = std::wstring());
	CMFCPropertyGridCheckProperty* check(const std::wstring& name, bool& check, const std::wstring& desc = std::wstring());
	CMFCPropertyGridFileProperty* file(
		const std::wstring& name, std::wstring& file,
		const std::wstring& defext = std::wstring(), DWORD flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, const std::wstring& filter = std::wstring(),
		const std::wstring& desc = std::wstring());
	CMFCPropertyGridFolderProperty* folder(
		const std::wstring& name, std::wstring& folder,
		const std::wstring& title = std::wstring(), DWORD flags = BIF_NEWDIALOGSTYLE,
		const std::wstring& desc = std::wstring());


	int add(CMFCPropertyGridProperty* prop, BOOL redraw = TRUE, BOOL adjustLayout = TRUE);
	void get();
	void set();
	void clear();
};
