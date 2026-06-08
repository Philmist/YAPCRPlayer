// ResSampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ResSampleView.h"


// CResSampleView

IMPLEMENT_DYNAMIC(CResSampleView, CSampleView)

CResSampleView::CResSampleView()
{
	::memset(&time_, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&time_);
}

CResSampleView::~CResSampleView()
{
}


BEGIN_MESSAGE_MAP(CResSampleView, CSampleView)
END_MESSAGE_MAP()



// CResSampleView メッセージ ハンドラー

void CResSampleView::OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc)
{
	CDC tmpDC;
	tmpDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* oldBmp = tmpDC.SelectObject(&bmp);
	//////////////////////////////////////////////////////

	// 背景コピー
	tmpDC.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

	std::array<std::wstring, 3> number = { L"1", L"10", L"100", };
	std::array<std::wstring, 3> message = { L"メール欄なし", L"メール欄あり", L"メール欄sage / 新着\r\n", };
	for (int i = 0; i < 3; ++i) { message.back() += L"0123456789"; }
	std::array<std::wstring, 3> mail = { L"", L"age", L"sage", };

	auto bold = [this](int index){ return cfg_.color.bold[index] ? &font_.bold.normal : NULL; };

	std::array<boost::function<COLORREF(int)>, 3> color = {
		[this](int index){ return cfg_.color.text[index]; },
		[this](int index){ return cfg_.color.link[index]; },
		[this](int index){ return cfg_.color.sage[index]; },
	};

	std::wstring punct;
	switch(cfg_.format.punct)
	{
	case sl::ResConfig::PUNCT_NONE: break;
	case sl::ResConfig::PUNCT_SPACE: punct = L" "; break;
	case sl::ResConfig::PUNCT_COLON:
	default: punct = L" : "; break;
	}

	int space = cfg_.format.space;
	int width = cfg_.format.width;
	bool ids = cfg_.id.enable;
	int threshold = cfg_.id.threshold;
	int marginX = cfg_.box.marginX;
	int marginY = cfg_.box.marginY;
	int punctX = cfg_.box.punct ? cfg_.box.marginX : 0;
	int punctY = cfg_.box.punct ? cfg_.box.marginY : 0;
	bool trim = false;
	
	//layout::GridLayout layout;
	layout_ = layout::GridLayout();
	switch (cfg_.format.type)
	{
	default:
	case sl::ResConfig::FORMAT_NORMAL1:
	case sl::ResConfig::FORMAT_NORMAL2:
		{
			std::wstring datetime =
				(boost::wformat(L"%04d/%02d/%02d(%s) %02d:%02d:%02d")
					% time_.wYear %  time_.wMonth % time_.wDay % utl::convertWeek(time_.wDayOfWeek)
					% time_.wHour % time_.wMinute % time_.wSecond).str();

			std::array<layout::TextLayout, 3> head;
			std::array<layout::TextLayout, 3> line;
			std::array<layout::GridRow, 3> row;
			for (int i = 0; i < 3; ++i)
			{
				bool last = i == 2 ? true : false;
				std::array<COLORREF, 6>& boxColor = last ? cfg_.box.latest : cfg_.box.color;
				head[i].addX(memDC, number[i], color[i](0), bold(0));
				head[i].addX(memDC, punct, color[i](8), bold(8));
				if (i != 2)
				{
					if (cfg_.format.type == sl::ResConfig::FORMAT_NORMAL2)
					{
						line[i].addX(memDC, L"", color[i](1), bold(1));
					}
					else
					{
						line[i].addX(memDC, L"名無しさん ", color[i](1), bold(1));
					}
						
				}
				else
				{
					if (cfg_.format.type != sl::ResConfig::FORMAT_NORMAL2)
					{
						line[i].addX(memDC, L"名無しさん", color[i](1), bold(1));
					}
					line[i].addX(memDC, L"◆TEST ", color[i](1), NULL);
				}
				line[i].addX(memDC, L"[", color[i](9), bold(9));
				line[i].addX(memDC, mail[i], color[i](2), bold(2));
				line[i].addX(memDC, L"] ", color[i](9), bold(9));
				line[i].addX(memDC, datetime, color[i](3), bold(3));
				line[i].addX(memDC, L" ID:", color[i](10), bold(10));
				line[i].addX(memDC, L"TEST", color[i](4), bold(4));
				if (ids)
				{
					switch (i)
					{
					case 1: line[i].addX(memDC, L" [1/2]", color[i](4), bold(4)); break;
					case 2: line[i].addX(memDC, (boost::wformat(L" [1/%d]") % threshold).str(), color[i](4), bold(4)); break;
					}
				}
				line[i].add(memDC, message[i], color[i](5), bold(5), space, width, trim);
				row[i].add(head[i], boxColor, CRect(marginX, marginY, punctX, marginY), layout::GRID_ALIGN_TOP_RIGHT);
				row[i].add(line[i], boxColor, CRect(punctX, marginY, marginX, marginY), layout::GRID_ALIGN_LEFT_TOP, cfg_.box.separate);
				layout_.add(row[i], cfg_.box.space);
			}
			break;
		}
	case sl::ResConfig::FORMAT_SIMPLE1:
	case sl::ResConfig::FORMAT_SIMPLE2:
		{
			std::wstring datetime;
			if (cfg_.format.second)
			{
				datetime =
					(boost::wformat(L"%02d:%02d:%02d") % time_.wHour % time_.wMinute % time_.wSecond).str();
			}
			else
			{
				datetime =
					(boost::wformat(L"%02d:%02d") % time_.wHour % time_.wMinute).str();
			}


			std::array<layout::TextLayout, 3> top;
			std::array<layout::TextLayout, 3> head;
			std::array<layout::TextLayout, 3> line;
			std::array<layout::GridRow, 3> row;
			for (int i = 0; i < 3; ++i)
			{
				bool last = i == 2 ? true : false;
				std::array<COLORREF, 6>& boxColor = last ? cfg_.box.latest : cfg_.box.color;

				top[i].addX(memDC, number[i] + L" ", color[i](6), bold(6));
				head[i].addX(memDC, datetime, color[i](7), bold(7));
				head[i].addX(memDC, L" ID:", color[i](10), bold(10));
				head[i].addX(memDC, L"TEST", color[i](4), bold(4));
				if (ids)
				{
					switch (i)
					{
					case 1: head[i].addX(memDC, L" [1/2] ", color[i](4), bold(4)); break;
					case 2: head[i].addX(memDC, (boost::wformat(L" [1/%d] ") % threshold).str(), color[i](4), bold(4)); break;
					}
				}

				head[i].addX(memDC, punct, color[i](8), bold(8));

				if (cfg_.format.type == sl::ResConfig::FORMAT_SIMPLE1)
				{
					line[i].addX(memDC, L"名無しさん ", color[i](1), bold(1));
				}
				if (last)
				{
					line[i].addX(memDC, L"◆TEST ", color[i](1), NULL);
				}

				line[i].add(memDC, message[i], color[i](5), bold(5), space, width, trim);

				row[i].add(top[i], boxColor, CRect(marginX, marginY, 0, marginY), layout::GRID_ALIGN_TOP_RIGHT);
				row[i].add(head[i], boxColor, CRect(0, marginY, punctX, marginY));
				row[i].add(line[i], boxColor, CRect(punctX, marginY, marginX, marginY), layout::GRID_ALIGN_LEFT_TOP, cfg_.box.separate);
				layout_.add(row[i], cfg_.box.space);
			}
			break;
		}
	case sl::ResConfig::FORMAT_COMPACT1:
	case sl::ResConfig::FORMAT_COMPACT2:
		{
			std::wstring datetime;
			if (cfg_.format.second)
			{
				datetime =
					(boost::wformat(L"%02d:%02d:%02d") % time_.wHour % time_.wMinute % time_.wSecond).str();
			}
			else
			{
				datetime =
					(boost::wformat(L"%02d:%02d") % time_.wHour % time_.wMinute).str();
			}


			std::array<layout::TextLayout, 3> top;
			std::array<layout::TextLayout, 3> head;
			std::array<layout::TextLayout, 3> line;
			std::array<layout::GridRow, 3> row;
			for (int i = 0; i < 3; ++i)
			{
				bool last = i == 2 ? true : false;
				std::array<COLORREF, 6>& boxColor = last ? cfg_.box.latest : cfg_.box.color;

				top[i].addX(memDC, number[i] + L" ", color[i](6), bold(6));
				head[i].addX(memDC, datetime, color[i](7), bold(7));
				head[i].addX(memDC, punct, color[i](8), bold(8));

				if (cfg_.format.type == sl::ResConfig::FORMAT_COMPACT1)
				{
					line[i].addX(memDC, L"名無しさん ", color[i](1), bold(1));
				}
				if (last)
				{
					line[i].addX(memDC, L"◆TEST ", color[i](1), NULL);
				}

				line[i].add(memDC, message[i], color[i](5), bold(5), space, width, trim);

				row[i].add(top[i], boxColor, CRect(marginX, marginY, 0, marginY), layout::GRID_ALIGN_TOP_RIGHT);
				row[i].add(head[i], boxColor, CRect(0, marginY, punctX, marginY));
				row[i].add(line[i], boxColor, CRect(punctX, marginY, marginX, marginY), layout::GRID_ALIGN_LEFT_TOP, cfg_.box.separate);
				layout_.add(row[i], cfg_.box.space);
			}
			break;
		}
	}

	CRect outline(layout_.outline());
	outline.InflateRect(cfg_.back.marginX, cfg_.back.marginY);

	CRect org(rc);

	org.left = (org.Width() / 2) - (outline.Width() / 2);
	org.right = org.left + outline.Width();
	org.top = (org.Height() / 2) - (outline.Height() / 2);
	org.bottom = org.top + outline.Height();

	if (cfg_.back.opaque.enable)
	{
		utl::LayeredWindow layer(rc.Width(), rc.Height(), font_.normal);
		if (layer)
		{
			layer.getMaskCDC().FillRect(rc, &CBrush(RGB(0, 0, 0)));
			draw(layer.getImageCDC(), org);
			draw(layer.getMaskCDC(), org, true, cfg_.back.opaque.text, cfg_.back.opaque.back);
			if (layer.create())
			{
				BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
				AlphaBlend(tmpDC, 0, 0, rc.Width(), rc.Height(), layer.getImageCDC(), 0, 0, rc.Width(), rc.Height(), bf);
				dc.BitBlt(0, 0, rc.Width(), rc.Height(), &tmpDC, 0, 0, SRCCOPY);
			}
		}
	}
	else
	{
		draw(memDC, org);
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)cfg_.back.opaque.back, 0 };
		AlphaBlend(tmpDC, 0, 0, rc.Width(), rc.Height(), memDC, 0, 0, rc.Width(), rc.Height(), bf);
		dc.BitBlt(0, 0, rc.Width(), rc.Height(), &tmpDC, 0, 0, SRCCOPY);
	}
	//////////////////////////////////////////////////////
	tmpDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}


