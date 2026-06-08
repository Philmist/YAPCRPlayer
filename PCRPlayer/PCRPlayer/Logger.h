#pragma once

#include "SyncLog.h"

namespace utl {

class Logger {
public:
	Logger(SyncLog& log, int type);
	virtual ~Logger();

protected:
	SyncLog& log_;
	int type_;

public:
	void clearLog();
	void addLog(const wchar_t* desc, ...);
	void addLog(const wchar_t* desc, va_list args);
	void detailLog(const std::wstring& desc, const wchar_t* detail, ...);
	void detailLog(const std::wstring& desc, const wchar_t* detail, va_list args);
	bool addLog(std::deque<Log>& log);
	bool getLog(std::deque<Log>& log);
	bool getLog(std::deque<Log>& log, const boost::function<bool(const Log&)>& pred);

	void setStatus(const StatusLog& status);
	void getStatus(StatusLog& status);
};

};

