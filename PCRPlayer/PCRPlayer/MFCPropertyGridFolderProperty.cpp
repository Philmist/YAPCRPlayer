#include "stdafx.h"
#include "MFCPropertyGridFolderProperty.h"


#ifndef AFX_PROP_HAS_BUTTON
	#define AFX_PROP_HAS_BUTTON 0x0002
#endif

IMPLEMENT_DYNAMIC(CMFCPropertyGridFolderProperty, CMFCPropertyGridProperty)

CMFCPropertyGridFolderProperty::CMFCPropertyGridFolderProperty(
	const CString& strName, const CString& strFolderName,
	LPCTSTR lpszTitle, UINT ulFlags,
	LPCTSTR lpszDescr, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, COleVariant((LPCTSTR)strFolderName), lpszDescr , dwData)
	//, m_bIsFolder(TRUE)
	, m_strTitle(lpszTitle)
	, m_ulFlags(ulFlags)
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
}

//CMFCPropertyGridFolderProperty::CMFCPropertyGridFolderProperty( const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, LPCTSTR lpszDefExt, DWORD dwFileFlags, LPCTSTR lpszFilter, LPCTSTR lpszDescr, DWORD_PTR dwData) : CMFCPropertyGridProperty(strName, COleVariant((LPCTSTR)strFileName), lpszDescr, dwData), m_bOpenFileDialog(bOpenFileDialog), m_dwFileOpenFlags(dwFileFlags)
//{
//	m_dwFlags = AFX_PROP_HAS_BUTTON;
//	m_strDefExt = lpszDefExt == NULL ? _T("") : lpszDefExt;
//	m_strFilter = lpszFilter == NULL ? _T("") : lpszFilter;
//
//	m_bIsFolder  = FALSE;
//}

CMFCPropertyGridFolderProperty::~CMFCPropertyGridFolderProperty()
{
}

void CMFCPropertyGridFolderProperty::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	m_bButtonIsDown = TRUE;
	Redraw();

	CString strPath = m_varValue.bstrVal;
	BOOL bUpdate = FALSE;

	//if (m_bIsFolder)
	{
		if (afxShellManager == NULL)
		{
			CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
			if (pApp != NULL)
			{
				pApp->InitShellManager();
			}
		}

		if (afxShellManager == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			bUpdate = afxShellManager->BrowseForFolder(strPath, m_pWndList, strPath, m_strTitle, m_ulFlags);
		}
	}
	//else
	//{
	//	CFileDialog dlg(m_bOpenFileDialog, m_strDefExt, strPath, m_dwFileOpenFlags, m_strFilter, m_pWndList);
	//	if (dlg.DoModal() == IDOK)
	//	{
	//		bUpdate = TRUE;
	//		strPath = dlg.GetPathName();
	//	}
	//}

	if (bUpdate)
	{
		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText(strPath);
		}

		m_varValue = (LPCTSTR) strPath;
	}

	m_bButtonIsDown = FALSE;
	Redraw();

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus();
	}
	else
	{
		m_pWndList->SetFocus();
	}
}
