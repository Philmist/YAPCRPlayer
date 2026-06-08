#pragma once

#include "BasePropDlg.h"

// CDetailTypeWMVDlg ダイアログ

class CDetailTypeWMVDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailTypeWMVDlg)

public:
	//CDetailTypeWMVDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailTypeWMVDlg(utl::PropSetting& setting, sl::TypeConfig::WMV& type);   // 標準コンストラクター
	virtual ~CDetailTypeWMVDlg();

	sl::TypeConfig::WMV &cfg_;

protected:
	struct Member {
		struct Packet { 
			int time; 
		} packet;
		struct Frame {
			int time;
		} frame;
	} m_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};

