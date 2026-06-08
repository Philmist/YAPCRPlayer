#include "stdafx.h"

namespace dsp {

IBaseBuilder::IBaseBuilder(utl::SyncLog& log, utl::SyncOptional<std::wstring>& text)
	: Logger(log, utl::Log::LOGTYPE_GRAPH)
	, text_(text)
	, graph_(log)
	, volume_(100)
	, balance_(0)
{}

IBaseBuilder::~IBaseBuilder()
{
	//terminate();
}


void IBaseBuilder::mainloop(HWND hwnd, UINT msg, const std::wstring& path)
{
	CoInitialize(NULL);
	addLog(L"IBaseBuilder::mainloop (CoInitialize)");

	if (!build(hwnd, msg, path)) { goto exit_loop; }

	try { boost::this_thread::interruption_point(); }
	catch (...) { addLog(L"IBaseBuilder::mainloop (Interrupt Start)"); goto exit_loop; }

	// スレッドが終了しないためのループ
	while (true)
	{
		if (run_.joinable())
		{
			// 接続時のrunの停止を検出する
			// joinしないとリセットされないのでノータイムのjoin
			if (run_.timed_join(boost::posix_time::seconds(0)))
			{
				getStateImpl();
			}
		}
		else
		{
			getDurationImpl();
			getCurrentPositionImpl();
		}

		if (!onLoop(hwnd, msg, path)) { goto exit_loop; }

		// interruptで例外が発生する
		try { boost::this_thread::sleep(boost::posix_time::millisec(100)); }
		catch (...) { addLog(L"IBaseBuilder::mainloop (Interrupt Loop)"); goto exit_loop; }

		long code;
		LONG_PTR p1, p2;

		while (getEvent(code, p1, p2))
		{
			switch (code)
			{
			case EC_BUFFERING_DATA:
			{
				if (p1)
				{
					if (!onBufferEnter(hwnd, msg, path, p1, p2)) { goto exit_loop; }
				}
				else
				{
					if (!onBufferExit(hwnd, msg, path, p1, p2)) { goto exit_loop; }
				}
				break;
			}


			case EC_USERABORT:
			{// 異常終了
				addLog(L"EC_USERABORT 0x%08X (%d)", p1, p2);
				if (p1 == E_ABORT)
				{
					if (!onAbort(hwnd, msg, path, p1, p2)) { goto exit_loop; }
				}
				else
				{
					if (!onComplete(hwnd, msg, path, p1, p2)) { goto exit_loop; }
				}
				break;
			}

			case EC_ERRORABORT:
			{
				addLog(L"EC_ERRORABORT 0x%08X (%d)", p1, p2);
				// MF_E_NOT_INITIALIZED
				// 再生開始時に画面サイズを変更しようとすると起きるエラーコード
				if (p1 != MF_E_NOT_INITIALIZED)
				{
					if (!onComplete(hwnd, msg, path, p1, p2)) { goto exit_loop; }
				}
				break;
			}
			case EC_COMPLETE:
			{
				addLog(L"EC_COMPLETE 0x%08X (%d)", p1, p2);
				if (!onComplete(hwnd, msg, path, p1, p2)) { goto exit_loop; }
				break;
			}

			//case EC_LOADSTATUS:
			//	{
			//		switch (p1)
			//		{
			//		case AM_LOADSTATUS_CLOSED: addLog(L"EC_LOADSTATUS AM_LOADSTATUS_CLOSED\n"); break;
			//		case AM_LOADSTATUS_CONNECTING: addLog(L"EC_LOADSTATUS AM_LOADSTATUS_CONNECTING\n"); break;
			//		case AM_LOADSTATUS_LOCATING: addLog(L"EC_LOADSTATUS AM_LOADSTATUS_LOCATING\n"); break;
			//		case AM_LOADSTATUS_OPEN: addLog(L"EC_LOADSTATUS AM_LOADSTATUS_OPEN\n"); break;
			//		case AM_LOADSTATUS_OPENING: addLog(L"EC_LOADSTATUS AM_LOADSTATUS_OPENING\n"); break;
			//		default: break;
			//		}
			//		break;
			//	}

			//case EC_ACTIVATE: addLog(L"EC_ACTIVATE\n"); break;
			//case EC_CLOCK_CHANGED: addLog(L"EC_CLOCK_CHANGED\n"); break;
			//case EC_PAUSED: addLog(L"EC_PAUSED\n"); break;
			//case EC_VIDEO_SIZE_CHANGED: addLog(L"EC_VIDEO_SIZE_CHANGED\n"); break;

			default:
				//DebugText(L"DS event code: 0x%08X(%d)\n", code, code);
				break;
			}

		}
	}

exit_loop:
	boost::recursive_mutex::scoped_lock lock(mutex_);

	text_ = L"停止中";

	state_.init();
	graph_.uninit();

	CoUninitialize();
	addLog(L"IBaseBuilder::mainloop (CoUninitialize)");
	addLog(L"");

	::PostMessage(hwnd, msg, GRAPH_NOTIFY_END, 0);
	text_ = L"停止";
}

bool IBaseBuilder::build(HWND hwnd, UINT msg, const std::wstring& path)
{
	if (!onBuild(hwnd, msg, path)) { return false; }

	boost::recursive_mutex::scoped_lock lock(mutex_);

	state_.init();

	if (!graph_.init())
	{
		return false;
	}

	if (!render(hwnd, msg, path))
	{
		graph_.uninit();
		return false;
	}

	{// 初期値設定 run前
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		setVolume(cfg_.audio.volume);
		setBalance(cfg_.audio.balance);
		setMute(cfg_.audio.mute);
		setFLVConfig(cfg_.type.flv);
	}

	// runは最初にバッファ読み込みがあるのでブロックされる
	boost::thread thr(&IBaseBuilder::runThread, this, hwnd, msg);
	run_.detach();
	run_.swap(thr);

	return onEnterLoop(hwnd, msg, path);
}

void IBaseBuilder::runThread(HWND hwnd, UINT msg)
{
	//try { boost::this_thread::sleep(boost::posix_time::millisec(250)); }
	//catch(...) { return; }

	addLog(L"IBaseBuilder::run (Enter)");
	graph_.run();
	addLog(L"IBaseBuilder::run (Exit)");
	//DebugText(L"run()-----------------------\n");

	{// 初期値設定 run後
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		// run前は失敗する
		setProcAmp(cfg_.video.procamp);

		// run後にしないとEC_ERRORABORTが出てストリームが止まる
		setNominalRange(cfg_.video.nominal);

		setResizer(cfg_.video.resizer);
	}

	::PostMessage(hwnd, msg, GRAPH_NOTIFY_START, 0);
}


void IBaseBuilder::execute(HWND hwnd, UINT msg, const std::wstring& path, const BuildConfig& cfg)
{
	if (!terminate()) { return; }

	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_ = cfg;
	}

