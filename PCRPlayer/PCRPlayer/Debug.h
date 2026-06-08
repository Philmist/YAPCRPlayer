
#pragma once


#include <stdarg.h>
#include <fstream>
#include <codecvt>

#ifdef _DEBUG
#	define DebugText(str, ...) \
	{ \
		wchar_t c[1024]; \
		swprintf_s(c, str, __VA_ARGS__); \
		OutputDebugString(c); \
	}
#else
#    define DebugText(str, ...) // 空実装
#endif

#ifdef _DEBUG
#	define DebugRect(str, rect) \
	{ \
		wchar_t c[1024]; \
		swprintf_s(c, str##L"l:%d t:%d r:%d b:%d\n", rect.left, rect.top, rect.right, rect.bottom); \
		OutputDebugString(c); \
	}
#else
#    define DebugRect(str, rect) // 空実装
#endif


// ログ出力クラス
// インスタンスを作れず、staticメンバの呼び出しのみできる

//#define LOGGER 1 // スイッチ

#ifdef LOGGER

#define LOGGER_PATH L"test.log" // ログファイルパス

#define LogText(str, ...) Logger::log(str, __VA_ARGS__)

class Logger {
	std::wofstream ofs_;

	Logger()
		: ofs_()
	{}

	~Logger()
	{
		if (ofs_.is_open())
		{
			std::wstring text(L"\n");
			ofs_.write(text.c_str(), text.size());
			ofs_.close();
		}
	}

public:
	static void log(const wchar_t* format, ...)
	{
		if (instance().open())
		{
			va_list args;
			wchar_t buf[1024];
			va_start(args, format);
			int size = vswprintf_s(buf, 1024, format, args);
			va_end(args);

			instance().ofs_.write(buf, size);
		}
	}

private:
	static Logger& instance()
	{
		static Logger log_;
		return log_;
	}

	bool open()
	{
		if (ofs_.is_open()) { return true; }

		ofs_.open(LOGGER_PATH, std::ios::app);

		if (ofs_.is_open())
		{
			ofs_.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

			std::wstring text(L"[" _STR2WSTR(__DATE__) L" " _STR2WSTR(__TIME__) L"]\n");
			ofs_.write(text.c_str(), text.size());
			return true;
		}
		return false;
	}
};


#else

#define LogText(str, ...) // 空実装

#endif // #ifdef LOGGER