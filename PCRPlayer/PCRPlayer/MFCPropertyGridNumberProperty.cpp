#include "stdafx.h"
#include "MFCPropertyGridNumberProperty.h"

IMPLEMENT_DYNAMIC(CMFCPropertyGridNumberProperty, CMFCPropertyGridProperty)

class NumberPropertyEdit : public CEdit
{
public:
	NumberPropertyEdit(const CMFCPropertyGridNumberProperty *pProp)
		: m_pProp(pProp)
	{}

protected:
	const CMFCPropertyGridNumberProperty *m_pProp;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChange();
};

BEGIN_MESSAGE_MAP(NumberPropertyEdit, CEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, &NumberPropertyEdit::OnEnChange)
END_MESSAGE_MAP()

void NumberPropertyEdit::OnEnChange()
{
	CString str;
	GetWindowText(str);

	int value = _wtoi((LPCTSTR)str);

	bool change = false;
	if (value > m_pProp->GetMaxValue())
	{
		change = true;
		value = m_pProp->GetMaxValue();
	}
	else if(value < m_pProp->GetMinValue())
	{
		change = true;
		value = m_pProp->GetMinValue();
	}

	if (change)
	{
		str.Format(_T("%d"), value);
		SetSel(0, -1);
		ReplaceSel(str);
	}
}



CMFCPropertyGridNumberProperty::CMFCPropertyGridNumberProperty(
	const CString& strName, int value, LPCTSTR lpszDescr, DWORD_PTR dwData)
	:CMFCPropertyGridProperty(strName, variant_t(value), lpszDescr, dwData)
{
	EnableSpinControl(TRUE, 0, INT_MAX);
}

CMFCPropertyGridNumberProperty::~CMFCPropertyGridNumberProperty()
{
}

CWnd* CMFCPropertyGridNumberProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	NumberPropertyEdit *edit = new NumberPropertyEdit(this);
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_NUMBER;
	if (!m_bEnabled) { dwStyle |= ES_READONLY; }

	edit->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
	bDefaultFormat = TRUE;
	return edit;
}

BOOL CMFCPropertyGridNumberProperty::TextToVar(const CString& strText)
{
	int val = _wtoi((LPCTSTR)strText);
	m_varValue = (variant_t)val;
	return TRUE;
}
