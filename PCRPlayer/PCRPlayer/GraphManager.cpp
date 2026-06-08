#include "stdafx.h"




namespace dsp {

#ifdef _DEBUG

void DebugEnumFilters_(const CComPtr<IGraphBuilder>& graph)
{
	CComPtr<IEnumFilters> ef;
	graph->EnumFilters(&ef);

	CComPtr<IBaseFilter> f;
	ULONG fetch = 0;
	while (ef->Next(1, &f, &fetch) == S_OK)
	{
		FILTER_INFO fi;
		f->QueryFilterInfo(&fi);
		DebugText(L":%s\n", fi.achName);
		fi.pGraph->Release();
		f.Release();
	}
}

#define		DebugEnumFilters(graph)  { DebugEnumFilters_(graph); }

#else

#define		DebugEnumFilters(graph)

#endif


#ifdef LOGGER

void LogEnumFilters_(const CComPtr<IGraphBuilder>& graph)
{
	CComPtr<IEnumFilters> ef;
	graph->EnumFilters(&ef);

	CComPtr<IBaseFilter> f;
	ULONG fetch = 0;
	while (ef->Next(1, &f, &fetch) == S_OK)
	{
		FILTER_INFO fi;
		f->QueryFilterInfo(&fi);
		LogText(L":%s\n", fi.achName);
		fi.pGraph->Release();
		f.Release();
	}
}

#define		LogEnumFilters(graph)  { LogEnumFilters_(graph); }

#else

#define		LogEnumFilters(graph)

#endif


GraphManager::GraphManager(utl::SyncLog& log)
	: Logger(log, utl::Log::LOGTYPE_GRAPH)
	, init_(false)
{
	::memset(&bmiHeader_, 0, sizeof(BITMAPINFOHEADER));
	AvgTimePerFrame_ = 0;
}

GraphManager::~GraphManager()
{
	detach();
	//uninit();
}


void GraphManager::addLogEnumFilters(const CComPtr<IGraphBuilder>& graph)
{
	std::wstring detail;
	CComPtr<IEnumFilters> ef;
	if (SUCCEEDED(graph->EnumFilters(&ef)))
	{
		CComPtr<IBaseFilter> f;
		ULONG fetch = 0;
		while (ef->Next(1, &f, &fetch) == S_OK)
		{
			if (f)
			{
				FILTER_INFO fi = {};
				f->QueryFilterInfo(&fi);
				if (fi.achName)
				{
					if (!detail.empty())
					{
						detail += L"\r\n";
					}
					detail += fi.achName;
				}
				if (fi.pGraph) { fi.pGraph->Release(); }
				f.Release();
			}
		}

		detailLog(L"Filter Graph", L"%s", detail.c_str());
	}
}


bool GraphManager::init()
{
	if (init_)
	{
		uninit();
		addLog(L"");
	}

	addLog(L"GraphManager::init (Enter)");

	::memset(&bmiHeader_, 0, sizeof(BITMAPINFOHEADER));
	AvgTimePerFrame_ = 0;
	property_.clear();

	HRESULT hr = S_OK;
	hr = graph_.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr)) { return false; }

	CustomReferenceClock *clock = new CustomReferenceClock(L"CustomReferenceClock", NULL, &hr, NULL);
	if (clock)
	{
		clock->AddRef();
		clock_.Attach(clock);

		CComPtr<IMediaFilter> filter;
		hr = graph_->QueryInterface<IMediaFilter>(&filter);
		if (filter)
		{
			filter->SetSyncSource(clock_);
		}
	}
			
	hr = graph_->QueryInterface<IMediaControl>(&control_);
	if (FAILED(hr)) { return false; }

	hr = graph_->QueryInterface<IMediaEventEx>(&event_);
	if (FAILED(hr)) { return false; }

	hr = graph_->QueryInterface<IMediaSeeking>(&seek_);
	if (FAILED(hr)) { return false; }


	addLog(L"GraphManager::init (Exit)");
	init_ = true;
	return true;
}

