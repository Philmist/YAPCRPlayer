#pragma once
#include "afxpropertygridctrl.h"


class CMFCPropertyGridNameCheckProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridNameCheckProperty)

public:
	CMFCPropertyGridNameCheckProperty(bool check, const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL);
	virtual ~CMFCPropertyGridNameCheckProperty();

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

protected:
	bool check_;
	CRect m_rectCheck;
};
