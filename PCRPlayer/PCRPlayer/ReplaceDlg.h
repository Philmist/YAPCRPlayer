#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "TemplateListCtrl.h"

#include <boost/functional.hpp>

// CReplaceDlg ダイアログ


class CReplaceDlg : public CDialog
{
	DECLARE_DYNAMIC(CReplaceDlg)

public:
	//CReplaceDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CReplaceDlg(std::vector<utl::ReplaceElement>& list, const boost::function<void(utl::ReplaceElementList&)>& reset);
	virtual ~CReplaceDlg();

	void InitControls();
	void EnableControls(BOOL enable);

	// ダイアログ データ
	enum { IDD = IDD_REPLACE };

	utl::ReplaceElementList list_;
	boost::function<void(utl::ReplaceElementList&)> reset_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnCopy();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnEnChangeEditDesc();
	afx_msg void OnEnChangeEditRegex();
	afx_msg void OnEnChangeEditFormat();
	afx_msg void OnBnClickedCheckIcase();
	afx_msg void OnBnClickedBtnAllDefault();

	CTemplateListCtrl m_xcList;
	CSimpleEdit m_xcEditDesc;
	CSimpleEdit m_xcEditRegex;
	CSimpleEdit m_xcEditFormat;
	CButton m_xcCheckIcase;
};