void GraphManager::uninit()
{
	if (!init_) { return; }

	addLog(L"GraphManager::uninit (Enter)");
	
	// ※重要
	// FLVはabort()でバッファリング停止してからstop()する必要がある
	// stop()だけだと永遠に停止しない
	bool ab = abort();
	addLog(L"GraphManager::uninit abort (%s)", ab ? L"true" : L"false");

	bool st = stop();
	addLog(L"GraphManager::uninit stop (%s)", st ? L"true" : L"false");

	clock_.Release();
	control_.Release();
	event_.Release();
	seek_.Release();

	source_.reset();
	video_.reset();
	audio_.reset();

	splitter_.Release();

	decVideo_.Release();
	decAudio_.Release();

	if (graph_)
	{
		graph_->Abort();
		addLog(L"GraphManager::uninit graph (Abort)");
	}
	graph_.Release();
	addLog(L"GraphManager::uninit graph (Release)");

	init_ = false;

	::memset(&bmiHeader_, 0, sizeof(BITMAPINFOHEADER));
	AvgTimePerFrame_ = 0;
	property_.clear();

	addLog(L"GraphManager::uninit (Exit)");
}

void GraphManager::detach()
{
	// 停止できなかった場合最終的にIGraphBuilderが残る
	// メインスレッドでCComPtrがデストラクタでリリースしようとするため強制終了できなくなる
	// デタッチしてリリースを防ぐ

	if (source_) { source_->detach(); }
	if (video_) { video_->detach(); }
	if (audio_) { audio_->detach(); }

	splitter_.Detach();

	decVideo_.Detach();
	decAudio_.Detach();

	control_.Detach();
	event_.Detach();
	seek_.Detach();

	graph_.Detach();
}


