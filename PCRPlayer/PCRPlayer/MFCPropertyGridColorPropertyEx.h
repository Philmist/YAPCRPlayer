#pragma once
#include "afxpropertygridctrl.h"

class CMFCPropertyGridColorPropertyEx : public CMFCPropertyGridColorProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridColorPropertyEx)

public:
	CMFCPropertyGridColorPropertyEx(const CString& strName, const COLORREF& color, CPalette* pPalette = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridColorPropertyEx();

public:
	virtual void OnClickButton(CPoint point);
};

