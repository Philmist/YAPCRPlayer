#pragma once

#include <stdarg.h>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/optional.hpp>

namespace utl {

#define LOGLIMIT 1000

struct Log {
	enum LOGTYPE {
		LOGTYPE_GRAPH,
		LOGTYPE_FILTER,
		LOGTYPE_BBS,
		LOGTYPE_PEERCAST,
		LOGTYPE_NONE,
	};

	int type;
	SYSTEMTIME time;
	std::wstring desc;
	std::wstring detail;
};

struct StatusLog {
	struct Queue {
		Queue() : total(0), count(0) {}
		LONGLONG total;
		int count;
	};

	StatusLog() : valid(false), buffer(0) {}

	bool valid;
	LONGLONG buffer;
	Queue audio;
	Queue video;
};

class SyncLog {
	boost::recursive_mutex mutex_;
	std::deque<Log> log_;
	StatusLog status_;

public:
	SyncLog();
	virtual ~SyncLog();

	void clear();
	void add(int type, const wchar_t* desc, ...);
	void add(int type, const wchar_t* desc, va_list args);
	void add(int type, const std::wstring& desc, const wchar_t* detail, ...);
	void add(int type, const std::wstring& desc, const wchar_t* detail, va_list args);
	bool get(std::deque<Log>& log);
	bool get(std::deque<Log>& log, const boost::function<bool(const Log&)>& pred);

	void set(const StatusLog& status);
	void get(StatusLog& status);
};

} // namespace utl