bool GraphManager::render(
	HWND hwnd, const std::wstring& path,
	const SourcePtr& source, const VideoPtr& video, const AudioPtr& audio, const DecoderIDSet& decoder)
{
	addLog(L"GraphManager::render (Enter)");

	HRESULT hr = S_OK;

	CComPtr<ICaptureGraphBuilder2> cgb;
	cgb.CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER);
	cgb->SetFiltergraph(graph_);

	source_ = source;
	if (!source_ || !source_->addToGraph(graph_, path))
	{
		addLog(L"GraphManager::addToGraph (Source Error)");
		return false;
	}
	addLog(L"GraphManager::addToGraph (Source)");

	video_ = video;
	if (!video_ || !video_->addToGraph(graph_, hwnd))
	{
		graph_->RemoveFilter(*source_);
		source_.reset();
		addLog(L"GraphManager::addToGraph (Video error)");
		return false;
	}
	addLog(L"GraphManager::addToGraph (Video)");

	audio_ = audio;
	if (!audio_ || !audio_->addToGraph(graph_))
	{
		graph_->RemoveFilter(*source_);
		source_.reset();
		graph_->RemoveFilter(*video_);
		video_.reset();
		addLog(L"GraphManager::addToGraph (Audio Error)");
		return false;
	}
	addLog(L"GraphManager::addToGraph (Audio)");

	// Decoder
	if (!IsEqualGUID(decoder.video.id, GUID_NULL))
	{
		decVideo_.CoCreateInstance(decoder.video.id, 0, CLSCTX_INPROC_SERVER);
		if (decVideo_)
		{
			graph_->AddFilter(decVideo_, decoder.video.name.c_str());
		}
	}

	if (!IsEqualGUID(decoder.audio.id, GUID_NULL))
	{
		decAudio_.CoCreateInstance(decoder.audio.id, 0, CLSCTX_INPROC_SERVER);
		if (decAudio_)
		{
			graph_->AddFilter(decAudio_, decoder.audio.name.c_str());
		}
	}

	hr = cgb->RenderStream(NULL, &MEDIATYPE_Video, *source_, NULL, *video_);
	addLog(L"GraphManager::renderStream (Video)");

	cgb->RenderStream(NULL, &MEDIATYPE_Audio, *source_, NULL, *audio_);
	addLog(L"GraphManager::renderStream (Audio)");

	if (decVideo_)
	{
		CComPtr<IPin> pin;
		findConnectTo(decVideo_, pin, PINDIR_INPUT);
		if (!pin)
		{
			graph_->RemoveFilter(decVideo_);
			decVideo_.Release();
		}
	}

	if (decAudio_)
	{
		CComPtr<IPin> pin;
		findConnectTo(decAudio_, pin, PINDIR_INPUT);
		if (!pin)
		{
			graph_->RemoveFilter(decAudio_);
			decAudio_.Release();
		}
	}


	CComPtr<IPin> videoPin, audioPin;

	findConnectTo(*video_, videoPin, PINDIR_INPUT);
	findConnectTo(*audio_, audioPin, PINDIR_INPUT);

	if (!videoPin && !audioPin)
	{
		graph_->RemoveFilter(*source_);
		source_.reset();
		graph_->RemoveFilter(*video_);
		video_.reset();
		graph_->RemoveFilter(*audio_);
		audio_.reset();
		if (decVideo_)
		{
			graph_->RemoveFilter(decVideo_);
			decVideo_.Release();
		}
		if (decAudio_)
		{
			graph_->RemoveFilter(decAudio_);
			decAudio_.Release();
		}
		addLog(L"GraphManager::renderStream (Error)");
		return false;
	}
	else if (videoPin && !audioPin)
	{
		graph_->RemoveFilter(*audio_);
		audio_.reset();
		if (decAudio_)
		{
			graph_->RemoveFilter(decAudio_);
			decAudio_.Release();
		}
	}
	else if (!videoPin && audioPin)
	{
		graph_->RemoveFilter(*video_);
		video_.reset();
		if (decVideo_)
		{
			graph_->RemoveFilter(decVideo_);
			decVideo_.Release();
		}
	}

	rendered();

	addLog(L"GraphManager::render (Exit)");

	addLogEnumFilters(graph_);

	LogEnumFilters(graph_);
	return true;
}

