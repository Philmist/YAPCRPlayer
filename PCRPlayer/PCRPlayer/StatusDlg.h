#pragma once


#include "ChannelDlg.h"


// CStatusDlg ダイアログ

class CStatusDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatusDlg)

public:
	CStatusDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CStatusDlg();

// ダイアログ データ
	enum { IDD = IDD_STATUS };

protected:
	virtual ULONG GetGestureStatus(__in CPoint /*ptTouch*/) { return 0; }

private:
	CFont font_;
	int height_;
	sl::StatusConfig status_;

	std::wstring desc_;

	// チャンネルステータス
	CChannelDlg channel_;

	bool click_;
	bool capture_;
	bool show_;

	std::wstring nametext_;

	utl::StatusLayout layout_;

	void SetMouseHover();


	struct TypeSet {
		DWORD id;
		int number;
	};
	template <std::size_t N>
	void TypeMenu(int number, int& type, int& gl_type, const std::array<TypeSet, N>& set);

public:
	bool isValid() { return ::IsWindow(GetSafeHwnd()) == TRUE && getShow(); }
	void setShow(bool show) { show_ = show; ShowWindow(show ? SW_SHOWNA : SW_HIDE); EnableWindow(show ? TRUE : FALSE); }
	bool getShow() { return show_; }

	int getHeight() { return height_; }

	void setNametext(const std::wstring& nametext) { nametext_ = nametext; }
	void clearNametext() { nametext_.clear(); }

	void SetConfig(const sl::StatusConfig& status, const sl::ChannelConfig& channel);

	void setDesc(const std::wstring& desc);
	void clearDesc();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	//bool ShowTooltip(const CRect& rect, const CPoint& point);
	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
};
