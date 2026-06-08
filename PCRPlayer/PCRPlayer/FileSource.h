#pragma once

#pragma comment(lib, "strmiids.lib")


#include <atlbase.h>
#include <dshow.h>

//IWMReader 
//IWMReaderAdvanced2
#include <Wmsdkidl.h>

#include <string>


namespace dsp {

class IBaseSource {
public:
	IBaseSource(){}
	virtual ~IBaseSource() {}

	virtual operator IBaseFilter*() = 0;
	virtual void detach() = 0;
	virtual bool addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename) = 0;
	virtual bool getProgress(int& percent) = 0;
	virtual bool getCurrentBitrate(int& bitrate) = 0;
	virtual bool getPackets(DWORD& packets) { return false; }
	virtual bool abort() { return false; }
	virtual bool preroll() { return false; }
	virtual bool isTimeout() { return false; }
	virtual SOURCE_TYPE type() { return SOURCE_TYPE_UNKNOWN; }
};

class DefaultSource : public IBaseSource {
protected:
	CComPtr<IBaseFilter> source_;
	CComPtr<IAMOpenProgress> control_;

public:
	DefaultSource(){}
	virtual ~DefaultSource(){}

	operator IBaseFilter*() { return source_; }
	void detach() { source_.Detach(); control_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename);
	bool getProgress(int& percent);
	bool getCurrentBitrate(int& bitrate) { return false; }
	bool abort();
};

class AsyncSource : public IBaseSource {
protected:
	CComPtr<IBaseFilter> source_;
	SOURCE_TYPE type_;

public:
	AsyncSource();
	virtual ~AsyncSource(){}

	operator IBaseFilter*() { return source_; }
	void detach() { source_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename);
	bool getProgress(int& percent) { return false; }
	bool getCurrentBitrate(int& bitrate) { return false; }
	bool abort() { return false; }
	SOURCE_TYPE type() { return type_; }
};

class URLFileSource : public IBaseSource {
protected:
	CComPtr<IBaseFilter> source_;
	CComPtr<IAsyncReader> reader_;

	int bitrate_;
	DWORD time_;
	LONGLONG length_;

public:
	URLFileSource();
	virtual ~URLFileSource(){}

	operator IBaseFilter*() { return source_; }
	void detach() { source_.Detach(); reader_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename);
	bool getProgress(int& percent) { return false; }
	bool getCurrentBitrate(int& bitrate);
	bool abort() { return false; }
};

class URLBufferSource : public IBaseSource {
protected:
	CComPtr<MemSourceFilter> source_;
	CComPtr<IAsyncReader> reader_;
	SOURCE_TYPE type_;

	int bitrate_;
	DWORD time_;
	LONGLONG length_;
	bool isTimeout_;

	std::wstring userAgent_;
	int timeout_;

public:
	URLBufferSource(const std::wstring& userAgent, int timeout);
	virtual ~URLBufferSource(){}

	operator IBaseFilter*() { return source_; }
	void detach() { source_.Detach(); reader_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename);
	bool getProgress(int& percent) { return false; }
	bool getCurrentBitrate(int& bitrate);
	bool abort();
	bool isTimeout() { return isTimeout_; }
	SOURCE_TYPE type(){ return type_; }
};

class WMASFSource : public IBaseSource {
protected:
	CComPtr<IBaseFilter> source_;
	CComPtr<IWMReaderAdvanced2> control_;

public:
	WMASFSource() {}
	virtual ~WMASFSource(){}

	operator IBaseFilter*() { return source_; }
	void detach() { source_.Detach(); control_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, const std::wstring& filename);
	bool getProgress(int& percent);
	bool getCurrentBitrate(int& bitrate);
	bool getPackets(DWORD& packets);
	bool abort();
	bool preroll();
	SOURCE_TYPE type() { return SOURCE_TYPE_ASF; }
};

};