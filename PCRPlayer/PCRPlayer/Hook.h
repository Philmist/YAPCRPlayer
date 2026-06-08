#pragma once

namespace utl {

class FilterPropertyHook {
protected:
	static HWND parent_;
	static HHOOK hook_;
	static WNDPROC wndproc_;

public:
	FilterPropertyHook(){};
	virtual ~FilterPropertyHook(){};
	static void Hook(HWND parent, HINSTANCE hi, DWORD id);
	static void Unhook();

private:

	static LRESULT CALLBACK CallWndRetProc(int code, WPARAM wp, LPARAM lp);

	static void Subclass(HWND hWnd);
	static void Unsubclass(HWND hWnd);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);

	static void SetCenterWindow(HWND hWnd);
};

} // namespace utl
