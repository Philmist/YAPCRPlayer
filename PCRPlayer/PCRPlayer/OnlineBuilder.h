#pragma once

#include "BaseBuilder.h"

namespace dsp {

class OnlineBuilder : public IBaseBuilder {
public:
	OnlineBuilder(utl::SyncLog& log, utl::SyncOptional<std::wstring>& text)
		: IBaseBuilder(log, text)
	{}
	virtual ~OnlineBuilder() {}

	virtual bool render(HWND hwnd, UINT msg, const std::wstring& path);
	virtual bool pause() { return false; }
	virtual bool togglePause() { return false; }
	virtual bool seek(LONGLONG current) { return false; }
	virtual bool getProgress(int& percent);

protected:
	virtual bool onBuild(HWND hwnd, UINT msg, const std::wstring& path);
	virtual bool onBufferEnter(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2);
	virtual bool onBufferExit(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2);
	virtual bool onLoop(HWND hwnd, UINT msg, const std::wstring& path);

	virtual bool rebuild(HWND hwnd, UINT msg, const std::wstring& path);

	virtual bool onAbort(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2);
	virtual bool onComplete(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2);
	virtual bool onRebuild(HWND hwnd, UINT msg, const std::wstring& path);

protected:
	struct Buffer {
		Buffer() : enable(false), count(0) {}
		bool enable;
		int count;
	} buffer_;

	struct Packets {
		Packets() : time(0), recent(0) {}
		DWORD time;
		DWORD recent;
	} packets_;

	struct Frames {
		Frames() : time(0), drawn(0), dropped(0) {}
		DWORD time;
		int drawn;
		int dropped;
	} frames_;

	// çƒç\íz
	class Rebuild {
		int count_;
		std::wstring state_;

	public:
		Rebuild() : count_(0) {}

		int count() { return count_; }
		int count(int count) { count_ = count; return count_; }
		int counter() { ++count_; return count_; }

		std::wstring& state() { return state_; }
		std::wstring& state(const std::wstring& state) { state_ = state; return state_; }
		std::wstring& state(long state);
	} rebuild_;
};

} // namespace dsp
