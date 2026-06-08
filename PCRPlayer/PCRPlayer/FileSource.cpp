#include "stdafx.h"

namespace dsp {

bool DefaultSource::addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename)
{
	if (graph == NULL) { return false; }

	HRESULT hr = graph->AddSourceFilter(filename.c_str(), L"Default Source", &source_);
	if (FAILED(hr)) { return false; }

	source_->QueryInterface(IID_IAMOpenProgress, (void**)&control_);

	return true;
}

bool DefaultSource::getProgress(int& percent)
{
	if (control_)
	{
		LONGLONG total = 0, current = 0;
		HRESULT hr = control_->QueryProgress(&total, &current);
		if (FAILED(hr)) { return false; }

		if (total == 0) { return false; }
		percent = static_cast<int>((current * 100) / total);
		return true;
	}
	return false;
}

bool DefaultSource::abort()
{
	if (control_)
	{
		HRESULT hr = control_->AbortOperation();
		//DebugText(L"DefaultSource::abort() AbortOperation hr = %08X\n", hr);
		return SUCCEEDED(hr);
	}
	return false;
}


//******************************************************************************

AsyncSource::AsyncSource()
	: type_(SOURCE_TYPE_UNKNOWN)
{}


bool AsyncSource::addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename)
{
	if (graph == NULL) { return false; }

	type_ = SOURCE_TYPE_UNKNOWN;

	HRESULT hr = S_OK;

	source_.CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER);
	if (!source_) { return false; }

	hr = graph->AddFilter(source_, L"Async Reader");
	if (FAILED(hr)) { return false; }

	CComQIPtr<IFileSourceFilter> reader(source_);
	if (!reader)
	{
		graph->RemoveFilter(source_);
		return false;
	}

	hr = reader->Load(filename.c_str(), NULL);
	if (FAILED(hr))
	{
		graph->RemoveFilter(source_);
		return false;
	}

	// ファイルタイプ判別
	std::ifstream ifs(filename, std::ios::in|std::ios::binary);
	if (ifs)
	{
		std::vector<char> data;
		data.resize(4, 0);
		ifs.read((char*)data.data(), data.size());
		if (ifs)
		{
			if (data[0] == 'F' && data[1] == 'L' && data[2] == 'V' && data[3] == 1)
			{
				type_ = SOURCE_TYPE_FLV;
			}
			else if (data[0] == 0x1A && data[1] == 0x45 && data[2] == 0xDF && data[3] == 0xA3)
			{
				type_ = SOURCE_TYPE_MKV;
			}
		}

		ifs.close();
	}

	return true;
}

//******************************************************************************

URLFileSource::URLFileSource()
	: bitrate_(0)
	, time_(0)
	, length_(0)
{}

bool URLFileSource::addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename)
{
	if (graph == NULL) { return false; }

	// ビットレート関連変数初期化
	bitrate_ = 0;
	length_ = 0;
	time_ = 0;

	HRESULT hr = S_OK;

	// FLV以外
	source_.CoCreateInstance(CLSID_URLReader, NULL, CLSCTX_INPROC_SERVER);
	if (!source_) { return false; }

	hr = graph->AddFilter(source_, L"URL File Source");
	if (FAILED(hr)) { return false; }

	CComQIPtr<IFileSourceFilter> reader(source_);
	if (!reader)
	{
		graph->RemoveFilter(source_);
		return false;
	}

	hr = reader->Load(filename.c_str(), NULL);
	if (FAILED(hr))
	{
		graph->RemoveFilter(source_);
		return false;
	}

	// ソースフィルタの出力ピンからIAsyncReaderを取得
	CComPtr<IEnumPins> pins;
	source_->EnumPins(&pins);
	
	CComPtr<IPin> pin;
	ULONG fetched = 0;
	pins->Reset();
	while (S_OK == pins->Next(1, &pin, &fetched))
	{
		if (pin != NULL)
		{
			PIN_INFO info = {0};
			if (SUCCEEDED(pin->QueryPinInfo(&info)))
			{
				if (info.dir == PINDIR_OUTPUT)
				{
					pin->QueryInterface(&reader_);
					if (info.pFilter)
					{
						info.pFilter->Release();
					}
					break;
				}
			}
			pin.Release();
		}
	}
	return true;
}

bool URLFileSource::getCurrentBitrate(int& bitrate)
{
	if (reader_)
	{
		DWORD time = timeGetTime();

		if (time - time_ > 1000)
		{
			LONGLONG total = 0, length = 0;
			BOOL res = SUCCEEDED(reader_->Length(&total, &length));
			double test = (((double)(length - length_))*1000)/((double)(time - time_));

			//DebugText(L"length:%lld\n", length);
			//DebugText(L"(%s) length:%lld time:%u bitrate:%u\n",
			//	res?L"O":L"X",
			//	length - length_,
			//	time - time_,
			//	(DWORD)((test * 8) / 1024));

			bitrate_ = (DWORD)((test * 8) / 1000);
			length_ = length;
			time_ = time;

			bitrate = bitrate_;
			return res ? true : false;
		}
		bitrate = bitrate_;
		return true;
	}
	return false;
}



