#pragma once

#include "afxpropertygridctrl.h"

class CMFCPropertyGridFolderProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridFolderProperty)

	// Construction
public:

	CMFCPropertyGridFolderProperty(
		const CString& strName, const CString& strFolderName,
		LPCTSTR lpszTitle = NULL, UINT ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	//CMFCPropertyGridFolderProperty(const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, LPCTSTR lpszDefExt = NULL,
	//	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridFolderProperty();

	// Overrides
public:
	virtual void OnClickButton(CPoint point);

	// Attributes
protected:
	//BOOL    m_bIsFolder;

	// File open dialog atributes:
	//BOOL    m_bOpenFileDialog; // TRUE - use "File Open/Save" diaog; otherwise - folder selection dialog
	//DWORD   m_dwFileOpenFlags;
	//CString m_strDefExt;
	//CString m_strFilter;

	UINT m_ulFlags;
	CString m_strTitle;
};
