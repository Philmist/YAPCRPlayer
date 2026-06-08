#include "stdafx.h"

namespace layout {

TextElement::TextElement(CDC& dc, const std::wstring& text, COLORREF color, CFont* font)
	: text_(text)
	, color_(color)
	, font_(font)
{
	CFont* old = NULL;
	if (font) { old = dc.SelectObject(font); }

	if (text.empty())
	{// ãÛï∂éöóÒÇÕãÛîíï∂éöÇ©ÇÁçÇÇ≥ÇæÇØãÅÇﬂÇÈ
		CSize size = dc.GetTextExtent(L" ");
		outline_.SetRect(0, 0, 0, size.cy);
	}
	else
	{
		CSize size = dc.GetTextExtent(text.c_str());
		outline_.SetRect(0, 0, size.cx, size.cy);
	}

	if (old) { dc.SelectObject(old); }
}

void TextElement::draw(CDC& dc, const CPoint& offset)
{
	CRect rc(outline());

	rc.OffsetRect(offset);

	CFont* old = NULL;
	if (font_) { old = dc.SelectObject(font_); }

	COLORREF org = dc.GetTextColor();
	dc.SetTextColor(color_);

	dc.DrawText(text_.c_str(), -1, rc, DT_NOPREFIX|DT_LEFT);

	dc.SetTextColor(org);

	if (old) { dc.SelectObject(old); }
}

void TextElement::mask(CDC& dc, const CPoint& offset, COLORREF color)
{
	CRect rc(outline());

	rc.OffsetRect(offset);

	CFont* old = NULL;
	if (font_) { old = dc.SelectObject(font_); }

	COLORREF org = dc.GetTextColor();
	dc.SetTextColor(color);

	dc.DrawText(text_.c_str(), -1, rc, DT_NOPREFIX | DT_LEFT);

	dc.SetTextColor(org);

	if (old) { dc.SelectObject(old); }
}

//=============================================================

void BaseLayout::addX(const boost::shared_ptr<BaseElement>& element, int space)
{
	if (element)
	{
		CSize size(element->size());
		outline_.right += layout_.empty() ? 0 : space;
		element->offset(outline_.right, 0);

		outline_.right += size.cx;
		outline_.bottom = outline_.top + max(outline_.Height(), size.cy);

		layout_.push_back(element);
	}
}

void BaseLayout::addY(const boost::shared_ptr<BaseElement>& element, int space)
{
	if (element)
	{
		CSize size(element->size());
		outline_.bottom += layout_.empty() ? 0 : space;
		element->offset(0, outline_.bottom);

		outline_.bottom += size.cy;
		outline_.right = outline_.left + max(outline_.Width(), size.cx);

		layout_.push_back(element);
	}
}

void BaseLayout::draw(CDC& dc, const CPoint& offset)
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->draw(dc, outline_.TopLeft() + offset); }
	}
}

void BaseLayout::mask(CDC& dc, const CPoint& offset, COLORREF color)
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->mask(dc, outline_.TopLeft() + offset, color); }
	}
}


//=============================================================

void TextLayout::add(CDC& dc, const std::wstring& text, COLORREF color, CFont* font, int space, int width, bool trim)
{
	utl::getLines(text, trim, [&](const std::wstring str){
		if (str.empty() || width == 0)
		{
			this->BaseLayout::addY(boost::make_shared<TextElement>(dc, str, color, font), space);
		}
		else
		{
			std::wstring tmp = str;
			while (!tmp.empty())
			{
				int fit = 0;
				CSize size;
				GetTextExtentExPoint(
					dc.GetSafeHdc(),
					tmp.c_str(),
					(int)tmp.size(),
					width,
					&fit,
					NULL,
					&size);

				if (fit <= 0) { break; }
				this->BaseLayout::addY(boost::make_shared<TextElement>(dc, tmp.substr(0, fit), color, font), space);
				tmp = tmp.substr(fit);
			}
		}
	});
}

//=============================================================

GridElement::GridElement(const boost::shared_ptr<BaseElement>& element, const std::array<COLORREF, 6>& color, const CRect& margin, int align)
	: element_(element)
	, color_(color)
	, margin_(margin_)
	, align_(align)
{
	init();
}

