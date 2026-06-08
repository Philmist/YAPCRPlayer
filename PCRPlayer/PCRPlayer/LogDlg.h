#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "TemplateListCtrl.h"

// CLogDlg ダイアログ

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CLogDlg();

	void InitControls();
	void SizeHelper();

// ダイアログ データ
	enum { IDD = IDD_LOG };

	std::deque<utl::Log> log_;

	class List : public utl::TemplateList<std::deque<utl::Log>>
	{
	public:
		List(std::deque<utl::Log>& list)
			: utl::TemplateList<std::deque<utl::Log>>(list)
		{}
		virtual ~List() {}

	protected:
		virtual std::wstring textImpl(value_type& value, int sub);
	};

	List list_;

protected:
	CMenu menu_;
	void CreateMenu();
	void CheckMenuItem(int id, bool check);

	CStatusBar status_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnLogGraph();
	afx_msg void OnLogFilter();
	afx_msg void OnLogBbs();
	afx_msg void OnLogPeerCast();
	afx_msg void OnLogNone();
	afx_msg void OnLogAllCopy();
	afx_msg void OnLogCopy();
	afx_msg void OnLogClear();

	CTemplateListCtrl m_xcList;
	CSimpleEdit m_xcEdit;
};
