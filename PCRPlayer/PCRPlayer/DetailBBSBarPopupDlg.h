#pragma once

#include "BasePropDlg.h"

// CDetailBBSBarPopupDlg ダイアログ

class CDetailBBSBarPopupDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSBarPopupDlg)

public:
	//CDetailBBSBarPopupDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBarPopupDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSBarPopupDlg();

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
