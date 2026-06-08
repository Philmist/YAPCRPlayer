#pragma once

#include "BasePropDlg.h"

// CDetailBBSBrowserThreadDlg ダイアログ

class CDetailBBSBrowserThreadDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSBrowserThreadDlg)

public:
	//CDetailBBSBrowserThreadDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBrowserThreadDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSBrowserThreadDlg();

	sl::BoardConfig& cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