	// ステータス初期化
	status_.set(hwnd, msg, path);

	boost::thread thr(&IBaseBuilder::mainloop, this, hwnd, msg, path);
	thread_.detach();
	thread_.swap(thr);
}

bool IBaseBuilder::terminate()
{
	if (thread_.joinable())
	{
		text_ = L"停止中";
		thread_.interrupt();
		//DebugText(L"IBaseBuilder::thread_.interrupt();\n");

		if (!thread_.timed_join(boost::posix_time::seconds(10)))
		{
			return false;
		}
		//DebugText(L"IBaseBuilder::thread_.join();\n");
	}

	if (run_.joinable())
	{
		text_ = L"停止中";
		run_.interrupt();
		//DebugText(L"IBaseBuilder::run_.interrupt();\n");

		if (!run_.timed_join(boost::posix_time::seconds(10)))
		{
			return false;
		}
		//DebugText(L"IBaseBuilder::run_.join();\n");
	}
	text_ = L"停止";
	return true;
}

bool IBaseBuilder::getState(long& state)
{
	boost::recursive_mutex::scoped_lock lock(state_.mutex);
	if (state_.state.result)
	{
		state = state_.state.value;
		return true;
	}
	return false;
}

bool IBaseBuilder::isRunning()
{
	long state;
	if (getState(state))
	{
		if (state == State_Running)
		{
			return true;
		}
	}
	return false;
}

bool IBaseBuilder::hasAudio()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.hasAudio();
	}
	return false;
}

bool IBaseBuilder::hasVideo()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.hasVideo();
	}
	return false;
}

bool IBaseBuilder::isAudioRunning(long timeout)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.isAudioRunning(timeout);
	}
	return false;
}

