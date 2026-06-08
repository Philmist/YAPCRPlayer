#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "CustomListCtrl.h"

// CDisplayAspectDlg ダイアログ

class CDisplayAspectDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayAspectDlg)

public:
	CDisplayAspectDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayAspectDlg();

	void InitAspectControls();
	void EnableAspectControls(BOOL enable);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_ASPECT };

	bool orderAspect_;

	std::vector<sl::WindowAspect> aspect_;

	utl::WindowAspectList listAspect_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnItemchangedListAspect(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListAspect(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditAspect1();
	afx_msg void OnEnChangeEditAspect2();
	afx_msg void OnBnClickedBtnAspectUp();
	afx_msg void OnBnClickedBtnAspectDown();

	CTemplateListCtrl m_xcListAspect;
	CButton m_xcBtnAspectUp;
	CButton m_xcBtnAspectDown;
	CSimpleEdit m_xcEditAspect1;
	CSpinButtonCtrl m_xcSpinAspect1;
	CSimpleEdit m_xcEditAspect2;
	CSpinButtonCtrl m_xcSpinAspect2;

	afx_msg void OnBnClickedBtnAspectAllDefault();
};
