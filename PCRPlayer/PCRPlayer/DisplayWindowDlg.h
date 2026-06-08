#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "CustomListCtrl.h"

// CDisplayWindowDlg ダイアログ

class CDisplayWindowDlg : public CDialog
{
	DECLARE_DYNAMIC(CDisplayWindowDlg)

public:
	CDisplayWindowDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDisplayWindowDlg();

	void InitZoomControls();
	void InitSizeControls();
	void EnableZoomControls(BOOL enable);
	void EnableSizeControls(BOOL enable);

// ダイアログ データ
	enum { IDD = IDD_DISPLAY_WINDOW };

	bool orderZoom1_;
	bool orderZoom2_;
	bool orderSize1_;
	bool orderSize2_;

	std::vector<sl::WindowZoom> zoom_;
	std::vector<sl::WindowSize> size_;

	utl::WindowZoomList listZoom_;
	utl::WindowSizeList listSize_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnItemchangedListZoom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListSize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListZoom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListSize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditZoom();
	afx_msg void OnEnChangeEditSize1();
	afx_msg void OnEnChangeEditSize2();
	afx_msg void OnBnClickedBtnZoomAdd();
	afx_msg void OnBnClickedBtnSizeAdd();
	afx_msg void OnBnClickedBtnZoomDel();
	afx_msg void OnBnClickedBtnSizeDel();
	afx_msg void OnBnClickedBtnZoomUp();
	afx_msg void OnBnClickedBtnSizeUp();
	afx_msg void OnBnClickedBtnZoomDown();
	afx_msg void OnBnClickedBtnSizeDown();

	CTemplateListCtrl m_xcListZoom;
	CButton m_xcBtnZoomAdd;
	CButton m_xcBtnZoomDel;
	CButton m_xcBtnZoomUp;
	CButton m_xcBtnZoomDown;
	CSimpleEdit m_xcEditZoom;
	CSpinButtonCtrl m_xcSpinZoom;

	CTemplateListCtrl m_xcListSize;
	CButton m_xcBtnSizeAdd;
	CButton m_xcBtnSizeDel;
	CButton m_xcBtnSizeUp;
	CButton m_xcBtnSizeDown;
	CSimpleEdit m_xcEditSize1;
	CSpinButtonCtrl m_xcSpinSize1;
	CSimpleEdit m_xcEditSize2;
	CSpinButtonCtrl m_xcSpinSize2;
	afx_msg void OnBnClickedBtnZoomAllDefault();
	afx_msg void OnBnClickedBtnSizeAllDefault();
};
