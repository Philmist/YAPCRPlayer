
#include "stdafx.h"

namespace utl {

LayeredWindow::LayeredWindow(int width, int height, CFont& font)
	: imgOld_(NULL), maskOld_(NULL)
{
	img_.Create(width, -height, 32);
	mask_.Create(width, -height, 32);

	imgDC_.Attach(img_.GetDC());
	maskDC_.Attach(mask_.GetDC());

	imgOld_ = imgDC_.SelectObject(&font);
	maskOld_ = maskDC_.SelectObject(&font);
}

LayeredWindow::~LayeredWindow()
{
	imgDC_.SelectObject(imgOld_);
	maskDC_.SelectObject(maskOld_);

	imgDC_.Detach();
	maskDC_.Detach();

	img_.ReleaseDC();
	mask_.ReleaseDC();
}

LayeredWindow::operator bool()
{
	return
		img_.IsDIBSection() && mask_.IsDIBSection() &&
		imgDC_.GetSafeHdc() && maskDC_.GetSafeHdc();
}

bool LayeredWindow::create()
{
	LPDWORD src = (LPDWORD)mask_.GetBits();
	LPDWORD dst = (LPDWORD)img_.GetBits();

	if (src &&
		(img_.GetWidth() == mask_.GetWidth()) &&
		(img_.GetHeight() == mask_.GetHeight()))
	{
		UINT total = mask_.GetWidth() * abs(mask_.GetHeight());

		DWORD r, g, b, a;
		for (UINT i = 0; i < total; ++i)
		{
			r = (*src & 0x00FF0000) >> 16;
			g = (*src & 0x0000FF00) >> 8;
			b = (*src & 0x000000FF);

			if (r == g && g == b) { a = r; }
			else { a = ((77 * r) + (150 * g) + (29 * b)) >> 8; }

			r = ((((*dst & 0x00FF0000) >> 16) * a) / 255);
			g = ((((*dst & 0x0000FF00) >> 8) * a) / 255);
			b = ((((*dst & 0x000000FF)) * a) / 255);

			*dst = (a << 24) | (r << 16) | (g << 8) | b;
			++src;
			++dst;
		}
		return true;
	}
	return false;
}

bool LayeredWindow::update(HWND hwnd, CDC& dc)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	CPoint ptDst(0, 0);
	CSize szDst(img_.GetWidth(), img_.GetHeight());

	if (UpdateLayeredWindow(hwnd, dc.GetSafeHdc(), NULL, &szDst, imgDC_.GetSafeHdc(), &ptDst, 0, &bf, ULW_ALPHA))
	{
		return true;
	}
	return false;
}

};