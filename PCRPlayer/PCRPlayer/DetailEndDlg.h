#pragma once
#include "afxwin.h"


// CDetailEndDlg ダイアログ

class CDetailEndDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailEndDlg)

public:
	CDetailEndDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDetailEndDlg();

	void InitControls();

// ダイアログ データ
	enum { IDD = IDD_DETAIL_END };

	sl::EndConfig end_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnWindowCurrent();
	afx_msg void OnBnClickedBtnWindowInit();
	afx_msg void OnBnClickedBtnAudioCurrent();
	afx_msg void OnBnClickedBtnAudioInit();
	afx_msg void OnBnClickedBtnAllDefault();


	CButton m_xcCheckPos;
	CButton m_xcCheckSize;
	CButton m_xcCheckAspect;
	CButton m_xcCheckVolume;
	CButton m_xcCheckBalance;
	CButton m_xcCheckMute;
};
