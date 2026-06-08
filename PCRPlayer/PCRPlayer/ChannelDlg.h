#pragma once


// CChannelDlg ダイアログ

class CChannelDlg : public CDialog
{
	DECLARE_DYNAMIC(CChannelDlg)

public:
	CChannelDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CChannelDlg();

	void SetConfig(const sl::ChannelConfig& cfg);

// ダイアログ データ
	enum { IDD = IDD_CHANNEL };

protected:
	std::wstring text_;
	CFont font_;
	CSize size_;
	DWORD align_;
	
	sl::ChannelConfig cfg_;

	DWORD modified_;

	void draw(CDC& dc, const CRect& org, bool mask = false, int text = 255, int back = 255);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	virtual void setAlign(DWORD align);
	virtual void setPanel();
	virtual void setText(const std::wstring& text);
	virtual void setPosition(long x, long y);
	virtual void showWindow(bool show);

	afx_msg void OnUpdateChannelInfoStart();
	afx_msg void OnUpdateChannelInfoEnd();
};
