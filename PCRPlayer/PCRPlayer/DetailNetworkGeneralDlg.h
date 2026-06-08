#pragma once

#include "BasePropDlg.h"

// CDetailNetworkGeneralDlg ダイアログ

class CDetailNetworkGeneralDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailNetworkGeneralDlg)

public:
	//CDetailNetworkGeneralDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailNetworkGeneralDlg(utl::PropSetting& setting, sl::NetworkConfig& network);   // 標準コンストラクター
	virtual ~CDetailNetworkGeneralDlg();

	sl::NetworkConfig &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
