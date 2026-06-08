// SeekSampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "SeekSampleView.h"


// CSeekSampleView

IMPLEMENT_DYNAMIC(CSeekSampleView, CSampleView)

CSeekSampleView::CSeekSampleView()
	: height_(16)
{
	::memset(&time_, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&time_);
}

CSeekSampleView::~CSeekSampleView()
{
}

void CSeekSampleView::SetConfig(const sl::SeekConfig& cfg)
{
	cfg_ = cfg;
	SetFontIndirect(cfg_.font);
}

void CSeekSampleView::SetFontIndirect(const LOGFONT& lf)
{
	CSampleView::SetFontIndirect(lf);
	height_  = GetFontHeight() + (cfg_.marginY * 2);
}


BEGIN_MESSAGE_MAP(CSeekSampleView, CSampleView)
END_MESSAGE_MAP()



// CSeekSampleView メッセージ ハンドラー


void CSeekSampleView::OnCustomPaint(CDC& dc, CDC& memDC, CRect& rc)
{
	// 全体
	CRect org(rc);

	// シークバー
	int center = rc.Height() / 2;
	int fix = height_ / 2;

	rc.left += 10;
	rc.right -= 10; 
	rc.top = center - fix;
	rc.bottom = rc.top + height_;

	CRect src(rc);

	// つまみ位置
	int pos = src.Width() / 2;


	utl::drawSeekBar(memDC, &src, pos, cfg_.knob, cfg_.color);


	// 文字位置
	rc.DeflateRect(cfg_.marginX, 0, cfg_.marginX, 0);

	memDC.SetTextColor(cfg_.text[0]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);

	// 右テキスト
	CRect rrc(rc);
	CRect calc;

	//CString tmp;
	//tmp.Format(L"%04d%02d%02d_%02d%02d%02d_%03d",
	//	time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);


	CString right;
	right.Format(L"%04d年%02d月%02d日(%s) %02d:%02d:%02d",
		time_.wYear, time_.wMonth, time_.wDay, utl::convertWeek(time_.wDayOfWeek),
		time_.wHour, time_.wMinute, time_.wSecond, time_.wMilliseconds);

	CString left;
	left.Format(L"ファイル名 %04d%02d%02d%s%02d%02d%02d.wmv",
		time_.wYear, time_.wMonth, time_.wDay, utl::convertWeekEn(time_.wDayOfWeek),
		time_.wHour, time_.wMinute, time_.wSecond, time_.wMilliseconds);

	memDC.DrawText(right, -1, &rrc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	memDC.DrawText(right, -1, &calc, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

	// 左テキスト
	rc.right = max(rc.left, rc.right - calc.Width());
	CRect lrc(rc);
	memDC.DrawText(left, -1, &lrc, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);


	dc.BitBlt(0, 0, org.Width(), org.Height(), &memDC, 0, 0, SRCCOPY);


	// 第二色
	memDC.SetTextColor(cfg_.text[1]);
	memDC.SetBkColor(RGB(0, 0, 0));
	memDC.SetBkMode(TRANSPARENT);


	memDC.DrawText(right, -1, &rrc, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	memDC.DrawText(left, -1, &lrc, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

	src.left = src.left + pos;
	dc.BitBlt(src.left, src.top, src.Width(), src.Height(), &memDC, src.left, src.top, SRCCOPY);
}