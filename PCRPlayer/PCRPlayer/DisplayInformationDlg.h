#pragma once

#include "afxcmn.h"
#include "DisplayInformationGeneralDlg.h"
#include "ReplaceDlg.h"

// CDisplayInformationDlg ダイアログ

class CDisplayInformationDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayInformationDlg)

public:
	//CDisplayInformationDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayInformationDlg(sl::OptionConfing::Display::Information& option, sl::InformationConfig& information);
	virtual ~CDisplayInformationDlg();

	// ダイアログ データ
	enum { IDD = IDD_DISPLAY_INFORMATION };

	int select_;

	struct Dialog {
		Dialog(sl::OptionConfing::Display::Information& option, sl::InformationConfig& info)
			: general(option.general, info)
			, name(info.name, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().name); })
			, type(info.type, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().type); })
			, genre(info.genre, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().genre); })
			, desc(info.desc, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().desc); })
			, comment(info.comment, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().comment); })
			, artist(info.artist, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().artist); })
			, title(info.title, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().title); })
			, album(info.album, [](utl::ReplaceElementList& list) { list.set(sl::InformationConfig().album); })
		{}

		CDisplayInformationGeneralDlg general;
		CReplaceDlg name;
		CReplaceDlg type;
		CReplaceDlg genre;
		CReplaceDlg desc;
		CReplaceDlg comment;
		CReplaceDlg artist;
		CReplaceDlg title;
		CReplaceDlg album;
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
