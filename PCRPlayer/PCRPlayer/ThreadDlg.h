#pragma once

#include "FavoriteDlg.h"
#include "CustomEdit.h"
#include "CustomListCtrl.h"
#include "afxwin.h"
#include "afxcmn.h"

// メニューのお気に入り掲示板の範囲
#define IDM_FAVORITE_RANGE_BEGIN			51000
#define IDM_FAVORITE_RANGE_END				51099

// メニューのお気に入りスレッドの範囲
#define IDM_FAVORITE_THREAD_RANGE_BEGIN		51100
#define IDM_FAVORITE_THREAD_RANGE_END		51199

// メニューの掲示板履歴の範囲
#define IDM_HISTORY_RANGE_BEGIN				51200
#define IDM_HISTORY_RANGE_END				51299

// メニューのスレッド履歴の範囲
#define IDM_HISTORY_THREAD_RANGE_BEGIN		51300
#define IDM_HISTORY_THREAD_RANGE_END		51399

// CThreadDlg ダイアログ

class CThreadDlg : public CDialog
{
	DECLARE_DYNAMIC(CThreadDlg)

public:
	CThreadDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CThreadDlg();

	//virtual INT_PTR DoModal();

// ダイアログ データ
	enum { IDD = IDD_THREAD };

	void setFont(const std::wstring& name, int point);
	void createMenu();
	sl::DialogConfig::Thread cfg_;

protected:
	CFont font_;
	CMenu menu_;
	CStatusBar status_;

	int current_;
	bool order_;
	bool focus_;
	std::vector<bbs::ThreadInfo> org_;
	std::vector<bbs::ThreadInfo> subject_;

	struct This {
		This()
			: height(21)
		{}
		int height;
	} this_;

	void GetContactURL(std::wstring& url);
	void CreateMenu();
	void CheckMenuItem(int id, bool check);
	void SizeHelper();
	void ResetList();
	void ChangeStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnUpdate();
	afx_msg void OnDestroy();

	afx_msg void OnUpdateThreadStart();
	afx_msg void OnUpdateThreadEnd();

	afx_msg void OnFavoriteAdd();
	afx_msg void OnFavoriteEdit();
	afx_msg void OnFavoriteRange(UINT nID);

	afx_msg void OnFavoriteThreadAdd();
	afx_msg void OnFavoriteThreadEdit();
	afx_msg void OnFavoriteThreadRange(UINT nID);

	afx_msg void OnHistoryClear();
	afx_msg void OnHistoryRange(UINT nID);

	afx_msg void OnHistoryThreadClear();
	afx_msg void OnHistoryThreadRange(UINT nID);

	afx_msg void OnThreadStop();
	afx_msg void OnThreadStretch();
	afx_msg void OnThreadDatetime();
	afx_msg void OnThreadClose();

	afx_msg void OnThreadContactUrl();
	afx_msg void OnThreadBbsBrowser();
	afx_msg void OnThreadWebBrowser();
	afx_msg void OnThreadClipboardUrl();

	CCustomEdit m_xcEdit;
	CButton m_xcBtnUpdate;
	CCustomListCtrl m_xcList;
};
