// SampleView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "SampleView.h"


// CSampleView

IMPLEMENT_DYNAMIC(CSampleView, CWnd)

CSampleView::CSampleView()
	: height_(0)
{
	RegisterWindowClass();

	bmp_.LoadBitmap(IDB_BLANK);
	brush_.CreatePatternBrush(&bmp_);

	utl::createFont(font_.normal);
	utl::createFont(font_.bold.normal, true);
	utl::createFont(font_.underline, false, true);
	utl::createFont(font_.bold.underline, true, true);
}

CSampleView::~CSampleView()
{
	if (bmp_.GetSafeHandle()) { bmp_.DeleteObject(); }
	if (brush_.GetSafeHandle()) { brush_.DeleteObject(); }
	if (font_.normal.GetSafeHandle()) { font_.normal.DeleteObject(); }
	if (font_.bold.normal.GetSafeHandle()) { font_.bold.normal.DeleteObject(); }
	if (font_.underline.GetSafeHandle()) { font_.underline.DeleteObject(); }
	if (font_.bold.underline.GetSafeHandle()) { font_.bold.underline.DeleteObject(); }
}

BOOL CSampleView::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (::GetClassInfo(hInst, CSAMPLEVIEW_CLASSNAME, &wndcls))
	return TRUE;

	wndcls.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc    = ::DefWindowProc;
	wndcls.cbClsExtra     = wndcls.cbWndExtra = 0;
	wndcls.hInstance      = hInst;
	wndcls.hIcon          = NULL;
	wndcls.hCursor        = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground  = (HBRUSH) (COLOR_3DFACE + 1);
	wndcls.lpszMenuName   = NULL;
	wndcls.lpszClassName  = CSAMPLEVIEW_CLASSNAME;

	if (AfxRegisterClass(&wndcls))
	return TRUE;

	AfxThrowResourceException();
	return FALSE;
}

int CSampleView::GetFontHeight()
{
	return height_;
}

void CSampleView::SetFontIndirect(const LOGFONT& lf)
{
	font_.normal.DeleteObject();
	font_.normal.CreateFontIndirect(&lf);

	LOGFONT copy = {};
	font_.bold.normal.DeleteObject();
	copy = lf;
	copy.lfWeight = FW_BOLD;
	font_.bold.normal.CreateFontIndirect(&copy);

	font_.underline.DeleteObject();
	copy = lf;
	copy.lfUnderline = TRUE;
	font_.underline.CreateFontIndirect(&copy);

	font_.bold.underline.DeleteObject();
	copy = lf;
	copy.lfWeight = FW_BOLD;
	copy.lfUnderline = TRUE;
	font_.bold.underline.CreateFontIndirect(&copy);

	height_ = utl::getFontHeight(GetSafeHwnd(), font_.normal);
}


BOOL CSampleView::GetLogFont(LOGFONT& lf)
{
	return font_.normal.GetLogFont(&lf);
}


BEGIN_MESSAGE_MAP(CSampleView, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CSampleView メッセージ ハンドラー




void CSampleView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rc;
	GetClientRect(&rc);
	
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* oldBmp = memDC.SelectObject(&bmp);
	CFont* oldFont = memDC.SelectObject(&font_.normal);


	memDC.FillRect(&rc, &brush_);
	////////////////////////////////////////////////////
	OnCustomPaint(dc, memDC, rc);
	////////////////////////////////////////////////////

	memDC.SelectObject(oldFont);
	memDC.SelectObject(oldBmp);
	bmp.DeleteObject();
}
