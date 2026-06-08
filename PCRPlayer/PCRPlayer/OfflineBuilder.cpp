#include "stdafx.h"

namespace dsp {

bool OfflineBuilder::render(HWND hwnd, UINT msg, const std::wstring& path)
{
	DecoderControl decoder;
	DecoderIDSet decoderSet;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		decoder = cfg_.decoder;
		decoderSet = cfg_.decoderSet;
	}

	boost::recursive_mutex::scoped_lock lock(mutex_);

	text_ = L"構築中";

	SourcePtr source;
	VideoPtr video;
	AudioPtr audio;

	// WMV
	source.reset(new WMASFSource());
	video.reset(new EVRCustom(decoder.dxva2));
	audio.reset(new DSR());
	if (graph_.render(hwnd, path, source, video, audio, decoderSet))
	{
		text_.clear();
		return true;
	}

	try { boost::this_thread::interruption_point(); }
	catch (...) { text_.clear(); return false; }

	// FLV && UNKNOWN
	source.reset(new AsyncSource());
	video.reset(new EVRCustom(decoder.dxva2));
	audio.reset(new DSR());
	if (graph_.renderEx(hwnd, path, source, video, audio, decoderSet, false))
	{
		text_.clear();
		return true;
	}

	text_ = L"構築失敗";

	::PostMessage(hwnd, msg, GRAPH_NOTIFY_RESTART, 0);
	return false;
}


bool OfflineBuilder::onLoop(HWND hwnd, UINT msg, const std::wstring& path)
{
	if (!isRunning()) { return true; }

	// 終了位置まで進んだらリスタート
	LONGLONG dur = 0, cur = 0;
	if (getCurrentPosition(cur))
	{
		// 短いファイルがDurationだけ無効になることがあるのでチェックしない
		getDuration(dur);
		if (dur == 0 || dur == cur)
		{// 長さが0か長さと再生位置が同じなら
			if (end_) { return true; }
			end_ = true; // 一回だけ実行するフラグ

			detailLog(L"OfflineBuilder::onLoop (Restart)", L"current:%lld\r\nduration:%lld", cur, dur);
			::PostMessage(hwnd, msg, GRAPH_NOTIFY_RESTART, 0);
			return true;
		}
	}

	end_ = false;
	return true;
}

} // namespace dsp
