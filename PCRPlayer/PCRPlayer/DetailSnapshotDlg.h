#pragma once

#include "BasePropDlg.h"

// CDetailSnapshotDlg ダイアログ

class CDetailSnapshotDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailSnapshotDlg)

public:
	//CDetailSnapshotDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailSnapshotDlg(utl::PropSetting& setting, sl::SnapshotConfig& snapshot);   // 標準コンストラクター
	virtual ~CDetailSnapshotDlg();

	sl::SnapshotConfig &cfg_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};


