#pragma once

#include "afxcmn.h"

#include "DetailBBSBrowserThreadDlg.h"
#include "DetailBBSBrowserPopupDlg.h"
#include "DetailBBSBrowserScrollDlg.h"

#include "SkinConfigDlg.h"

// CDetailBBSBrowserDlg ダイアログ

class CDetailBBSBrowserDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailBBSBrowserDlg)

public:
	//CDetailBBSBrowserDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBrowserDlg(sl::OptionConfing::BBS& option, sl::BoardConfig& board, PopupBrowserConfig& popup);   // 標準コンストラクター
	virtual ~CDetailBBSBrowserDlg();

	// ダイアログ データ
	enum { IDD = IDD_DETAIL_BBS_COMMON };

	int select_;

	struct Dialog {
		Dialog(sl::OptionConfing::BBS& option, sl::BoardConfig& board, PopupBrowserConfig& popup)
			: thread(option.browser.thread, board)
			, popup(option.browser.popup, popup)
			, scroll(option.browser.scroll)
			, skin(option)
		{}

		CDetailBBSBrowserThreadDlg thread;
		CDetailBBSBrowserPopupDlg popup;
		CDetailBBSBrowserScrollDlg scroll;

		struct Skin {
			Skin(sl::OptionConfing::BBS& option)
				: thread(option.browser.skin.thread, DEFAULT_SKIN_THREAD, RGB(240, 240, 240), bbs::SKIN_NAME_FORMAT_NODEFAULT, false, true)
				, popup(option.browser.skin.popup, DEFAULT_SKIN_POPUP, RGB(255, 255, 224), bbs::SKIN_NAME_FORMAT_NORMAL, true, false)
			{}
			CSkinConfigDlg thread;
			CSkinConfigDlg popup;
		} skin;
	} dlg_;


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
