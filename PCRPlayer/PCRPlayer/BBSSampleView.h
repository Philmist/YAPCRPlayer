#pragma once
#include "SampleView.h"

// CBBSSampleView

class CBBSSampleView : public CSampleView
{
	DECLARE_DYNAMIC(CBBSSampleView)

	int height_;
	sl::BBSConfig cfg_;

public:
	CBBSSampleView();
	virtual ~CBBSSampleView();

	void SetConfig(const sl::BBSConfig& cfg);

	virtual void SetFontIndirect(const LOGFONT& lf);

	void SetColor(int index, COLORREF color) { cfg_.color[index] = color; }
	void SetText(int index, COLORREF color) { cfg_.text[index] = color; }
	void SetMarginX(int margin) { cfg_.marginX = margin; }
	void SetMarginY(int margin) { cfg_.marginY = margin; height_ = GetFontHeight() + (cfg_.marginY * 2); }

	void SetTitleBold(bool bold) { cfg_.title.bold = bold; }
	void SetTitleText(int index, COLORREF color) { cfg_.title.text[index] = color; }
	

	virtual void OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc);

protected:
	DECLARE_MESSAGE_MAP()
};


