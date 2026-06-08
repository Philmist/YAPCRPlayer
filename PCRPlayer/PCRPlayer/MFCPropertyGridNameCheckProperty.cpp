#include "stdafx.h"

#include "MFCPropertyGridNameCheckProperty.h"

IMPLEMENT_DYNAMIC(CMFCPropertyGridNameCheckProperty, CMFCPropertyGridProperty)


CMFCPropertyGridNameCheckProperty::CMFCPropertyGridNameCheckProperty(bool check, const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr, DWORD_PTR dwData,
	LPCTSTR lpszEditMask, LPCTSTR lpszEditTemplate, LPCTSTR lpszValidChars)
	: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
	, check_(check)
{
	m_rectCheck.SetRectEmpty();
	m_bEnabled = check ? TRUE : FALSE;
}

CMFCPropertyGridNameCheckProperty::~CMFCPropertyGridNameCheckProperty()
{

}

bool CMFCPropertyGridNameCheckProperty::GetCheck()
{
	return check_;
}

void CMFCPropertyGridNameCheckProperty::SetCheck(bool check)
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

void CMFCPropertyGridNameCheckProperty::OnDrawName(CDC* pDC, CRect rect)
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

void CMFCPropertyGridNameCheckProperty::OnClickName(CPoint point)
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

BOOL CMFCPropertyGridNameCheckProperty::OnDblClk(CPoint point)
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

void CMFCPropertyGridNameCheckProperty::OnDrawCheckBox(CDC * pDC, CRect rect, bool bChecked)
{
	COLORREF clrTextOld = pDC->GetTextColor();

	CMFCVisualManager::GetInstance()->OnDrawCheckBox(pDC, rect, FALSE, bChecked, /*m_bEnabled*/TRUE);

	pDC->SetTextColor(clrTextOld);
}

//BOOL CMFCPropertyGridNameCheckProperty::PushChar(UINT nChar)
//{
//	if (nChar == VK_SPACE)
//	{
//		OnDblClk(CPoint(-1, -1));
//	}
//
//	return TRUE;
//}
//
