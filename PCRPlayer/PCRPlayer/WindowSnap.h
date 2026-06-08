#pragma once

namespace utl {

struct SnapDistance {
	SnapDistance()
		: screen(10)
		, window(10)
	{}

	int screen; // スクリーンスナップ距離
	int window; // ウィンドウスナップ距離
};

class WindowSnap
{
	SnapDistance distance_;
	int x_;
	int y_;
	CRect diff_;

public:
	WindowSnap();

	void Init();
	void OnEnterSizeMove(HWND hwnd);
	void OnMoving(HWND hwnd, LPRECT pRect, bool screen, bool window, const SnapDistance& distance);

private:
	inline bool snapS(int a, int b);
	inline bool snapW(int a, int b);

	bool screenSnapX(HWND hwnd, const MONITORINFO& mi, CRect& rc);
	bool screenSnapY(HWND hwnd, const MONITORINFO& mi, CRect& rc);
	bool windowSnapX(HWND hwnd, const CRect& org, CRect& rc);
	bool windowSnapY(HWND hwnd, const CRect& org, CRect& rc);
};

class SizeMove {
	bool sizemove_; // WM_ENTERSIZEMOVE/WM_EXITSIZEMOVE
	CRect rc_;
	bool reset_;

public:
	SizeMove()
		: sizemove_(false)
	{}

	operator bool() { return sizemove_; }

	void setRect(const CRect& rc);
	bool getRect(CRect& rc);

	void OnEnterSizeMove() { sizemove_ = true; }
	void OnExitSizeMove() { sizemove_ = false; }
};

};