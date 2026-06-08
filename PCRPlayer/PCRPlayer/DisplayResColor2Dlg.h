#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"

// CDisplayResColor2Dlg ダイアログ

class CDisplayResColor2Dlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResColor2Dlg)

public:
	//CDisplayResColor2Dlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResColor2Dlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResColor2Dlg();

	void InitControls();
	void OnBnClickedBtnColorText(int index);
	void OnBnClickedBtnColorLink(int index);
	void OnBnClickedBtnColorSage(int index);
	void OnBnClickedCheckBold(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_COLOR2 };

	sl::ResConfig& res_;
	CResSampleView& m_xcSample;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnColorText1();
	afx_msg void OnBnClickedBtnColorText2();
	afx_msg void OnBnClickedBtnColorText3();
	afx_msg void OnBnClickedBtnColorText4();
	afx_msg void OnBnClickedBtnColorLink1();
	afx_msg void OnBnClickedBtnColorLink2();
	afx_msg void OnBnClickedBtnColorLink3();
	afx_msg void OnBnClickedBtnColorLink4();
	afx_msg void OnBnClickedBtnColorSage1();
	afx_msg void OnBnClickedBtnColorSage2();
	afx_msg void OnBnClickedBtnColorSage3();
	afx_msg void OnBnClickedBtnColorSage4();
	afx_msg void OnBnClickedCheckBold1();
	afx_msg void OnBnClickedCheckBold2();
	afx_msg void OnBnClickedCheckBold3();
	afx_msg void OnBnClickedCheckBold4();

	CColorButton m_xcBtnColorText[4];
	CColorButton m_xcBtnColorLink[4];
	CColorButton m_xcBtnColorSage[4];
	CButton m_xcCheckBold[4];
};
