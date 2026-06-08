#pragma once

#include "BasePropDlg.h"

// CDetailBBSBrowserScrollDlg ダイアログ

class CDetailBBSBrowserScrollDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSBrowserScrollDlg)

public:
	//CDetailBBSBrowserScrollDlg(CWnd* pParent = NULL);    // 標準コンストラクター
	CDetailBBSBrowserScrollDlg(utl::PropSetting& setting);
	virtual ~CDetailBBSBrowserScrollDlg();

	sl::ScrollConfig cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
