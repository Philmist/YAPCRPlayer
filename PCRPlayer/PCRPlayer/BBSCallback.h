#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>

namespace bbs {

template<typename T>
class BBSCallback {
public:
	typedef boost::function<void(T)> CallbackType;
	typedef boost::function<void(CWnd *, T)> ChildType;
	typedef boost::function<void(const CallbackType&)> ParentType;

protected:
	CWnd *wnd_;
	boost::recursive_mutex mutex_;
	ParentType parent_;
	ChildType child_;

public:
	BBSCallback(const ParentType& parent, const ChildType& callback)
		: wnd_(NULL)
		, parent_(parent)
		, child_(callback)
	{}

	void push(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (!wnd) { return; }
		wnd_ = wnd;
		parent_([this](T value){
			boost::recursive_mutex::scoped_lock lock(mutex_);
			child_(wnd_, value);
		});
	}

	void pop(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (wnd_ == wnd)
		{// 最後に設定したウィンドウだけコールバックを解除できる
			parent_(CallbackType());
			wnd_ = NULL;
		}
	}
};

template<>
class BBSCallback<void> {
public:
	typedef boost::function<void()> CallbackType;
	typedef boost::function<void(CWnd *)> ChildType;
	typedef boost::function<void(const CallbackType&)> ParentType;

protected:
	CWnd *wnd_;
	boost::recursive_mutex mutex_;
	ParentType parent_;
	ChildType child_;

public:
	BBSCallback(const ParentType& parent, const ChildType& callback)
		: wnd_(NULL)
		, parent_(parent)
		, child_(callback)
	{}

	void push(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (!wnd) { return; }
		wnd_ = wnd;
		parent_([this](){
			boost::recursive_mutex::scoped_lock lock(mutex_);
			child_(wnd_);
		});
	}

	void pop(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (wnd_ == wnd)
		{// 最後に設定したウィンドウだけコールバックを解除できる
			parent_(CallbackType());
			wnd_ = NULL;
		}
	}
};


template<typename T>
class BBSMultiCallback {
public:
	typedef boost::function<void(T)> CallbackType;
	typedef boost::function<void(CWnd *, T)> ChildType;
	typedef boost::function<void(const CallbackType&)> ParentType;

protected:
	std::set<CWnd*> set_;
	boost::recursive_mutex mutex_;
	ParentType parent_;
	ChildType child_;

public:
	BBSMultiCallback(const ParentType& parent, const ChildType& callback)
		: parent_(parent)
		, child_(callback)
	{}

	void push(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (!wnd) { return; }
		if (set_.find(wnd) != set_.end()) { return; }
		if (set_.empty())
		{
			parent_([this](T value){
				boost::recursive_mutex::scoped_lock lock(mutex_);
				for (auto it = set_.begin(); it != set_.end(); ++it)
				{
					child_(*it, value);
				}
			});
		}
		set_.insert(wnd);
	}

	void pop(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (!wnd) { return; }
		set_.erase(wnd);
		if (set_.empty())
		{
			parent_(CallbackType());
		}
	}
};


template<>
class BBSMultiCallback<void> {
public:
	typedef boost::function<void()> CallbackType;
	typedef boost::function<void(CWnd *)> ChildType;
	typedef boost::function<void(const CallbackType&)> ParentType;

protected:
	std::set<CWnd*> set_;
	boost::recursive_mutex mutex_;
	ParentType parent_;
	ChildType child_;

public:
	BBSMultiCallback(const ParentType& parent, const ChildType& callback)
		: parent_(parent)
		, child_(callback)
	{}

	void push(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (!wnd) { return; }
		if (set_.find(wnd) != set_.end()) { return; }
		if (set_.empty())
		{
			parent_([this](){
				boost::recursive_mutex::scoped_lock lock(mutex_);
				for (auto it = set_.begin(); it != set_.end(); ++it)
				{
					child_(*it);
				}
			});
		}
		set_.insert(wnd);
	}

	void pop(CWnd *wnd)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (!wnd) { return; }
		set_.erase(wnd);
		if (set_.empty())
		{
			parent_(CallbackType());
		}
	}
};

}// namespace bbs
