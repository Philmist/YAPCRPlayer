// ChannelSampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ChannelSampleView.h"


// CChannelSampleView

IMPLEMENT_DYNAMIC(CChannelSampleView, CSampleView)

CChannelSampleView::CChannelSampleView()
{
}

CChannelSampleView::~CChannelSampleView()
{
}


BEGIN_MESSAGE_MAP(CChannelSampleView, CSampleView)
END_MESSAGE_MAP()



// CChannelSampleView メッセージ ハンドラー

void CChannelSampleView::OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc)
{
	CDC tmpDC;
	tmpDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* oldBmp = tmpDC.SelectObject(&bmp);
	//////////////////////////////////////////////////////

	// 背景コピー
	tmpDC.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

	CString text =
		L"チャンネル名 (TYPE) [0/0] 1000kbps\r\n"
		L"  [ジャンル - 詳細]\r\n"
		L"  「コメント」\r\n"
		L"  アーティスト/タイトル/アルバム";

	text_ = (LPCWSTR)text;

	CRect calc;
	memDC.DrawText(text, -1, &calc, DT_CALCRECT|DT_NOPREFIX|DT_LEFT);

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


void CChannelSampleView::draw(CDC& dc, const CRect& org, bool mask, int text, int back)
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

