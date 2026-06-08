// BBSSampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "BBSSampleView.h"


// CBBSSampleView

IMPLEMENT_DYNAMIC(CBBSSampleView, CSampleView)

CBBSSampleView::CBBSSampleView()
	: height_(16)
{

}

CBBSSampleView::~CBBSSampleView()
{
}

void CBBSSampleView::SetConfig(const sl::BBSConfig& cfg)
{
	cfg_ = cfg;
	SetFontIndirect(cfg_.font);
}

void CBBSSampleView::SetFontIndirect(const LOGFONT& lf)
{
	CSampleView::SetFontIndirect(lf);
	height_  = GetFontHeight() + (cfg_.marginY * 2);
}


BEGIN_MESSAGE_MAP(CBBSSampleView, CSampleView)
END_MESSAGE_MAP()



// CBBSSampleView メッセージ ハンドラー


void CBBSSampleView::OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc)
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

	auto drawText = [this, &memDC](const std::wstring text, CRect& rc, DWORD align, bool bold){
		CFont* old = NULL;
		if (bold) { old = memDC.SelectObject(&font_.bold.normal); }
		memDC.DrawText(text.c_str(), -1, rc, DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|align);
		if (bold) { memDC.SelectObject(old); }
	};

	auto calcText = [this, &memDC](const std::wstring text, CRect& rc, DWORD align, bool bold){
		CFont* old = NULL;
		if (bold) { old = memDC.SelectObject(&font_.bold.normal); }
		memDC.DrawText(text.c_str(), -1, rc, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|align);
		if (bold) { memDC.SelectObject(old); }
	};

	drawText(L"10", rc, DT_RIGHT, false);
	CRect calc;
	calcText(L"0000", calc, DT_RIGHT, false);
	int width = calc.Width();

	// タイトル
	std::wstring title = L"[スレッドタイトル] (1001)";
	std::wstring status = L" 読み込み状態";
	std::wstring post = L"書き込み状態 ";

	calcText(title, calc, DT_LEFT, cfg_.title.bold);
	int left = max(rc.left, (org.Width() - calc.Width()) / 2);
	int right = min(rc.right - width, (org.Width() + calc.Width()) / 2);
	right = max(rc.left, right);

	CRect rcCenter(rc);
	rcCenter.left = left;
	rcCenter.right = right;

	//-----------------------------------------------
	calcText(status, calc, DT_LEFT, false);
	left = max(rc.left, rc.right - width - calc.Width());
	right = max(rc.left, rc.right - width);

	CRect rcRight(rc);
	rcRight.left = left;
	rcRight.right = right;

	//-----------------------------------------------
	calcText(post, calc, DT_LEFT, false);
	left = rc.left;
	right = min(rc.left + calc.Width(), rcRight.left);

	CRect rcLeft(rc);
	rcLeft.left = left;
	rcLeft.right = right;

	//-----------------------------------------------
	rcCenter.left = max(rcCenter.left, rcLeft.right);
	rcCenter.right = rcRight.left;

	COLORREF oldColor = memDC.GetTextColor();
	memDC.SetTextColor(cfg_.title.text[0]);
	drawText(title, rcCenter, DT_LEFT, cfg_.title.bold);
	memDC.SetTextColor(oldColor);

	drawText(status, rcRight, DT_LEFT, false);
	drawText(post, rcLeft, DT_LEFT, false);

	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);
}