bool GraphManager::renderEx(
	HWND hwnd, const std::wstring& path,
	const SourcePtr& source, const VideoPtr& video, const AudioPtr& audio, const DecoderIDSet& decoder,
	bool supportedTypeOnly)
{
	addLog(L"GraphManager::renderEx (Enter)");

	HRESULT hr = S_OK;

	CComPtr<ICaptureGraphBuilder2> cgb;
	cgb.CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER);
	cgb->SetFiltergraph(graph_);


	source_ = source;
	if (!source_ || !source_->addToGraph(graph_, path))
	{
		addLog(L"GraphManager::addToGraph (Source Error)");
		return false;
	}
	addLog(L"GraphManager::addToGraph (Source)");

	video_ = video;
	if (!video_ || !video_->addToGraph(graph_, hwnd))
	{
		graph_->RemoveFilter(*source_);
		source_.reset();
		addLog(L"GraphManager::addToGraph (Video Error)");
		return false;
	}
	addLog(L"GraphManager::addToGraph (Video)");

	audio_ = audio;
	if (!audio_ || !audio_->addToGraph(graph_))
	{
		graph_->RemoveFilter(*source_);
		source_.reset();
		graph_->RemoveFilter(*video_);
		video_.reset();
		addLog(L"GraphManager::addToGraph (Audio Error)");
		return false;
	}
	addLog(L"GraphManager::addToGraph (Audio)");

	// Decoder
	if (!IsEqualGUID(decoder.video.id, GUID_NULL))
	{
		decVideo_.CoCreateInstance(decoder.video.id, 0, CLSCTX_INPROC_SERVER);
		if (decVideo_)
		{
			graph_->AddFilter(decVideo_, decoder.video.name.c_str());
		}
	}

	if (!IsEqualGUID(decoder.audio.id, GUID_NULL))
	{
		decAudio_.CoCreateInstance(decoder.audio.id, 0, CLSCTX_INPROC_SERVER);
		if (decAudio_)
		{
			graph_->AddFilter(decAudio_, decoder.audio.name.c_str());
		}
	}


	if (source_->type() == SOURCE_TYPE_FLV)
	{
		splitter_ = CComQIPtr<IBaseFilter>(new CFLVSplitterFilter(NULL, &hr, log_));
		graph_->AddFilter(splitter_, L"FLV Splitter");

		//DebugEnumFilters(graph_);
	
		CComPtr<IPin> sourceOut, splitterIn;
		findPin((IBaseFilter*)*source_, sourceOut, PINDIR_OUTPUT);
		findPin(splitter_, splitterIn, PINDIR_INPUT);

		hr = graph_->Connect(sourceOut, splitterIn);

		// RenderStream
		hr = cgb->RenderStream(NULL, &MEDIATYPE_Video, *source_, NULL, *video_);

		CComPtr<IPin> splitterPin;
		findConnectTo(splitter_, splitterPin, PINDIR_INPUT);
	
		if (!splitterPin)
		{
			graph_->RemoveFilter(*source_);
			source_.reset();
			graph_->RemoveFilter(*video_);
			video_.reset();
			graph_->RemoveFilter(*audio_);
			audio_.reset();

			graph_->RemoveFilter(splitter_);
			splitter_.Release();
			return false;
		}
	}
	else
	{
		if (supportedTypeOnly) // 対応形式以外は失敗
		{
			graph_->RemoveFilter(*source_);
			source_.reset();
			graph_->RemoveFilter(*video_);
			video_.reset();
			graph_->RemoveFilter(*audio_);
			audio_.reset();

			return false;
		}

		hr = cgb->RenderStream(NULL, NULL, *source_, NULL, *video_);
		if (FAILED(hr))
		{
			cgb->RenderStream(NULL, &MEDIATYPE_Video, *source_, NULL, *video_);
		}
	}
	addLog(L"GraphManager::renderStream (Video)");


	hr = cgb->RenderStream(NULL, NULL, *source_, NULL, *audio_);
	if (FAILED(hr))
	{
		cgb->RenderStream(NULL, &MEDIATYPE_Audio, *source_, NULL, *audio_);
	}
	addLog(L"GraphManager::renderStream (Audio)");

	if (decVideo_)
	{
		CComPtr<IPin> pin;
		findConnectTo(decVideo_, pin, PINDIR_INPUT);
		if (!pin)
		{
			graph_->RemoveFilter(decVideo_);
			decVideo_.Release();
		}
	}

	if (decAudio_)
	{
		CComPtr<IPin> pin;
		findConnectTo(decAudio_, pin, PINDIR_INPUT);
		if (!pin)
		{
			graph_->RemoveFilter(decAudio_);
			decAudio_.Release();
		}
	}



	CComPtr<IPin> videoPin, audioPin;

	findConnectTo(*video_, videoPin, PINDIR_INPUT);
	findConnectTo(*audio_, audioPin, PINDIR_INPUT);

	if (!videoPin && !audioPin)
	{
		graph_->RemoveFilter(*source_);
		source_.reset();
		graph_->RemoveFilter(*video_);
		video_.reset();
		graph_->RemoveFilter(*audio_);
		audio_.reset();

		if (splitter_)
		{
			graph_->RemoveFilter(splitter_);
			splitter_.Release();
		}
		if (decVideo_)
		{
			graph_->RemoveFilter(decVideo_);
			decVideo_.Release();
		}
		if (decAudio_)
		{
			graph_->RemoveFilter(decAudio_);
			decAudio_.Release();
		}
		addLog(L"GraphManager::renderStream (Error)");
		return false;
	}
	else if (videoPin && !audioPin)
	{
		graph_->RemoveFilter(*audio_);
		audio_.reset();

		if (decAudio_)
		{
			graph_->RemoveFilter(decAudio_);
			decAudio_.Release();
		}
	}
	else if (!videoPin && audioPin)
	{
		graph_->RemoveFilter(*video_);
		video_.reset();

		if (splitter_)
		{
			graph_->RemoveFilter(splitter_);
			splitter_.Release();
		}
		if (decVideo_)
		{
			graph_->RemoveFilter(decVideo_);
			decVideo_.Release();
		}
	}

	rendered();
	addLog(L"GraphManager::renderEx (Exit)");

	addLogEnumFilters(graph_);

	LogEnumFilters(graph_);
	return true;
}

