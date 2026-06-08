#include "stdafx.h"
#include "MFCPropertyGridCheckProperty.h"

IMPLEMENT_DYNAMIC(CMFCPropertyGridCheckProperty, CMFCPropertyGridProperty)

CMFCPropertyGridCheckProperty::CMFCPropertyGridCheckProperty(const CString& strName, bool value, LPCTSTR lpszDescr, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, _variant_t(value), lpszDescr, dwData, NULL, NULL, NULL)
{
	m_bAllowEdit = FALSE;
}


CMFCPropertyGridCheckProperty::~CMFCPropertyGridCheckProperty()
{
}


BOOL CMFCPropertyGridCheckProperty::IsBoolValue() const
{
	if (m_varValue.vt == VT_BOOL)
	{
		return TRUE;
	}
	return FALSE;
}

void CMFCPropertyGridCheckProperty::ToggleBool()
{
	ASSERT(IsBoolValue());
	m_varValue.boolVal = m_varValue.boolVal ? VARIANT_FALSE : VARIANT_TRUE;
	EnableLinks();

	m_pWndList->OnPropertyChanged(this);
	m_pWndList->InvalidateRect(GetRect());
}

void CMFCPropertyGridCheckProperty::GetCheckRect(CRect *pRcCheck)
{
	CRect rcSpin;
	AdjustInPlaceEditRect(*pRcCheck, rcSpin);
	pRcCheck->right = pRcCheck->left + pRcCheck->Height();
}


void CMFCPropertyGridCheckProperty::AddLink(CMFCPropertyGridProperty* prop, BOOL reverse)
{
	ASSERT(IsBoolValue());
	BOOL enable = m_varValue.boolVal ? TRUE : FALSE;

	if (prop)
	{
		prop->Enable(reverse ? (enable ? FALSE : TRUE) : enable);
		links_.insert(std::make_pair(prop, reverse));
	}
}

void CMFCPropertyGridCheckProperty::ClearLinks()
{
	links_.clear();
}

void CMFCPropertyGridCheckProperty::EnableLinks()
{
	ASSERT(IsBoolValue());
	BOOL enable = m_varValue.boolVal ? TRUE : FALSE;

	for (auto it = links_.begin(); it != links_.end(); ++it)
	{
		if (it->first)
		{
			it->first->Enable(it->second ? (enable ? FALSE : TRUE) : enable);
		}
	}
}

//virtual
void CMFCPropertyGridCheckProperty::OnDrawValue(CDC* pDC, CRect rect)
{
	if (IsBoolValue())
	{
		check_ = rect;
		check_.DeflateRect(1, 1);
		check_.right = check_.left + check_.Height();

		COLORREF clrTextOld = pDC->GetTextColor();
		CMFCVisualManager::GetInstance()->OnDrawCheckBox(pDC, check_, FALSE, m_varValue.boolVal, m_bEnabled);
		pDC->SetTextColor(clrTextOld);

		rect.left += rect.Height();
	}
	__super::OnDrawValue(pDC, rect);
}

//virtual
CWnd* CMFCPropertyGridCheckProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	if (IsBoolValue())
	{
		CWnd *wnd = CMFCPropertyGridProperty::CreateInPlaceEdit(rectEdit, bDefaultFormat);   
		if (wnd) {wnd -> ShowWindow(SW_HIDE); }
		return wnd;

		//rectEdit.left += m_Rect.Height();
	}
	return __super::CreateInPlaceEdit(rectEdit, bDefaultFormat);
}

//virtual
BOOL CMFCPropertyGridCheckProperty::OnClickValue(UINT uiMsg, CPoint point)
{
	if (m_bEnabled && IsBoolValue())
	{
		//OnEndEdit();//m_pWndList->EndEditItem();
		//ToggleBool();
		//return TRUE;
		OnEndEdit();

		CRect rcCheck;
		GetCheckRect(&rcCheck);
		if(rcCheck.PtInRect(point))
		{
			ToggleBool();
			return TRUE;
		}
		return FALSE;
	}
	return __super::OnClickValue(uiMsg,point);
}

//virtual
BOOL CMFCPropertyGridCheckProperty::OnDblClk(CPoint point)
{
	if (m_bEnabled && IsBoolValue())
	{
		OnEndEdit();//m_pWndList->EndEditItem();
		ToggleBool();
		return TRUE;
	}
	return __super::OnDblClk(point);
}



//virtual
BOOL CMFCPropertyGridCheckProperty::PushChar(UINT nChar)
{
	if (IsBoolValue() && nChar == VK_SPACE)
	{
		OnEndEdit();//m_pWndList->EndEditItem();
		ToggleBool();
		return TRUE;
	}
	return __super::PushChar(nChar);
}

//virtual
BOOL CMFCPropertyGridCheckProperty::HasButton() const
{
	if (IsBoolValue())
	{
		return FALSE;
	}
	return __super::HasButton();
}