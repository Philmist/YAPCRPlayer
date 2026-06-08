#pragma once

#include "BasePropDlg.h"

// COperationMouseDlg ダイアログ

class COperationMouseDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(COperationMouseDlg)

public:
	//COperationMouseDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	COperationMouseDlg(utl::PropSetting& setting, sl::MouseOperation& mouse);   // 標準コンストラクター
	virtual ~COperationMouseDlg();

	sl::MouseOperation &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
