#include "stdafx.h"



namespace utl {

////////////////////////////////////////
// OSバージョン

#define STATUS_SUCCESS (0x00000000)
typedef NTSTATUS(WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

bool getOSVersion(DWORD& major, DWORD& minor, DWORD& build)
{
	HMODULE hmod = ::LoadLibrary(TEXT("ntdll.dll"));
	if (hmod)
	{
		RtlGetVersionPtr fnc = (RtlGetVersionPtr)GetProcAddress(hmod,"RtlGetVersion");
		if (fnc)
		{
			RTL_OSVERSIONINFOW info = { sizeof(RTL_OSVERSIONINFOW) };
			if (fnc(&info) == STATUS_SUCCESS)
			{
				major = info.dwMajorVersion;
				minor = info.dwMinorVersion;
				build = info.dwBuildNumber;
				FreeLibrary(hmod);
				return true;
			}
		}
		FreeLibrary(hmod);
	}

	OSVERSIONINFO info = { sizeof(OSVERSIONINFO) };
	if (GetVersionEx(&info))
	{
		major = info.dwMajorVersion;
		minor = info.dwMinorVersion;
		build = info.dwBuildNumber;
		return true;
	}
	return false;
}

bool isWindows10()
{
	static DWORD ver = 0;

	if (ver == 0)
	{
		DWORD major = 0, minor = 0, build = 0;
		if (getOSVersion(major, minor, build))
		{
			ver = major;
		}
	}
	return ver == 10;
}

////////////////////////////////////////
// コマンドライン

LPWSTR* CommandLine::line_ = nullptr;
int CommandLine::size_ = 0;

CommandLine::CommandLine()
{
	line_ = CommandLineToArgvW(GetCommandLineW(), &size_);
}

CommandLine::~CommandLine()
{
	if (line_) { GlobalFree(line_); }
}

void CommandLine::init()
{
	static CommandLine instance; // シングルトン
}

int CommandLine::size()
{
	if (line_)
	{
		return size_;
	}
	return 0;
}

std::wstring CommandLine::str(int i)
{
	if (line_ && i < size_)
	{
		return line_[i];
	}
	return std::wstring();
}

////////////////////////////////////////
// カレントフォルダ
bool getModuleFolder(std::wstring &path)
{
	namespace fs = boost::filesystem;

	WCHAR pPath[MAX_PATH + 1];

	::ZeroMemory(pPath, sizeof(pPath) / sizeof(pPath[0]));

	// 自身のプログラムパスを取得
	if (0 == ::GetModuleFileName(NULL, pPath, MAX_PATH))
	{
		return false;
	}

	// parent_pathは最後の\\を含まない
	fs::path tmp(pPath);
	path = tmp.parent_path().wstring();

	// パスがあるなら必ず最後に\\がつくようにする
	if (!path.empty()) { path += L"\\"; }
	return true;
}

////////////////////////////////////////
// フォルダパスブラウザ

// フォルダ選択ダイアログのコールバック
int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam,  LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED) {
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	}
	return 0;
}

bool folderBrowser(HWND hwnd, const std::wstring& src, std::wstring& dst)
{
	BROWSEINFO bInfo;
	LPITEMIDLIST pIDList;
	WCHAR wszDisplayName[MAX_PATH];
	memset(wszDisplayName, 0, MAX_PATH);

	// BROWSEINFO構造体に値を設定
	bInfo.hwndOwner			= hwnd; // ダイアログの親ウインドウのハンドル
	bInfo.pidlRoot			= NULL; // ルートフォルダを示すITEMIDLISTのポインタ (NULLの場合デスクトップフォルダが使われます）
	bInfo.pszDisplayName	= wszDisplayName; // 選択されたフォルダ名を受け取るバッファのポインタ
	bInfo.lpszTitle			= L""; // ツリービューの上部に表示される文字列 
	bInfo.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE; // 表示されるフォルダの種類を示すフラグ
	bInfo.lpfn				= BrowseCallbackProc; // BrowseCallbackProc関数のポインタ
	bInfo.lParam			= (LPARAM)src.c_str(); // コールバック関数に渡す値

	// フォルダ選択ダイアログを表示
	pIDList = ::SHBrowseForFolder(&bInfo);
	// 戻り値がNULLの場合、フォルダが選択されずにダイアログが閉じられた
	if (pIDList == NULL) { return false; }

    // ItemIDListをパス名に変換します
    if (!::SHGetPathFromIDList(pIDList, wszDisplayName))
	{
		::CoTaskMemFree(pIDList);
		return false;
    }

	/*
	std::wstring path;
	path = wszDisplayName;

	int index = (int)path.find_last_of(L'\\');
	if (index != path.npos && index != (((int)path.size()) - 1))
	{
		path += L"\\";
	}
	*/

    // szDisplayNameに選択されたフォルダのパスが入っています
	dst = wszDisplayName;

    ::CoTaskMemFree(pIDList);
	return true;
}



////////////////////////////////////////
// ファイル関連
std::wstring toFullpath(const std::wstring& current, const std::wstring& path)
{
	namespace fs = boost::filesystem;

	std::wstring folder;
	fs::path p(path);

	if (p.is_absolute())
	{
		folder = p.wstring();
	}
	else
	{
		folder = (current / p.relative_path()).wstring();
	}
	return folder;
}

std::wstring pathToFile(const std::wstring& path)
{
	std::wstring str;
	size_t pos = path.find_last_of(L'\\');
	if (pos == std::wstring::npos)
	{
		return path;
	}
	return std::wstring(path.begin() + pos + 1, path.end());
}

bool getNextFilePath(const std::wstring& src, std::wstring& dst, bool loop, bool prev)
{
	if (src.empty()) { return false; }

	namespace fs = boost::filesystem;

	try
	{
		fs::path path(src);

		//DebugText(L"path.parent_path():%s\n", (path.parent_path() / L"*").wstring().c_str());

		CFileFind find;

		std::vector<std::wstring> data;
		BOOL res = find.FindFile((path.parent_path() / L"*").wstring().c_str());
		while (res)
		{
			res = find.FindNextFile();
			if (!find.IsDirectory() && !find.IsDots())
			{
				CString v = find.GetFilePath();
				data.push_back((LPCWSTR)v);
			}
		}

		/*
		fs::path path(src);

		std::vector<std::wstring> data;
		for (auto it = fs::directory_iterator(path.parent_path()); it != fs::directory_iterator(); ++it)
		{
			if (!fs::is_directory(*it))
			{
				data.push_back(it->path().wstring());
			}
		}
		*/
		
		std::stable_sort(data.begin(), data.end(),
			[=](const std::wstring& l, const std::wstring& r)->bool{
				bool res = StrCmpLogicalW(l.c_str(), r.c_str()) < 0;
				return prev ? !res : res;
		});

		bool flg = false;
		std::wstring org = path.wstring();
		for (auto it = data.begin(); it != data.end(); ++it)
		{
			if (flg)
			{
				dst = *it;
				return true;
			}

			if (org == *it)
			{
				flg = true;
			}
		}

		if (loop)
		{
			try {
				dst = data.at(0);
				return true;
			} catch (...) {}
		}
	}
	catch (...) {}
	return false;
}

