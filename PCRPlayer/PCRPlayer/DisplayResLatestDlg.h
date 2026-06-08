#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "afxcmn.h"
#include "afxwin.h"

// CDisplayResLatestDlg ダイアログ

class CDisplayResLatestDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResLatestDlg)

public:
	//CDisplayResLatestDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResLatestDlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResLatestDlg();

	void InitControls();
	void OnBnClickedBtnColor(int index);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_LATEST };

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

	CColorButton m_xcBtnColor[6];
};
