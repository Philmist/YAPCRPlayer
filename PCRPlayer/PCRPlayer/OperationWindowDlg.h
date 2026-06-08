#pragma once

#include "BasePropDlg.h"

// COperationWindowDlg ダイアログ

class COperationWindowDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(COperationWindowDlg)

public:
	//COperationWindowDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	COperationWindowDlg(utl::PropSetting& setting, sl::WindowOperation& window);   // 標準コンストラクター
	virtual ~COperationWindowDlg();

	sl::WindowOperation &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};

