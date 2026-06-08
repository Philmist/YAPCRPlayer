#pragma once

#include "BasePropDlg.h"

// CDisplayInformationGeneralDlg ダイアログ

class CDisplayInformationGeneralDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDisplayInformationGeneralDlg)

public:
	//CDisplayInformationGeneralDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDisplayInformationGeneralDlg(utl::PropSetting& setting, sl::InformationConfig& information);   // 標準コンストラクター
	virtual ~CDisplayInformationGeneralDlg();

	sl::InformationConfig &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};

