#pragma once


// CHintDlg ダイアログ

class CHintDlg : public CDialog
{
	DECLARE_DYNAMIC(CHintDlg)

public:
	CHintDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CHintDlg();

	void SetConfig(const sl::HintConfig& cfg);
	virtual void showWindow(bool show);

// ダイアログ データ
	enum { IDD = IDD_HINT };

protected:
	std::wstring text_;
	CFont font_;
	CSize size_;
	DWORD align_;
	
	sl::HintConfig hint_;

	void draw(CDC& dc, const CRect& org, bool mask = false, int text = 255, int back = 255);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	virtual void setAlign(DWORD align);
	virtual void setText(const std::wstring& text);
	virtual void setPosition(long x, long y);
};
