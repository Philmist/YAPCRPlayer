#pragma once

#define WM_GRAPH_NOTIFY	(WM_APP + 20)

#include <boost/thread.hpp>
#include <boost/functional.hpp>

#include "GraphManager.h"

namespace dsp {

// グラフ通知
typedef enum {
	GRAPH_NOTIFY_START, // 再生開始
	GRAPH_NOTIFY_END, // 再生終了
	GRAPH_NOTIFY_RESTART, // 再再生
	GRAPH_NOTIFY_CONNECT, // チャンネル接続終了
	GRAPH_NOTIFY_RETRY_END, // チャンネル接続リトライ終了
} GRAPH_NOTIFY;

// オーディオ
struct AudioControl {
	AudioControl()
		: volume(50)
		, balance(0)
		, mute(false)
	{}

	int volume; // 音量
	int balance; // 音量バランス
	bool mute; // ミュート
};

// ビデオ
struct VideoControl {
	VideoControl()
		: procamp()
		, nominal(dsp::NOMINAL_RANGE_0_255)
		, resizer(d3dutl::RESIZER_DEFAULT)
	{}

	virtual ~VideoControl() {}

	ProcAmpValue procamp; // 色補正
	int nominal; // 出力範囲
	int resizer; // リサイズ処理
};

// ネットワーク
struct NetworkControl {
	// チャンネル接続(PeerCast)
	struct Connect {
		Connect()
			: timeout(10000)
			, count(5)
			, wait(5)
			, stop(false)
		{}

		int timeout; // タイムアウト
		int count; // リトライ数
		int wait; // リトライ間隔
		bool stop; // リトライ終了時にチャンネルを切断する
	};

	// チャンネル再接続(PeerCast)
	struct Reconnect {
		Reconnect()
			: interval(60000)
		{}

		int interval; // チャンネル再接続(自動)の連続実行を制限する時間
	};

	// リレー(PeerCast)
	struct Relay {
		// 手動
		struct Manual {
			Manual()
				: reconnect(false)
				, stop(false)
			{}

			bool reconnect; // リレーがあるときチャンネル再接続を無効
			bool stop; // リレーがあるときチャンネル切断を無効
		};

		// 自動
		struct Automatic {
			Automatic()
				: reconnect(false)
				, stop(false)
			{}

			bool reconnect; // リレーがあるときチャンネル再接続を無効
			bool stop; // リレーがあるときチャンネル切断を無効
		};

		Manual manual;
		Automatic automatic;
	};

	// 自動再構築
	struct Rebuild {
		Rebuild()
			: wait(0)
		{}

		int wait; // 再構築待機時間
	};

	// URL Buffer Reader
	struct Reader {
		Reader()
			: userAgent(L"PCRPlayer/1.00")
			, timeout(10000)
		{}

		std::wstring userAgent; // userAgent
		int timeout; // タイムアウト(ms)
	};

public:
	Connect connect; // リトライ(PeerCast)
	Reconnect reconnect; // 自動再構築(PeerCast)
	Relay relay; // リレー(PeerCast)
	Rebuild rebuild; // 自動再構築
	Reader reader; // URL Buffer Reader
};

// タイプ別設定
struct TypeControl {
	struct WMV {
		struct Rebuild {
			Rebuild()
				: enable(true)
				, time(20000)
			{}

			bool enable; // 自動再構築を有効
			DWORD time; // 指定時間で再構築
		};

		Rebuild packet; // 指定時間パケットを受け取らなかったら再構築
		Rebuild frame; // 指定時間フレームが欠落し続けたら再構築
	};

	struct FLV : public FLVConfig {
		FLV()
			: timestamp(true)
		{}

		bool timestamp; // 再生時間をタイムスタンプで表示
	};

	WMV wmv;
	FLV flv;
};

// デコーダ設定
struct DecoderControl {
	DecoderControl()
		: dxva2(true)
	{}

	bool dxva2; // DXVA2を有効にする
};


struct BuildConfig {
	BuildConfig() {}

	AudioControl audio; // オーディオ
	VideoControl video; // ビデオ
	NetworkControl network; // ネットワーク
	TypeControl type; // タイプ別設定
	DecoderControl decoder; // デコーダ設定
	DecoderIDSet decoderSet; // デコーダセット
};

struct BuildStatus {
	BuildStatus()
		: hwnd(NULL)
		, msg(0)
		, path()
	{}

