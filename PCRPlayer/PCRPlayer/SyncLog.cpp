#include "stdafx.h"

namespace utl {

SyncLog::SyncLog()
{
}

SyncLog::~SyncLog()
{
}

void SyncLog::clear()
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	log_.clear();
}

void SyncLog::add(int type, const wchar_t* desc, ...)
{
	Log log = {0};
	log.type = type;
	GetLocalTime(&log.time);

	CString format;

	va_list args;
	va_start(args, desc);
	format.FormatV(desc, args);
	va_end(args);

	log.detail = std::wstring(format);

	boost::recursive_mutex::scoped_lock lock(mutex_);
	log_.push_back(log);
	if (log_.size() > LOGLIMIT)
	{
		log_.pop_front();
	}
}

void SyncLog::add(int type, const wchar_t* desc, va_list args)
{
	Log log = {0};
	log.type = type;
	GetLocalTime(&log.time);

	CString format;
	format.FormatV(desc, args);

	log.desc = std::wstring(format);

	boost::recursive_mutex::scoped_lock lock(mutex_);
	log_.push_back(log);
	if (log_.size() > LOGLIMIT)
	{
		log_.pop_front();
	}
}


void SyncLog::add(int type, const std::wstring& desc, const wchar_t* detail, ...)
{
	Log log = {0};
	log.type = type;
	GetLocalTime(&log.time);
	log.desc = desc;

	CString format;

	va_list args;
	va_start(args, detail);
	format.FormatV(detail, args);
	va_end(args);

	log.detail = std::wstring(format);

	boost::recursive_mutex::scoped_lock lock(mutex_);
	log_.push_back(log);
	if (log_.size() > LOGLIMIT)
	{
		log_.pop_front();
	}
}

void SyncLog::add(int type, const std::wstring& desc, const wchar_t* detail, va_list args)
{
	Log log = {0};
	log.type = type;
	GetLocalTime(&log.time);
	log.desc = desc;

	CString format;
	format.FormatV(detail, args);

	log.detail = std::wstring(format);

	boost::recursive_mutex::scoped_lock lock(mutex_);
	log_.push_back(log);
	if (log_.size() > LOGLIMIT)
	{
		log_.pop_front();
	}
}


bool SyncLog::get(std::deque<Log>& log)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	if (!log_.empty())
	{
		log = log_;
		log_.clear();
		return true;
	}
	return false;
}

bool SyncLog::get(std::deque<Log>& log, const boost::function<bool(const Log&)>& pred)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	if (!log_.empty())
	{
		bool exist = false;
		for (auto it = log_.begin(); it != log_.end(); ++it)
		{
			if (pred(*it))
			{
				log.push_back(*it);
				exist = true;
			}
		}
		log_.clear();
		return exist;
	}
	return false;
}

void SyncLog::set(const StatusLog& status)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	status_ = status;
}

void SyncLog::get(StatusLog& status)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	status = status_;
}

};