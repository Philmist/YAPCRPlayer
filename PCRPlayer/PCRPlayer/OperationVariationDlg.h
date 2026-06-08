#pragma once

#include "BasePropDlg.h"

// COperationVariationDlg ダイアログ

class COperationVariationDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(COperationVariationDlg)

public:
	//COperationVariationDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	COperationVariationDlg(utl::PropSetting& setting, sl::Variation& variation);   // 標準コンストラクター
	virtual ~COperationVariationDlg();

	sl::Variation &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};

