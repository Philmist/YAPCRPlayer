#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "StatusSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "TemplateComboBox.h"

// CDisplayStatusDlg ダイアログ

class CDisplayStatusDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayStatusDlg)

public:
	CDisplayStatusDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayStatusDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);
	void OnBnClickedBtnText(int index);


// ダイアログ データ
	enum { IDD = IDD_DISPLAY_STATUS };

	sl::StatusConfig status_;
	std::vector<sl::StatusPair> custom_;

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
	afx_msg void OnBnClickedBtnSample();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnCbnSelchangeCmbCustom();
	afx_msg void OnDestroy();

	CStatusSampleView m_xcSample;
	CColorButton m_xcBtnColor[4];
	
	CColorButton m_xcBtnText[1];
	CFontButton m_xcBtnFont;

	CSimpleEdit m_xcEditMarginX;
	CSpinButtonCtrl m_xcSpinMarginX;
	CSimpleEdit m_xcEditMarginY;
	CSpinButtonCtrl m_xcSpinMarginY;

	CTemplateComboBox m_xcCmbCustom;
};
