#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "afxcmn.h"

// CDisplayResBackDlg ダイアログ

class CDisplayResBackDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResBackDlg)

public:
	//CDisplayResBackDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResBackDlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResBackDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_BACK };

	sl::ResConfig& res_;
	CResSampleView& m_xcSample;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnColor1();
	afx_msg void OnBnClickedBtnColor2();
	afx_msg void OnBnClickedBtnColor3();
	afx_msg void OnBnClickedBtnColor4();
	afx_msg void OnBnClickedBtnColor5();
	afx_msg void OnBnClickedBtnColor6();
	afx_msg void OnEnChangeEditMarginX();
	afx_msg void OnEnChangeEditMarginY();
	afx_msg void OnBnClickedCheckOpaque();
	afx_msg void OnEnChangeEditOpaqueText();
	afx_msg void OnEnChangeEditOpaqueBack();

	CColorButton m_xcBtnColor[6];
	CSpinButtonCtrl m_xcSpinMarginX;
	CSpinButtonCtrl m_xcSpinMarginY;

	CButton m_xcCheckOpaque;
	CSimpleEdit m_xcEditOpaqueText;
	CSpinButtonCtrl m_xcSpinOpaqueText;
	CSimpleEdit m_xcEditOpaqueBack;
	CSpinButtonCtrl m_xcSpinOpaqueBack;
};
