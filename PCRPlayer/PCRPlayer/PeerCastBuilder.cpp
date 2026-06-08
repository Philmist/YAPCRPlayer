#include "stdafx.h"

namespace dsp {

bool PeerCastBuilder::render(HWND hwnd, UINT msg, const std::wstring& path)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);

	text_.clear();

	network::PeerCast peca(path);
	if (!peca)
	{
		text_ = L"–¢‘Î‰žURL";
		return false;
	}


	NetworkControl network;
	DecoderControl decoder;
	DecoderIDSet decoderSet;
	auto connect = [&](std::wstring& stream, const std::wstring str)->bool {
		text_ = L"Ú‘±’†" + str;
		if (!peca.connect(stream, network.connect.timeout) || stream.empty())
		{
			text_ = L"Ú‘±Ž¸”s" + str;
			return false;
		}
		text_.clear();
		return true;

		//text_ = L"Ú‘±’†" + str;
		//if (!peca.connect(stream, 1000) || stream.empty())
		//{
		//	text_ = L"Ú‘±’†..." + str;
		//	if (!peca.connect(stream, network.connect.timeout) || stream.empty())
		//	{
		//		text_ = L"Ú‘±Ž¸”s" + str;
		//		return false;
		//	}
		//}
		//text_.clear();
		//return true;
	};


	std::wstring stream;
	int retry = 0;
	while (true)
	{
		{
			boost::recursive_mutex::scoped_lock lock(mxcfg_);
			network = cfg_.network;
			decoder = cfg_.decoder;
			decoderSet = cfg_.decoderSet;
		}


		CString fmt;
		if (retry > 0)
		{
			fmt.Format(L" (%d/%d)", retry, network.connect.count);
		}
		std::wstring str = fmt;
		std::wstring fail;

		if (connect(stream, str))
		{
			::PostMessage(hwnd, msg, GRAPH_NOTIFY_CONNECT, 0);

			text_ = L"\’z’†" + str;

			try { boost::this_thread::interruption_point(); }
			catch (...) { text_.clear(); return false; }

			SourcePtr source;
			VideoPtr video;
			AudioPtr audio;


			// FLV
			source.reset(new URLBufferSource(network.reader.userAgent, network.reader.timeout));
			video.reset(new EVRCustom(decoder.dxva2));
			audio.reset(new DSR());
			if (graph_.renderEx(hwnd, stream, source, video, audio, decoderSet, true))
			{
				text_.clear();
				return true;
			}

			if (source->type() != SOURCE_TYPE_ASF)
			{
				fail = L"\’zŽ¸”s";
			}
			else if (source->isTimeout())
			{
				fail = L"ƒ^ƒCƒ€ƒAƒEƒg";
			}
			else
			{
				try { boost::this_thread::interruption_point(); }
				catch (...) { text_.clear(); return false; }

				// WMV
				source.reset(new WMASFSource());
				video.reset(new EVRCustom(decoder.dxva2));
				audio.reset(new DSR());
				if (graph_.render(hwnd, stream, source, video, audio, decoderSet))
				{
					text_.clear();
					return true;
				}

				fail = L"\’zŽ¸”s";
			}
		}
		else
		{
			fail = L"Ú‘±Ž¸”s";
		}

		if (retry >= network.connect.count) { break; }
		retry++;

		for (int i = 0; i < network.connect.wait; ++i)
		{
			CString fmt;
			fmt.Format(L"%s (%d/%d) %d", fail.c_str(), retry, network.connect.count, network.connect.wait - i);
			text_ = std::wstring(fmt);

			try { boost::this_thread::sleep(boost::posix_time::seconds(1)); }
			catch (...) { text_.clear(); return false; }
		}
	}

	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		network = cfg_.network;
	}

	if (network.connect.stop)
	{
		peca.stop(network.relay.automatic.stop);
	}

	::PostMessage(hwnd, msg, GRAPH_NOTIFY_RETRY_END, 0);

	text_ = L"’âŽ~";
	return false;
}

bool PeerCastBuilder::rebuild(bool bump, HWND hwnd, UINT msg, const std::wstring& path)
{
	addLog(L"Ž©“®Ä\’z");

	NetworkControl network;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		network = cfg_.network;
	}

	if (bump)
	{
		network::PeerCast peca(path);
		bool relay = network.relay.automatic.reconnect;
		int interval = network.reconnect.interval;
		int remain = 0;
		if (peca && peca.check(relay, interval, remain))
		{
			text_ = rebuild_.state() + L" (ƒ`ƒƒƒ“ƒlƒ‹ÄÚ‘±)";
			if (peca.bump())
			{
				addLog(L"ƒ`ƒƒƒ“ƒlƒ‹ÄÚ‘± (Bump)");
			}
			else
			{
				text_ = rebuild_.state() + L" (ƒ`ƒƒƒ“ƒlƒ‹ÄÚ‘±Ž¸”s)";
			}

			try { boost::this_thread::sleep(boost::posix_time::seconds(3)); }
			catch (...) { text_.clear(); return false; }
		}
		else
		{
			addLog(L"ƒ`ƒƒƒ“ƒlƒ‹ÄÚ‘±–³Œø (Remain:%dms)", remain);
		}
	}

	int wait = network.rebuild.wait;
	if (wait > 0)
	{
		text_ = rebuild_.state() + L" (’âŽ~’†)";
	}
	else
	{
		text_ = L"’âŽ~’†";
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

bool PeerCastBuilder::onAbort(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2)
{
	bool loop = (p2 == USERABORT_TIMESTAMP_AUDIO) || (p2 == USERABORT_TIMESTAMP_VIDEO);
	bool bump = false;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		bump = loop && cfg_.type.flv.loop;
	}

	addLog(L"PeerCastBuilder::onAbort (Rebuild Count:%d)", rebuild_.counter());
	text_ = rebuild_.state((long)p2);
	return rebuild(bump, hwnd, msg, path);
}

bool PeerCastBuilder::onComplete(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2)
{
	addLog(L"PeerCastBuilder::onComplete (Rebuild Count:%d)", rebuild_.counter());
	text_ = rebuild_.state(USERABORT_DEFAULT);
	return rebuild(false, hwnd, msg, path);
}

bool PeerCastBuilder::onRebuild(HWND hwnd, UINT msg, const std::wstring& path)
{
	addLog(L"PeerCastBuilder::onRebuild (Rebuild Count:%d)", rebuild_.counter());
	text_ = rebuild_.state(USERABORT_DEFAULT);
	return rebuild(false, hwnd, msg, path);
}

} // namespace dsp
