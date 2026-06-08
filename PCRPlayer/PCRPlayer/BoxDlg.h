#pragma once


// CBoxDlg ダイアログ

class CBoxDlg : public CDialog
{
	DECLARE_DYNAMIC(CBoxDlg)

public:
	CBoxDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CBoxDlg();

// ダイアログ データ
	enum { IDD = IDD_BOX };

	virtual void showWindow(bool show);

	CBitmap bmp_;
	bool click_;
	bool hoverMin_;
	bool hoverMax_;
	bool hoverClose_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
};
