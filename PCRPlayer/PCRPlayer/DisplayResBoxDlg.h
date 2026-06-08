#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "afxcmn.h"
#include "afxwin.h"

// CDisplayResBoxDlg ダイアログ

class CDisplayResBoxDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResBoxDlg)

public:
	//CDisplayResBoxDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResBoxDlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResBoxDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_BOX };

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
	afx_msg void OnEnChangeEditSpace();
	afx_msg void OnBnClickedCheckPunct();
	afx_msg void OnEnChangeEditSeparate();

	CColorButton m_xcBtnColor[6];
	CSpinButtonCtrl m_xcSpinMarginX;
	CSpinButtonCtrl m_xcSpinMarginY;
	CSpinButtonCtrl m_xcSpinSpace;
	CButton m_xcCheckPunct;
	CSpinButtonCtrl m_xcSpinSeparate;
};