boost::posix_time::ptime getFileTime(const std::wstring& path)
{
	using namespace boost::posix_time;
	using namespace boost::gregorian;

	HANDLE file;
	FILETIME filetime;
	FILETIME localtime;
	SYSTEMTIME systime;

	file = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		if (GetFileTime(file , &filetime , NULL , NULL))
		{
			CloseHandle(file);

			if (FileTimeToLocalFileTime(&filetime, &localtime))
			{
				if (FileTimeToSystemTime(&localtime , &systime))
				{
					date d(systime.wYear, systime.wMonth, systime.wDay);
					return ptime(d, hours(systime.wHour) + minutes(systime.wMinute) + seconds(systime.wSecond));
				}
			}
		}
		CloseHandle(file);
	}

	/*
	namespace fs = boost::filesystem;

	try
	{
		std::time_t last = fs::last_write_time(path);
		boost::posix_time::ptime time = boost::posix_time::from_time_t(last);

		boost::date_time::c_local_adjustor<boost::posix_time::ptime> adj1;

		return adj1.utc_to_local(time);
	}
	catch (...) {}
	*/
	return ptime();
}


bool selectExecute(bool def, const std::wstring& path, const std::wstring& command)
{
	if (command.empty())
	{
		return false;
	}

	if (def)
	{
		return ::ShellExecute(NULL, L"open", command.c_str(), NULL, NULL, SW_SHOWNORMAL) > (HINSTANCE)32;
	}
	return utl::processExecute(path, command);
}

bool processExecute(const std::wstring& path, const std::wstring& command, DWORD& id)
{
	namespace fs = boost::filesystem;

	if (path.empty()) { return false; }

	std::wstring line = L"\"" + path + L"\"";
	if (!command.empty())
	{
		line += L" " + command;
	}

	std::vector<wchar_t> buff(line.begin(), line.end());
	buff.push_back(L'\0');

	fs::path p(path);
	std::wstring parent = p.parent_path().wstring();

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};
	BOOL res = CreateProcess(
		NULL,
		buff.data(),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		//p.is_absolute() ? parent.c_str() : NULL,
		parent.empty() ? NULL : parent.c_str(),
		&si,
		&pi);

	if (pi.hProcess) { CloseHandle(pi.hProcess); }
	if (pi.hThread) { CloseHandle(pi.hThread); }

	if (res)
	{
		id = pi.dwProcessId;
	}

	return res == TRUE;
}

bool processExecute(const std::wstring& path, const std::wstring& command)
{
	DWORD id;
	return processExecute(path, command, id);
}

bool getFolderPath(const std::wstring& src, std::vector<std::wstring>& path)
{
	if (src.empty()) { return false; }

	namespace fs = boost::filesystem;

	try
	{
		fs::path p(src);
		CFileFind find;
		BOOL res = find.FindFile((p.parent_path() / L"*").wstring().c_str());
		while (res)
		{
			res = find.FindNextFile();
			if (find.IsDirectory() && !find.IsDots())
			{
				CString v = find.GetFilePath();
				path.push_back((LPCWSTR)v);
			}
		}

		std::stable_sort(path.begin(), path.end(),
			[=](const std::wstring& l, const std::wstring& r)->bool{
			bool res = StrCmpLogicalW(l.c_str(), r.c_str()) < 0;
			return res;
		});

		return true;
	}
	catch (...) {}
	return false;
}


////////////////////////////////////////
// 時間関連

bool DateTime::set(const boost::posix_time::ptime& time)
{
	time_ = time;
	return true;
}

bool DateTime::set(
	const std::wstring& text,
	const std::wstring& rx,
	int year, int month, int day,
	int hour, int min, int sec)
{
	using namespace boost::xpressive;
	using namespace boost::posix_time;
	using namespace boost::gregorian;

	try {
		wsmatch what;
		if (regex_search(text, what, wsregex::compile(rx)))//throw
		{
			int yearx, monthx, dayx, hourx, minx, secx;

			yearx	= wcstol(what[year].str().c_str(), NULL, 10);
			monthx	= wcstol(what[month].str().c_str(), NULL, 10);
			dayx	= wcstol(what[day].str().c_str(), NULL, 10);
			hourx	= wcstol(what[hour].str().c_str(), NULL, 10);
			minx	= wcstol(what[min].str().c_str(), NULL, 10);
			secx	= wcstol(what[sec].str().c_str(), NULL, 10);

			date d(yearx, monthx, dayx);//throw
			time_ = ptime(d, hours(hourx) + minutes(minx) + seconds(secx));
			return true;
		}
	}
	catch(...) {}

	time_ = ptime();
	return false;
}

boost::posix_time::ptime DateTime::get(LONGLONG nanodiff)
{
	using namespace boost::posix_time;
	return time_ + seconds(static_cast<long>(static_cast<double>(nanodiff) / 10000000.0));
}

void DateTime::clear()
{
	time_ = boost::posix_time::ptime();
}

bool DateTime::valid()
{
	return !time_.is_not_a_date_time();
}



