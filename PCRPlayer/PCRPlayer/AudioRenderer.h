#pragma once

#pragma comment(lib, "strmiids.lib")

#include <atlbase.h>
#include <dshow.h>

#include <string>

namespace dsp {


class IBaseAudioRenderer {
protected:
	CComPtr<IBaseFilter> render_;
	CComPtr<IBasicAudio> control_;
	CComPtr<IPin> pin_;

public:
	virtual ~IBaseAudioRenderer() {}

	virtual operator IBaseFilter*() = 0;
	virtual operator CComPtr<IBaseFilter>&() = 0;
	virtual void detach() = 0;
	virtual bool addToGraph(CComPtr<IGraphBuilder>& graph) = 0;

	virtual bool getBalance(int& balance);
	virtual bool getVolume(int& volume);
	virtual bool setBalance(int balance);
	virtual bool setVolume(int volume);
	virtual bool isRunning(DWORD timeout);

protected:
	bool findPin(const CComPtr<IBaseFilter>& filter, CComPtr<IPin>& pin, PIN_DIRECTION dir);
};


class DSR : public IBaseAudioRenderer {
public:
	operator IBaseFilter*() { return render_; }
	operator CComPtr<IBaseFilter>&() { return render_; }
	void detach() { render_.Detach(); control_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph);
};





}

