#pragma once

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace utl {

class SyncThread {
	boost::recursive_mutex mutex_;
	boost::thread thread_;

public:
	void interrupt()
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		thread_.interrupt();
	}

	void join()
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		thread_.join();
	}

	bool timed_join(long seconds)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		return thread_.timed_join(boost::posix_time::seconds(seconds));
	}

	bool joinable()
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		return thread_.joinable();
	}

	void swap(boost::thread& x)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		thread_.swap(x);
	}
};

} // namespace utl