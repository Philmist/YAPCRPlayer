#pragma once


// CResDlg ダイアログ

class CResDlg : public CDialog
{
	DECLARE_DYNAMIC(CResDlg)

public:
	CResDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CResDlg();

	void SetConfig(const sl::ResConfig& cfg, long height);

// ダイアログ データ
	enum { IDD = IDD_RES };

protected:
	CWnd* target_;
	std::wstring text_;

	struct Font {
		CFont normal;
		CFont underline;
		struct Bold {
			CFont normal;
			CFont underline;
		} bold;
	} font_;

	CSize size_;
	layout::GridLayout layout_;
	DWORD align_;
	
	sl::ResConfig res_;
	long height_;

	void draw(CDC& dc, const CRect& org, bool mask = false, int text = 255, int back = 255);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	virtual void setAlign(DWORD align);
	virtual void setDat();
	virtual void setDat(const std::vector<bbs::ResInfo>& dat);

private:
	virtual int calc(const CRect& rc, long y);
	virtual void setPosition(long x, long y);
	virtual void setPositionLeft(long x, long y);
	virtual void setPositionRight(long x, long y);

public:
	virtual void setTarget(CWnd* target) { target_ = target; }
	virtual void setPosition();
	virtual void showWindow(bool show);
	afx_msg void OnUpdateResEnd();
	afx_msg void OnUpdateScrollEnd();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};
