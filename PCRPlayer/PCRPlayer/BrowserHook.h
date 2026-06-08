#pragma once

namespace utl {

class BrowserHook {
protected:
	struct Target {
		Target()
			: hwnd(NULL)
			, wndproc(NULL)
		{}

		Target(HWND hwnd, WNDPROC wndproc)
			: hwnd(hwnd)
			, wndproc(wndproc)
		{}
		HWND hwnd;
		WNDPROC wndproc;
	} target_;

	typedef std::unordered_map<HWND, Target> BrowserHookMap;

	static BrowserHookMap map_;

public:
	static bool Subclass(HWND hwnd, HWND target);
	static bool Unsubclass(HWND hwnd);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
};

} // namespace utl
