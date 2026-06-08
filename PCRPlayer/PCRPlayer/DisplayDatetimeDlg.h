#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CDisplayDatetimeDlg ダイアログ

class CDisplayDatetimeDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayDatetimeDlg)

public:
	CDisplayDatetimeDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayDatetimeDlg();

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_DATETIME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	void InitControls();
	void EnableControls(BOOL enable);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();

	sl::DatetimeConfig datetime_;

	CComboBox m_xcCmbDate;
	CComboBox m_xcCmbTime;
	CButton m_xcRadio[3];
	CSimpleEdit m_xcEditRegex;
	CSimpleEdit m_xcEdit[6];
	CSpinButtonCtrl m_xcSpin[6];
	afx_msg void OnBnClickedBtnAllDefault();
};
