#pragma once
#include "afxpropertygridctrl.h"

class CMFCPropertyGridNameCheckNumberProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridNameCheckNumberProperty)

public:
	CMFCPropertyGridNameCheckNumberProperty(bool check, const CString& strName, int value, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridNameCheckNumberProperty();

	bool GetCheck();
	void SetCheck(bool check);

	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);

	int GetMaxValue() const { return m_nMaxValue; }
	int GetMinValue() const { return m_nMinValue; }

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

	virtual BOOL TextToVar(const CString& strText);

protected:
	bool check_;
	CRect m_rectCheck;
};
