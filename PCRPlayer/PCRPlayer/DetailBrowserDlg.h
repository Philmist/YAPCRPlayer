#pragma once

#include "BasePropDlg.h"

// CDetailBrowserDlg ダイアログ

class CDetailBrowserDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBrowserDlg)

public:
	//CDetailBrowserDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBrowserDlg(utl::PropSetting& setting, sl::BrowserConfig& browser);   // 標準コンストラクター
	virtual ~CDetailBrowserDlg();

	sl::BrowserConfig &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
