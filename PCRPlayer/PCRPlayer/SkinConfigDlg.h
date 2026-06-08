#pragma once

#include "BasePropDlg.h"
#include "afxwin.h"

// CSkinConfigDlg ダイアログ

class CSkinConfigDlg : public CBasePropDlg
{
	DECLARE_DYNAMIC(CSkinConfigDlg)

public:
	//CSkinConfigDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	CSkinConfigDlg(utl::PropSetting& setting, const std::wstring& skin, COLORREF back, int name, bool id, bool splitter);
	virtual ~CSkinConfigDlg();

	bbs::SkinConfig cfg_;
	sl::SplitterConfig splitter_;
	void setSkins(const std::vector<std::wstring>& skins) { this_.skins = skins; }

protected:
	struct This {
		This(const std::wstring& skin, COLORREF back, int name, bool id, bool splitter)
			: skin(skin)
			, back(back)
			, name(name)
			, id(id)
			, splitter(splitter)
		{}

		LOGFONT lf;
		std::wstring skin;
		COLORREF back;
		int name;
		bool id;
		bool splitter;
		std::vector<std::wstring> skins;
	} this_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSample1();
	afx_msg void OnBnClickedBtnSample2();

protected:
	virtual void OnGetProp();
	virtual void OnSetProp();
};
