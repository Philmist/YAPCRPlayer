#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "SeekSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "templatecombobox.h"


// CDisplaySeekDlg ダイアログ

class CDisplaySeekDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplaySeekDlg)

public:
	CDisplaySeekDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplaySeekDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);
	void OnBnClickedBtnText(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_SEEK };

	sl::SeekConfig seek_;
	std::vector<sl::SeekPair> custom_;

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
	afx_msg void OnBnClickedBtnColor7();
	afx_msg void OnBnClickedBtnColor8();
	afx_msg void OnBnClickedBtnColor9();
	afx_msg void OnBnClickedBtnColor10();
	afx_msg void OnBnClickedBtnText1();
	afx_msg void OnBnClickedBtnText2();
	afx_msg void OnBnClickedBtnFont();
	afx_msg void OnEnChangeEditMarginX();
	afx_msg void OnEnChangeEditMarginY();
	afx_msg void OnEnChangeEditKnob();
	afx_msg void OnBnClickedBtnSample();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnCbnSelchangeCmbCustom();
	afx_msg void OnDestroy();

	CSeekSampleView m_xcSample;
	CColorButton m_xcBtnColor[10];
	
	CColorButton m_xcBtnText[2];
	CFontButton m_xcBtnFont;

	CSimpleEdit m_xcEditMarginX;
	CSpinButtonCtrl m_xcSpinMarginX;
	CSimpleEdit m_xcEditMarginY;
	CSpinButtonCtrl m_xcSpinMarginY;
	CSimpleEdit m_xcEditKnob;
	CSpinButtonCtrl m_xcSpinKnob;

	CTemplateComboBox m_xcCmbCustom;
};
