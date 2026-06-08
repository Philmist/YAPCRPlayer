#include "stdafx.h"


namespace utl {

bool ProcessManager::open(const std::wstring& path, const std::wstring& command)
{
	DWORD id = 0;
	if (processExecute(path, command, id))
	{
		info_.push_back(id);
		return true;
	}
	return false;
}

void ProcessManager::close()
{
	for (auto it = info_.begin(); it != info_.end(); ++it)
	{
		EnumWindows(closeWindowProc, (LPARAM)*it);

		HANDLE p = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *it);
		if (p == NULL || GetProcessId(p) != *it)
		{
			return;
		}

		// プロセスの終了を待つ
		DWORD result = WaitForSingleObject(p, 500);
		if (result == WAIT_TIMEOUT) // タイムアウトした
		{
			// 強制終了
			TerminateProcess(p, 0);
			DWORD dwExitCode = 0;
			GetExitCodeProcess(p, &dwExitCode);
			CloseHandle(p);
		}
	}
}

BOOL CALLBACK ProcessManager::closeWindowProc(HWND hwnd , LPARAM lpProcessId)
{
	DWORD id;
	GetWindowThreadProcessId(hwnd, &id);

	if (id == lpProcessId)
	{
		::PostMessage(hwnd, WM_CLOSE, 0, 0);
	}
	return TRUE;
}

} // namespace utl

