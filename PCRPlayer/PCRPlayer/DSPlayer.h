#pragma once

#include <boost/thread.hpp>

#include "BaseBuilder.h"
#include "OfflineBuilder.h"
#include "OnlineBuilder.h"
#include "PeerCastBuilder.h"

namespace dsp {

// グラフ操作とウィンドウを関連付ける
class DSPlayer : protected utl::Logger {
public:
	typedef enum{
		MODE_OFFLINE,
		MODE_ONLINE,
		MODE_PEERCAST,
	} PLAYER_MODE;

protected:
	utl::SyncOptional<std::wstring> text_; // ステータステキスト

	boost::recursive_timed_mutex mutex_; // builder_;
	boost::shared_ptr<IBaseBuilder> builder_;

	boost::thread thExecute_; // executeThread
	boost::thread thTerminate_; // terminateThread

	HWND hwnd_;
	UINT msg_;
	std::wstring path_;
	BuildConfig cfg_;

	network::URLParser url_;
	network::PeerCast peca_;
	PLAYER_MODE mode_;

	void executeThread(HWND hwnd, UINT msg, const std::wstring& path, const BuildConfig& cfg);
	void terminateThread();

public:
	DSPlayer(utl::SyncLog& log);
	virtual ~DSPlayer();

	bool init(HWND hwnd, UINT msg, const std::wstring& path, const BuildConfig& cfg);
	void execute();
	bool terminate();
	void UnblockTerminate();
	PLAYER_MODE mode() { return mode_; }
	bool online() { return !(mode_ == MODE_OFFLINE); }
	bool peercast() { return mode_ == MODE_PEERCAST; }

	bool getState(long& state);
	bool isRunning();

	bool run();
	bool pause();
	bool togglePause();
	bool stop();
	bool abort();

	bool getEvent(long& code, LONG_PTR& param1, LONG_PTR& param2);
	bool repaint(HDC hdc);
	bool setVideoPosition(RECT& dst);
	bool getNativeVideoSize(SIZE& size);
	bool saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit);

	bool getStatusText(std::wstring& text);

	bool getSampleTime(LONGLONG& time);
	bool getPositionDiff(LONGLONG& diff);
	bool getDuration(LONGLONG& duration);
	bool getCurrentPosition(LONGLONG& pos);
	bool setPositions(LONGLONG current, LONGLONG stop);
	bool getPositions(LONGLONG& current, LONGLONG& stop);
	bool seek(LONGLONG current);
	bool getProgress(int& percent);
	bool getCurrentBitrate(int& bitrate);
	bool getType(long& type);
	bool getPackets(DWORD& packets);
	bool getFramerate(int& rate);
	bool getCurrentFramerate(int& rate);
	bool getFramesDrawn(int& frames);
	bool getFramesDropped(int& frames);

	bool getBalance(int& balance);
	bool getVolume(int& volume);
	bool getMute(bool& mute);

	bool setBalance(int balance);
	bool setVolume(int volume);
	bool setMute(bool mute);

	bool getProcAmpRange(ProcAmpRangeGroup& group);
	bool setProcAmp(const ProcAmpValue& value);
	bool getProcAmp(ProcAmpValue& value);

	bool setNominalRange(int value);

	bool setResizer(int type);

	bool setNetwork(const NetworkControl& network);
	bool setType(const TypeControl& type);

	bool setDecoderSet(const DecoderIDSet& decoder);

	bool getFilterProperty(std::vector<FilterProperty>& prop);
	bool showFilterProperty(int index, HWND parent, HINSTANCE hi, DWORD id);
};

}