double round(double r)
{
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

long lround(double r)
{
	return (r > 0.0) ? (long)floor(r + 0.5) : (long)ceil(r - 0.5);
}


void convertTimes(LONGLONG time, int& hour, int& min, int& sec)
{
	int total = static_cast<int>(static_cast<double>(time) / 10000000.0);
	hour = total / 3600;
	min = (total % 3600) / 60;
	sec = (total % 3600) % 60;
}

std::wstring convertTimes(LONGLONG time, bool variable)
{
	int hour = 0, min = 0, sec = 0;
	convertTimes(time, hour, min, sec);

	wchar_t buff[1024];

	if (variable)
	{
		if (hour > 0)
		{
			swprintf_s(buff, L"%02d:%02d:%02d", hour, min, sec);
		}
		else
		{
			swprintf_s(buff, L"%02d:%02d", min, sec);
		}
	}
	else
	{
		swprintf_s(buff, L"%02d:%02d:%02d", hour, min, sec);
	}

	return buff;
}

std::wstring convertDate(const boost::posix_time::ptime& time, int dateFormat, int timeFormat)
{
	wchar_t dateBuff[256] = {0}, timeBuff[256] = {0};

	switch(dateFormat)
	{
	case sl::DATETIME_DATE_FORMAT_ENGLISH:
		swprintf_s(
			dateBuff,
			L"%04d/%02d/%02d (%s) ",
			(unsigned short)time.date().year(), time.date().month().as_number(), time.date().day().as_number(),
			convertWeekEn(time.date().day_of_week().as_number()));
			// time.date().day_of_week().as_short_wstring()
		break;

	case sl::DATETIME_DATE_FORMAT_KANJI:
		swprintf_s(
			dateBuff,
			L"%d年%d月%d日(%s) ",
			(unsigned short)time.date().year(), time.date().month().as_number(), time.date().day().as_number(),
			convertWeek(time.date().day_of_week().as_number()));
		break;

	case sl::DATETIME_DATE_FORMAT_NONE:
	default:
		swprintf_s(
			dateBuff,
			L"%02d/%02d/%02d ",
			(unsigned short)time.date().year() % 100, time.date().month().as_number(), time.date().day().as_number());
		break;
	}

	switch(timeFormat)
	{
	case sl::DATETIME_TIME_FORMAT_12_ENGLISH:
		{
			int hour = time.time_of_day().hours();
			if (hour > 12)
			{
				swprintf_s(
					timeBuff,
					L"PM %02d:%02d:%02d",
					hour - 12, time.time_of_day().minutes(), time.time_of_day().seconds());
			}
			else
			{
				swprintf_s(
					timeBuff,
					L"AM %02d:%02d:%02d",
					hour, time.time_of_day().minutes(), time.time_of_day().seconds());
			}
			break;
		}

	case sl::DATETIME_TIME_FORMAT_12_KANJI:
		{
			int hour = time.time_of_day().hours();
			if (hour > 12)
			{
				swprintf_s(
					timeBuff,
					L"午後 %02d:%02d:%02d",
					hour - 12, time.time_of_day().minutes(), time.time_of_day().seconds());
			}
			else
			{
				swprintf_s(
					timeBuff,
					L"午前 %02d:%02d:%02d",
					hour, time.time_of_day().minutes(), time.time_of_day().seconds());
			}
			break;
		}

	case sl::DATETIME_TIME_FORMAT_24:
	default:
		swprintf_s(
			timeBuff,
			L"%02d:%02d:%02d",
			time.time_of_day().hours(), time.time_of_day().minutes(), time.time_of_day().seconds());
		break;
	}

	return std::wstring(dateBuff) + std::wstring(timeBuff);
}

const wchar_t* convertWeek(WORD wDayOfWeek)
{
	const wchar_t* week[] = { L"日", L"月", L"火", L"水", L"木", L"金", L"土" };

	if (wDayOfWeek < 0 || wDayOfWeek > 6)
	{
		return L"";
	}

	return week[wDayOfWeek];
}

const wchar_t* convertWeekEn(WORD wDayOfWeek)
{
	const wchar_t* week[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };

	if (wDayOfWeek < 0 || wDayOfWeek > 6)
	{
		return L"";
	}

	return week[wDayOfWeek];
}

////////////////////////////////////////////////////////////////////
// アスペクト比適用

bool getAspectRect(const SIZE& aspect, const RECT& src, RECT& dst)
{
	if (aspect.cx <= 0 || aspect.cy <= 0)
	{
		return false;
	}

	RECT rc = src;
	
	long cx = rc.right - rc.left;
	long cy = rc.bottom - rc.top;

	long tx = (long)utl::lround((double)cy * ((double)aspect.cx / (double)aspect.cy));
	long ty = (long)utl::lround((double)cx * ((double)aspect.cy / (double)aspect.cx));

	long sx = tx > cx ? cx : tx;
	long sy = tx > cx ? ty : cy;

	long px = (long)(((double)cx / 2.0) - ((double)sx / 2.0));
	long py = (long)(((double)cy / 2.0) - ((double)sy / 2.0));

	rc.left = px;
	rc.top = py;
	rc.right = px + sx;
	rc.bottom = py + sy;

	dst = rc;
	return true;
}

////////////////////////////////////////////////////////////////////
// ウィンドウ枠
// srcに枠を足す
void inflateWindowRect(RECT& src, bool title, bool frame)
{
	CRect rc(src);

	if (title && frame)
	{ // フレーム/タイトルバー
		int cyCaption = GetSystemMetrics(SM_CYCAPTION);
		int cxFrame = GetSystemMetrics(SM_CXFRAME);
		int cyFrame = GetSystemMetrics(SM_CYFRAME);

		rc.InflateRect(0, cyCaption, 0, 0);
		rc.InflateRect(cxFrame, cyFrame);

		//DebugText(L"+(title && frame) cyCaption:%d cxFrame:%d cyFrame:%d\n", cyCaption, cxFrame, cyFrame);
	}
	else if (title)
	{// タイトルバー
		int cyCaption = GetSystemMetrics(SM_CYCAPTION);
		int cxBorder = GetSystemMetrics(SM_CXBORDER);
		int cyBorder = GetSystemMetrics(SM_CYBORDER);

		rc.InflateRect(0, cyCaption, 0, 0);
		rc.InflateRect(cxBorder, cyBorder);

		//DebugText(L"+(title) cyCaption:%d cxBorder:%d cyBorder:%d\n", cyCaption, cxBorder, cyBorder);
	}
	else if (frame)
	{ // フレーム
		int cxFrame = GetSystemMetrics(SM_CXFRAME);
		int cyFrame = GetSystemMetrics(SM_CYFRAME);
		int cxBorder = GetSystemMetrics(SM_CXBORDER);
		int cyBorder = GetSystemMetrics(SM_CYBORDER);

		rc.InflateRect(cxFrame - cxBorder, cyFrame - cyBorder);

		//DebugText(L"+(frame) cxFrame:%d cyFrame:%d cxBorder:%d cyBorder:%d\n", cxFrame, cyFrame, cxBorder, cyBorder);
	}
	src = rc;
}

// srcから枠を引く
void deflateWindowRect(RECT& src, bool title, bool frame)
{
	CRect rc(src);

	if (title && frame)
	{ // フレーム/タイトルバー
		int cyCaption = GetSystemMetrics(SM_CYCAPTION);
		int cxFrame = GetSystemMetrics(SM_CXFRAME);
		int cyFrame = GetSystemMetrics(SM_CYFRAME);

		rc.DeflateRect(0, cyCaption, 0, 0);
		rc.DeflateRect(cxFrame, cyFrame);

		//DebugText(L"-(title && frame) cyCaption:%d cxFrame:%d cyFrame:%d\n", cyCaption, cxFrame, cyFrame);
	}
	else if (title)
	{// タイトルバー
		int cyCaption = GetSystemMetrics(SM_CYCAPTION);
		int cxBorder = GetSystemMetrics(SM_CXBORDER);
		int cyBorder = GetSystemMetrics(SM_CYBORDER);

		rc.DeflateRect(0, cyCaption, 0, 0);
		rc.DeflateRect(cxBorder, cyBorder);

		//DebugText(L"-(title) cyCaption:%d cxBorder:%d cyBorder:%d\n", cyCaption, cxBorder, cyBorder);
	}
	else if (frame)
	{ // フレーム
		int cxFrame = GetSystemMetrics(SM_CXFRAME);
		int cyFrame = GetSystemMetrics(SM_CYFRAME);
		int cxBorder = GetSystemMetrics(SM_CXBORDER);
		int cyBorder = GetSystemMetrics(SM_CYBORDER);

		rc.DeflateRect(cxFrame - cxBorder, cyFrame - cyBorder);

		//DebugText(L"-(frame) cxFrame:%d cyFrame:%d cxBorder:%d cyBorder:%d\n", cxFrame, cyFrame, cxBorder, cyBorder);
	}
	src = rc;
}



int windowFramePos(const RECT& src, CPoint &pt, int size)
{
	CRect rc(src);
	if (!rc.PtInRect(pt))
	{
		return FRAMEPOS_NONE;
	}
	
	rc.DeflateRect(size, size);// サイズ変更境界幅

	int nEx = size; // 隅の拡張サイズ


	if (pt.y > (rc.bottom - nEx) && pt.x > (rc.right - nEx))
	{
		return FRAMEPOS_BOTTOMRIGHT;
	}
	else if (pt.y > (rc.bottom - nEx) && pt.x < (rc.left + nEx))
	{
		return FRAMEPOS_BOTTOMLEFT;
	}
	else if (pt.y < (rc.top + nEx) && pt.x > (rc.right - nEx))
	{
		return FRAMEPOS_TOPRIGHT;
	}
	else if (pt.y < (rc.top + nEx) && pt.x < (rc.left + nEx))
	{
		return FRAMEPOS_TOPLEFT;
	}
	else if (pt.y > rc.bottom)
	{
		return FRAMEPOS_BOTTOM;
	}
	else if (pt.x > rc.right)
	{
		return FRAMEPOS_RIGHT;
	}
	else if (pt.x < rc.left)
	{
		return FRAMEPOS_LEFT;
	}
	else if (pt.y < rc.top) // 上位置
	{
		return FRAMEPOS_TOP;
	}

	return FRAMEPOS_NONE;
}


////////////////////////////////////////
// ウィンドウデフォルトサイズ指定
RECT defWindowRect(int width, int height)
{
	RECT dst = {};
	CRect rc;
	::GetWindowRect(GetDesktopWindow(), &rc);

	if ((rc.Width() / 2) - (width / 2) < 0 || (rc.Height() / 2) - (height / 2) < 0)
	{// デスクトップがウィンドウより小さい
		dst.left = 0;
		dst.top = 0;
		dst.right = width;
		dst.bottom = height;
	}
	else
	{// デスクトップ中央に配置
		dst.left = (rc.Width() / 2) - (width / 2);
		dst.top = (rc.Height() / 2) - (height / 2);
		dst.right = dst.left + width;
		dst.bottom = dst.top + height;
	}
	return dst;
}

WINDOWPLACEMENT defWindowPlacement(int width, int height)
{
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(WINDOWPLACEMENT);

	wp.showCmd = SW_SHOWNORMAL;

	CRect rc;
	::GetWindowRect(GetDesktopWindow(), &rc);

	if ((rc.Width() / 2) - (width / 2) < 0 || (rc.Height() / 2) - (height / 2) < 0)
	{// デスクトップがウィンドウより小さい
		wp.rcNormalPosition.left = 0;
		wp.rcNormalPosition.top = 0;
		wp.rcNormalPosition.right = width;
		wp.rcNormalPosition.bottom = height;
	}
	else
	{// デスクトップ中央に配置
		wp.rcNormalPosition.left = (rc.Width() / 2) - (width / 2);
		wp.rcNormalPosition.top = (rc.Height() / 2) - (height / 2);
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + width;
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + height;
	}
	return wp;
}


////////////////////////////////////////
// デスクトップ関数
BOOL getDesktopRectFromPoint(const POINT pt, MONITORINFO &mi)
{
	HMONITOR mon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

	if (mon != NULL)
	{
		mi.cbSize = sizeof(MONITORINFO);
		if (::GetMonitorInfo(mon, &mi))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL getDesktopRectFromRect(const RECT &rcFrom, MONITORINFO &mi)
{
	HMONITOR mon = ::MonitorFromRect(&rcFrom, MONITOR_DEFAULTTOPRIMARY);

	if (mon != NULL)
	{
		mi.cbSize = sizeof(MONITORINFO);
		if (::GetMonitorInfo(mon, &mi))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL getDesktopRectFromWindow(const HWND hWnd, MONITORINFO &mi)
{
	HMONITOR mon = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

	if (mon != NULL)
	{
		mi.cbSize = sizeof(MONITORINFO);
		if (::GetMonitorInfo(mon, &mi))
		{
			return TRUE;
		}
	}

	return FALSE;
}


//void setWindowSnap(HWND hwnd, LPPOINT lppt, int distance)
//{
//	if (hwnd == NULL) { return; }
//	
//	bool move = false;
//	MONITORINFO mi;
//	CRect desktop, client, org, shape;
//
//	GetWindowRect(hwnd, &org);
//	getWindowShapeRect(hwnd, &shape);
//
//	LONG left = shape.left - org.left;
//	LONG top = shape.top - org.top;
//
//	lppt->x += left;
//	lppt->y += top;
//
//	client.left = lppt->x - left;
//	client.top = lppt->y - top;
//	client.right = lppt->x + shape.Width();
//	client.bottom = lppt->y + shape.Height();
//
//	if (getDesktopRectFromWindow(hwnd, mi))
//	{
//		// ワークエリア(タスクバーを除外したサイズ)
//		desktop = mi.rcWork;
//
//		desktop.left -= left;
//		desktop.top -= top;
//		
//		if (client.left < (desktop.left + distance) &&
//			client.left > (desktop.left - distance) &&
//			client.top < (desktop.top + distance) &&
//			client.top > (desktop.top - distance))
//		{
//			SetWindowPos(hwnd, NULL, desktop.left, desktop.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//
//		if (client.left < (desktop.left + distance) &&
//			client.left > (desktop.left - distance) &&
//			client.bottom < (desktop.bottom + distance) &&
//			client.bottom > (desktop.bottom - distance))
//		{
//			SetWindowPos(hwnd, NULL, desktop.left, desktop.bottom - client.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//
//		if (client.right < (desktop.right + distance) &&
//			client.right > (desktop.right - distance) &&
//			client.top < (desktop.top + distance) &&
//			client.top > (desktop.top - distance))
//		{
//			SetWindowPos(hwnd, NULL, desktop.right - client.Width(), desktop.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//
//		if (client.right < (desktop.right + distance) &&
//			client.right > (desktop.right - distance) &&
//			client.bottom < (desktop.bottom + distance) &&
//			client.bottom > (desktop.bottom - distance))
//		{
//			SetWindowPos(hwnd, NULL, desktop.right - client.Width(), desktop.bottom - client.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//
//
//		if (client.left < (desktop.left + distance) &&
//			client.left > (desktop.left - distance))
//		{
//			SetWindowPos(hwnd, NULL, desktop.left, client.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//		
//		if (client.right > (desktop.right - distance) &&
//			client.right < (desktop.right + distance))
//		{
//			SetWindowPos(hwnd, NULL, desktop.right - client.Width(), client.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//
//		if (client.top < (desktop.top + distance) &&
//			client.top > (desktop.top - distance))
//		{
//			SetWindowPos(hwnd, NULL, client.left, desktop.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//		
//		if (client.bottom > (desktop.bottom - distance) &&
//			client.bottom < (desktop.bottom + distance))
//		{
//			SetWindowPos(hwnd, NULL, client.left, desktop.bottom - client.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//			return;
//		}
//	}
//	SetWindowPos(hwnd, NULL, client.left, client.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//}





HMODULE DWMLoader::module_ = NULL;
DWMLoader::PDwmIsCompositionEnabled DWMLoader::fpDwmIsCompositionEnabled = NULL;
DWMLoader::PDwmGetWindowAttribute DWMLoader::fpDwmGetWindowAttribute = NULL;



DWMLoader::DWMLoader()
{
	if (module_ == NULL)
	{
		TCHAR dir[MAX_PATH];
		GetSystemDirectory(dir, sizeof(dir));

		wcscat_s(dir, _T("\\dwmapi.dll"));

		module_ = ::LoadLibrary(dir);
		if (module_)
		{
			fpDwmIsCompositionEnabled = (PDwmIsCompositionEnabled)GetProcAddress(module_, "DwmIsCompositionEnabled");
			fpDwmGetWindowAttribute = (PDwmGetWindowAttribute)GetProcAddress(module_, "DwmGetWindowAttribute");
		}
	}
}

DWMLoader::~DWMLoader()
{
	if (module_) { ::FreeLibrary(module_); }
}

void DWMLoader::Init()
{
	static DWMLoader instance; // シングルトン
}

HRESULT WINAPI DWMLoader::DwmIsCompositionEnabled(_Out_ BOOL *pfEnabled)
{
	if (!fpDwmIsCompositionEnabled) { return E_NOTIMPL; }
	return fpDwmIsCompositionEnabled(pfEnabled);
}

HRESULT WINAPI DWMLoader::DwmGetWindowAttribute(HWND hwnd, DWORD dwAttribute, _Out_ PVOID pvAttribute, DWORD cbAttribute)
{
	if (!fpDwmGetWindowAttribute) { return E_NOTIMPL; }
	return fpDwmGetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
}

////////////////////////////////////////
// DWM関数を使用して見かけ上のウィンドウ外形を得る

bool checkWindowShapeRect(const CRect& window, const CRect& shape)
{
	CRect diff;
	diff.TopLeft() = shape.TopLeft() - window.TopLeft();
	diff.BottomRight() = window.BottomRight() - shape.BottomRight();

	if (diff.top >= 0 && diff.left >= 0 && diff.right >= 0 && diff.bottom >= 0)
	{// 可視と不可視外形の差がマイナスでなければ正常
		return true;
	}
	return false;
}

BOOL getWindowShapeRect(HWND hwnd, LPRECT lprc)
{
	CRect window;
	GetWindowRect(hwnd, window);

	BOOL enable = FALSE;
	HRESULT hr = DWMLoader::DwmIsCompositionEnabled(&enable);
	if (SUCCEEDED(hr))
	{
		if (enable)
		{
			CRect shape;
			hr = DWMLoader::DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, shape, sizeof(RECT));
			if (SUCCEEDED(hr))
			{
				if (checkWindowShapeRect(window, shape))
				{
					*lprc = shape;
					return TRUE;
				}

				//gl_.addLog(L"window l:%d t:%d r:%d b:%d", window.left, window.top, window.right, window.bottom);
				//gl_.addLog(L"shape l:%d t:%d r:%d b:%d", shape.left, shape.top, shape.right, shape.bottom);

				CClientDC dc(CWnd::FromHandle(hwnd));
				double scaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0;
				double scaleY = GetDeviceCaps(dc, LOGPIXELSX) / 96.0;
				if (scaleX > 1.0 || scaleY > 1.0)
				{
					shape.left = (LONG)(shape.left * scaleX);
					shape.top = (LONG)(shape.top * scaleY);
					shape.right = (LONG)(shape.right * scaleX);
					shape.bottom = (LONG)(shape.bottom * scaleY);

					//gl_.addLog(L"scaleX:%0.2f scaleY:%0.2f", scaleX, scaleY);
					//gl_.addLog(L"shape+ l:%d t:%d r:%d b:%d", shape.left, shape.top, shape.right, shape.bottom);

					if (checkWindowShapeRect(window, shape))
					{
						*lprc = shape;
						return TRUE;
					}
				}
			}
		}
	}

	if (isWindows10())
	{// Windows10の場合のみ
		CRect client;
		GetClientRect(hwnd, client);
		ClientToScreen(hwnd, &client.TopLeft());
		ClientToScreen(hwnd, &client.BottomRight());
		if (client != window)
		{// ウィンドウ枠があるなら
			CRect diff;
			diff.TopLeft() = client.TopLeft() - window.TopLeft();
			diff.BottomRight() = window.BottomRight() - client.BottomRight();

			if (diff.top >= 0 && diff.left >= 0 && diff.right >= 0 && diff.bottom >= 0)
			{// 可視と不可視外形の差がマイナスでなければ正常
			 //gl_.addLog(L"window l:%d t:%d r:%d b:%d", window.left, window.top, window.right, window.bottom);
			 //gl_.addLog(L"client l:%d t:%d r:%d b:%d", client.left, client.top, client.right, client.bottom);
			 //gl_.addLog(L"diff l:%d t:%d r:%d b:%d", diff.left, diff.top, diff.right, diff.bottom);

				window.DeflateRect(
					min(diff.left, 7),
					0,
					min(diff.right, 7),
					min(diff.bottom, 7));
			}
		}
	}
	*lprc = window;
	return TRUE;
}


////////////////////////////////////////
// ドラッグ＆ドロップ
void dragAcceptFiles(HWND hwnd)
{
	DragAcceptFiles(hwnd, TRUE);

	// Windows Vista以降のメッセージフィルターのための設定
	// LoadLibはXP以前だとエラーになる関数を呼ぶため
	HINSTANCE hInstance;
	if (hInstance = LoadLibrary(L"user32.dll"))
	{
		typedef BOOL (WINAPI *CWMFP)(UINT, DWORD);
		CWMFP cwmf = (CWMFP)GetProcAddress(HMODULE(hInstance), "ChangeWindowMessageFilter");
		if (cwmf)
		{
			cwmf(WM_DROPFILES, MSGFLT_ADD);
			cwmf(WM_COPYDATA, MSGFLT_ADD);
			cwmf(0x0049, MSGFLT_ADD);
			FreeLibrary(hInstance);
		}
	}
}

bool dragQueryFiles(HDROP drop, std::vector<std::wstring>& files)
{
	UINT count = DragQueryFile(drop, 0xffffffff, NULL, 0);

	for (int i = 0; i != count; ++i)
	{
		UINT size = DragQueryFile(drop, i, NULL, 0);
		std::vector<wchar_t> buff;
		buff.resize(size + 1);

		DragQueryFile(drop, i, buff.data(), (UINT)buff.size());
		std::wstring str(buff.data());

		files.push_back(str);
	}

	return count > 0 ? true : false;
}


// 2色のグラデーションを描画する関数です
BOOL twoColorsGradient(
    HDC hdc,            // 描画先のデバイスコンテキスト・ハンドルです
    const RECT *pRect,  // 描画する範囲の矩形です
    COLORREF Color1,    // 描画する一つ目の色です
    COLORREF Color2,    // 描画する二つ目の色です
    BOOL fHorizontal    // 水平のグラデーションを描画する場合は TRUE にします
)
{
    TRIVERTEX vert[2];
    GRADIENT_RECT rect = {0, 1};

    // 描画範囲と色を設定します
    vert[0].x = pRect->left;
    vert[0].y = pRect->top;
    vert[0].Red   = GetRValue(Color1) << 8;
    vert[0].Green = GetGValue(Color1) << 8;
    vert[0].Blue  = GetBValue(Color1) << 8;
    vert[0].Alpha = 0;
    vert[1].x = pRect->right;
    vert[1].y = pRect->bottom;
    vert[1].Red   = GetRValue(Color2) << 8;
    vert[1].Green = GetGValue(Color2) << 8;
    vert[1].Blue  = GetBValue(Color2) << 8;
    vert[1].Alpha = 0;
    return GradientFill(hdc, vert, 2, &rect, 1,
                        fHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
}

// 二つの色を混ぜ合わせる関数です
COLORREF mixColor(COLORREF Color1, COLORREF Color2, BYTE Ratio)
{
    int Alpha = 255 - Ratio;
    return RGB((GetRValue(Color1) * Ratio + GetRValue(Color2) * Alpha) / 255,
               (GetGValue(Color1) * Ratio + GetGValue(Color2) * Alpha) / 255,
               (GetBValue(Color1) * Ratio + GetBValue(Color2) * Alpha) / 255);
}

// テカったグラデーションを描画する関数です
void glossyGradient(
    HDC hdc,                   // 描画先のデバイスコンテキスト・ハンドルです
    const RECT *pRect,         // 描画する範囲の矩形です
    COLORREF Color1,           // 描画する一つ目の色です
    COLORREF Color2,           // 描画する二つ目の色です
    BOOL fHorizontal,  // 水平のグラデーションを描画する場合は TRUE にします
    BYTE GlossRatio1,     // 端のテカりの強さです
    BYTE GlossRatio2     // 中央のテカりの強さです
)
{
    // 中央の色を求めます
    COLORREF CenterColor = mixColor(Color1, Color2);

    RECT rc;

    // テカり部分の範囲を計算します(描画領域の半分です)
    rc.left = pRect->left;
    rc.top  = pRect->top;
    if (fHorizontal) {
        rc.right  = (rc.left + pRect->right) / 2;
        rc.bottom = pRect->bottom;
    } else {
        rc.right  = pRect->right;
        rc.bottom = (rc.top + pRect->bottom) / 2;
    }
    // テカり部分を描画します
    twoColorsGradient(hdc, &rc,
                      mixColor(RGB(255, 255, 255), Color1, GlossRatio1),
                      mixColor(RGB(255, 255, 255), CenterColor, GlossRatio2),
                      fHorizontal);

    // テカり以外の部分を描画します
    if (fHorizontal) {
        rc.left  = rc.right;
        rc.right = pRect->right;
    } else {
        rc.top    = rc.bottom;
        rc.bottom = pRect->bottom;
    }
    twoColorsGradient(hdc, &rc, CenterColor, Color2, fHorizontal);
}

void drawFrameRect(CDC& dc, const CRect& rc,
	COLORREF color1,
	COLORREF color2,
	COLORREF color3,
	COLORREF color4,
	COLORREF color5,
	COLORREF color6)
{
	twoColorsGradient(dc, &rc, color1, color2);

	{
		CPen pen(PS_SOLID, 1, color6);
		CPen* oldPen = dc.SelectObject(&pen);
		// 下
		dc.MoveTo(rc.left, rc.bottom - 1);
		dc.LineTo(rc.right, rc.bottom - 1);

		dc.SelectObject(oldPen);
	}

	{
		CPen pen(PS_SOLID, 1, color5);
		CPen* oldPen = dc.SelectObject(&pen);
		// 右
		dc.MoveTo(rc.right - 1, rc.top);
		dc.LineTo(rc.right - 1, rc.bottom);

		dc.SelectObject(oldPen);
	}

	{
		CPen pen(PS_SOLID, 1, color3);
		CPen* oldPen = dc.SelectObject(&pen);
		// 左
		dc.MoveTo(rc.left, rc.top);
		dc.LineTo(rc.left, rc.bottom);

		dc.SelectObject(oldPen);
	}

	{
		CPen pen(PS_SOLID, 1, color4);
		CPen* oldPen = dc.SelectObject(&pen);
		// 上
		dc.MoveTo(rc.left, rc.top);
		dc.LineTo(rc.right, rc.top);

		dc.SelectObject(oldPen);
	}
}

void drawFrameRect(CDC& dc, const CRect& rc, const std::array<COLORREF, 6>& color)
{
	drawFrameRect(dc, rc,
		color[0],
		color[1],
		color[2],
		color[3],
		color[4],
		color[5]);
}

void drawStatusBar(CDC& dc, const CRect& rc,
	COLORREF color1,
	COLORREF color2,
	COLORREF color3,
	COLORREF color4)
{
	twoColorsGradient(dc, &rc, color1, color2);

	{
		CPen pen(PS_SOLID, 1, color4);
		CPen* oldPen = dc.SelectObject(&pen);
		// 下
		dc.MoveTo(rc.left, rc.bottom - 1);
		dc.LineTo(rc.right, rc.bottom - 1);

		dc.SelectObject(oldPen);
	}

	{
		CPen pen(PS_SOLID, 1, color3);
		CPen* oldPen = dc.SelectObject(&pen);
		// 上
		dc.MoveTo(rc.left, rc.top);
		dc.LineTo(rc.right, rc.top);

		dc.SelectObject(oldPen);
	}
}

void drawStatusBar(CDC& dc, const CRect& rc, const std::array<COLORREF, 4>& color)
{
	drawStatusBar(dc, rc,
		color[0],
		color[1],
		color[2],
		color[3]);
}

void drawSeekBar(CDC& dc, const CRect& rc, int pos, int knob,
	COLORREF color1,
	COLORREF color2,
	COLORREF color3,
	COLORREF color4,
	COLORREF color5,
	COLORREF color6,
	COLORREF color7,
	COLORREF color8,
	COLORREF color9,
	COLORREF color10)
{
	// 再生済み背景
	CRect lc(rc);

	pos += lc.left;

	lc.right = pos;
	twoColorsGradient(dc.GetSafeHdc(), &lc, color1, color2);

	lc = rc;
	lc.left = pos;
	twoColorsGradient(dc.GetSafeHdc(), &lc, color3, color4);

	lc = rc;
	lc.left = pos - knob;
	lc.right = pos;
	lc.top = rc.top;
	lc.bottom = rc.bottom;
	twoColorsGradient(dc.GetSafeHdc(), &lc, color5, color6);


	// 再生済み境界
	{
		CPen pen(PS_SOLID, 1, color7);
		CPen* penOld = dc.SelectObject(&pen);
		// 上線(左上)
		dc.MoveTo(rc.left, rc.top);
		dc.LineTo(pos + 1, rc.top);

		dc.SelectObject(&penOld);
	}
	

	// 未再生境界
	{
		CPen pen(PS_SOLID, 1, color8);
		CPen* penOld = dc.SelectObject(&pen);
		// 上線(右上)
		dc.MoveTo(pos, rc.top);
		dc.LineTo(rc.right, rc.top);

		dc.SelectObject(&penOld);
	}

	// 再生済み境界
	{
		CPen pen(PS_SOLID, 1, color9);
		CPen* penOld = dc.SelectObject(&pen);
		// 下線(左下)
		dc.MoveTo(rc.left, rc.bottom - 1);
		dc.LineTo(pos, rc.bottom - 1);

		dc.SelectObject(&penOld);
	}

	// 未再生境界
	{
		CPen pen(PS_SOLID, 1, color10);
		CPen* penOld = dc.SelectObject(&pen);
		// 下線(右下)
		dc.MoveTo(pos, rc.bottom - 1);
		dc.LineTo(rc.right, rc.bottom - 1);

		dc.SelectObject(&penOld);
	}

	




	/*
	{// シーク位置
		CPen pen(PS_SOLID, 1, RGB(95, 95, 95));
		CPen* penOld = dc.SelectObject(&pen);

		dc.MoveTo(pos - 1, rc.top);
		dc.LineTo(pos - 1, rc.bottom + 1);

		dc.MoveTo(pos, rc.top);
		dc.LineTo(pos, rc.bottom + 1);

		dc.MoveTo(pos + 1, rc.top);
		dc.LineTo(pos + 1, rc.bottom + 1);

		dc.SelectObject(&penOld);
	}
	*/
}

void drawSeekBar(CDC& dc, const CRect& rc, int pos, int knob, const std::array<COLORREF, 10>& color)
{
	drawSeekBar(dc, rc, pos, knob,
		color[0],
		color[1],
		color[2],
		color[3],
		color[4],
		color[5],
		color[6],
		color[7],
		color[8],
		color[9]);
}

void drawSlider(CDC& dc, const CRect& rc, int pos)
{
	int center = rc.Height() / 2;
	int height = max(3, rc.Height() / 8);

	CRect bar(rc);
	bar.top = bar.bottom = center;
	bar.InflateRect(1, height);
	dc.FillRect(&bar, &CBrush(RGB(0, 0, 0)));


	bar.DeflateRect(1, 1);
	dc.FillRect(&bar, &CBrush(RGB(127, 127, 127)));

	CRect knob(rc);
	knob.top = knob.bottom = center;
	knob.left = knob.right = rc.left + pos;
	knob.InflateRect(height, height*2);
	dc.FillRect(&knob, &CBrush(RGB(0, 0, 0)));

	knob.DeflateRect(1, 1);
	dc.FillRect(&knob, &CBrush(RGB(255, 255, 255)));

	bar.right = max(bar.left, (bar.left + pos));
	dc.FillRect(&bar, &CBrush(RGB(255, 255, 255)));

	/*

	LOGBRUSH logBrush;
	logBrush.lbColor = RGB(127, 127, 127);
	logBrush.lbHatch = 0;
	logBrush.lbStyle = BS_SOLID;
	CPen pen1(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_FLAT|PS_JOIN_MITER, 3, &logBrush);
	CPen* penOld = dc.SelectObject(&pen1);

	dc.MoveTo(rc.left, center-1);
	dc.LineTo(rc.right, center-1);


	logBrush.lbColor = RGB(255, 255, 255);
	logBrush.lbHatch = 0;
	logBrush.lbStyle = BS_SOLID;
	CPen pen2(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_FLAT|PS_JOIN_MITER, 3, &logBrush);
	dc.SelectObject(&pen2);

	dc.MoveTo(rc.left, center-1);
	dc.LineTo(rc.left + pos, center-1);

	CRect rect(rc);

	rect.left = (rc.left + pos) - 2;
	rect.right = (rc.left + pos) + 2;

	rect.top = center - 5;
	rect.bottom = center + 5;
	dc.FillRect(&rect, &CBrush(RGB(255, 255, 255)));

	//dc.FillRect(&rc, &CBrush(RGB(127, 127, 127)));

	//CRect rect(rc);
	//rect.right = rc.left + pos;
	//dc.FillRect(&rect, &CBrush(RGB(255, 255, 255)));

	dc.SelectObject(&penOld);

	*/
}

void drawCross(CDC& dc, const CRect& rc)
{
	CRect xrc(rc);
	xrc.left = xrc.right - 18;

	xrc.top = xrc.Height() / 2;


	LOGBRUSH logBrush;
	logBrush.lbColor = RGB(127, 127, 127);
	logBrush.lbHatch = 0;
	logBrush.lbStyle = BS_SOLID;
	CPen pen(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_FLAT|PS_JOIN_MITER, 2, &logBrush);
	CPen* penOld = dc.SelectObject(&pen);

	dc.MoveTo(xrc.left, xrc.top-1);
	dc.LineTo(xrc.right, xrc.top-1);

	//dc.MoveTo(xrc.left, xrc.top);
	//dc.LineTo(xrc.right, xrc.bottom);

	//dc.MoveTo(xrc.right, xrc.top);
	//dc.LineTo(xrc.left, xrc.bottom);

	// 上線
	//dc.MoveTo(pos, rc.top);
	//dc.LineTo(rc.right + 1, rc.top);
	// 下線
	//dc.MoveTo(rc.left, rc.bottom - 1);
	//dc.LineTo(rc.right + 1, rc.bottom - 1);

	dc.SelectObject(&penOld);
}



CCBitmap::CCBitmap(CDC& dc, const CRect& rc)
	: oldBmp_(NULL)
{
	memDC_.CreateCompatibleDC(&dc);
	bmp_.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	oldBmp_ = memDC_.SelectObject(&bmp_);
}

CCBitmap::~CCBitmap()
{
	memDC_.SelectObject(oldBmp_);
	bmp_.DeleteObject();
}

BOOL CCBitmap::BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
	int xSrc, int ySrc, DWORD dwRop)
{
	return memDC_.BitBlt(x, y, nWidth, nHeight, pSrcDC, xSrc, ySrc, dwRop);
}

BOOL CCBitmap::AlphaBlend(
	HDC hdcDest,
	int xoriginDest,
	int yoriginDest,
	int wDest,
	int hDest,
	int xoriginSrc,
	int yoriginSrc,
	BYTE byteAlpha)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, byteAlpha, 0 };
	return ::AlphaBlend(hdcDest, xoriginDest, yoriginDest, wDest, hDest,
		memDC_, xoriginSrc, yoriginSrc, wDest, hDest, bf);
}







/////////////////////////////////////////////////////////////
// フォント

bool createFont(CFont& font, bool bold, bool underline, int point, const std::wstring& name)
{
	if (font.GetSafeHandle()) { font.DeleteObject(); }

	if (font.CreatePointFont(point, name.c_str()))
	{
		if (bold || underline)
		{
			LOGFONT lf = {};
			if (font.GetLogFont(&lf))
			{
				if (bold) { lf.lfWeight = FW_BOLD; }
				if (underline) { lf.lfUnderline = TRUE; }
				font.DeleteObject();
				font.CreateFontIndirect(&lf);
			}
		}
		return true;
	}
	return false;
	/*
	int weight = bold ? FW_BOLD : FW_REGULAR;

	return !!font.CreateFont(
		-height,				//フォント高さ
        0,                      //文字幅（高さと同じ）
        0,						//テキストの角度
        0,                      //ベースラインとｘ軸との角度
        weight,					//フォントの重さ（太さ）
        FALSE,                  //イタリック体
        FALSE,                  //アンダーライン
        FALSE,                  //打ち消し線
        SHIFTJIS_CHARSET,       //文字セット
        OUT_DEFAULT_PRECIS,     //出力精度
        CLIP_DEFAULT_PRECIS,    //クリッピング精度
        PROOF_QUALITY,          //出力品質
        FIXED_PITCH|FF_MODERN,  //ピッチとファミリー
        name.c_str());
	*/
}

int getFontHeight(HWND hwnd, CFont& font)
{
	HDC hdc = GetDC(hwnd);
	if (!hdc) { return 0; }

	CDC* dc = CDC::FromHandle(hdc);

	CFont *oldFont = dc->SelectObject(&font);

	int height = 0;
	TEXTMETRIC tm;
	if (dc->GetTextMetrics(&tm))
	{
		height = tm.tmHeight + tm.tmExternalLeading;
	}

	dc->SelectObject(oldFont);

	ReleaseDC(hwnd, dc->GetSafeHdc());
	return height;
}


int pointToHeight(int point)
{
    CClientDC dc(AfxGetMainWnd());

    int nHeight = ::MulDiv(point, dc.GetDeviceCaps(LOGPIXELSY), 72);
    return nHeight;
}
	
int heightToPoint(int height)
{
    CClientDC dc(AfxGetMainWnd());

    int nPoint = ::MulDiv(height, 72, dc.GetDeviceCaps(LOGPIXELSY));
    return nPoint;
}

int getEditHeight(HWND hwnd, CFont& font)
{
	int height = getFontHeight(hwnd, font);
	if (height <= 0)
	{
		CRect rc(0, 0, 0, 14); // 14 dialog units
		MapDialogRect(hwnd, rc);
		return rc.Height();
	}
	return (height * 65) / 40;
}

/////////////////////////////////////////////////////////////
// 文字列関連


void replaceAll(std::wstring& input, const std::wstring& search, const std::wstring& format)
{
	boost::algorithm::replace_all(input, search, format);
}

std::wstring replaceAllCopy(const std::wstring& input, const std::wstring& search, const std::wstring& format)
{
	return boost::algorithm::replace_all_copy(input, search, format);
}


void getLines(const std::wstring& text, bool trim, std::vector<std::wstring>& lines)
{
	using namespace boost::xpressive;
	try {
		wsregex rx;
		if (trim) { rx = bos >> _s | !_s >> _ln >> !_s | !_s >> eos; }
		else { rx = _ln | eos; } 

		for (wsregex_token_iterator it(text.begin(), text.end(), rx, -1), end; it != end; ++it)
		{
			lines.push_back(it->str());
		}
	} catch (...) {}
}

void getLines(const std::wstring& text, bool trim, boost::function<void(const std::wstring&)> func)
{
	using namespace boost::xpressive;
	try {
		wsregex rx;
		if (trim) { rx = bos >> _s | !_s >> _ln >> !_s | !_s >> eos; }
		else { rx = _ln | eos; } 

		for (wsregex_token_iterator it(text.begin(), text.end(), rx, -1), end; it != end; ++it)
		{
			func(it->str());
		}
	} catch (...) {}
}


bool decompress(std::vector<char>& dst, const std::vector<char>& src)
{
	namespace io = boost::iostreams;
	try
	{
		dst.clear();
		io::filtering_istreambuf in;
		in.push(io::gzip_decompressor());
		in.push(boost::make_iterator_range(src));
		io::copy(in, io::back_inserter(dst));
	}
	catch (const io::gzip_error&) { return false; }
	catch(...) { return false; }
	return true;
}

bool LCMapStr(DWORD flag, const std::wstring& src, std::wstring& dst)
{
	int len = LCMapStringW(GetUserDefaultLCID(),
		flag,
		src.c_str(), (int)src.size(),
		NULL, 0);

	std::vector<wchar_t> data;
	data.resize(len);

	len = LCMapStringW(GetUserDefaultLCID(),
		LCMAP_HALFWIDTH,
		src.c_str(), (int)src.size(),
		data.data(), (int)data.size());

	if (len == 0) { return false; }

	dst.assign(data.begin(), data.end());
	return true;
}

bool unencode(char x)
{
	const char code[] =
		"0123456789" 
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"*-._";

	const int size = sizeof(code) / sizeof(code[0]);

	for (int i = 0; i < size; ++i)
	{
		if (code[i] == x) { return true; }
	}
	return false;
}

std::string urlencode(const std::string& src)
{
	std::string dst;
	for (auto it = src.begin(); it != src.end(); ++it)
	{
		if (unencode(*it))
		{
			dst += *it;
		}
		else
		{
			dst += (boost::format("%%%02X") % (*it & 0xFF)).str();
		}
	}
	return dst;
}

std::string toUTF8(const std::wstring& src)
{
	const int size = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, NULL, 0, NULL, NULL);

	if (size < 0) { return std::string(); }

	std::vector<char> buff(size);

	int result = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, buff.data(), size, NULL, NULL);

	if (result < 0) { return std::string(); }

	return buff.data();
}

std::wstring toUTF16(const std::string& src)
{
	const int size = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, NULL, 0);

	if (size < 0) { return std::wstring(); }

	std::vector<wchar_t> buff(size);

	int result = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, buff.data(), size);

	if (result < 0) { return std::wstring(); }

	return buff.data();
}


/////////////////////////////////////////////////////////////
// IME

void IMEDisable()
{
	HWND hwnd = GetFocus();
	if (hwnd)
	{
		ImmAssociateContext(hwnd, NULL);
	}
}

void IMEDisable(HWND hwnd)
{
	if (hwnd)
	{
		ImmAssociateContext(hwnd, NULL);
	}
}

} // namespace utl

