#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "BBSSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "TemplateComboBox.h"

// CDisplayBBSDlg ダイアログ

class CDisplayBBSDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayBBSDlg)

public:
	CDisplayBBSDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayBBSDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);
	void OnBnClickedBtnText(int index);
	void OnBnClickedBtnTitleText(int index);


// ダイアログ データ
	enum { IDD = IDD_DISPLAY_BBS };

	sl::BBSConfig bbs_;
	std::vector<sl::BBSPair> custom_;

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
	afx_msg void OnBnClickedBtnText1();
	afx_msg void OnBnClickedBtnFont();
	afx_msg void OnEnChangeEditMarginX();
	afx_msg void OnEnChangeEditMarginY();
	afx_msg void OnBnClickedCheckTitleBold();
	afx_msg void OnBnClickedBtnTitleText1();
	afx_msg void OnBnClickedBtnEditFont();
	afx_msg void OnBnClickedBtnSample();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnCbnSelchangeCmbCustom();
	afx_msg void OnDestroy();

	CBBSSampleView m_xcSample;
	CColorButton m_xcBtnColor[4];
	
	CColorButton m_xcBtnText[1];
	CFontButton m_xcBtnFont;

	CSpinButtonCtrl m_xcSpinMarginX;
	CSpinButtonCtrl m_xcSpinMarginY;

	CButton m_xcCheckTitleBold;
	CColorButton m_xcBtnTitleText[1];

	CFontButton m_xcBtnEditFont;

	CTemplateComboBox m_xcCmbCustom;
};
