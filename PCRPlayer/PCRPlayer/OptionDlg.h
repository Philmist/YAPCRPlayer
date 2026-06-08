#pragma once
#include "afxcmn.h"

#include "VideoAdjustDlg.h"

#include "DetailNetworkDlg.h"
#include "DetailTypeDlg.h"
#include "DetailBrowserDlg.h"
#include "DetailStartDlg.h"
#include "DetailEndDlg.h"
#include "DetailDecoderDlg.h"
#include "DetailSnapshotDlg.h"

#include "DetailBBSCommonDlg.h"
#include "DetailBBSBarDlg.h"
#include "DetailBBSBrowserDlg.h"

#include "OperationShortcutDlg.h"
#include "OperationMouseDlg.h"
#include "OperationVariationDlg.h"
#include "OperationWindowDlg.h"

#include "DisplayWindowDlg.h"
#include "DisplayAspectDlg.h"
#include "DisplayDatetimeDlg.h"
#include "DisplayInformationDlg.h"
#include "DisplayStatusDlg.h"
#include "DisplaySeekDlg.h"
#include "DisplayHintDlg.h"
#include "DisplayChannelDlg.h"
#include "DisplayBBSDlg.h"
#include "DisplayResDlg.h"
#include "afxwin.h"




// COptionDlg ダイアログ

class COptionDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionDlg)

public:
	COptionDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~COptionDlg();

// ダイアログ データ
	enum { IDD = IDD_OPTION };

	int select_;

	struct Video {
		CVideoAdjustDlg adjust;
	};

	struct Detail {
		CDetailNetworkDlg network;
		CDetailTypeDlg type;
		CDetailDecoderDlg decoder;
		CDetailBrowserDlg browser;
		CDetailSnapshotDlg snapshot;
		CDetailStartDlg start;
		CDetailEndDlg end;

		sl::OptionConfing::Detail option;
		struct Config {
			sl::NetworkConfig network;
			sl::TypeConfig type;
			sl::BrowserConfig browser;
			sl::SnapshotConfig snapshot;
			sl::StartConfig start;
		} cfg;

		Detail()
			: network(option.network, cfg.network)
			, type(option.type, cfg.type)
			, browser(option.browser, cfg.browser)
			, snapshot(option.snapshot, cfg.snapshot)
			, start(option.start, cfg.start)
		{}


		struct BBS {
			CDetailBBSCommonDlg common;
			CDetailBBSBarDlg bar;
			CDetailBBSBrowserDlg browser;

			sl::OptionConfing::BBS option;
			struct Config {
				sl::BoardConfig board;
				sl::FontConfig font;
				PopupBrowserConfig popup;
			} cfg;

			BBS()
				: common(option, cfg.board, cfg.font)
				, bar(option, cfg.board)
				, browser(option, cfg.board, cfg.popup)
			{}
		} bbs;
	};

	struct Operation {
		COperationShortcutDlg shortcut;
		COperationMouseDlg mouse;
		COperationVariationDlg variation;
		COperationWindowDlg window;

		sl::OptionConfing::Operation option;
		struct Config {
			sl::MouseOperation mouse;
			sl::Variation variation;
			sl::WindowOperation window;
		} cfg;

		Operation()
			: mouse(option.mouse, cfg.mouse)
			, variation(option.variation, cfg.variation)
			, window(option.window, cfg.window)
		{}
	};

	struct Display {
		CDisplayWindowDlg window;
		CDisplayAspectDlg aspect;
		CDisplayDatetimeDlg datetime;
		CDisplayInformationDlg information;
		CDisplayStatusDlg status;
		CDisplaySeekDlg seek;
		CDisplayHintDlg hint;
		CDisplayChannelDlg channel;
		CDisplayBBSDlg bbs;
		CDisplayResDlg res;

		sl::OptionConfing::Display option;
		struct Config {
			sl::InformationConfig information;
		} cfg;

		Display()
			: information(option.information, cfg.information)
		{}
	};

	Video video_;
	Detail detail_;
	Operation operation_;
	Display display_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	struct Item {
		HTREEITEM item;
		std::wstring title;
		CDialog* dlg;
		int idd;
	};

	std::vector<Item> items_;
	CPoint pos_;

	void createItem(const Item& item);
	void setTitle(const std::wstring& title);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	CTreeCtrl m_xcTree;
	CStatic m_xcTitle;
};
