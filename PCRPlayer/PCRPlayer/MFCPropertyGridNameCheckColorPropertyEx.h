#pragma once
#include "afxpropertygridctrl.h"

class CMFCPropertyGridNameCheckColorPropertyEx : public CMFCPropertyGridColorProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridNameCheckColorPropertyEx)

public:
	CMFCPropertyGridNameCheckColorPropertyEx(bool check, const CString& strName, const COLORREF& color, CPalette* pPalette = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridNameCheckColorPropertyEx();

	bool GetCheck();
	void SetCheck(bool check);

protected:
	//virtual BOOL OnEdit(LPPOINT /*lptClick*/) { return FALSE; }
	//virtual void OnDrawButton(CDC* /*pDC*/, CRect /*rectButton*/) {}
	//virtual void OnDrawValue(CDC* /*pDC*/, CRect /*rect*/) {}
	//virtual BOOL HasButton() const { return FALSE; }

	//virtual BOOL PushChar(UINT nChar);
	virtual void OnDrawCheckBox(CDC * pDC, CRect rectCheck, bool bChecked);
	virtual void OnDrawName(CDC* pDC, CRect rect);
	virtual void OnClickName(CPoint point);
	virtual BOOL OnDblClk(CPoint point);

public:
	virtual void OnClickButton(CPoint point);

protected:
	bool check_;
	CRect m_rectCheck;
};

