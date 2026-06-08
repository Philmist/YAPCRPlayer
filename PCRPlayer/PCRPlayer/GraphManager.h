#pragma once


#pragma comment(lib, "strmiids.lib")

//#pragma warning(push)
//#pragma warning(disable:4005)

#include <atlbase.h>
#include <dshow.h>

#include <boost/shared_ptr.hpp>

#include "FileSource.h"
#include "VideoRenderer.h"
#include "AudioRenderer.h"
#include "CustomReferenceClock.h"


#include "Splitter/DSUtil.h"
#include "Splitter/GolombBuffer.h"
#include "Splitter/MultiFiles.h"
#include "Splitter/BaseSplitterFile.h"
#include "Splitter/BaseSplitterFileEx.h"
#include "Splitter/FLVSplitter.h"

//#pragma warning(pop)

#ifndef _DEBUG
#pragma comment(lib, "strmbase.lib")
#else
#pragma comment(lib, "strmbasd.lib")
#endif


namespace dsp {

// デコーダID
struct DecoderID {
	DecoderID() : name(), id(GUID_NULL) {}
	std::wstring name; // フィルタ名
	GUID id; // フィルタID
};

// デコーダIDセット
struct DecoderIDSet {
	DecoderID video; // ビデオデコーダ
	DecoderID audio; // オーディオデコーダ
};


typedef boost::shared_ptr<IBaseSource> SourcePtr;
typedef boost::shared_ptr<IBaseVideoRenderer> VideoPtr;
typedef boost::shared_ptr<IBaseAudioRenderer> AudioPtr;

struct FilterProperty {
	FilterProperty()
		: name(), enable(false)
	{}

	bool enable;
	std::wstring name;
};

// グラフを直接操作するクラス
class GraphManager : protected utl::Logger {
	CComPtr<IGraphBuilder> graph_;

	CComPtr<CustomReferenceClock> clock_;
	CComPtr<IMediaControl> control_;
	CComPtr<IMediaEventEx> event_;
	CComPtr<IMediaSeeking> seek_;

	SourcePtr source_;
	VideoPtr video_;
	AudioPtr audio_;

	CComPtr<IBaseFilter> splitter_;

	CComPtr<IBaseFilter> decVideo_;
	CComPtr<IBaseFilter> decAudio_;

	bool init_;

	// 追加取得情報
	// フィルタグラフ構築後にrendered()で取得
	BITMAPINFOHEADER bmiHeader_;
	REFERENCE_TIME AvgTimePerFrame_;
	std::vector<FilterProperty> property_;

public:
	GraphManager(utl::SyncLog& log);
	virtual ~GraphManager();

	operator IGraphBuilder*() { return graph_; }
	operator CComPtr<IGraphBuilder>&() { return graph_; }

	void addLogEnumFilters(const CComPtr<IGraphBuilder>& graph);

	bool init();
	void uninit();
	void detach();

	bool render(
		HWND hwnd, const std::wstring& path,
		const SourcePtr& source, const VideoPtr& video, const AudioPtr& audio, const DecoderIDSet& decoder);

	bool renderEx(
		HWND hwnd, const std::wstring& path,
		const SourcePtr& source, const VideoPtr& video, const AudioPtr& audio, const DecoderIDSet& decoder,
		bool supportedTypeOnly);

private:
	void rendered();

public:
	bool getEvent(long& code, LONG_PTR& param1, LONG_PTR& param2);
	bool getState(long timeout, long& state);

	bool run();
	bool pause();
	bool stop();
	bool abort();

	bool repaint(HDC hdc);
	bool setVideoPosition(RECT& dst);
	bool getNativeVideoSize(SIZE& size);
	bool saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit);

	bool getSampleTime(LONGLONG& time);
	bool getPositionDiff(LONGLONG& diff);
	bool getDuration(LONGLONG& dur);
	bool getCurrentPosition(LONGLONG& pos, bool timestamp);
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

	IMediaSeeking* getSeek() { return seek_; }
	IMediaControl* getControl() { return control_; }

	bool getBalance(int& balance);
	bool getVolume(int& volume);
	bool setBalance(int balance);
	bool setVolume(int volume);

	bool getProcAmpRange(ProcAmpRangeGroup& group);
	bool setProcAmp(const ProcAmpValue& value);
	bool getProcAmp(ProcAmpValue& value);

	bool setNominalRange(int value);

	bool setResizer(int type);

	bool preroll();

	bool getFilterProperty(std::vector<FilterProperty>& prop);
	bool showFilterProperty(int index, HWND parent, HINSTANCE hi, DWORD id);

	bool setFLVConfig(const FLVConfig& cfg);

	bool hasAudio();
	bool hasVideo();
	bool isAudioRunning(DWORD timeout);
	bool isVideoRunning(DWORD timeout);

private:
	bool findPin(const CComPtr<IBaseFilter>& filter, CComPtr<IPin>& pin, PIN_DIRECTION dir);
	bool findConnectTo(const CComPtr<IBaseFilter>& filter, CComPtr<IPin>& pin, PIN_DIRECTION dir);

	template<typename T>
	bool getService(const CComPtr<IPin>& pin, REFGUID guidService, CComPtr<T>& dst);

	template<typename T>
	bool getService(const CComPtr<IBaseFilter>& filter, REFGUID guidService, CComPtr<T>& dst);

	bool getFilterProperty(const CComPtr<IGraphBuilder>& graph, std::vector<FilterProperty>& prop);
};

}
