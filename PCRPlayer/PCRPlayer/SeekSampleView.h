#pragma once


// CSeekSampleView

class CSeekSampleView : public CSampleView
{
	DECLARE_DYNAMIC(CSeekSampleView)

	SYSTEMTIME time_;
	int height_;
	sl::SeekConfig cfg_;

public:
	CSeekSampleView();
	virtual ~CSeekSampleView();

	void SetConfig(const sl::SeekConfig& cfg);

	virtual void SetFontIndirect(const LOGFONT& lf);

	void SetColor(int index, COLORREF color) { cfg_.color[index] = color; }
	void SetText(int index, COLORREF color) { cfg_.text[index] = color; }
	void SetMarginX(int margin) { cfg_.marginX = margin; }
	void SetMarginY(int margin) { cfg_.marginY = margin; height_ = GetFontHeight() + (cfg_.marginY * 2); }
	void SetKnob(int knob) { cfg_.knob = knob; }

	virtual void OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc);

protected:
	DECLARE_MESSAGE_MAP()
};