void GraphManager::rendered()
{
	// フィルタプロパティ取得
	getFilterProperty(graph_, property_);

	// BITMAPINFOHEADERとフレームレート取得
	if (video_)
	{
		CComPtr<IPin> pin;
		if (!findPin(*video_, pin, PINDIR_INPUT)) { return; }
		if (!pin) { return; }

		// ビデオレンダラのメディアタイプを取得する
		AM_MEDIA_TYPE mt;
		if (!SUCCEEDED(pin->ConnectionMediaType(&mt))) { return; }

		if (IsEqualGUID(mt.formattype, FORMAT_VideoInfo))
		{
			if (NULL == mt.pbFormat || mt.cbFormat < sizeof(VIDEOINFOHEADER))
			{
				return;
			}

			VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER *)mt.pbFormat;
			::memcpy(&bmiHeader_, &(pVIH->bmiHeader), sizeof(BITMAPINFOHEADER));
			AvgTimePerFrame_ = pVIH->AvgTimePerFrame;
		}
		else if (IsEqualGUID(mt.formattype, FORMAT_VideoInfo2))
		{
			if (NULL == mt.pbFormat || mt.cbFormat < sizeof(VIDEOINFOHEADER2))
			{
				return;
			}

			VIDEOINFOHEADER2 *pVIH = (VIDEOINFOHEADER2 *)mt.pbFormat;
			::memcpy(&bmiHeader_, &(pVIH->bmiHeader), sizeof(BITMAPINFOHEADER));
			AvgTimePerFrame_ = pVIH->AvgTimePerFrame;
		}

		FreeMediaType(mt);
	}
}





bool GraphManager::getEvent(long& code, LONG_PTR& param1, LONG_PTR& param2)
{
	if (!event_) { return false; }
	bool res = SUCCEEDED(event_->GetEvent(&code, &param1, &param2, 0));
	if (res) { event_->FreeEventParams(code, param1, param2); }
	return res;
}

bool GraphManager::getState(long timeout, long& state)
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->GetState(timeout, &state));
}

bool GraphManager::run()
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->Run());
}

bool GraphManager::pause()
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->Pause());
}

bool GraphManager::stop()
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->Stop());
}

bool GraphManager::abort()
{
	if (!source_) { return false; }
	return source_->abort();
}


bool GraphManager::repaint(HDC hdc)
{
	if (!video_) { return false; }
	return video_->repaint(hdc);
}

bool GraphManager::setVideoPosition(RECT& dst)
{
	if (!video_) { return false; }

	return video_->setVideoPosition(dst);
}

bool GraphManager::getNativeVideoSize(SIZE& size)
{
	if (!video_) { return false; }
	return video_->getNativeVideoSize(size);
}

bool GraphManager::saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit)
{
	if (!video_) { return false; }
	return video_->saveCurrentImage(path, format, jpegQuality, width, height, fit);
}



bool GraphManager::getSampleTime(LONGLONG& time)
{
	if (!video_) { return false; }
	return video_->getSampleTime(time);
}

