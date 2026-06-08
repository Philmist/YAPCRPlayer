#pragma once

#include "BasePropDlg.h"

// CDetailTypeFLVDlg ダイアログ

class CDetailTypeFLVDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailTypeFLVDlg)

public:
	//CDetailTypeFLVDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailTypeFLVDlg(utl::PropSetting& setting, sl::TypeConfig::FLV& type);   // 標準コンストラクター
	virtual ~CDetailTypeFLVDlg();

	sl::TypeConfig::FLV &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};

