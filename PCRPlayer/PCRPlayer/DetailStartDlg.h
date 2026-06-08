#pragma once

#include "BasePropDlg.h"

// CDetailStartDlg ダイアログ

class CDetailStartDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailStartDlg)

public:
	//CDetailStartDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailStartDlg(utl::PropSetting& setting, sl::StartConfig& start);   // 標準コンストラクター
	virtual ~CDetailStartDlg();

	sl::StartConfig &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};

