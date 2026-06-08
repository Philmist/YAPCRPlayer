#pragma once

namespace layout {


enum GRID_ALIGN {
	GRID_ALIGN_LEFT_TOP,
	GRID_ALIGN_TOP_RIGHT,
};

//================================================================

class BaseElement {
protected:
	CRect outline_;

public:
	BaseElement() {}
	virtual ~BaseElement(){}

	virtual CRect outline() const { return outline_; }
	virtual CSize size() const { return outline_.Size(); }
	virtual void offset(int x, int y) { outline_.OffsetRect(x, y); }
	virtual void inflate(int x, int y) { outline_.InflateRect(x, y); }
	virtual void alignX(int x) { outline_.right = outline_.left + x; }
	virtual void alignY(int y) { outline_.bottom = outline_.top + y; }

	virtual void draw(CDC& dc, const CPoint& offset) = 0;
	virtual void mask(CDC& dc, const CPoint& offset, COLORREF color) = 0;
};

//================================================================

class TextElement : public BaseElement {
protected:
	std::wstring text_;
	COLORREF color_;
	CFont* font_;

public:
	TextElement(CDC& dc, const std::wstring& text, COLORREF color, CFont* font = NULL);
	virtual ~TextElement(){}

	virtual void draw(CDC& dc, const CPoint& offset);
	virtual void mask(CDC& dc, const CPoint& offset, COLORREF color);
};

//================================================================

class BaseLayout: public BaseElement {
protected:
	std::vector<boost::shared_ptr<BaseElement> > layout_;

public:
	BaseLayout(){}
	virtual ~BaseLayout(){}

	virtual void addX(const boost::shared_ptr<BaseElement>& element, int space = 0);
	virtual void addY(const boost::shared_ptr<BaseElement>& element, int space = 0);
	virtual void draw(CDC& dc, const CPoint& offset);
	virtual void mask(CDC& dc, const CPoint& offset, COLORREF color);
};

//================================================================

class TextLayout : public BaseLayout {
protected:

public:
	TextLayout(){}
	virtual ~TextLayout(){}

	virtual void add(CDC& dc, const std::wstring& text, COLORREF color, CFont* font = NULL, int space = 0, int width = 0, bool trim = false);

	virtual void addX(CDC& dc, const std::wstring& text, COLORREF color, CFont* font = NULL, int space = 0)
	{
		BaseLayout::addX(boost::make_shared<TextElement>(dc, text, color, font), space);
	}

	virtual void addY(CDC& dc, const std::wstring& text, COLORREF color, CFont* font = NULL, int space = 0)
	{
		BaseLayout::addY(boost::make_shared<TextElement>(dc, text, color, font), space);
	}
};

//================================================================

class GridElement : public BaseElement {
protected:
	boost::shared_ptr<BaseElement> element_;
	std::array<COLORREF, 6> color_;
	CRect margin_;
	int align_;

public:
	GridElement(const boost::shared_ptr<BaseElement>& element, const std::array<COLORREF, 6>& color, const CRect& margin, int align = GRID_ALIGN_LEFT_TOP);
	GridElement(const TextLayout& layout, const std::array<COLORREF, 6>& color, const CRect& margin, int align = GRID_ALIGN_LEFT_TOP);

	virtual ~GridElement(){}

	std::array<COLORREF, 6> color() { return color_; }

	virtual void align();

	virtual void draw(CDC& dc, const CPoint& offset);
	virtual void mask(CDC& dc, const CPoint& offset, COLORREF color);
	virtual void drawText(CDC& dc, const CPoint& offset);
	virtual void drawBox(CDC& dc, const CPoint& offset);

private:
	void init();
};

//================================================================

class GridRow: public BaseElement {
protected:
	std::vector<boost::shared_ptr<GridElement> > row_;

public:
	GridRow(){}
	virtual ~GridRow(){}

	virtual void add(const TextLayout& layout, const std::array<COLORREF, 6>& color, const CRect& margin, int align = GRID_ALIGN_LEFT_TOP, int space = 0)
	{
		add(boost::make_shared<GridElement>(GridElement(layout, color, margin, align)), space);
	}

	virtual void add(const GridElement& element, int space = 0)
	{
		add(boost::make_shared<GridElement>(element), space);
	}

	virtual void add(const boost::shared_ptr<GridElement>& element, int space = 0);
	virtual void adjust(GridRow& x); // êÆóÒÇ≥ÇπÇÈ
	virtual void fit(GridRow& x); // ññîˆÇëµÇ¶ÇÈ
	virtual void fix();	// êÆóÒÇ≈ÉYÉåÇΩoutlineÇíºÇ∑
	virtual void align();

	virtual void draw(CDC& dc, const CPoint& offset);
	virtual void mask(CDC& dc, const CPoint& offset, COLORREF color);
	virtual void drawText(CDC& dc, const CPoint& offset);
	virtual void drawBox(CDC& dc, const CPoint& offset, int first = 0, int last = -1);
};

//================================================================

class GridLayout : public BaseElement {
protected:
	std::vector<boost::shared_ptr<GridRow> > layout_;

public:
public:
	GridLayout(){}
	virtual ~GridLayout(){}

	virtual void add(const GridRow& row, int space = 0)
	{
		add(boost::make_shared<GridRow>(row), space);
	}

	virtual void add(const boost::shared_ptr<GridRow>& row, int space = 0, bool adjust = false);

	virtual void align();
	virtual void draw(CDC& dc, const CPoint& offset);
	virtual void mask(CDC& dc, const CPoint& offset, COLORREF color);
	virtual void drawText(CDC& dc, const CPoint& offset);
	virtual void drawBox(CDC& dc, const CPoint& offset, int first = 0, int last = -1);
};

}; // using namespace layout