	HWND hwnd;
	UINT msg;
	std::wstring path;
};

class BuildStatusMutex {
	boost::recursive_mutex mutex_;
	BuildStatus status_;

public:
	void set(HWND hwnd, UINT msg, const std::wstring& path)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		status_.hwnd = hwnd;
		status_.msg = msg;
		status_.path = path;
	}

	BuildStatus get()
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		return status_;
	}
};

// グラフ操作をマルチスレッド化
class IBaseBuilder : protected utl::Logger {
protected:
	utl::SyncOptional<std::wstring>& text_; // ステータステキスト

protected:
	BuildStatusMutex status_;

	boost::thread thread_;
	boost::thread run_;
	boost::recursive_mutex mutex_;
	GraphManager graph_;

	boost::recursive_mutex mxcfg_;
	BuildConfig cfg_;

protected: // 設定値
	int volume_;
	int balance_;

public:
	IBaseBuilder(utl::SyncLog& log, utl::SyncOptional<std::wstring>& text);
	virtual ~IBaseBuilder();

	void mainloop(HWND hwnd, UINT msg, const std::wstring& path);
	bool build(HWND hwnd, UINT msg, const std::wstring& path);

	void execute(HWND hwnd, UINT msg, const std::wstring& path, const BuildConfig& cfg);
	bool terminate();

	bool getState(long& state);
	bool isRunning();
	bool hasAudio();
	bool hasVideo();
	bool isAudioRunning(long timeout = 0);
	bool isVideoRunning(long timeout = 0);

	bool run();
	virtual bool pause();
	virtual bool togglePause();
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
	virtual bool seek(LONGLONG current);

	virtual bool getProgress(int& percent) { return false; }
	bool getCurrentBitrate(int& bitrate);
	bool getType(long& type);
	bool getPackets(DWORD& packets);
	bool getFramerate(int& rate);
	bool getCurrentFramerate(int& rate);
	bool getFramesDrawn(int& frames);
	bool getFramesDropped(int& frames);

	void getBalance(int& balance);
	void getVolume(int& volume);
	void getMute(bool& mute);

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

	bool setDecoderSet(const DecoderIDSet& decoderSet);

	bool getFilterProperty(std::vector<FilterProperty>& prop);
	bool showFilterProperty(int index, HWND parent, HINSTANCE hi, DWORD id);

private:
	bool setFLVConfig(const FLVConfig& cfg);
	void runThread(HWND hwnd, UINT msg);

protected:
	virtual bool render(HWND hwnd, UINT msg, const std::wstring& path) = 0;
	virtual bool onBuild(HWND hwnd, UINT msg, const std::wstring& path) { return true; }
	virtual bool onBufferEnter(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2) { return true; }
	virtual bool onBufferExit(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2) { return true; }
	virtual bool onEnterLoop(HWND hwnd, UINT msg, const std::wstring& path) { return true; }
	virtual bool onLoop(HWND hwnd, UINT msg, const std::wstring& path) { return true; }
	virtual bool onExitLoop(HWND hwnd, UINT msg, const std::wstring& path) { return true; }
	virtual bool onAbort(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2) { return true; }
	virtual bool onComplete(HWND hwnd, UINT msg, const std::wstring& path, LONG_PTR p1, LONG_PTR p2) { return true; }
	virtual bool onRebuild(HWND hwnd, UINT msg, const std::wstring& path) { return true; }

protected:
	struct Position {
		Position() : result(false), value(0) {}
		bool result;
		LONGLONG value;
	};

	struct State {
		State() : result(false), value(State_Stopped) {}
		bool result;
		long value;
	};

	struct AsyncState {
		AsyncState(){}

		boost::recursive_mutex mutex;
		Position duration;
		Position current;
		State state;

		void init()
		{
			boost::recursive_mutex::scoped_lock lock(mutex);
			duration = Position();
			current = Position();
			state = State();
		}
	};

	AsyncState state_;

	bool getStateImpl();
	bool getStateImpl(long timeout, long& state);
	bool getDurationImpl();
	bool getDurationImpl(LONGLONG& duration);
	bool getCurrentPositionImpl();
	bool getCurrentPositionImpl(LONGLONG& pos);
};

} // namespace dsp

