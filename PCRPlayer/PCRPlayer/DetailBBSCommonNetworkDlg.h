#pragma once

#include "BasePropDlg.h"

// CDetailBBSCommonNetworkDlg ダイアログ

class CDetailBBSCommonNetworkDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSCommonNetworkDlg)

public:
	//CDetailBBSCommonNetworkDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSCommonNetworkDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSCommonNetworkDlg();

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
