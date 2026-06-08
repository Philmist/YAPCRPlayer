#include "stdafx.h"

namespace dsp {

DSPlayer::DSPlayer(utl::SyncLog& log)
	: Logger(log, utl::Log::LOGTYPE_GRAPH)
	, hwnd_(NULL)
	, msg_(0)
	, mode_(MODE_OFFLINE)
{
}

DSPlayer::~DSPlayer()
{
	// ここはスレッド外なため強制終了しても終了しなくなるのでなにもしない
	// terminate(true);
}

void DSPlayer::executeThread(HWND hwnd, UINT msg, const std::wstring& path, const BuildConfig& cfg)
{
	boost::recursive_timed_mutex::scoped_lock lock(mutex_);

	//DebugText(L"executeThread\n");

	try { boost::this_thread::interruption_point(); }
	catch(...) { return; }

	if (builder_)
	{
		// リセットしてしまうのでスレッドは自動的に停止しない
		// 新しいbuilderのスレッドを起動する前に必ず古いスレッドを停止させる
		// この終了待ちで操作がブロックされるのでこのメソッドもスレッドにしている
		while (true)
		{
			if (builder_->terminate())
			{
				break;
			}

			try { boost::this_thread::sleep(boost::posix_time::millisec(100)); }
			catch(...) { return; }
		}
	}

	try { boost::this_thread::interruption_point(); }
	catch(...) { return; }

	if (peca_)
	{
		builder_.reset(new PeerCastBuilder(log_, text_));
	}
	else if (url_)
	{
		builder_.reset(new OnlineBuilder(log_, text_));
	}
	else
	{
		builder_.reset(new OfflineBuilder(log_, text_));
	}

	builder_->execute(hwnd, msg, path, cfg);
	return;
}



bool DSPlayer::init(HWND hwnd, UINT msg, const std::wstring& path, const BuildConfig& cfg)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		// 初期化
		hwnd_ = hwnd;
		msg_ = msg;
		path_ = path;
		cfg_ = cfg;
		url_ = network::URLParser();
		peca_ = network::PeerCast();

		// URL判別
		network::URLParser url(path);
		if (url)
		{
			url_ = url;
			// PeerCast判別
			network::PeerCast peca(path);
			if (peca)
			{
				peca_ = peca;
			
				mode_ = MODE_PEERCAST;
				return true;
			}
			mode_ = MODE_ONLINE;
			return true;
		}
		mode_ = MODE_OFFLINE;
		return true;
	}
	return false;
}


void DSPlayer::execute()
{
	if (thExecute_.joinable())
	{
		thExecute_.interrupt();
		if (!thExecute_.timed_join(boost::posix_time::seconds(0)))
		{// ノータイムのjoin
			return;
		}
	}

	boost::thread thr(&DSPlayer::executeThread, this, hwnd_, msg_, path_, cfg_);
	thExecute_.detach();
	thExecute_.swap(thr);
}


// プログラム終了用
// シングルスレッドの終了処理
// ブロックするが完全に終了するまで待たない
bool DSPlayer::terminate()
{
	if (thTerminate_.joinable())
	{
		thTerminate_.interrupt();
		//DebugText(L" DSPlayer::terminate thTerminate_.interrupt();\n");
		if (!thTerminate_.timed_join(boost::posix_time::seconds(10)))
		{
			return false;
		}
		//DebugText(L" DSPlayer::terminate thTerminate_.join();\n");
	}

	boost::recursive_timed_mutex::scoped_timed_lock lock(mutex_, boost::posix_time::seconds(10));
	if (lock)
	{
		if (builder_)
		{
			if (!builder_->terminate())
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	if (thExecute_.joinable())
	{
		thExecute_.interrupt();
		if (!thExecute_.timed_join(boost::posix_time::seconds(10)))
		{
			return false;
		}
	}

	return true;
}

// 手動停止用
// マルチスレッドの終了処理
// ブロックしないが完全に終了するまで待つ
void DSPlayer::UnblockTerminate()
{
	if (thTerminate_.joinable())
	{
		thTerminate_.interrupt();
		//DebugText(L" DSPlayer::terminate thTerminate_.interrupt();\n");
		if (!thTerminate_.timed_join(boost::posix_time::seconds(0)))
		{// ノータイムのjoin
			return;
		}
		//DebugText(L" DSPlayer::terminate thTerminate_.join();\n");
	}

	boost::thread thr(&DSPlayer::terminateThread, this);
	thTerminate_.detach();
	thTerminate_.swap(thr);

	//DebugText(L"DSPlayer::UnblockTerminate thread\n");
}

void DSPlayer::terminateThread()
{
	//DebugText(L"terminateThread\n");
	while (true)
	{
		try { boost::this_thread::sleep(boost::posix_time::millisec(100)); }
		catch(...) { return; }

		boost::recursive_timed_mutex::scoped_lock lock(mutex_);
		if (!builder_) { break; }
		if (builder_->terminate()) { break; }
	}

	while (true)
	{
		try { boost::this_thread::sleep(boost::posix_time::millisec(100)); }
		catch(...) { return; }

		if (!thExecute_.joinable()) { break; }
		thExecute_.interrupt();
		thExecute_.timed_join(boost::posix_time::seconds(10));
	}
}

bool DSPlayer::getState(long& state)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getState(state);
	}
	return false;
}

