#pragma once

#include "ResDlg.h"
#include "afxwin.h"
#include "IMEEdit.h"

// CBBSDlg ダイアログ

class CBBSDlg : public CDialog
{
	DECLARE_DYNAMIC(CBBSDlg)

public:
	CBBSDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CBBSDlg();

// ダイアログ データ
	enum { IDD = IDD_BBS };

protected:
	virtual ULONG GetGestureStatus(__in CPoint /*ptTouch*/) { return 0; }

private:

	enum {
		BUTTON_WIDTH = 30,
	};

	struct Height {
		Height()
			: bbs(0)
			, edit(0)
		{}

		operator int() { return bbs + edit; }
		int bbs;
		int edit;
	};

	struct Layout {
		Layout()
			: width(0)
		{}

		int width;
	};

	struct Font {
		CFont main;
		CFont bold;
		CFont edit;
	} font_;

	Height height_;

	sl::BBSConfig bbs_;

	// レスポップアップ
	CResDlg res_;

	bool click_;
	bool show_;

	HICON icon_; // 書き込みアイコン

	Layout counter_; // カウンター位置情報

	// ポップアップ化でエディットが消去されるため保存する
	struct Edit {
		Edit() : start(0), end(0) {}
		CString text; 
		int start;
		int end;
	} edit_;

	// ポップアップ化でタイマーが消去されるため保存する
	struct Timer {
		Timer() : status(false), post(false) {}
		bool status;
		bool post;
	} timer_;

	bool posting_;


public:
	bool isValid() { return ::IsWindow(GetSafeHwnd()) == TRUE && getShow(); }
	void setShow(bool show, bool edit);
	bool getShow() { return show_; }

	int getHeight() { return height_; }

	HWND getRes() { return res_.GetSafeHwnd(); }
	void setMessage(const std::wstring& message);

	void SetConfig(const sl::BBSConfig& bbs, const sl::ResConfig& res);

	void SetFocusEdit();

protected:
	void enableControl(bool enable);
	bool enableIfControl(bool enable);
	int getEditHeight();
	void changeEditHeight();
	void OnSizeHelper();
	void popup(bool show);
	void delayPopup();

	void postLog(const std::wstring& name, const std::wstring& mail, const std::wstring& msg);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);

	afx_msg void OnUpdateChannelInfoEnd();
	afx_msg void OnBnClickedBtnWrite();
	afx_msg void OnEnChangeEdit();
	
	afx_msg void OnUpdatePostSucceeded();
	afx_msg void OnUpdatePostFailed();
	afx_msg void OnUpdateBbsAll();
	afx_msg void OnUpdateBbsCounter();
	afx_msg void OnUpdateBbsTitle();
	afx_msg void OnUpdateBbsStatus();
	afx_msg void OnUpdateBbsPost();
	afx_msg void OnUpdateBbsDelayStatus();
	afx_msg void OnUpdateBbsDelayPost();

	CIMEEdit m_xcEdit;
	CButton m_xcBtnWrite;
	afx_msg void OnUpdateBbsBrowser();
};