bool IBaseBuilder::isVideoRunning(long timeout)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.isVideoRunning(timeout);
	}
	return false;
}

bool IBaseBuilder::run()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		bool res = graph_.run();
		getStateImpl();
		return res;
	}
	return false;
}

bool IBaseBuilder::pause()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		bool res = graph_.pause();
		getStateImpl();
		return res;
	}
	return false;
}

bool IBaseBuilder::togglePause()
{
	long state = State_Stopped;
	if (getStateImpl(0, state))
	{
		if (state == State_Paused || state == State_Stopped)
		{
			return run();
		}
		else
		{
			return pause();
		}
	}
	return false;
}

bool IBaseBuilder::stop()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		//DebugText(L"IBaseBuilder::stop()\n");
		return graph_.stop();
	}
	return false;
}

bool IBaseBuilder::abort()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		//DebugText(L"IBaseBuilder::abort()\n");
		return graph_.abort();
	}
	return false;
}

bool IBaseBuilder::getEvent(long& code, LONG_PTR& param1, LONG_PTR& param2)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getEvent(code, param1, param2);
	}
	return false;
}

bool IBaseBuilder::repaint(HDC hdc)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.repaint(hdc);
	}
	return false;
}

bool IBaseBuilder::setVideoPosition(RECT& dst)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setVideoPosition(dst);
	}
	return false;
}

bool IBaseBuilder::getNativeVideoSize(SIZE& size)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getNativeVideoSize(size);
	}
	return false;
}

bool IBaseBuilder::saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.saveCurrentImage(path, format, jpegQuality, width, height, fit);
	}
	return false;
}

bool IBaseBuilder::getStatusText(std::wstring& text)
{
	return text_.get(text);
}

bool IBaseBuilder::getSampleTime(LONGLONG& time)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!run_.joinable())
		{
			return graph_.getSampleTime(time);
		}
	}
	return false;
}

bool IBaseBuilder::getPositionDiff(LONGLONG& diff)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!run_.joinable())
		{
			return graph_.getPositionDiff(diff);
		}
	}
	return false;
}

bool IBaseBuilder::getDuration(LONGLONG& duration)
{
	boost::recursive_mutex::scoped_lock lock(state_.mutex);
	if (state_.duration.result)
	{
		duration = state_.duration.value;
		return true;
	}
	return false;
}

bool IBaseBuilder::getCurrentPosition(LONGLONG& pos)
{
	boost::recursive_mutex::scoped_lock lock(state_.mutex);
	if (state_.current.result)
	{
		pos = state_.current.value;
		return true;
	}
	return false;
}

bool IBaseBuilder::setPositions(LONGLONG current, LONGLONG stop)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setPositions(current, stop);
	}
	return false;
}

bool IBaseBuilder::getPositions(LONGLONG& current, LONGLONG& stop)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getPositions(current, stop);
	}
	return false;
}

bool IBaseBuilder::seek(LONGLONG current)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (graph_.seek(current))
		{
			boost::recursive_mutex::scoped_lock lock(state_.mutex);
			state_.current.result = true;
			state_.current.value = current;
			return true;
		}
	}
	return false;
}

bool IBaseBuilder::getCurrentBitrate(int& bitrate)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getCurrentBitrate(bitrate);
	}
	return false;
}

bool IBaseBuilder::getType(long& type)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getType(type);
	}
	return false;
}

bool IBaseBuilder::getPackets(DWORD& packets)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getPackets(packets);
	}
	return false;
}

bool IBaseBuilder::getFramerate(int& rate)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getFramerate(rate);
	}
	return false;
}

bool IBaseBuilder::getCurrentFramerate(int& rate)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getCurrentFramerate(rate);
	}
	return false;
}

bool IBaseBuilder::getFramesDrawn(int& frames)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getFramesDrawn(frames);
	}
	return false;
}

bool IBaseBuilder::getFramesDropped(int& frames)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getFramesDropped(frames);
	}
	return false;
}


void IBaseBuilder::getBalance(int& balance)
{
	boost::recursive_mutex::scoped_lock lock(mxcfg_);
	balance = cfg_.audio.balance;
}

void IBaseBuilder::getVolume(int& volume)
{
	boost::recursive_mutex::scoped_lock lock(mxcfg_);
	volume = cfg_.audio.volume;
}

