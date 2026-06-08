#pragma once

#include "OnlineBuilder.h"

namespace dsp {

class PeerCastBuilder : public OnlineBuilder {
public:
	PeerCastBuilder(utl::SyncLog& log, utl::SyncOptional<std::wstring>& text)
		: OnlineBuilder(log, text)
	{}
	virtual ~PeerCastBuilder() {}

	virtual bool render(HWND hwnd, UINT msg, const std::wstring& path);

protected:
	bool rebuild(bool bump, HWND hwnd, UINT msg, const std::wstring& path);

	virtual bool onAbort(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2);
	virtual bool onComplete(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2);
	virtual bool onRebuild(HWND hwnd, UINT msg, const std::wstring& path);
};

} // namespace dsp
