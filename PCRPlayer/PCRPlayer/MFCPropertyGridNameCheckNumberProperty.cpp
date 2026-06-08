#include "stdafx.h"

#include "MFCPropertyGridNameCheckNumberProperty.h"

IMPLEMENT_DYNAMIC(CMFCPropertyGridNameCheckNumberProperty, CMFCPropertyGridProperty)


class NameCheckNumberPropertyEdit : public CEdit
{
public:
	NameCheckNumberPropertyEdit(const CMFCPropertyGridNameCheckNumberProperty *pProp)
		: m_pProp(pProp)
	{}

protected:
	const CMFCPropertyGridNameCheckNumberProperty *m_pProp;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChange();
};

BEGIN_MESSAGE_MAP(NameCheckNumberPropertyEdit, CEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, &NameCheckNumberPropertyEdit::OnEnChange)
END_MESSAGE_MAP()

void NameCheckNumberPropertyEdit::OnEnChange()
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


CMFCPropertyGridNameCheckNumberProperty::CMFCPropertyGridNameCheckNumberProperty(
	bool check, const CString& strName, int value, LPCTSTR lpszDescr, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, variant_t(value), lpszDescr, dwData)
	, check_(check)
{
	m_rectCheck.SetRectEmpty();
	m_bEnabled = check ? TRUE : FALSE;

	EnableSpinControl(TRUE, 0, INT_MAX);
}

CMFCPropertyGridNameCheckNumberProperty::~CMFCPropertyGridNameCheckNumberProperty()
{

}

CWnd* CMFCPropertyGridNameCheckNumberProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	NameCheckNumberPropertyEdit *edit = new NameCheckNumberPropertyEdit(this);
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_NUMBER;
	if (!m_bEnabled) { dwStyle |= ES_READONLY; }

	edit->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
	bDefaultFormat = TRUE;
	return edit;
}

bool CMFCPropertyGridNameCheckNumberProperty::GetCheck()
{
	return check_;
}

void CMFCPropertyGridNameCheckNumberProperty::SetCheck(bool check)
{
	check_ = check;
	if (check_)
	{
		Enable(TRUE);
	}
	else
	{
		Enable(FALSE);
	}
}

void CMFCPropertyGridNameCheckNumberProperty::OnDrawName(CDC* pDC, CRect rect)
{
	m_rectCheck = rect;
	m_rectCheck.DeflateRect(1, 1);

	m_rectCheck.right = m_rectCheck.left + m_rectCheck.Height();

	rect.left = m_rectCheck.right + 1;

	//COLORREF clrTextOld = pDC->SetTextColor(GetGlobalData()->clrBtnText);

	CMFCPropertyGridProperty::OnDrawName(pDC, rect);

	OnDrawCheckBox(pDC, m_rectCheck, check_);

	//pDC->SetTextColor(clrTextOld);
}

void CMFCPropertyGridNameCheckNumberProperty::OnClickName(CPoint point)
{
	if (m_rectCheck.PtInRect(point))
	{
		if (check_)
		{
			Enable(FALSE);
		}
		else
		{
			Enable(TRUE);
		}

		check_ = !check_;
		m_pWndList->InvalidateRect(m_rectCheck);
	}
}

BOOL CMFCPropertyGridNameCheckNumberProperty::OnDblClk(CPoint point)
{
	if (m_rectCheck.PtInRect(point))
	{
		OnEndEdit();
		if (check_)
		{
			Enable(FALSE);
		}
		else
		{
			Enable(TRUE);
		}

		check_ = !check_;
		m_pWndList->InvalidateRect(m_rectCheck);
		return TRUE;
	}

	//m_varValue.boolVal = !(m_varValue.boolVal);
	//m_pWndList->InvalidateRect(m_rectCheck);
	return TRUE;
}

void CMFCPropertyGridNameCheckNumberProperty::OnDrawCheckBox(CDC * pDC, CRect rect, bool bChecked)
{
	COLORREF clrTextOld = pDC->GetTextColor();

	CMFCVisualManager::GetInstance()->OnDrawCheckBox(pDC, rect, FALSE, bChecked, /*m_bEnabled*/TRUE);

	pDC->SetTextColor(clrTextOld);
}

//BOOL CMFCPropertyGridNameCheckNumberProperty::PushChar(UINT nChar)
//{
//	if (nChar == VK_SPACE)
//	{
//		OnDblClk(CPoint(-1, -1));
//	}
//
//	return TRUE;
//}
//



BOOL CMFCPropertyGridNameCheckNumberProperty::TextToVar(const CString& strText)
{
	int val = _wtoi((LPCTSTR)strText);
	m_varValue = (variant_t)val;
	return TRUE;
}

