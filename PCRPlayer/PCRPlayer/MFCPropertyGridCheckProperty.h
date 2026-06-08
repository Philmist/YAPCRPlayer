#pragma once
#include "afxpropertygridctrl.h"

// CMFCPropertyGridCtrl::SetBoolLabels(LPCTSTR lpszTrue, LPCTSTR lpszFalse);

class CMFCPropertyGridCheckProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridCheckProperty)

public:
	CMFCPropertyGridCheckProperty(const CString& strName, bool value, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridCheckProperty();

	BOOL IsBoolValue() const;
	void ToggleBool();
	void GetCheckRect(CRect *pRcCheck);
	
	void AddLink(CMFCPropertyGridProperty* prop, BOOL reverse = FALSE);
	void ClearLinks();
	void EnableLinks();

public:
	virtual void OnDrawValue(CDC* pDC, CRect rect);

	virtual CWnd* CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat);

	virtual BOOL OnClickValue (UINT uiMsg, CPoint point);
	virtual BOOL OnDblClk (CPoint point);

	virtual BOOL PushChar (UINT nChar);

protected:
	virtual BOOL HasButton() const;

	CRect check_;
	std::map<CMFCPropertyGridProperty*, BOOL> links_;
};

