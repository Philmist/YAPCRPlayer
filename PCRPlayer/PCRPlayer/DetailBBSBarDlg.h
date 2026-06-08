#pragma once

#include "afxcmn.h"

#include "DetailBBSBarThreadDlg.h"
#include "DetailBBSBarPopupDlg.h"
#include "DetailBBSBarPostDlg.h"

// CDetailBBSBarDlg ダイアログ

class CDetailBBSBarDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailBBSBarDlg)

public:
	//CDetailBBSBarDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBarDlg(sl::OptionConfing::BBS& option, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSBarDlg();

	// ダイアログ データ
	enum { IDD = IDD_DETAIL_BBS_COMMON };

	int select_;

	struct Dialog {
		Dialog(sl::OptionConfing::BBS& option, sl::BoardConfig& board)
			: thread(option.bar.thread, board)
			, popup(option.bar.popup, board)
			, post(option.bar.post, board)
			, url(board.url, [](utl::ReplaceElementList& list){ list.set(sl::BoardConfig().url); })
			, message(board.message, [](utl::ReplaceElementList& list) { list.set(sl::BoardConfig().message); })
		{}

		CDetailBBSBarThreadDlg thread;
		CDetailBBSBarPopupDlg popup;
		CDetailBBSBarPostDlg post;
		CReplaceDlg url;
		CReplaceDlg message;
	};

	Dialog dlg_;

	struct Item {
		CDialog* dlg;
		int idd;
		int index;
	};

	std::vector<Item> vec_;
	CRect rect_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);

	CTabCtrl m_xcTab;
};
