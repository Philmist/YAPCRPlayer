#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"

// CDisplayResColor3Dlg ダイアログ

class CDisplayResColor3Dlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResColor3Dlg)

public:
	//CDisplayResColor3Dlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResColor3Dlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResColor3Dlg();

	void InitControls();
	void OnBnClickedBtnColorText(int index);
	void OnBnClickedBtnColorLink(int index);
	void OnBnClickedBtnColorSage(int index);
	void OnBnClickedCheckBold(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_COLOR3 };

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
	afx_msg void OnBnClickedBtnColorLink1();
	afx_msg void OnBnClickedBtnColorLink2();
	afx_msg void OnBnClickedBtnColorLink3();
	afx_msg void OnBnClickedBtnColorSage1();
	afx_msg void OnBnClickedBtnColorSage2();
	afx_msg void OnBnClickedBtnColorSage3();
	afx_msg void OnBnClickedCheckBold1();
	afx_msg void OnBnClickedCheckBold2();
	afx_msg void OnBnClickedCheckBold3();

	CColorButton m_xcBtnColorText[3];
	CColorButton m_xcBtnColorLink[3];
	CColorButton m_xcBtnColorSage[3];
	CButton m_xcCheckBold[3];
};
