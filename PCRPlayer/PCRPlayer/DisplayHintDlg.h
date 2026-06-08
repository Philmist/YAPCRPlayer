#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "HintSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "templatecombobox.h"

// CDisplayHintDlg ダイアログ

class CDisplayHintDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayHintDlg)

public:
	CDisplayHintDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayHintDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);
	void OnBnClickedBtnText(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_HINT };

	sl::HintConfig hint_;
	std::vector<sl::HintPair> custom_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnColor1();
	afx_msg void OnBnClickedBtnColor2();
	afx_msg void OnBnClickedBtnColor3();
	afx_msg void OnBnClickedBtnColor4();
	afx_msg void OnBnClickedBtnColor5();
	afx_msg void OnBnClickedBtnColor6();
	afx_msg void OnBnClickedBtnText1();
	afx_msg void OnBnClickedBtnFont();
	afx_msg void OnEnChangeEditMarginX();
	afx_msg void OnEnChangeEditMarginY();
	afx_msg void OnBnClickedCheckOpaque();
	afx_msg void OnEnChangeEditOpaqueText();
	afx_msg void OnEnChangeEditOpaqueBack();
	afx_msg void OnBnClickedBtnSample();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnCbnSelchangeCmbCustom();
	afx_msg void OnDestroy();

	CHintSampleView m_xcSample;
	CColorButton m_xcBtnColor[6];
	
	CColorButton m_xcBtnText[1];
	CFontButton m_xcBtnFont;

	CSpinButtonCtrl m_xcSpinMarginX;
	CSpinButtonCtrl m_xcSpinMarginY;

	CButton m_xcCheckOpaque;
	CSimpleEdit m_xcEditOpaqueText;
	CSpinButtonCtrl m_xcSpinOpaqueText;
	CSimpleEdit m_xcEditOpaqueBack;
	CSpinButtonCtrl m_xcSpinOpaqueBack;

	CTemplateComboBox m_xcCmbCustom;
};
