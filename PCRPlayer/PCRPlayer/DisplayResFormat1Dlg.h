#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "afxwin.h"
#include "afxcmn.h"

// CDisplayResFormat1Dlg ダイアログ

class CDisplayResFormat1Dlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResFormat1Dlg)

public:
	//CDisplayResFormat1Dlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResFormat1Dlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResFormat1Dlg();

	void InitControls();

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_FORMAT1 };

	sl::ResConfig& res_;
	CResSampleView& m_xcSample;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeCmbType();
	afx_msg void OnBnClickedBtnFont();
	afx_msg void OnEnChangeEditSpace();
	afx_msg void OnEnChangeEditWidth();


	CComboBox m_xcCmbType;
	CFontButton m_xcBtnFont;
	CSpinButtonCtrl m_xcSpinSpace;
	CSpinButtonCtrl m_xcSpinWidth;
};
