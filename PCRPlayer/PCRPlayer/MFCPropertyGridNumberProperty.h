#pragma once
#include "afxpropertygridctrl.h"

class CMFCPropertyGridNumberProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridNumberProperty)

public:
	CMFCPropertyGridNumberProperty(const CString& strName, int value, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridNumberProperty();
	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);

	int GetMaxValue() const { return m_nMaxValue; }
	int GetMinValue() const { return m_nMinValue; }

protected:
	virtual BOOL TextToVar(const CString& strText);
};
