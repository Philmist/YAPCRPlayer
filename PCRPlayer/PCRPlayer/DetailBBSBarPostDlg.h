#pragma once

#include "BasePropDlg.h"

// CDetailBBSBarPostDlg ダイアログ

class CDetailBBSBarPostDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSBarPostDlg)

public:
	//CDetailBBSBarPostDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSBarPostDlg(utl::PropSetting& setting, sl::BoardConfig& board);   // 標準コンストラクター
	virtual ~CDetailBBSBarPostDlg();

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
