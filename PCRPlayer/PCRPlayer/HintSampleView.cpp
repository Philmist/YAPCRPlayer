// HintSampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "HintSampleView.h"


// CHintSampleView

IMPLEMENT_DYNAMIC(CHintSampleView, CSampleView)

CHintSampleView::CHintSampleView()
{
	::memset(&time_, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&time_);
}

CHintSampleView::~CHintSampleView()
{
}


BEGIN_MESSAGE_MAP(CHintSampleView, CSampleView)
END_MESSAGE_MAP()

// CHintSampleView メッセージ ハンドラー

void CHintSampleView::OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc)
{
	CDC tmpDC;
	tmpDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* oldBmp = tmpDC.SelectObject(&bmp);
	//////////////////////////////////////////////////////

	// 背景コピー
	tmpDC.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

	CString text;
	text.Format(L"%04d年%02d月%02d日(%s) %02d:%02d:%02d\r\n12:34:56",
		time_.wYear, time_.wMonth, time_.wDay, utl::convertWeek(time_.wDayOfWeek),
		time_.wHour, time_.wMinute, time_.wSecond, time_.wMilliseconds);

	text_ = (LPCWSTR)text;

	CRect calc;
	memDC.DrawText(text, -1, &calc, DT_CALCRECT | DT_NOPREFIX | DT_CENTER);

	calc.InflateRect(cfg_.marginX, cfg_.marginY);

	CRect org(rc);

	org.left = (org.Width() / 2) - (calc.Width() / 2);
	org.right = org.left + calc.Width();
	org.top = (org.Height() / 2) - (calc.Height() / 2);
	org.bottom = org.top + calc.Height();

	if (cfg_.opaque.enable)
	{
		utl::LayeredWindow layer(rc.Width(), rc.Height(), font_.normal);
		if (layer)
		{
			layer.getMaskCDC().FillRect(rc, &CBrush(RGB(0, 0, 0)));
			draw(layer.getImageCDC(), org);
			draw(layer.getMaskCDC(), org, true, cfg_.opaque.text, cfg_.opaque.back);
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
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)cfg_.opaque.back, 0 };
		AlphaBlend(tmpDC, 0, 0, rc.Width(), rc.Height(), memDC, 0, 0, rc.Width(), rc.Height(), bf);
		dc.BitBlt(0, 0, rc.Width(), rc.Height(), &tmpDC, 0, 0, SRCCOPY);
	}

	//////////////////////////////////////////////////////
	tmpDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}

void CHintSampleView::draw(CDC& dc, const CRect& org, bool mask, int text, int back)
{
	COLORREF color = cfg_.text[0];
	if (mask)
	{
		color = RGB(text, text, text);
		dc.FillRect(org, &CBrush(RGB(back, back, back)));
	}
	else
	{
		utl::drawFrameRect(dc, org, cfg_.color);
	}

	CRect rc(org);
	rc.DeflateRect(cfg_.marginX, cfg_.marginY);

	dc.SetTextColor(color);
	dc.SetBkColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(text_.c_str(), -1, &rc, DT_NOPREFIX | DT_CENTER);

}