bool GraphManager::getPositionDiff(LONGLONG& diff)
{
	if (!splitter_) { return false; }
	CComQIPtr<IFLVSplitterProp, &IID_IFLVSplitterProp> prop(splitter_);
	if (!prop) { return false; }
	return SUCCEEDED(prop->GetPositionDiff(&diff));
}

bool GraphManager::getDuration(LONGLONG& dur)
{
	if (!seek_) { return false; }
	return SUCCEEDED(seek_->GetDuration(&dur));
}

bool GraphManager::getCurrentPosition(LONGLONG& dur, bool timestamp)
{
	if (splitter_)
	{
		LONGLONG time = 0, diff = 0;
		if (getPositionDiff(diff))
		{//getPositionDiffが失敗した時はオフライン
			if (getSampleTime(time))
			{
				dur = time + (timestamp ? diff : 0);
				return true;
			}
		}
	}

	if (!seek_) { return false; }
	return SUCCEEDED(seek_->GetCurrentPosition(&dur));
}

bool GraphManager::setPositions(LONGLONG current, LONGLONG stop)
{//TODO
	if (!seek_) { return false; }
	return SUCCEEDED(seek_->SetPositions(&current, AM_SEEKING_AbsolutePositioning, &stop, AM_SEEKING_AbsolutePositioning));
}

bool GraphManager::getPositions(LONGLONG& current, LONGLONG& stop)
{//TODO
	if (!seek_) { return false; }
	HRESULT hr = seek_->GetPositions(&current, &stop);

	return SUCCEEDED(hr);
}


bool GraphManager::seek(LONGLONG current)
{
	if (!seek_) { return false; }
	return SUCCEEDED(seek_->SetPositions(&current, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning));
}

bool GraphManager::getProgress(int& percent)
{
	if (!source_) { return false; }
	return source_->getProgress(percent);
}

bool GraphManager::getCurrentBitrate(int& bitrate)
{
	if (!source_) { return false; }
	return source_->getCurrentBitrate(bitrate);
}


bool GraphManager::getType(long& type)
{
	if (!source_) { return false; }
	type = source_->type();
	return true;
}

bool GraphManager::getPackets(DWORD& packets)
{
	if (!source_) { return false; }
	return source_->getPackets(packets);
}

bool GraphManager::getFramerate(int& bitrate)
{
	if (splitter_)
	{
		CComQIPtr<IFLVSplitterProp, &IID_IFLVSplitterProp> prop(splitter_);
		if (prop)
		{
			prop->GetFramerate(&bitrate);
			return true;
		}
	}

	if (!video_) { return false; }
	if (AvgTimePerFrame_ > 0)
	{
		bitrate = static_cast<int>(1000000000.0 / (double)AvgTimePerFrame_);
		return true;
	}
	return false;
}

bool GraphManager::getCurrentFramerate(int& rate)
{
	if (!video_) { return false; }
	return video_->getCurrentFramerate(rate);
}

bool GraphManager::getFramesDrawn(int& frames)
{
	if (!video_) { return false; }
	return video_->getFramesDrawn(frames);
}

bool GraphManager::getFramesDropped(int& frames)
{
	if (!video_) { return false; }
	return video_->getFramesDropped(frames);
}

bool GraphManager::getBalance(int& balance)
{
	if (!audio_) { return false; }
	return audio_->getBalance(balance);
}

bool GraphManager::getVolume(int& volume)
{
	if (!audio_) { return false; }
	return audio_->getVolume(volume);
}

bool GraphManager::setBalance(int balance)
{
	if (!audio_) { return false; }
	return audio_->setBalance(balance);
}

bool GraphManager::setVolume(int volume)
{
	if (!audio_) { return false; }
	return audio_->setVolume(volume);
}

bool GraphManager::getProcAmpRange(ProcAmpRangeGroup& group)
{
	if (!video_) { return false; }
	return video_->getProcAmpRange(group);
}

bool GraphManager::setProcAmp(const ProcAmpValue& value)
{
	if (!video_) { return false; }
	return video_->setProcAmp(value);
}