//******************************************************************************

URLBufferSource::URLBufferSource(const std::wstring& userAgent, int timeout)
	: type_(SOURCE_TYPE_UNKNOWN)
	, bitrate_(0)
	, time_(0)
	, length_(0)
	, isTimeout_(false)
	, userAgent_(userAgent)
	, timeout_(timeout)
{}

bool URLBufferSource::addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename)
{
	if (graph == NULL) { return false; }

	type_ = SOURCE_TYPE_UNKNOWN;
	// ビットレート関連変数初期化
	bitrate_ = 0;
	length_ = 0;
	time_ = 0;

	HRESULT hr = S_OK;

	source_.Attach(new MemSourceFilter());
	if (!source_) { return false; }


	hr = graph->AddFilter(source_, L"URL Buffer Reader");
	if (FAILED(hr))
	{
		graph->RemoveFilter(source_);
		return false;
	}

	bool res = source_->SetURL(filename, userAgent_, timeout_);
	type_ = source_->SourceType();

	if (!res || type_ != SOURCE_TYPE_FLV)
	{
		if (source_->IsTimeout())
		{
			isTimeout_ = true;
		}

		source_->Abort();

		graph->RemoveFilter(source_);
		return false;
	}

	// ソースフィルタの出力ピンからIAsyncReaderを取得
	CComPtr<IEnumPins> pins;
	source_->EnumPins(&pins);
	
	CComPtr<IPin> pin;
	ULONG fetched = 0;
	pins->Reset();
	while (S_OK == pins->Next(1, &pin, &fetched))
	{
		if (pin != NULL)
		{
			PIN_INFO info = {0};
			if (SUCCEEDED(pin->QueryPinInfo(&info)))
			{
				if (info.dir == PINDIR_OUTPUT)
				{
					pin->QueryInterface(&reader_);
					if (info.pFilter)
					{
						info.pFilter->Release();
					}
					break;
				}
			}
			pin.Release();
		}
	}
	return true;
}

bool URLBufferSource::getCurrentBitrate(int& bitrate)
{
	if (reader_)
	{
		DWORD time = timeGetTime();

		if (time - time_ > 1000)
		{
			LONGLONG total = 0, length = 0;
			bool res = SUCCEEDED(reader_->Length(&total, &length));

			if (length < 0)
			{
				bitrate = bitrate_ = 0;
				return true;
			}

			double test = (((double)(length - length_))*1000)/((double)(time - time_));

			bitrate_ = (DWORD)((test * 8) / 1000);
			length_ = length;
			time_ = time;

			bitrate = bitrate_;
			return res;

			//DebugText(L"length:%lld\n", length);
			//DebugText(L"(%s) length:%lld time:%u bitrate:%u\n",
			//	res?L"O":L"X",
			//	length - length_,
			//	time - time_,
			//	(DWORD)((test * 8) / 1024));
		}
		bitrate = bitrate_;
		return true;
	}
	return false;
}


bool URLBufferSource::abort()
{
	if (source_)
	{
		//DebugText(L"URLBufferSource::abort()\n"); 
		source_->Abort();
		return true;
	}
	return false;
}

//******************************************************************************

bool WMASFSource::addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename)
{
	if (graph == NULL) { return false; }

	HRESULT hr = S_OK;

	source_.CoCreateInstance(CLSID_WMAsfReader, NULL, CLSCTX_INPROC_SERVER);
	if (!source_) { return false; }

	hr = graph->AddFilter(source_, L"WMASF Reader");
	if (FAILED(hr)) { return false; }

	CComQIPtr<IFileSourceFilter> reader(source_);
	if (!reader)
	{
		graph->RemoveFilter(source_);
		return false;
	}

	
	hr = reader->Load(filename.c_str(), NULL);
	if (FAILED(hr))
	{
		graph->RemoveFilter(source_);
		return false;
	}


	// LoadしないとIWMDRMReaderは取得できない
	//CComPtr<IWMDRMReader2> drm;
	//CComQIPtr<IServiceProvider> sp(reader);
	//sp->QueryService(IID_IWMDRMReader2, IID_IWMDRMReader2, (void **)&drm);

	//CComPtr<IPin> pin = GetFirstPin(source_, PINDIR_OUTPUT);

	//CComPtr<IWMStreamConfig2> drm;
	//CComQIPtr<IServiceProvider> sp(pin);
	//sp->QueryService(IID_IWMStreamConfig2, IID_IWMStreamConfig2, (void **)&drm);

	source_->QueryInterface(IID_IWMReaderAdvanced2, (void**)&control_);
	return true;
}

