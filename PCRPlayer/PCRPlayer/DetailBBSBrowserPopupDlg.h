#pragma once

#include "BasePropDlg.h"

// CDetailBBSBrowserPopupDlg ダイアログ

class CDetailBBSBrowserPopupDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSBrowserPopupDlg)

public:
	//CDetailBBSBrowserPopupDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBrowserPopupDlg(utl::PropSetting& setting, PopupBrowserConfig& popup);   // 標準コンストラクター
	virtual ~CDetailBBSBrowserPopupDlg();

	PopupBrowserConfig& cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
