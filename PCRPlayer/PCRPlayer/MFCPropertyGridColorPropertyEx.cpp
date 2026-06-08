#include "stdafx.h"
#include "MFCPropertyGridColorPropertyEx.h"

#ifndef AFX_PROP_HAS_BUTTON
#define AFX_PROP_HAS_BUTTON 0x0002
#endif

IMPLEMENT_DYNAMIC(CMFCPropertyGridColorPropertyEx, CMFCPropertyGridColorProperty)

CMFCPropertyGridColorPropertyEx::CMFCPropertyGridColorPropertyEx(
	const CString& strName, const COLORREF& color, CPalette* pPalette, LPCTSTR lpszDescr, DWORD_PTR dwData)
	: CMFCPropertyGridColorProperty(strName, color, pPalette, lpszDescr, dwData)
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
}


CMFCPropertyGridColorPropertyEx::~CMFCPropertyGridColorPropertyEx()
{
}

void CMFCPropertyGridColorPropertyEx::OnClickButton(CPoint /*point*/)
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