void IBaseBuilder::getMute(bool& mute)
{
	boost::recursive_mutex::scoped_lock lock(mxcfg_);
	mute = cfg_.audio.mute;
}


bool IBaseBuilder::setBalance(int balance)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.audio.balance = balance;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setBalance(balance);
	}
	return false;
}

bool IBaseBuilder::setVolume(int volume)
{
	bool mute = false;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.audio.volume = volume;
		mute = cfg_.audio.mute;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (mute)
		{
			return graph_.setVolume(0);
		}
		else
		{
			return graph_.setVolume(volume);
		}
	}
	return false;
}


bool IBaseBuilder::setMute(bool mute)
{
	int volume = 100;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.audio.mute = mute;
		volume = cfg_.audio.volume;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (mute)
		{
			return graph_.setVolume(0);
		}
		else
		{
			return graph_.setVolume(volume);
		}
	}
	return false;
}

bool IBaseBuilder::getProcAmpRange(ProcAmpRangeGroup& group)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getProcAmpRange(group);
	}
	return false;
}

bool IBaseBuilder::setProcAmp(const ProcAmpValue& value)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.video.procamp = value;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setProcAmp(value);
	}
	return false;
}

bool IBaseBuilder::getProcAmp(ProcAmpValue& value)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getProcAmp(value);
	}
	return false;
}


bool IBaseBuilder::setNominalRange(int value)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.video.nominal = value;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setNominalRange(value);
	}
	return false;
}


bool IBaseBuilder::setResizer(int type)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.video.resizer = type;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setResizer(type);
	}
	return false;
}


bool IBaseBuilder::setNetwork(const NetworkControl& network)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.network = network;
	}
	return true;
}

bool IBaseBuilder::setType(const TypeControl& type)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.type = type;
	}
	return true;
}

bool IBaseBuilder::setDecoderSet(const DecoderIDSet& decoderSet)
{
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		cfg_.decoderSet = decoderSet;
	}
	return true;
}


bool IBaseBuilder::getFilterProperty(std::vector<FilterProperty>& prop)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getFilterProperty(prop);
	}
	return false;
}

bool IBaseBuilder::showFilterProperty(int index, HWND parent, HINSTANCE hi, DWORD id)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.showFilterProperty(index, parent, hi, id);
	}
	return false;
}

bool IBaseBuilder::setFLVConfig(const FLVConfig& cfg)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.setFLVConfig(cfg);
	}
	return false;
}

//==========================================================================

bool IBaseBuilder::getStateImpl()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		long state = State_Stopped;
		bool result = graph_.getState(0, state);

		boost::recursive_mutex::scoped_lock lock(state_.mutex);
		state_.state.result = result;
		state_.state.value = state;
		return result;
	}
	return false;
}

bool IBaseBuilder::getStateImpl(long timeout, long& state)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		return graph_.getState(timeout, state);
	}
	return false;
}

bool IBaseBuilder::getDurationImpl()
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!run_.joinable())
		{
			LONGLONG duration = 0;
			bool result = graph_.getDuration(duration);

			boost::recursive_mutex::scoped_lock lock(state_.mutex);
			state_.duration.result = result;
			state_.duration.value = duration;
			return result;
		}
	}
	return false;
}

bool IBaseBuilder::getDurationImpl(LONGLONG& duration)
{
	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!run_.joinable())
		{
			return graph_.getDuration(duration);
		}
	}
	return false;
}

bool IBaseBuilder::getCurrentPositionImpl()
{
	bool timestamp = true;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		timestamp = cfg_.type.flv.timestamp;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!run_.joinable())
		{
			LONGLONG pos = 0;
			bool result = graph_.getCurrentPosition(pos, timestamp);

			boost::recursive_mutex::scoped_lock lock(state_.mutex);
			state_.current.result = result;
			state_.current.value = pos;
			return result;
		}
	}
	return false;
}

bool IBaseBuilder::getCurrentPositionImpl(LONGLONG& pos)
{
	bool timestamp = true;
	{
		boost::recursive_mutex::scoped_lock lock(mxcfg_);
		timestamp = cfg_.type.flv.timestamp;
	}

	boost::recursive_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!run_.joinable())
		{
			return graph_.getCurrentPosition(pos, timestamp);
		}
	}
	return false;
}

} // namespace dsp
