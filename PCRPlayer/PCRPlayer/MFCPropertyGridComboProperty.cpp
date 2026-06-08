#include "stdafx.h"
#include "MFCPropertyGridComboProperty.h"

IMPLEMENT_DYNAMIC(CMFCPropertyGridComboProperty, CMFCPropertyGridProperty)

// Simple property
CMFCPropertyGridComboProperty::CMFCPropertyGridComboProperty(const CString& strName, int value, LPCTSTR lpszDescr, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, _variant_t(value), lpszDescr, dwData, NULL, NULL, NULL)
{
	m_bAllowEdit = FALSE;
}

CMFCPropertyGridComboProperty::~CMFCPropertyGridComboProperty()
{
}

BOOL CMFCPropertyGridComboProperty::AddComboValue(const CString& first, int second, BOOL bInsertUnique)
{
	if (CMFCPropertyGridProperty::AddOption(first, bInsertUnique))
	{
		values_.push_back(std::make_pair(first, second));
		return TRUE;
	}
	return FALSE;
}

void CMFCPropertyGridComboProperty::SetComboValues(const ComboValues& values)
{
	values_ = values;
	CMFCPropertyGridProperty::RemoveAllOptions();
	for (auto it = values.begin(); it != values.end(); ++it)
	{
		CMFCPropertyGridProperty::AddOption(it->first);
	}
}

void CMFCPropertyGridComboProperty::RemoveComboValues()
{
	values_.clear();
	CMFCPropertyGridProperty::RemoveAllOptions();
}

CString CMFCPropertyGridComboProperty::FormatProperty()
{
	ASSERT(GetValue().vt == VT_INT);
	const int v = GetValue().intVal;

	for (auto it = values_.begin(); it != values_.end(); ++it)
	{
		if (v == it->second) { return it->first; }
	}
	return CString();
}

BOOL CMFCPropertyGridComboProperty::TextToVar(const CString& strText)
{
	for (auto it = values_.begin(); it != values_.end(); ++it)
	{
		if (strText == it->first)
		{
			m_varValue = variant_t(it->second);
			return TRUE;
		}
	}
	return FALSE;
}