bool GraphManager::getProcAmp(ProcAmpValue& value)
{
	if (!video_) { return false; }
	return video_->getProcAmp(value);
}

bool GraphManager::setNominalRange(int value)
{
	if (!video_) { return false; }
	return video_->setNominalRange(value);
}

bool GraphManager::setResizer(int type)
{
	if (!video_) { return false; }
	return video_->setResizer(type);
}

bool GraphManager::preroll()
{
	if (!source_) { return false; }
	return source_->preroll();
}

bool GraphManager::getFilterProperty(std::vector<FilterProperty>& prop)
{
	if (!graph_) { return false; }
	prop = property_;
	return true;
}

bool GraphManager::showFilterProperty(int index, HWND parent, HINSTANCE hi, DWORD id)
{
	if (!graph_) { return false; }

	if (index < 0 || index >= (int)property_.size()) { return false; }

	std::wstring name;

	try {
		name = property_.at(index).name;
	}
	catch (...) {
		return false;
	}

	HRESULT hr = S_OK;

	CComPtr<IBaseFilter> filter;
	hr = graph_->FindFilterByName(name.c_str(), &filter);
	if (FAILED(hr) || !filter) { return false; }


	CComPtr<ISpecifyPropertyPages> spp;
	hr = filter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&spp);
	if (FAILED(hr) || !spp) { return false; }

	CAUUID guid = {};
	hr = spp->GetPages(&guid);
	if (FAILED(hr) || guid.cElems <= 0 || guid.pElems == NULL)
	{
		if (guid.pElems) { CoTaskMemFree(guid.pElems); }
		return false;
	}

	IUnknown* unk = NULL;
	hr = filter->QueryInterface(IID_IUnknown, (void **)&unk);
	if (FAILED(hr) || !unk) { return false; }

	utl::FilterPropertyHook::Hook(parent, hi, id);

	hr = OleCreatePropertyFrame(
		parent,					// 親ウィンドウ
		0,						// x (予約済み)
		0,						// y (予約済み)
		name.c_str(),			// ダイアログ ボックスのキャプション
		1,						// フィルタの数
		&unk,					// フィルタへのポインタ
		guid.cElems,			// プロパティ ページの数
		guid.pElems,			// プロパティ ページ CLSID へのポインタ
		LOCALE_USER_DEFAULT,	// ロケール識別子
		0,						// 予約済み
		NULL					// 予約済み
		);

	utl::FilterPropertyHook::Unhook();

	if (guid.pElems) { CoTaskMemFree(guid.pElems); }

	SAFE_RELEASE(unk);

	return true;
}

bool GraphManager::setFLVConfig(const FLVConfig& cfg)
{
	if (!splitter_) { return false; }
	CComQIPtr<IFLVSplitterProp, &IID_IFLVSplitterProp> prop(splitter_);
	if (!prop) { return false; }
	prop->SetConfig(cfg);
	return true;
}

bool GraphManager::hasAudio()
{
	if (!audio_) { return false; }
	return true;
}

bool GraphManager::hasVideo()
{
	if (!video_) { return false; }
	return true;
}

bool GraphManager::isAudioRunning(DWORD timeout)
{
	if (!audio_) { return false; }
	return audio_->isRunning(timeout);
}

bool GraphManager::isVideoRunning(DWORD timeout)
{
	if (!video_) { return false; }
	return video_->isRunning(timeout);
}

//==============================================================================

bool GraphManager::findPin(const CComPtr<IBaseFilter>& filter, CComPtr<IPin>& pin, PIN_DIRECTION dir)
{
	CComPtr<IEnumPins> pins;
	IEnumPins* pins_ = NULL;
	filter->EnumPins(&pins_);
	pins.Attach(pins_);

	if (!pins) { return false; }

	CComPtr<IPin> pin_;
	pins->Reset();
	while (pins->Next(1, &pin_, NULL) == S_OK)
	{
		PIN_INFO info;
		if (SUCCEEDED(pin_->QueryPinInfo(&info)))
		{
			info.pFilter->Release();
			if (info.dir == dir)
			{
				pin = pin_;
				return true;
			}
		}
		pin_.Release();
	}
	return false;
}

