#pragma once

namespace utl {

class LayeredWindow {
	CImage img_, mask_;
	CDC imgDC_, maskDC_;
	CFont *imgOld_, *maskOld_;

public:
	LayeredWindow(int width, int height, CFont& font);
	virtual ~LayeredWindow();

	operator bool();

	CDC& getImageCDC() { return imgDC_; }
	CDC& getMaskCDC() { return maskDC_; }

	bool create();
	bool update(HWND hwnd, CDC& dc);
};

};