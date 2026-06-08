#pragma once

#include "ResSampleView.h"
#include "ColorButton.h"
#include "FontButton.h"
#include "afxwin.h"
#include "afxcmn.h"

// CDisplayResFormat2Dlg ダイアログ

class CDisplayResFormat2Dlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayResFormat2Dlg)

public:
	//CDisplayResFormat2Dlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayResFormat2Dlg(sl::ResConfig& res, CResSampleView& sample);
	virtual ~CDisplayResFormat2Dlg();

	void InitControls();

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_RES_FORMAT2 };

	sl::ResConfig& res_;
	CResSampleView& m_xcSample;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeCmbPunct();
	afx_msg void OnBnClickedCheckSecond();

	CComboBox m_xcCmbPunct;
	CButton m_xcCheckSecond;
};
