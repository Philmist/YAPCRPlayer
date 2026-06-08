#pragma once
#include "afxwin.h"


// CVideoAdjustDlg ダイアログ

class CVideoAdjustDlg : public CDialog
{
	DECLARE_DYNAMIC(CVideoAdjustDlg)

public:
	CVideoAdjustDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CVideoAdjustDlg();

// ダイアログ データ
	enum { IDD = IDD_VIDEO_ADJUST };

	dsp::ProcAmpRangeGroup group_;
	dsp::ProcAmpValue value_;
	int nominal_;
	int resizer_;

protected:
	void ApplyValue(int index, int value);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnCbnSelchangeCmbNominal();
	afx_msg void OnCbnSelchangeCmbResizer();

	CScrollBar m_xcScrollbar[4];
	CSimpleEdit m_xcEdit[4];
	CButton m_xcButton[4];
	CComboBox m_xcCmbNominal;
	CComboBox m_xcCmbResizer;
	afx_msg void OnBnClickedBtnSave();
};