GridElement::GridElement(const TextLayout& layout, const std::array<COLORREF, 6>& color, const CRect& margin, int align)
	: element_(boost::make_shared<TextLayout>(layout))
	, color_(color)
	, margin_(margin)
	, align_(align)
{
	init();
}


void GridElement::align()
{
	if (element_)
	{
		switch(align_)
		{
		case GRID_ALIGN_TOP_RIGHT:
			{
				CRect rc(outline());
				CRect erc(element_->outline());
				erc.OffsetRect(rc.TopLeft());
				rc.DeflateRect(margin_);

				if (rc.right > erc.right)
				{
					int x = rc.right - erc.right;
					element_->offset(x, 0);
				}
				break;
			}

		case GRID_ALIGN_LEFT_TOP:
		default:
			break;
		}
	}
}

void GridElement::draw(CDC& dc, const CPoint& offset)
{
	utl::drawFrameRect(dc, outline_ + offset, color_);

	if (element_)
	{
		element_->draw(dc, outline_.TopLeft() + offset);
	}
}

void GridElement::mask(CDC& dc, const CPoint& offset, COLORREF color)
{
	if (element_)
	{
		element_->mask(dc, outline_.TopLeft() + offset, color);
	}
}

void GridElement::drawText(CDC& dc, const CPoint& offset)
{
	if (element_)
	{
		element_->draw(dc, outline_.TopLeft() + offset);
	}
}

void GridElement::drawBox(CDC& dc, const CPoint& offset)
{
	utl::drawFrameRect(dc, outline_ + offset, color_);
}


void GridElement::init()
{
	if (element_)
	{
		element_->offset(margin_.left, margin_.top);
		outline_ = element_->outline();
		outline_.InflateRect(margin_);
	}
}

//=============================================================

void GridRow::add(const boost::shared_ptr<GridElement>& element, int space)
{
	if (element)
	{
		CSize size(element->size());
		outline_.right += row_.empty() ? 0 : space;
		element->offset(outline_.right, 0);

		outline_.right += size.cx;
		outline_.bottom = outline_.top + max(outline_.Height(), size.cy);

		row_.push_back(element);

		int height = outline_.Height();
		for (auto it = row_.begin(); it != row_.end(); ++it)
		{
			if (*it)
			{
				(*it)->alignY(height);
			}
		}
	}
}

void GridRow::adjust(GridRow& x)
{
	int idiff = 0;
	int jdiff = 0;
	auto it = row_.begin();
	auto jt = x.row_.begin();
	for (; it != row_.end() && jt != x.row_.end(); ++it, ++jt)
	{
		if (*it && *jt)
		{
			(*it)->offset(idiff, 0);
			(*jt)->offset(jdiff, 0);
			int ix = (*it)->size().cx;
			int jx = (*jt)->size().cx;
			if (ix < jx) { (*it)->alignX(jx); idiff += jx - ix; }
			else if (ix > jx) { (*jt)->alignX(ix); jdiff += ix - jx; }

			(*it)->align();
			(*jt)->align();
		}
	}
}

void GridRow::fit(GridRow& x)
{
	if (row_.empty() || x.row_.empty()) { return; }

	int idiff = 0;
	int jdiff = 0;
	int iwidth = 0;
	int jwidth = 0;
	auto it = row_.begin();
	auto jt = x.row_.begin();
	for (; it != row_.end() && jt != x.row_.end(); ++it, ++jt)
	{
		if (*it && *jt)
		{
			(*it)->offset(idiff, 0);
			(*jt)->offset(jdiff, 0);
			if (it == row_.begin() && jt == x.row_.begin())
			{
				int ix = (*it)->size().cx;
				int jx = (*jt)->size().cx;
				if (jx > ix) { (*it)->alignX(jx); idiff += jx - ix; }
				else if (ix > jx) { (*jt)->alignX(ix); jdiff += ix - jx; }

				(*it)->align();
				(*jt)->align();
			}
			else
			{
				iwidth += (*it)->size().cx;
				jwidth += (*jt)->size().cx;
			}
		}
	}

	if (iwidth > 0 || jwidth > 0)
	{
		if (row_.back() && x.row_.back())
		{
			if (jwidth > iwidth)
			{
				int cx = row_.back()->size().cx;
				row_.back()->alignX((jwidth - iwidth) + cx);
			}
			else if (iwidth > jwidth)
			{
				int cx = x.row_.back()->size().cx;
				x.row_.back()->alignX((iwidth - jwidth) + cx);
			}
			row_.back()->align();
			x.row_.back()->align();
		}
	}
}

