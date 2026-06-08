#pragma once

#include "BaseBuilder.h"

namespace dsp {

class OfflineBuilder : public IBaseBuilder {
private:
	bool end_;

public:
	OfflineBuilder(utl::SyncLog& log, utl::SyncOptional<std::wstring>& text)
		: IBaseBuilder(log, text)
		, end_(false)
	{}

	OfflineBuilder(utl::SyncLog& log);
	virtual ~OfflineBuilder() {}

	virtual bool render(HWND hwnd, UINT msg, const std::wstring& path);

protected:
	virtual bool onLoop(HWND hwnd, UINT msg, const std::wstring& path);
};

} // namespace dsp
