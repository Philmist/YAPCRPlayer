#pragma once

#pragma comment(lib, "strmiids.lib")

#include <atlbase.h>
#include <dshow.h>

#include <d3d9.h>
#include <vmr9.h>
#include <evr.h>
#include <evr9.h>

#include <initguid.h>
#include <qnetwork.h>
#include <qedit.h>

#include <streams.h>

#include <Mfidl.h>

#include <Dvdmedia.h>

//#include <Wmsdk.h>
#include <Wmsdkidl.h>

#include <string>

#include "EVRCustom/EVRPresenter.h"

namespace dsp {

struct ProcAmpRange {
	int min;
	int max;
	int def;
	int step;
};

struct ProcAmpRangeGroup {
	ProcAmpRange brightness;
	ProcAmpRange contrast;
	ProcAmpRange hue;
	ProcAmpRange saturation;
};

struct ProcAmpValue {
	ProcAmpValue()
		: init(false)
		, brightness(0)
		, contrast(0)
		, hue(0)
		, saturation(0)
	{}

	// 初期化フラグ
	// デフォルト値は取得しないとわからないので
	// 初期化されているかどうかフラグが必要
	bool init;

	int brightness;
	int contrast;
	int hue;
	int saturation;
};

enum NOMINAL_RANGE {
	NOMINAL_RANGE_NORMAL = 0,
	NOMINAL_RANGE_WIDE = 1,

	NOMINAL_RANGE_0_255 = 0,
	NOMINAL_RANGE_16_235 = 1,
	NOMINAL_RANGE_48_208 = 2,
	NOMINAL_RANGE_64_127 = 3,
	NOMINAL_RANGE_COUNT = 4,
};

class IBaseVideoRenderer {
protected:
	HWND hwnd_;
	CComPtr<IBaseFilter> render_;
	CComPtr<IQualProp> prop_;

public:
	IBaseVideoRenderer()
		: hwnd_(NULL)
	{}
	virtual ~IBaseVideoRenderer() {}

	HWND getHWND() { return hwnd_; };

	virtual operator IBaseFilter*() = 0;
	virtual operator CComPtr<IBaseFilter>&() = 0;
	virtual void detach() = 0;

	virtual bool addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd) = 0;
	virtual bool repaint(HDC hdc) = 0;
	virtual bool setVideoPosition(RECT& dst) = 0;
	virtual bool displayModeChanged() = 0;
	virtual bool getNativeVideoSize(SIZE& size) = 0;
	virtual bool saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit) { return false; }
	virtual bool getFramerate(int& rate);
	virtual bool getCurrentFramerate(int& rate);
	virtual bool getFramesDrawn(int& frames);
	virtual bool getFramesDropped(int& frames);
	virtual bool getSampleTime(LONGLONG& time) { return false; }
	virtual bool getProcAmpRange(ProcAmpRangeGroup& group) { return false; }
	virtual bool setProcAmp(const ProcAmpValue& value) { return false; }
	virtual bool getProcAmp(ProcAmpValue& value) { return false; }
	virtual bool setNominalRange(int value) { return false; }
	virtual bool setResizer(int type) { return false; }
	virtual bool isRunning(DWORD timeout);
};

class VMR7 : public IBaseVideoRenderer {
	CComPtr<IVMRWindowlessControl> control_;

public:
	operator IBaseFilter*() { return render_; }
	operator CComPtr<IBaseFilter>&() { return render_; }
	void detach() { render_.Detach(); control_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd);
	bool repaint(HDC hdc);
	bool setVideoPosition(RECT& dst);
	bool displayModeChanged();
	bool getNativeVideoSize(SIZE& size);
};


class EVR : public IBaseVideoRenderer {
	CComPtr<IMFVideoDisplayControl> control_;
	
public:
	EVR() {}
	~EVR() {}

	operator IBaseFilter*() { return render_; }
	operator CComPtr<IBaseFilter>&() { return render_; }
	void detach() { render_.Detach(); control_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd);
	bool repaint(HDC hdc);
	bool setVideoPosition(RECT& dst);
	bool displayModeChanged();
	bool getNativeVideoSize(SIZE& size);

private:
	template<typename T>
	bool getService(const CComPtr<IBaseFilter>& evr, REFGUID guidService, CComPtr<T>& dst);
};


class EVRCustom : public IBaseVideoRenderer {
	CComPtr<IMFVideoDisplayControl> control_;
	CComPtr<ICustomRenderProp> custom_;
	CComPtr<IMFVideoProcessor> proc_;
	
	bool dxva2_;

public:
	EVRCustom(bool dxva2) : dxva2_(dxva2) {}
	~EVRCustom() {}

	operator IBaseFilter*() { return render_; }
	operator CComPtr<IBaseFilter>&() { return render_; }
	void detach() { render_.Detach(); control_.Detach(); custom_.Detach(); proc_.Detach(); }
	bool addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd);
	bool repaint(HDC hdc);
	bool setVideoPosition(RECT& dst);
	bool displayModeChanged();
	bool getNativeVideoSize(SIZE& size);
	bool saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit);
	bool getSampleTime(LONGLONG& time);
	bool getProcAmpRange(ProcAmpRangeGroup& group);
	bool setProcAmp(const ProcAmpValue& value);
	bool getProcAmp(ProcAmpValue& value);
	bool setNominalRange(int value);
	bool setResizer(int type);

private:
	bool getProcAmpRange(DWORD prop, ProcAmpRange& range);

	template<typename T>
	bool getService(const CComPtr<IBaseFilter>& evr, REFGUID guidService, CComPtr<T>& dst);
};



}