bool GraphManager::findConnectTo(const CComPtr<IBaseFilter>& filter, CComPtr<IPin>& pin, PIN_DIRECTION dir)
{
	CComPtr<IPin> tmp;
	if (!findPin(filter, tmp, dir)) { return false; }

	return SUCCEEDED(tmp->ConnectedTo(&pin));
}


template<typename T>
bool GraphManager::getService(const CComPtr<IPin>& pin, REFGUID guidService, CComPtr<T>& dst)
{
	if (!pin) { return false; }

	CComPtr<IMFGetService> service;
	pin->QueryInterface<IMFGetService>(&service);

	if (!service) { return false; }


	service->GetService(
		guidService,
		__uuidof(T),
		(void**)&dst);

	return true;
}

template<typename T>
bool GraphManager::getService(const CComPtr<IBaseFilter>& filter, REFGUID guidService, CComPtr<T>& dst)
{
	if (!filter) { return false; }

	CComPtr<IMFGetService> service;
	filter->QueryInterface<IMFGetService>(&service);

	if (!service) { return false; }


	service->GetService(
		guidService,
		__uuidof(T),
		(void**)&dst);

	return true;
}


bool GraphManager::getFilterProperty(const CComPtr<IGraphBuilder>& graph, std::vector<FilterProperty>& prop)
{
	prop.clear();
	if (!graph) { return false; }

	HRESULT hr = S_OK;
	CComPtr<IEnumFilters> enums;

	hr = graph->EnumFilters(&enums);
	if (FAILED(hr) || !enums) { return false; }

	hr = enums->Reset();
	if (FAILED(hr)) { return false; }

	CComPtr<IBaseFilter> filter;
	ULONG fetched;
	while (enums->Next(1, &filter, &fetched) == S_OK)
	{
		if (!filter) { continue; }

		FilterProperty fp;
		
		// フィルタインフォ
		FILTER_INFO info = {};
		hr = filter->QueryFilterInfo(&info);
		if (FAILED(hr))
		{
			filter.Release();
			continue;
		}

		info.achName[128 - 1] = L'\0'; // 念のため
		fp.name = info.achName;

		SAFE_RELEASE(info.pGraph);


		// プロパティ
		CComPtr<ISpecifyPropertyPages> spp;
		hr = filter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&spp);
		if (FAILED(hr) || !spp)
		{
			fp.enable = false;
			prop.push_back(fp);
			filter.Release();
			continue;
		}

		CAUUID guid = {};
		hr = spp->GetPages(&guid);
		if (FAILED(hr) || guid.cElems <= 0 || guid.pElems == NULL)
		{
			if (guid.pElems) { CoTaskMemFree(guid.pElems); }
			fp.enable = false;
			prop.push_back(fp);
			filter.Release();
			continue;
		}

		fp.enable = true;
		prop.push_back(fp);

		if (guid.pElems) { CoTaskMemFree(guid.pElems); }
		filter.Release();
	}

	//std::reverse(prop.begin(), prop.end());
	return true;
}


} // namespace dsp



/*
//#define DECODER_FILTER 1
#if DECODER_FILTER
	CComPtr<IBaseFilter> decoder;
	
	decoder.CoCreateInstance(CLSID_CMPEG2VidDecoderDS, NULL, CLSCTX_INPROC_SERVER);

	CComPtr<ICodecAPI> codec;
	decoder.QueryInterface<ICodecAPI>(&codec);


	VARIANT variant;

	variant.vt = VT_UI4;
	variant.uiVal = (UINT32)1;
	hr = codec->SetValue(&CODECAPI_AVDecVideoAcceleration_H264, &variant);
	
#endif
*/