bool WMASFSource::getProgress(int& percent)
{
	if (control_)
	{
		DWORD per = 0;
		QWORD buf = 0;
		HRESULT hr = control_->GetBufferProgress(&per, &buf);
		if (FAILED(hr)) { return false; }
		percent = per;
		return true;
	}
	return false;
}

bool WMASFSource::getCurrentBitrate(int& bitrate)
{
	if (control_)
	{
		WM_READER_STATISTICS stat = {0};
		stat.cbSize = sizeof(stat);
		if (SUCCEEDED(control_->GetStatistics(&stat)))
		{
			bitrate = stat.dwBandwidth / 1000;
			return true;
		}
	}
	return false;
}

bool WMASFSource::getPackets(DWORD& packets)
{
	if (control_)
	{
		WM_READER_STATISTICS stat = {0};
		stat.cbSize = sizeof(stat);
		if (SUCCEEDED(control_->GetStatistics(&stat)))
		{
			packets = stat.cPacketsReceived;

			//	CString str;
			//	str.Format(L"WM_READER_STATISTICS (Packets:%u)", stat.cPacketsReceived);
			//	gl_.detailLog(std::wstring(str),
			//		L"cbSize:%u\r\n"
			//		L"dwBandwidth:%u\r\n"
			//		L"cPacketsReceived:%u\r\n"
			//		L"cPacketsRecovered:%u\r\n"
			//		L"cPacketsLost:%u\r\n"
			//		L"wQuality:%u\r\n"
			//		, stat.cbSize
			//		, stat.dwBandwidth
			//		, stat.cPacketsReceived
			//		, stat.cPacketsRecovered
			//		, stat.cPacketsLost
			//		, stat.wQuality
			//	);

			return true;
		}
	}
	return false;
}

bool WMASFSource::abort()
{
	//DebugText(L"WMASFSource::abort()\n");
	HRESULT hr = S_OK;
	if (!control_) { return false; }
	hr = control_->StopBuffering();

	//DebugText(L"WMASFSource::abort() StopBuffering hr = %08X\n", hr);

	return SUCCEEDED(hr);
}

bool WMASFSource::preroll()
{
	if (control_)
	{
		bool res = SUCCEEDED(control_->Preroll(0, 0, 1.0));
		//DebugText(L"Preroll:%s\n", res ? L"true" : L"false");
		return res;
	}
	return false;
	
	/*
	if (advanced_)
	{
		HRESULT hr = advanced_->NotifyLateDelivery(10*10000000);
		DebugText(L"NotifyLateDelivery:%08X\n", hr);
		return true;
	}
	return false;
	*/

	//if (network_)
	//{
	//	HRESULT hr = S_OK;
	//	{
	//		DWORD v = 0;
	//		hr = network_->GetConnectionBandwidth(&v);
	//		DebugText(L"GetAcceleratedStreamingDuration:%d 0x%08X\n", v, hr);
	//	}

	//	{
	//		BOOL v = 0;
	//		hr = network_->GetEnableHTTP (&v);
	//		DebugText(L"GetEnableHTTP:%d 0x%08X\n", v, hr);
	//	}

	//	{
	//		BOOL v = 0;
	//		hr = network_->GetEnableMulticast (&v);
	//		DebugText(L"GetEnableMulticast:%d 0x%08X\n", v, hr);
	//	}

	//	{
	//		BOOL v = 0;
	//		hr = network_->GetEnableTCP (&v);
	//		DebugText(L"GetEnableTCP:%d 0x%08X\n", v, hr);
	//	}

	//	{
	//		BOOL v = 0;
	//		hr = network_->GetEnableUDP (&v);
	//		DebugText(L"GetEnableUDP:%d 0x%08X\n", v, hr);
	//	}

	//	{
	//		DWORD n = 0;
	//		hr = network_->GetNumProtocolsSupported (&n);
	//		DebugText(L"GetNumProtocolsSupported:%d 0x%08X\n", n, hr);
	//		if (SUCCEEDED(hr))
	//		{
	//			for (int i = 0; i < (int)n; ++i)
	//			{
	//				DWORD size = 0;
	//				hr = network_->GetSupportedProtocolName(i, NULL, &size);
	//				DebugText(L"GetSupportedProtocolName:size:%d 0x%08X\n", size, hr);

	//				std::vector<WCHAR> x;
	//				x.resize(size, 0);
	//				network_->GetSupportedProtocolName(i, x.data(), &size);
	//				DebugText(L"GetSupportedProtocolName:\"%s\" 0x%08X\n", x.data(), hr);
	//			}
	//		}
	//	}
	//}
	//return false;
}

}