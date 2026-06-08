#include "stdafx.h"
#include "MFCPropertyGridNameCheckColorPropertyEx.h"

#ifndef AFX_PROP_HAS_BUTTON
#define AFX_PROP_HAS_BUTTON 0x0002
#endif

IMPLEMENT_DYNAMIC(CMFCPropertyGridNameCheckColorPropertyEx, CMFCPropertyGridColorProperty)

CMFCPropertyGridNameCheckColorPropertyEx::CMFCPropertyGridNameCheckColorPropertyEx(
	bool check, const CString& strName, const COLORREF& color, CPalette* pPalette, LPCTSTR lpszDescr, DWORD_PTR dwData)
	: CMFCPropertyGridColorProperty(strName, color, pPalette, lpszDescr, dwData)
	, check_(check)
{
	m_rectCheck.SetRectEmpty();
	m_bEnabled = check ? TRUE : FALSE;

	m_dwFlags = AFX_PROP_HAS_BUTTON;
}


CMFCPropertyGridNameCheckColorPropertyEx::~CMFCPropertyGridNameCheckColorPropertyEx()
{
}

bool CMFCPropertyGridNameCheckColorPropertyEx::GetCheck()
{
	return check_;
}

void CMFCPropertyGridNameCheckColorPropertyEx::SetCheck(bool check)
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

void CMFCPropertyGridNameCheckColorPropertyEx::OnDrawName(CDC* pDC, CRect rect)
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

void CMFCPropertyGridNameCheckColorPropertyEx::OnClickName(CPoint point)
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

BOOL CMFCPropertyGridNameCheckColorPropertyEx::OnDblClk(CPoint point)
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

void CMFCPropertyGridNameCheckColorPropertyEx::OnDrawCheckBox(CDC * pDC, CRect rect, bool bChecked)
{
	COLORREF clrTextOld = pDC->GetTextColor();

	CMFCVisualManager::GetInstance()->OnDrawCheckBox(pDC, rect, FALSE, bChecked, /*m_bEnabled*/TRUE);

	pDC->SetTextColor(clrTextOld);
}

//BOOL CMFCPropertyGridNameCheckColorPropertyEx::PushChar(UINT nChar)
//{
//	if (nChar == VK_SPACE)
//	{
//		OnDblClk(CPoint(-1, -1));
//	}
//
//	return TRUE;
//}
//



void CMFCPropertyGridNameCheckColorPropertyEx::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	COLORREF nColorPrev = m_Color;

	m_bButtonIsDown = TRUE;
	Redraw();

#pragma warning(disable : 4244)

	CColorDialog dlg(m_Color);

#pragma warning(default : 4244)

	dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;

	if (dlg.DoModal() == IDOK)
	{
		m_Color = dlg.GetColor();

		if (nColorPrev != m_Color)
		{
			m_pWndList->OnPropertyChanged(this);
		}

		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText(FormatProperty());
		}
		else
		{
			m_varValue = (LPCTSTR) FormatProperty();
		}
	}

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus();
	}
	else
	{
		m_pWndList->SetFocus();
	}

	m_bButtonIsDown = FALSE;
	Redraw();
}

