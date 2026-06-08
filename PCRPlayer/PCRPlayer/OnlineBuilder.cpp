#include "stdafx.h"

namespace dsp {

bool OnlineBuilder::getProgress(int& percent)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (buffer_.enable)
		{
			return graph_.getProgress(percent);
		}
	}
	return false;
}

bool OnlineBuilder::render(HWND hwnd, UINT msg, const std::wstring& path)
{
	NetworkControl network;
	DecoderControl decoder;
	DecoderIDSet decoderSet;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		network = cfg_.network;
		decoder = cfg_.decoder;
		decoderSet = cfg_.decoderSet;
	}

	boost::recursive_mutex::scoped_lock lock(mutex_);

	text_ = L"構築中";

	network::URLParser url(path);
	if (url)
	{
		SourcePtr source;
		VideoPtr video;
		AudioPtr audio;

		// FLV
		source.reset(new URLBufferSource(network.reader.userAgent, network.reader.timeout));
		video.reset(new EVRCustom(decoder.dxva2));
		audio.reset(new DSR());
		if (graph_.renderEx(hwnd, path, source, video, audio, decoderSet, true))
		{
			text_.clear();
			return true;
		}

		if (source->isTimeout())
		{
			text_ = L"タイムアウト";
			return false;
		}

		try { boost::this_thread::interruption_point(); }
		catch (...) { text_.clear(); return false; }

		// WMV
		source.reset(new WMASFSource());
		video.reset(new EVRCustom(decoder.dxva2));
		audio.reset(new DSR());
		if (graph_.render(hwnd, path, source, video, audio, decoderSet))
		{
			text_.clear();
			return true;
		}
	}

	text_ = L"構築失敗";
	return false;
}

bool OnlineBuilder::onBuild(HWND hwnd, UINT msg, const std::wstring& path)
{
	packets_ = Packets();
	frames_ = Frames();

	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		buffer_ = Buffer();
	}
	return true;
}

bool OnlineBuilder::onBufferEnter(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2)
{
	frames_ = Frames();

	int count = 0;
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		buffer_.enable = true;
		++buffer_.count;

		count = buffer_.count;
	}
	addLog(L"OnlineBuilder::onBufferEnter (Count:%d)", count);
	return true;
}

bool OnlineBuilder::onBufferExit(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2)
{
	frames_ = Frames();

	int count = 0;
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		buffer_.enable = false;

		count = buffer_.count;
	}
	addLog(L"OnlineBuilder::onBufferExit (Count:%d)", count);
	return true;
}

bool OnlineBuilder::onLoop(HWND hwnd, UINT msg, const std::wstring& path)
{
	long source = SOURCE_TYPE_UNKNOWN;
	if (!(getType(source) && source == SOURCE_TYPE_ASF)) { return true; }

	TypeControl type;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		type = cfg_.type;
	}

	DWORD time = timeGetTime();

	if (type.wmv.packet.enable)
	{
		// getPacketsはWMVのときのみ有効
		DWORD packets = 0;
		if (!getPackets(packets)) { return true; }

		if (packets_.time == 0 || packets > packets_.recent)
		{
			packets_.recent = packets;
			packets_.time = time;
		}
		else if (time - packets_.time > type.wmv.packet.time)
		{
			addLog(L"Packets:%u Recent:%u Diff:%ums", packets, packets_.recent, time - packets_.time);
			addLog(L"OnlineBuilder::onLoop (Rebuild Packets)");
			if (!onRebuild(hwnd, msg, path)) { return false; }
			return true;
		}
	}

	//----------------------------------------------------------------------
	// バッファ中ではない
	if (buffer_.enable) { return true; }

	if (type.wmv.frame.enable)
	{
		int drawn = 0;
		int dropped = 0;
		if (!(getFramesDrawn(drawn) && getFramesDropped(dropped))) { return true; }

		if (frames_.time == 0 || drawn > frames_.drawn)
		{
			frames_.drawn = drawn;
			frames_.dropped = dropped;
			frames_.time = time;
		}
		else if ((dropped > frames_.dropped) && (time - frames_.time > type.wmv.frame.time))
		{// frames_.droppedだけ増えている状態
			addLog(L"Drawn:%d Dropped:%d Diff:%ums", drawn, dropped, time - frames_.time);
			addLog(L"OnlineBuilder::onLoop (Rebuild Frames)");
			if (!onRebuild(hwnd, msg, path)) { return false; }
			return true;
		}
	}
	return true;
}

bool OnlineBuilder::rebuild(HWND hwnd, UINT msg, const std::wstring& path)
{
	addLog(L"自動再構築");

	NetworkControl network;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		network = cfg_.network;
	}

	int wait = network.rebuild.wait;
	if (wait > 0)
	{
		text_ = rebuild_.state() + L" (停止中)";
	}
	else
	{
		text_ = L"停止中";
	}
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		state_.init();
		graph_.uninit();
	}

	for (int i = 0; i < wait; ++i)
	{
		CString fmt;
		fmt.Format(L"%s (%d)", rebuild_.state().c_str(), wait - i);
		text_ = std::wstring(fmt);

		try { boost::this_thread::sleep(boost::posix_time::seconds(1)); }
		catch (...) { text_.clear(); return false; }
	}

	text_.clear();
	return build(hwnd, msg, path);
}

bool OnlineBuilder::onAbort(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2)
{
	addLog(L"OnlineBuilder::onAbort (Rebuild Count:%d)", rebuild_.counter());
	text_ = rebuild_.state((long)p2);
	return rebuild(hwnd, msg, path);
}

bool OnlineBuilder::onComplete(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2)
{
	addLog(L"OnlineBuilder::onComplete (Rebuild Count:%d)", rebuild_.counter());
	text_ = rebuild_.state(USERABORT_DEFAULT);
	return rebuild(hwnd, msg, path);
}

bool OnlineBuilder::onRebuild(HWND hwnd, UINT msg, const std::wstring& path)
{
	addLog(L"OnlineBuilder::onRebuild (Rebuild Count:%d)", rebuild_.counter());
	text_ = rebuild_.state(USERABORT_DEFAULT);
	return rebuild(hwnd, msg, path);
}

//-------------------------------------------------------------------------

std::wstring& OnlineBuilder::Rebuild::state(long state)
{
	switch (state)
	{
	//case USERABORT_TAG_SIZE:
	//case USERABORT_DATA_SIZE:		state_ = L"データ不足"; break;

	//case USERABORT_PREVIOUS_TAG_SIZE:
	//case USERABORT_STREAM_ID:
	//case USERABORT_SYNC_ERROR:
	//case USERABORT_DATA_ERROR:	state_ = L"不正データ"; break;

	case USERABORT_TIMESTAMP_AUDIO:
	case USERABORT_TIMESTAMP_VIDEO:	state_ = L"ループ検出"; break;

	default:						state_ = L"再構築"; break;
	}
	return state_;
}

} // namespace dsp
