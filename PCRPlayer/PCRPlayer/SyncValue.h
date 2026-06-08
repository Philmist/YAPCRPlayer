#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include <boost/optional.hpp>

namespace utl {

template<typename T, typename Mutex = boost::recursive_mutex>
class SyncValue {
protected:
	boost::recursive_mutex mutex_;
	T value_;

public:
	SyncValue(){};
	SyncValue(const SyncValue& x)
		: value_(x.get())
	{}
	SyncValue(const T& x)
		: value_(x)
	{}

	virtual ~SyncValue(){};


	SyncValue& operator=(const T& x)
	{
		set(x);
		return *this;
	}

	T get()
	{
		typename Mutex::scoped_lock lock(mutex_);
		return value_;
	}

	void set(const T& x)
	{
		typename Mutex::scoped_lock lock(mutex_);
		value_ = x;
	}

	template<typename F>
	void apply(F f)
	{
		typename Mutex::scoped_lock lock(mutex_);
		f(value_);
	}

	template<typename F>
	bool func(F f)
	{
		typename Mutex::scoped_lock lock(mutex_);
		return f(value_);
	}
};


template<typename T, typename Mutex = boost::recursive_mutex>
class SyncOptional : public SyncValue<boost::optional<T>, Mutex> {
	typedef SyncValue<boost::optional<T>, Mutex> super;

public:
	SyncOptional(){};
	SyncOptional(const SyncOptional& x)
		: value_(x.get())
	{}
	SyncOptional(const T& x)
		: value_(boost::optional<T>(x))
	{}

	virtual ~SyncOptional(){};

	operator bool()
	{
		typename Mutex::scoped_lock lock(mutex_);
		return !!value_;
	}

	SyncOptional& operator=(const SyncOptional& x)
	{
		set(x);
		return *this;
	}

	SyncOptional& operator=(const T& x)
	{
		set(x);
		return *this;
	}

	//boost::optional<T> get()
	//{
	//	typename Mutex::scoped_lock lock(mutex_);
	//	return value_;
	//}

	bool get(T& x)
	{
		typename Mutex::scoped_lock lock(mutex_);
		if (value_)
		{
			x = *value_;
			return true;
		}
		return false;
	}

	void set(const SyncOptional& x)
	{
		typename Mutex::scoped_lock lock(mutex_);
		value_ = x.get();
	}

	void set(const T& x)
	{
		typename Mutex::scoped_lock lock(mutex_);
		value_ = boost::optional<T>(x);
	}

	template<typename F>
	bool apply(F f)
	{
		typename Mutex::scoped_lock lock(mutex_);
		if (value_)
		{
			f(*value_);
			return true;
		}
		return false;
	}

	template<typename F>
	bool func(F f)
	{
		typename Mutex::scoped_lock lock(mutex_);
		if (value_)
		{
			return f(*value_);
		}
		return false;
	}

	void clear()
	{
		typename Mutex::scoped_lock lock(mutex_);
		value_ = boost::optional<T>();
	}
};

} // namespace utl