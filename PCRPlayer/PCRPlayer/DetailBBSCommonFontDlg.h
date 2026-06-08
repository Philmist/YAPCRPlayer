#pragma once

#include "BasePropDlg.h"

// CDetailBBSCommonFontDlg ダイアログ

class CDetailBBSCommonFontDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CDetailBBSCommonFontDlg)

public:
	//CDetailBBSCommonFontDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CDetailBBSCommonFontDlg(utl::PropSetting& setting, sl::FontConfig& font);   // 標準コンストラクター
	virtual ~CDetailBBSCommonFontDlg();

	sl::FontConfig& cfg_;

protected:
	struct Font {
		LOGFONT thread;
		LOGFONT search;
		LOGFONT extract;
	} font_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
