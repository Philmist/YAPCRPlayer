// StatusSampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "StatusSampleView.h"


// CStatusSampleView

IMPLEMENT_DYNAMIC(CStatusSampleView, CSampleView)

CStatusSampleView::CStatusSampleView()
	: height_(16)
{

}

CStatusSampleView::~CStatusSampleView()
{
}

void CStatusSampleView::SetConfig(const sl::StatusConfig& cfg)
{
	cfg_ = cfg;
	SetFontIndirect(cfg_.font);
}

void CStatusSampleView::SetFontIndirect(const LOGFONT& lf)
{
	CSampleView::SetFontIndirect(lf);
	height_  = GetFontHeight() + (cfg_.marginY * 2);
}


BEGIN_MESSAGE_MAP(CStatusSampleView, CSampleView)
END_MESSAGE_MAP()



// CStatusSampleView メッセージ ハンドラー


void CStatusSampleView::OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc)
{
	CRect org(rc);
	int center = rc.Height() / 2;
	int fix = height_ / 2;

	rc.left += 10;
	rc.right -= 10; 
	rc.top = center - fix;
	rc.bottom = rc.top + height_;

	utl::drawStatusBar(memDC, &rc, cfg_.color);

	rc.DeflateRect(cfg_.marginX, 0, cfg_.marginX, 0);

	memDC.SetTextColor(cfg_.text[0]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);

	CRect calc;
	CString right = L"00:00:00 / 12:34:56 100";
	memDC.DrawText(right, -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	memDC.DrawText(right, -1, &calc, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

	rc.right = max(rc.left, rc.right - calc.Width());
	memDC.DrawText(L"1000kbps (60)", -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
}