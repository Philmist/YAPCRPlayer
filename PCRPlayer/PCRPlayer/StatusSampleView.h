#pragma once
#include "SampleView.h"

// CStatusSampleView

class CStatusSampleView : public CSampleView
{
	DECLARE_DYNAMIC(CStatusSampleView)

	int height_;
	sl::StatusConfig cfg_;

public:
	CStatusSampleView();
	virtual ~CStatusSampleView();

	void SetConfig(const sl::StatusConfig& cfg);

	virtual void SetFontIndirect(const LOGFONT& lf);

	void SetColor(int index, COLORREF color) { cfg_.color[index] = color; }
	void SetText(int index, COLORREF color) { cfg_.text[index] = color; }
	void SetMarginX(int margin) { cfg_.marginX = margin; }
	void SetMarginY(int margin) { cfg_.marginY = margin; height_ = GetFontHeight() + (cfg_.marginY * 2); }

	virtual void OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc);

protected:
	DECLARE_MESSAGE_MAP()
};


