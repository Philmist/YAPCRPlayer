#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "CustomListCtrl.h"
#include "ShortcutEdit.h"

// COperationShortcutDlg ダイアログ

class COperationShortcutDlg : public CDialog
{
	DECLARE_DYNAMIC(COperationShortcutDlg)

public:
	COperationShortcutDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~COperationShortcutDlg();

// ダイアログ データ
	enum { IDD = IDD_OPERATION_SHORTCUT };

	utl::ShortcutManager shortcut_;
	CBrush brushKey_;
	CBrush brushGesture_;
	COLORREF colorKey_;
	COLORREF colorGesture_;

	utl::Gesture gesture_;
	bool capture_;

	void EnableControls(BOOL enable);
	void SetControls(int index);
	void GetCheckBox(int index);
	void SetEditKeyText(DWORD key, DWORD mod);
	void SetEditGestureText(const std::vector<int>& gesture);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);


	CCustomListCtrl m_xcList;
	CShortcutEdit m_xcEditKey;
	CButton m_xcCheckKeyShift;
	CButton m_xcCheckKeyCtrl;
	CButton m_xcCheckKeyAlt;
	CButton m_xcBtnKeyClear;

	CShortcutEdit m_xcEditGesture;
	CButton m_xcBtnGestureClear;

	CSpinButtonCtrl m_xcSpinMovement;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEnSetfocusEditKey();
	afx_msg void OnEnKillfocusEditKey();
	afx_msg void OnBnClickedCheckKeyShift();
	afx_msg void OnBnClickedCheckKeyCtrl();
	afx_msg void OnBnClickedCheckKeyAlt();
	afx_msg void OnBnClickedBtnKeyClear();
	afx_msg void OnEnSetfocusEditGesture();
	afx_msg void OnEnKillfocusEditGesture();
	afx_msg void OnBnClickedBtnGestureClear();
	afx_msg void OnBnClickedBtnAllClear();
	afx_msg void OnBnClickedBtnAllDefault();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnEnChangeEditMovement();
	afx_msg void OnDeltaposSpinMovement(NMHDR *pNMHDR, LRESULT *pResult);
};
