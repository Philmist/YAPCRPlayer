#pragma once

#include "MFCPropertyGridOperator.h"

namespace utl {

// CBasePropDlg
struct PropSetting {
	PropSetting()
		: check(true)
		, width(-1)
	{}
	bool check;
	int width;
};

} // namespace utl

// CBasePropDlg ダイアログ

class CBasePropDlg : public CDialog
{
	DECLARE_DYNAMIC(CBasePropDlg)

public:
	//CBasePropDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CBasePropDlg(utl::PropSetting& setting, CWnd* pParent = NULL);
	virtual ~CBasePropDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BASE_PROP };
#endif

protected:
	utl::PropSetting& setting_;
	CMFCPropertyGridOperator prop_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnBnClickedCheckDesc();

protected:
	CStatic m_xcPropPos;
	CMFCPropertyGridCtrl m_xcProp;
	CButton m_xcCheckDesc;

// オーバーライド
protected:
	virtual void OnGetProp() = 0;
	virtual void OnSetProp() = 0;
};
