#pragma once

#include "StatusDlg.h"
#include "SeekDlg.h"
#include "BBSDlg.h"

// CSubpaneDlg ダイアログ

class CSubpaneDlg : public CDialog
{
	DECLARE_DYNAMIC(CSubpaneDlg)

public:
	CSubpaneDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CSubpaneDlg();

	void SizeHelper();

// ダイアログ データ
	enum { IDD = IDD_SUBPANE, IDD_POPUP = IDD_SUBPANE_POPUP };

private:
	CBBSDlg		bbs_; // BBS
	CSeekDlg	seek_; // シーク
	CStatusDlg	status_; // ステータス

public:
	bool isPopup();
	void showBBS(bool show, bool edit);
	void showSeek(bool show);
	void showStatus(bool show);

	HWND getBBS() { return bbs_.GetSafeHwnd(); }
	HWND getSeek() { return seek_.GetSafeHwnd(); }
	HWND getStatus() { return status_.GetSafeHwnd(); }

	HWND getRes() { return bbs_.getRes(); }
	void setMessage(const std::wstring& message) { bbs_.setMessage(message); }

	void invalidateBBS() { bbs_.Invalidate(FALSE); }
	void invalidateSeek() { seek_.Invalidate(FALSE); }
	void invalidateStatus() { status_.Invalidate(FALSE); }

	int getHeight();

	bool setTime(const boost::posix_time::ptime& time) { return seek_.setTime(time); }
	void clearTime() { seek_.clearTime(); }

	void setFilename(const std::wstring& filename) { seek_.setFilename(filename); }
	void clearFilename() { seek_.clearFilename(); }

	void setNametext(const std::wstring& nametext) { status_.setNametext(nametext); }
	void clearNametext() { status_.clearNametext(); }

	void getVideoRect(CRect& rc);

	void setDesc(const std::wstring& desc) { status_.setDesc(desc); }
	void clearDesc() { status_.clearDesc(); }

	void SetConfig(const sl::StatusConfig& status, const sl::ChannelConfig& channel)
	{
		status_.SetConfig(status, channel);
	}

	void SetConfig(const sl::SeekConfig& seek, const sl::HintConfig& hint)
	{
		seek_.SetConfig(seek, hint);
	}

	void SetConfig(const sl::BBSConfig& bbs, const sl::ResConfig& res)
	{
		bbs_.SetConfig(bbs, res);
	}

	void SetFocusBBSEdit() { bbs_.SetFocusEdit(); }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
