#include "stdafx.h"

#include "BrowserHook.h"

namespace utl {

BrowserHook::BrowserHookMap BrowserHook::map_;

bool BrowserHook::Subclass(HWND hwnd, HWND target)
{
	if (!hwnd) { return false; }
	Unsubclass(hwnd);

	WNDPROC wndproc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	if (!wndproc) { return false; }

	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)BrowserHook::WindowProc);
	map_.insert(std::make_pair(hwnd, Target(target, wndproc)));
	return true;
}

bool BrowserHook::Unsubclass(HWND hwnd)
{
	if (!hwnd) { return false; }
	auto it = map_.find(hwnd);
	if (it == map_.end()) { return false; }
	if (!it->second.wndproc) { return false; }

	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)it->second.wndproc);
	map_.erase(hwnd);
	return true;
}

LRESULT CALLBACK BrowserHook::WindowProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	auto it = map_.find(hWnd);
	if (it != map_.end())
	{
		switch (uMsg)
		{
		case WM_MOUSEWHEEL:
			// 通常の状態ではポップアップのホイールが効かないため
			// 登録されていてマウス直下にあるブラウザにメッセージを渡す
			HWND hwnd = WindowFromPoint(CPoint(GET_X_LPARAM(lp), GET_Y_LPARAM(lp)));
			if (hwnd != hWnd)
			{
				auto it = map_.find(hwnd);
				if (it != map_.end())
				{
					return CallWindowProc(it->second.wndproc, hwnd, uMsg, wp, lp);
				}
			}

			if (GetKeyState(VK_CONTROL) & 0x8000)
			{// ズーム抑止のためCtrl入力を除去
				std::array<BYTE, 256> state;
				if (GetKeyboardState(state.data()))
				{
					state[VK_CONTROL] = 0;
					SetKeyboardState(state.data());
				}
			}
			return CallWindowProc(it->second.wndproc, hWnd, uMsg, wp, lp);
		}

		switch (uMsg)
		{
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			SendMessage(it->second.hwnd, uMsg, wp, lp);
		}
		return CallWindowProc(it->second.wndproc, hWnd, uMsg, wp, lp);
	}
	return DefWindowProc(hWnd, uMsg, wp, lp);
}


} // namespace utl

