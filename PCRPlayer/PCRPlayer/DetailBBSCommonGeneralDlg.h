#pragma once

#include "BasePropDlg.h"

// CDetailBBSCommonGeneralDlg ダイアログ

class CDetailBBSCommonGeneralDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSCommonGeneralDlg)

public:
	//CDetailBBSCommonGeneralDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSCommonGeneralDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSCommonGeneralDlg();

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
