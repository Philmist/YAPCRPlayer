#pragma once


// CSampleView

#define CSAMPLEVIEW_CLASSNAME    _T("CSampleViewCtrl")  // Window class name

class CSampleView : public CWnd
{
	DECLARE_DYNAMIC(CSampleView)

	CBitmap bmp_;
	CBrush brush_;
	struct Font {
		CFont normal;
		CFont underline;
		struct Bold {
			CFont normal;
			CFont underline;
		} bold;
	} font_;
	int height_;

public:
	CSampleView();
	virtual ~CSampleView();

	BOOL RegisterWindowClass();

	virtual int GetFontHeight();
	virtual void SetFontIndirect(const LOGFONT& lf);
	virtual BOOL GetLogFont(LOGFONT& lf);

	virtual void OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc) = 0;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
};


