#pragma once

#include "TemplateListCtrl.h"

// CFavoriteDlg ダイアログ

class CFavoriteDlg : public CDialog
{
	DECLARE_DYNAMIC(CFavoriteDlg)

public:
	CFavoriteDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CFavoriteDlg();

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FAVORITE };
#endif

	bbs::Links links_;

protected:
	class List : public utl::TemplateList<bbs::Links>
	{
	public:
		List(bbs::Links& list)
			: utl::TemplateList<bbs::Links>(list)
		{}
		virtual ~List() {}

	protected:
		virtual std::wstring textImpl(value_type& value, int sub)
		{
			switch (sub)
			{
			case 0: return value.title;
			case 1: return value.url;
			}
			return std::wstring();
		}
	};

	List list_;

	void EnableControls(BOOL enable);
	void SetControls(int index);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnEnChangeEditTitle();
	afx_msg void OnEnChangeEditUrl();

	CTemplateListCtrl m_xcList;
	CEdit m_xcEditTitle;
	CEdit m_xcEditURL;
};
