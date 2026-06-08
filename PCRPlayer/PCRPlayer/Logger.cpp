#include "stdafx.h"

namespace utl {

Logger::Logger(SyncLog& log, int type)
	: log_(log)
	, type_(type)
{
}

Logger::~Logger()
{
}

void Logger::clearLog()
{
	log_.clear();
}

void Logger::addLog(const wchar_t* desc, ...)
{
	va_list args;
	va_start(args, desc);
	log_.add(type_, desc, args);
	va_end(args);
}

void Logger::addLog(const wchar_t* desc, va_list args)
{
	log_.add(type_, desc, args);
}

void Logger::detailLog(const std::wstring& desc, const wchar_t* detail, ...)
{
	va_list args;
	va_start(args, detail);
	log_.add(type_, desc, detail, args);
	va_end(args);
}

void Logger::detailLog(const std::wstring& desc, const wchar_t* detail, va_list args)
{
	log_.add(type_, desc, detail, args);
}

bool Logger::getLog(std::deque<Log>& log)
{
	return log_.get(log);
}

bool Logger::getLog(std::deque<Log>& log, const boost::function<bool(const Log&)>& pred)
{
	return log_.get(log, pred);
}

void Logger::setStatus(const StatusLog& status)
{
	log_.set(status);
}

void Logger::getStatus(StatusLog& status)
{
	log_.get(status);
}

};

