#include "stdafx.h"

namespace utl {

WindowSnap::WindowSnap()
	: x_(0)
	, y_(0)
{}

void WindowSnap::Init()
{
	diff_.SetRectEmpty();
}

void WindowSnap::OnEnterSizeMove(HWND hwnd)
{
	CRect rc, shape;
	GetWindowRect(hwnd, &rc);

	if (utl::getWindowShapeRect(hwnd, shape))
	{
		diff_.TopLeft() = shape.TopLeft() - rc.TopLeft();
		diff_.BottomRight() = rc.BottomRight() - shape.BottomRight();
	}

	rc.DeflateRect(diff_);

	CPoint pt;
	GetCursorPos(&pt);

	x_ = pt.x - rc.left;
	y_ = pt.y - rc.top;
}

void WindowSnap::OnMoving(HWND hwnd, LPRECT pRect, bool screen, bool window, const SnapDistance& distance)
{
	if (!screen && !window) { return; }

	distance_ = distance;

	CRect rc(pRect);

	//gl_.addLog(L"diff l:%d t:%d r:%d b:%d", diff_.left, diff_.top, diff_.right, diff_.bottom);

	rc.DeflateRect(diff_);
	CRect org(rc);

	//-------------------

	CPoint pt;
	GetCursorPos(&pt);
	rc.OffsetRect(pt.x - (rc.left + x_), pt.y - (rc.top  + y_));

	if (screen && !window)
	{
		MONITORINFO mi;
		if (utl::getDesktopRectFromRect(rc, mi))
		{
			screenSnapX(hwnd, mi, rc);
			screenSnapY(hwnd, mi, rc);
		}
	}
	else if (!screen && window)
	{
		windowSnapX(hwnd, org, rc);
		windowSnapY(hwnd, org, rc);
	}
	else
	{
		MONITORINFO mi;
		if (utl::getDesktopRectFromRect(rc, mi))
		{
			//gl_.addLog(L"from l:%d t:%d r:%d b:%d", rc.left, rc.top, rc.right, rc.bottom);
			//gl_.addLog(L"work l:%d t:%d r:%d b:%d", mi.rcWork.left, mi.rcWork.top, mi.rcWork.right, mi.rcWork.bottom);
			//gl_.addLog(L"moni l:%d t:%d r:%d b:%d", mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
			if (!screenSnapX(hwnd, mi, rc)) { windowSnapX(hwnd, org, rc); }
			if (!screenSnapY(hwnd, mi, rc)) { windowSnapY(hwnd, org, rc); }
		}
		else
		{
			windowSnapX(hwnd, org, rc);
			windowSnapY(hwnd, org, rc);
		}
	}

	//-------------------
	rc.InflateRect(diff_);
	*pRect = rc;
}

bool WindowSnap::snapS(int a, int b)
{
	return (abs(a - b) < distance_.screen);
}

bool WindowSnap::snapW(int a, int b)
{
	return (abs(a - b) < distance_.window);
}


bool WindowSnap::screenSnapX(HWND hwnd, const MONITORINFO& mi, CRect& rc)
{
	CRect work(mi.rcWork);
	CRect monitor(mi.rcMonitor);
	if (snapS(work.left, rc.left))
	{
		rc.OffsetRect(work.left - rc.left, 0); return true;
	}
	else if (snapS(work.right, rc.right))
	{
		rc.OffsetRect(work.right - rc.right, 0); return true;
	}
	else if (snapS(monitor.left, rc.left))
	{
		rc.OffsetRect(monitor.left - rc.left, 0); return true;
	}
	else if (snapS(monitor.right, rc.right))
	{
		rc.OffsetRect(monitor.right - rc.right, 0); return true;
	}
	return false;
}

bool WindowSnap::screenSnapY(HWND hwnd, const MONITORINFO& mi, CRect& rc)
{
	CRect work(mi.rcWork);
	CRect monitor(mi.rcMonitor);

	if (snapS(work.top, rc.top))
	{
		rc.OffsetRect(0, work.top - rc.top); return true;
	}
	else if (snapS(work.bottom, rc.bottom))
	{
		rc.OffsetRect(0, work.bottom - rc.bottom); return true;
	}
	else if (snapS(monitor.top, rc.top))
	{
		rc.OffsetRect(0, monitor.top - rc.top); return true;
	}
	else if (snapS(monitor.bottom, rc.bottom))
	{
		rc.OffsetRect(0, monitor.bottom - rc.bottom); return true;
	}
	return false;
}

class Side {
	HWND hwnd_;
public:
	Side(LONG x, LONG y)
		: hwnd_(WindowFromPoint(CPoint(x, y)))
	{}

	bool getRect(CRect& other)
	{
		if (!hwnd_) { return false; }
		HWND hother = GetAncestor(hwnd_, GA_ROOT);
		if (!hother) { return false; }

		if (utl::getWindowShapeRect(hother, other)) { return true; }
		if (GetWindowRect(hother, other)) { return true; }
		return false;
	}
};

bool WindowSnap::windowSnapX(HWND hwnd, const CRect& org, CRect& rc)
{
	Side left(org.left - distance_.window, org.top + org.Height() / 2);
	Side right(org.right + distance_.window, org.top + org.Height() / 2);

	CRect other;
	if (left.getRect(other) && snapW(other.right, rc.left))
	{
		rc.OffsetRect(other.right - rc.left, 0); return true;
	}
	else if (right.getRect(other) && snapW(other.left, rc.right))
	{
		rc.OffsetRect(other.left - rc.right, 0); return true;
	}
	return false;
}

bool WindowSnap::windowSnapY(HWND hwnd, const CRect& org, CRect& rc)
{
	Side top(org.left + org.Width() / 2, org.top - distance_.window);
	Side bottom(org.left + org.Width() / 2, org.bottom + distance_.window);

	CRect other;
	if (top.getRect(other) && snapW(other.bottom, rc.top))
	{
		rc.OffsetRect(0, other.bottom - rc.top); return true;
	}
	else if (bottom.getRect(other) && snapW(other.top, rc.bottom))
	{
		rc.OffsetRect(0, other.top - rc.bottom); return true;
	}
	return false;
}




void SizeMove::setRect(const CRect& rc)
{
	rc_ = rc;
	reset_ = true;
}

bool SizeMove::getRect(CRect& rc)
{
	if (!reset_) { return false; }
	//if (!sizemove_) { return false; }
	rc = rc_;
	reset_ = false;
	return true;
}


};

