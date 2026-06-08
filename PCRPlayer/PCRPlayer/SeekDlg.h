#pragma once

#include "HintDlg.h"

// CSeekDlg ダイアログ


class CSeekDlg : public CDialog
{
	DECLARE_DYNAMIC(CSeekDlg)

public:
	CSeekDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CSeekDlg();

// ダイアログ データ
	enum { IDD = IDD_SEEK };

protected:
	virtual ULONG GetGestureStatus(__in CPoint /*ptTouch*/) { return 0; }

private:
	CFont font_;
	int height_;
	sl::SeekConfig seek_;

	// ヒントダイアログ
	CHintDlg hint_;

	bool show_;
	bool capture_;

	utl::DateTime datetime_;
	std::wstring filename_;
	
	LONGLONG getCursorTime();
	LONGLONG getCursorTime(const RECT& rc, const POINT& pt);

public:
	bool isValid() { return ::IsWindow(GetSafeHwnd()) == TRUE && getShow(); }
	void setShow(bool show) { show_ = show; ShowWindow(show ? SW_SHOWNA : SW_HIDE); EnableWindow(show ? TRUE : FALSE); }
	bool getShow() { return show_; }

	int getHeight() { return height_; }

	bool setTime(const boost::posix_time::ptime& time) { return datetime_.set(time); }
	void clearTime() { datetime_.clear(); }

	void setFilename(const std::wstring& filename) { filename_ = filename; }
	void clearFilename() { filename_.clear(); }

	void SetConfig(const sl::SeekConfig& seek, const sl::HintConfig& hint);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
};
