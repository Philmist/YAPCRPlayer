#pragma once

#include "afxcmn.h"
#include "DetailNetworkGeneralDlg.h"
#include "DetailNetworkPeercastDlg.h"

// CDetailNetworkDlg ダイアログ

class CDetailNetworkDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailNetworkDlg)

public:
	//CDetailNetworkDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailNetworkDlg(sl::OptionConfing::Detail::Network& option, sl::NetworkConfig& network);
	virtual ~CDetailNetworkDlg();

// ダイアログ データ
	enum { IDD = IDD_DETAIL_NETWORK };

	int select_;

	struct Dialog {
		Dialog(sl::OptionConfing::Detail::Network& option, sl::NetworkConfig& network)
			: general(option.general, network)
			, peercast(option.peercast, network)
		{}

		CDetailNetworkGeneralDlg general;
		CDetailNetworkPeercastDlg peercast;
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