void CResSampleView::draw(CDC& dc, const CRect& org, bool mask, int text, int back)
{
	dc.SetTextColor(RGB(255, 255, 255));
	dc.SetBkColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);

	CPoint offset(org.left + cfg_.back.marginX, org.top + cfg_.back.marginY);

	if (mask)
	{
		dc.FillRect(org, &CBrush(RGB(back, back, back)));
		layout_.mask(dc, offset, RGB(text, text, text));
	}
	else
	{
		utl::drawFrameRect(dc, org, cfg_.back.color);
		if (cfg_.box.punct)
		{
			switch (cfg_.format.type)
			{
			default:
			case sl::ResConfig::FORMAT_NORMAL1:
			case sl::ResConfig::FORMAT_NORMAL2:
				layout_.drawBox(dc, offset, 0, 0);
				layout_.drawBox(dc, offset, 1, 1);
				layout_.drawText(dc, offset);
				break;
			case sl::ResConfig::FORMAT_SIMPLE1:
			case sl::ResConfig::FORMAT_SIMPLE2:
			case sl::ResConfig::FORMAT_COMPACT1:
			case sl::ResConfig::FORMAT_COMPACT2:
				layout_.drawBox(dc, offset, 0, 1);
				layout_.drawBox(dc, offset, 2, 2);
				layout_.drawText(dc, offset);
				break;
			}
		}
		else
		{
			layout_.drawBox(dc, offset);
			layout_.drawText(dc, offset);
		}
	}
}