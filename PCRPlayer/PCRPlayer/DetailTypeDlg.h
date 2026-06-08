#pragma once

#include "afxcmn.h"
#include "DetailTypeWMVDlg.h"
#include "DetailTypeFLVDlg.h"

// CDetailTypeDlg ダイアログ

class CDetailTypeDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailTypeDlg)

public:
	//CDetailTypeDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailTypeDlg(sl::OptionConfing::Detail::Type& option, sl::TypeConfig& type);
	virtual ~CDetailTypeDlg();

// ダイアログ データ
	enum { IDD = IDD_DETAIL_TYPE };

	int select_;

	struct Dialog {
		Dialog(sl::OptionConfing::Detail::Type& option, sl::TypeConfig& type)
			: wmv(option.wmv, type.wmv)
			, flv(option.flv, type.flv)
		{}

		CDetailTypeWMVDlg wmv;
		CDetailTypeFLVDlg flv;
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
