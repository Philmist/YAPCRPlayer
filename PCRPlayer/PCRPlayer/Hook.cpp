
#include "stdafx.h"


namespace utl {

HWND FilterPropertyHook::parent_ = NULL;
HHOOK FilterPropertyHook::hook_ = NULL;
WNDPROC FilterPropertyHook::wndproc_ = NULL;

void FilterPropertyHook::Hook(HWND parent, HINSTANCE hi, DWORD id)
{
	parent_ = parent;
	hook_ = ::SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, hi, id);
}

void FilterPropertyHook::Unhook()
{
	::UnhookWindowsHookEx(hook_);
	hook_ = NULL;
}


LRESULT CALLBACK FilterPropertyHook::CallWndRetProc(int code, WPARAM wp, LPARAM lp)
{
	const CWPRETSTRUCT* p = (const CWPRETSTRUCT*)(LONG_PTR)lp;

	if (WM_NCCREATE == p->message &&
		GetParent(p->hwnd) == parent_)
	{
		Unhook();
		Subclass(p->hwnd);
	}
	return ::CallNextHookEx(hook_, code, wp, lp);
}

void FilterPropertyHook::Subclass(HWND hWnd)
{
	wndproc_ = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)FilterPropertyHook::WindowProc);
}

void FilterPropertyHook::Unsubclass(HWND hWnd)
{
	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)wndproc_);
	wndproc_ = NULL;
}

LRESULT CALLBACK FilterPropertyHook::WindowProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	LRESULT ret = CallWindowProc(wndproc_, hWnd, uMsg, wp, lp);
	if (WM_INITDIALOG == uMsg)
	{
		SetCenterWindow(hWnd);
		Unsubclass(hWnd);
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////

void FilterPropertyHook::SetCenterWindow(HWND hWnd)
{
	CRect rcParent, rcClient, rcDesktop;
	GetWindowRect(parent_, &rcParent);
	GetWindowRect(hWnd, &rcClient);

	POINT pt;
	if (::GetCursorPos(&pt))
	{
		MONITORINFO mi = {};
		if (utl::getDesktopRectFromPoint(pt, mi))
		{
			//rcDesktop.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
			//rcDesktop.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
			//rcDesktop.right = rcDesktop.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
			//rcDesktop.bottom = rcDesktop.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

			rcDesktop = mi.rcWork;

			int x = rcParent.left + (rcParent.Width() / 2) - (rcClient.Width() / 2);
			int y = rcParent.top + (rcParent.Height() / 2) - (rcClient.Height() / 2);


			if (x < rcDesktop.left)
			{
				x = rcDesktop.left;
			}

			if ((x + rcClient.Width()) > rcDesktop.right)
			{
				x = rcDesktop.right - (rcClient.Width());
			}

			if (y < rcDesktop.top)
			{
				y = rcDesktop.top;
			}

			if ((y + rcClient.Height()) > rcDesktop.bottom)
			{
				y = rcDesktop.bottom - rcClient.Height();
			}

			SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		}
	}
}

} // namespace utl

