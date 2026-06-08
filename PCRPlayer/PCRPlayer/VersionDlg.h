#pragma once
#include "afxcmn.h"


// CVersionDlg ダイアログ

class CVersionDlg : public CDialog
{
	DECLARE_DYNAMIC(CVersionDlg)

public:
	CVersionDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CVersionDlg();

// ダイアログ データ
	enum { IDD = IDD_VERSION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMClickLinkUrl(NMHDR *pNMHDR, LRESULT *pResult);
	CLinkCtrl m_xcLinkUrl;
};
