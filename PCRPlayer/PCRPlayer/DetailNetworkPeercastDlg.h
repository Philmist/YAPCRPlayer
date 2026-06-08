#pragma once

#include "BasePropDlg.h"

// CDetailNetworkPeercastDlg ダイアログ

class CDetailNetworkPeercastDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailNetworkPeercastDlg)

public:
	//CDetailNetworkPeercastDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailNetworkPeercastDlg(utl::PropSetting& setting, sl::NetworkConfig& network);   // 標準コンストラクター
	virtual ~CDetailNetworkPeercastDlg();

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
