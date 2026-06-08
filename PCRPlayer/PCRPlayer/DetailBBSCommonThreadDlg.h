#pragma once

#include "BasePropDlg.h"

// CDetailBBSCommonThreadDlg ダイアログ

class CDetailBBSCommonThreadDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSCommonThreadDlg)

public:
	//CDetailBBSCommonThreadDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSCommonThreadDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSCommonThreadDlg();

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
