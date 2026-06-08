#pragma once

#include "BasePropDlg.h"

// CDetailBBSBarThreadDlg ダイアログ

class CDetailBBSBarThreadDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSBarThreadDlg)

public:
	//CDetailBBSBarThreadDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBarThreadDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSBarThreadDlg();

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
