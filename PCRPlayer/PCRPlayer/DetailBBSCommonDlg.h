#pragma once

#include "afxcmn.h"

#include "DetailBBSCommonGeneralDlg.h"
#include "DetailBBSCommonNetworkDlg.h"
#include "DetailBBSCommonThreadDlg.h"
#include "DetailBBSCommonFontDlg.h"

#include "ReplaceDlg.h"

// CDetailBBSCommonDlg ダイアログ

class CDetailBBSCommonDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailBBSCommonDlg)

public:
	//CDetailBBSCommonDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSCommonDlg(sl::OptionConfing::BBS& option, sl::BoardConfig& board, sl::FontConfig& font);
	virtual ~CDetailBBSCommonDlg();

	// ダイアログ データ
	enum { IDD = IDD_DETAIL_BBS_COMMON };

	int select_;

	struct Dialog {
		Dialog(sl::OptionConfing::BBS& option, sl::BoardConfig& board, sl::FontConfig& font)
			: general(option.common.general, board)
			, network(option.common.network, board)
			, thread(option.common.thread, board)
			, font(option.common.font, font)
			, url(board.url, [](utl::ReplaceElementList& list){ list.set(sl::BoardConfig().url); })
			, message(board.message, [](utl::ReplaceElementList& list) { list.set(sl::BoardConfig().message); })
		{}

		CDetailBBSCommonGeneralDlg general;
		CDetailBBSCommonNetworkDlg network;
		CDetailBBSCommonThreadDlg thread;
		CDetailBBSCommonFontDlg font;
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
