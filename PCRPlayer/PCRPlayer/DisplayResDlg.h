#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "TemplateComboBox.h"

#include "DisplayResFormat1Dlg.h"
#include "DisplayResFormat2Dlg.h"
#include "DisplayResIDDlg.h"
#include "DisplayResColor1Dlg.h"
#include "DisplayResColor2Dlg.h"
#include "DisplayResColor3Dlg.h"
#include "DisplayResBoxDlg.h"
#include "DisplayResLatestDlg.h"
#include "DisplayResBackDlg.h"

// CDisplayResDlg ダイアログ

class CDisplayResDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResDlg)

public:
	CDisplayResDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayResDlg();

	void InitControls();

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES };

	int select_;

	struct Dialog {
		Dialog(sl::ResConfig& res, CResSampleView& sample)
			: format1(res, sample)
			, format2(res, sample)
			, id(res, sample)
			, color1(res, sample)
			, color2(res, sample)
			, color3(res, sample)
			, box(res, sample)
			, latest(res, sample)
			, back(res, sample)
		{}

		CDisplayResFormat1Dlg format1;
		CDisplayResFormat2Dlg format2;
		CDisplayResIDDlg id;
		CDisplayResColor1Dlg color1;
		CDisplayResColor2Dlg color2;
		CDisplayResColor3Dlg color3;
		CDisplayResBoxDlg box;
		CDisplayResLatestDlg latest;
		CDisplayResBackDlg back;
	};

	Dialog dlg_;

	struct Item {
		CDialog* dlg;
		int idd;
		int index;
		boost::function<void()> init;
	};

	std::vector<Item> vec_;
	CRect rect_;

	sl::ResConfig res_;
	std::vector<sl::ResPair> custom_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnSample();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnCbnSelchangeCmbCustom();
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);

	CResSampleView m_xcSample;
	CTemplateComboBox m_xcCmbCustom;
	CTabCtrl m_xcTab;
	
};