bool DSPlayer::isRunning()
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->isRunning();
	}
	return false;
}


bool DSPlayer::run()
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->run();
	}
	return false;
}

bool DSPlayer::pause()
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->pause();
	}
	return false;
}

bool DSPlayer::togglePause()
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->togglePause();
	}
	return false;
}


bool DSPlayer::stop()
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		//DebugText(L"DSPlayer::stop()\n");
		return builder_->stop();
	}
	return false;
}

bool DSPlayer::abort()
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		//DebugText(L"DSPlayer::abort()\n");
		return builder_->abort();
	}
	return false;
}



bool DSPlayer::getEvent(long& code, LONG_PTR& param1, LONG_PTR& param2)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getEvent(code, param1, param2);
	}
	return false;
}

bool DSPlayer::repaint(HDC hdc)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->repaint(hdc);
	}
	return false;
}

bool DSPlayer::setVideoPosition(RECT& dst)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setVideoPosition(dst);
	}
	return false;
}

bool DSPlayer::getNativeVideoSize(SIZE& size)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getNativeVideoSize(size);
	}
	return false;
}

bool DSPlayer::saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->saveCurrentImage(path, format, jpegQuality, width, height, fit);
	}
	return false;
}

bool DSPlayer::getStatusText(std::wstring& text)
{
	return text_.get(text);
}

bool DSPlayer::getSampleTime(LONGLONG& time)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getSampleTime(time);
	}
	return false;
}

bool DSPlayer::getPositionDiff(LONGLONG& diff)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getPositionDiff(diff);
	}
	return false;
}

bool DSPlayer::getDuration(LONGLONG& duration)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getDuration(duration);
	}
	return false;
}

bool DSPlayer::getCurrentPosition(LONGLONG& duration)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getCurrentPosition(duration);
	}
	return false;
}

bool DSPlayer::setPositions(LONGLONG current, LONGLONG stop)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setPositions(current, stop);
	}
	return false;
}

bool DSPlayer::getPositions(LONGLONG& current, LONGLONG& stop)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getPositions(current, stop);
	}
	return false;
}

bool DSPlayer::seek(LONGLONG current)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->seek(current);
	}
	return false;
}


bool DSPlayer::getProgress(int& percent)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getProgress(percent);
	}
	return false;
}

bool DSPlayer::getCurrentBitrate(int& bitrate)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getCurrentBitrate(bitrate);
	}
	return false;
}

bool DSPlayer::getType(long& type)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getType(type);
	}
	return false;
}

bool DSPlayer::getPackets(DWORD& packets)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getPackets(packets);
	}
	return false;
}

bool DSPlayer::getFramerate(int& rate)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getFramerate(rate);
	}
	return false;
}


bool DSPlayer::getCurrentFramerate(int& rate)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getCurrentFramerate(rate);
	}
	return false;
}

bool DSPlayer::getFramesDrawn(int& frames)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getFramesDrawn(frames);
	}
	return false;
}

bool DSPlayer::getFramesDropped(int& frames)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getFramesDropped(frames);
	}
	return false;
}




bool DSPlayer::getBalance(int& balance)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		builder_->getBalance(balance);
		return true;
	}
	return false;
}

bool DSPlayer::getVolume(int& volume)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		builder_->getVolume(volume);
		return true;
	}
	return false;
}

bool DSPlayer::getMute(bool& mute)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		builder_->getMute(mute);
		return true;
	}
	return false;
}

bool DSPlayer::setBalance(int balance)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setBalance(balance);
	}
	return false;
}

bool DSPlayer::setVolume(int volume)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setVolume(volume);
	}
	return false;
}

bool DSPlayer::setMute(bool mute)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setMute(mute);
	}
	return false;
}


bool DSPlayer::getProcAmpRange(ProcAmpRangeGroup& group)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getProcAmpRange(group);
	}
	return false;
}

bool DSPlayer::setProcAmp(const ProcAmpValue& value)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setProcAmp(value);
	}
	return false;
}

bool DSPlayer::getProcAmp(ProcAmpValue& value)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getProcAmp(value);
	}
	return false;
}

bool DSPlayer::setNominalRange(int value)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setNominalRange(value);
	}
	return false;
}

bool DSPlayer::setResizer(int type)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setResizer(type);
	}
	return false;
}

bool DSPlayer::setNetwork(const NetworkControl& network)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setNetwork(network);
	}
	return false;
}

bool DSPlayer::setType(const TypeControl& type)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setType(type);
	}
	return false;
}

bool DSPlayer::setDecoderSet(const DecoderIDSet& decoder)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->setDecoderSet(decoder);
	}
	return false;
}

bool DSPlayer::getFilterProperty(std::vector<FilterProperty>& prop)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->getFilterProperty(prop);
	}
	return false;
}

bool DSPlayer::showFilterProperty(int index, HWND parent, HINSTANCE hi, DWORD id)
{
	boost::recursive_timed_mutex::scoped_try_lock lock(mutex_);
	if (lock)
	{
		if (!builder_) { return false; }
		return builder_->showFilterProperty(index, parent, hi, id);
	}
	return false;
}

}