void GridRow::fix()
{
	for (auto it = row_.begin(); it != row_.end(); ++it)
	{
		if (*it)
		{
			outline_.right = max(outline_.right, (*it)->outline().right);
		}
	}
}

void GridRow::align()
{
	for (auto it = row_.begin(); it != row_.end(); ++it)
	{
		if (*it) { (*it)->align(); }
	}
}

void GridRow::draw(CDC& dc, const CPoint& offset)
{
	for (auto it = row_.begin(); it != row_.end(); ++it)
	{
		if (*it) { (*it)->draw(dc, outline_.TopLeft() + offset); }
	}
}

void GridRow::mask(CDC& dc, const CPoint& offset, COLORREF color)
{
	for (auto it = row_.begin(); it != row_.end(); ++it)
	{
		if (*it) { (*it)->mask(dc, outline_.TopLeft() + offset, color); }
	}
}

void GridRow::drawText(CDC& dc, const CPoint& offset)
{
	for (auto it = row_.begin(); it != row_.end(); ++it)
	{
		if (*it) { (*it)->drawText(dc, outline_.TopLeft() + offset); }
	}
}

void GridRow::drawBox(CDC& dc, const CPoint& offset, int first, int last)
{
	if (row_.empty()) { return; }

	CRect rc;
	std::array<COLORREF, 6> color;
	int i = 0;
	for (auto it = row_.begin(); it != row_.end(); ++it)
	{
		if (*it)
		{
			if (i == first)
			{
				CRect orc((*it)->outline());
				rc.left = orc.left;
				rc.top = orc.top;
				color = (*it)->color();
			}
			if ((last == -1 && it + 1 == row_.end()) || i == last)
			{
				CRect orc((*it)->outline());
				rc.right = orc.right;
				rc.bottom = orc.bottom;
				break;
			}
		}
		++i;
	}
	utl::drawFrameRect(dc, outline_.TopLeft() + rc + offset, color);
}

//=============================================================

void GridLayout::add(const boost::shared_ptr<GridRow>& row, int space, bool adjust)
{
	if (row)
	{
		outline_.bottom += layout_.empty() ? 0 : space;
		row->offset(0, outline_.bottom);

		auto x = boost::make_shared<GridRow>(*row);
		for (auto it = layout_.begin(); it != layout_.end(); ++it)
		{
			if (*it)
			{
				if (adjust)
				{
					(*it)->adjust(*x);
				}
				else
				{
					(*it)->fit(*x);
				}
				(*it)->fix();
			}
		}
		x->fix();

		CSize size(x->size());
		outline_.bottom += size.cy;
		outline_.right = outline_.left + max(outline_.Width(), size.cx);
		layout_.push_back(x);
	}
}

void GridLayout::align()
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->align(); }
	}
}

void GridLayout::draw(CDC& dc, const CPoint& offset)
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->draw(dc, outline_.TopLeft() + offset); }
	}
}

void GridLayout::mask(CDC& dc, const CPoint& offset, COLORREF color)
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->mask(dc, outline_.TopLeft() + offset, color); }
	}
}

void GridLayout::drawText(CDC& dc, const CPoint& offset)
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->drawText(dc, outline_.TopLeft() + offset); }
	}
}

void GridLayout::drawBox(CDC& dc, const CPoint& offset, int first, int last)
{
	for (auto it = layout_.begin(); it != layout_.end(); ++it)
	{
		if (*it) { (*it)->drawBox(dc, outline_.TopLeft() + offset, first, last); }
	}
}

//=============================================================


}; // using namespace layout


