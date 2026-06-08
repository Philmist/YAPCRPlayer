#pragma once
#include "afxpropertygridctrl.h"


#ifndef AFX_PROP_HAS_LIST
	#define AFX_PROP_HAS_LIST 0x0001
#endif

class CMFCPropertyGridComboProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridComboProperty)
	friend class CMFCPropertyGridCtrl;

public:
	CMFCPropertyGridComboProperty(const CString& strName, int value, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	virtual ~CMFCPropertyGridComboProperty();

public:
	virtual CString FormatProperty();

	typedef std::pair<CString, int> ComboValue;
	typedef std::vector<ComboValue> ComboValues;

	BOOL AddComboValue(const CString& first, int second, BOOL bInsertUnique = TRUE);
	void SetComboValues(const ComboValues& values);
	void RemoveComboValues();

protected:
	// オーバーライド出来ないので外部から使われないようにprotectedに変える
	BOOL AddOption(LPCTSTR lpszOption, BOOL bInsertUnique = TRUE);
	void RemoveAllOptions();

	virtual BOOL TextToVar(const CString& strText);

	ComboValues values_;
};

