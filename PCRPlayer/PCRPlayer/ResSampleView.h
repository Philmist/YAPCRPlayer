#pragma once


// CResSampleView

class CResSampleView : public CSampleView
{
	DECLARE_DYNAMIC(CResSampleView)

	SYSTEMTIME time_;
	sl::ResConfig cfg_;
	layout::GridLayout layout_;

public:
	CResSampleView();
	virtual ~CResSampleView();

	void SetConfig(const sl::ResConfig& cfg) { cfg_ = cfg; }

	void SetFormatType(int type) { cfg_.format.type = type; }
	void SetFormatSpace(int space) { cfg_.format.space = space; }
	void SetFormatWidth(int width) { cfg_.format.width = width; }

	void SetFormatPunct(int punct) { cfg_.format.punct = punct; }
	void SetFormatSecond(bool second) { cfg_.format.second = second; }

	void SetIDEnable(bool enable) { cfg_.id.enable = enable; }
	void SetIDThreshold(int threshold) { cfg_.id.threshold = threshold; }

	void SetColorText(int index, COLORREF color) { cfg_.color.text[index] = color; }
	void SetColorLink(int index, COLORREF color) { cfg_.color.link[index] = color; }
	void SetColorSage(int index, COLORREF color) { cfg_.color.sage[index] = color; }
	void SetColorBold(int index, BOOL bold) { cfg_.color.bold[index] = bold; }
	
	void SetBackMarginX(int margin) { cfg_.back.marginX = margin; }
	void SetBackMarginY(int margin) { cfg_.back.marginY = margin; }
	void SetBackOpaqueEnable(bool enable) { cfg_.back.opaque.enable = enable; }
	void SetBackOpaqueText(int text) { cfg_.back.opaque.text = text; }
	void SetBackOpaqueBack(int back) { cfg_.back.opaque.back = back; }
	void SetBackColor(int index, COLORREF color) { cfg_.back.color[index] = color; }

	void SetBoxMarginX(int margin) { cfg_.box.marginX = margin; }
	void SetBoxMarginY(int margin) { cfg_.box.marginY = margin; }
	void SetBoxSpace(int space) { cfg_.box.space = space; }
	void SetBoxPunct(bool punct) { cfg_.box.punct = punct; }
	void SetBoxSeparate(int separate) { cfg_.box.separate = separate; }
	void SetBoxColor(int index, COLORREF color) { cfg_.box.color[index] = color; }
	void SetBoxLatest(int index, COLORREF color) { cfg_.box.latest[index] = color; }

	virtual void OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc);

private:
	void draw(CDC& dc, const CRect& org, bool mask = false, int text = 255, int back = 255);

protected:
	DECLARE_MESSAGE_MAP()
};


