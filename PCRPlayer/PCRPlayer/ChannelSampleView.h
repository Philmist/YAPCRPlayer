#pragma once


// CChannelSampleView

class CChannelSampleView : public CSampleView
{
	DECLARE_DYNAMIC(CChannelSampleView)

	std::wstring text_;
	sl::ChannelConfig cfg_;

public:
	CChannelSampleView();
	virtual ~CChannelSampleView();

	void SetConfig(const sl::ChannelConfig& cfg) { cfg_ = cfg; }

	void SetColor(int index, COLORREF color) { cfg_.color[index] = color; }
	void SetText(int index, COLORREF color) { cfg_.text[index] = color; }
	void SetMarginX(int margin) { cfg_.marginX = margin; }
	void SetMarginY(int margin) { cfg_.marginY = margin; }
	void SetOpaqueEnable(bool enable) { cfg_.opaque.enable = enable; }
	void SetOpaqueText(int text) { cfg_.opaque.text = text; }
	void SetOpaqueBack(int back) { cfg_.opaque.back = back; }

	virtual void OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc);

private:
	void draw(CDC& dc, const CRect& org, bool mask = false, int text = 255, int back = 255);

protected:
	DECLARE_MESSAGE_MAP()
};


