#pragma once


// CFrameDlg ダイアログ

class CFrameDlg : public CDialog
{
	DECLARE_DYNAMIC(CFrameDlg)

public:
	CFrameDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CFrameDlg();

// ダイアログ データ
	enum { IDD = IDD_FRAME };

	void setMainWnd(CWnd* wnd) { this_.wnd = wnd; }
	void SizeHelper();

protected:
	virtual ULONG GetGestureStatus(__in CPoint /*ptTouch*/) { return 0; }

	struct This {
		This()
			: wnd(NULL)
		{}
		CWnd* wnd;
	} this_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
};
