#pragma once

#include <tlhelp32.h>

namespace utl {

class ProcessManager {
private:
	std::vector<DWORD> info_;

public:
	ProcessManager(){}
	virtual ~ProcessManager() {}

	bool open(const std::wstring& path, const std::wstring& command);
	void close();

private:
	static BOOL CALLBACK closeWindowProc(HWND hwnd , LPARAM lpProcessId);
};

} // namespace utl