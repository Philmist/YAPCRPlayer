#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "afxwin.h"
#include "afxcmn.h"

// CDisplayResIDDlg ダイアログ

class CDisplayResIDDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResIDDlg)

public:
	//CDisplayResIDDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResIDDlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResIDDlg();

	void InitControls();

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_ID };

	sl::ResConfig& res_;
	CResSampleView& m_xcSample;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCheckEnable();
	afx_msg void OnEnChangeEditThreshold();

	CButton m_xcCheckEnable;
	CSpinButtonCtrl m_xcSpinThreshold;
};
