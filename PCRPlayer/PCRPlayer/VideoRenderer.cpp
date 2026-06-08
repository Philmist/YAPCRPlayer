#include "stdafx.h"

namespace dsp {

//******************************************************************************
bool IBaseVideoRenderer::getFramerate(int& rate)
{
	if (render_)
	{
		CComQIPtr<IBasicVideo, &IID_IBasicVideo> video(render_);
		if (video)
		{
			REFTIME rt = 0;
			video->get_AvgTimePerFrame(&rt);
			//DebugText(L"getFramerate:%lld\n", rt);
			return true;
		}
	}
	return false;
}

bool IBaseVideoRenderer::getCurrentFramerate(int& rate)
{
	if (render_)
	{
		if (prop_)
		{
			return SUCCEEDED(prop_->get_AvgFrameRate(&rate));
		}
	}
	return false;
}

bool IBaseVideoRenderer::getFramesDrawn(int& frames)
{
	if (render_)
	{
		if (prop_)
		{
			return SUCCEEDED(prop_->get_FramesDrawn(&frames));
		}
	}
	return false;
}

bool IBaseVideoRenderer::getFramesDropped(int& frames)
{
	if (render_)
	{
		if (prop_)
		{
			return SUCCEEDED(prop_->get_FramesDroppedInRenderer(&frames));
		}
	}
	return false;
}


bool IBaseVideoRenderer::isRunning(DWORD timeout)
{
	if (render_)
	{
		FILTER_STATE state = State_Stopped;
		if (SUCCEEDED(render_->GetState(timeout, &state)))
		{
			if (state == State_Running)
			{
				return true;
			}
		}
	}
	return false;
}



bool VMR7::addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd)
{
	if (graph == NULL || hwnd == NULL) { return false; }

	hwnd_ = hwnd;

	HRESULT hr;

	render_.CoCreateInstance(CLSID_VideoMixingRenderer, 0, CLSCTX_INPROC_SERVER);
	if (!render_) { return false; }

	// ウィンドレスモード設定
	CComQIPtr<IVMRFilterConfig> fc(render_);
	if (!fc) { return false; }

	hr = fc->SetRenderingMode(VMRMode_Windowless);
	if (FAILED(hr)) { return false; }

	render_->QueryInterface<IVMRWindowlessControl>(&control_);
	if (!control_) { return false; }

	hr = control_->SetVideoClippingWindow(hwnd);
	if (FAILED(hr)) { return false; }

	hr = control_->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
	if (FAILED(hr)) { return false; }

	render_->QueryInterface(IID_IQualProp, (void**)&prop_);
	if (!prop_) { return false; }

	// フィルタをグラフへ追加
	hr = graph->AddFilter(render_, L"Video Mixing Renderer");
	if (FAILED(hr)) { return false; }

	return true;
}

bool VMR7::repaint(HDC hdc)
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->RepaintVideo(hwnd_, hdc));
}

bool VMR7::setVideoPosition(RECT& dst)
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->SetVideoPosition(NULL, &dst));
}

bool VMR7::displayModeChanged()
{
    if (!control_) { return false; }
	return SUCCEEDED(control_->DisplayModeChanged());
}

bool VMR7::getNativeVideoSize(SIZE& size)
{
    if (!control_) { return false; }
	long ax = 0, ay = 0;
	return SUCCEEDED(control_->GetNativeVideoSize(&size.cx, &size.cy, &ax, &ay));
}


//******************************************************************************


bool EVR::addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd)
{
	if (graph == NULL || hwnd == NULL) { return false; }

	hwnd_ = hwnd;

	render_.CoCreateInstance(CLSID_EnhancedVideoRenderer, 0, CLSCTX_INPROC_SERVER);
	if (!render_) { return false; }

	getService(render_, MR_VIDEO_RENDER_SERVICE, control_);
	if (!control_) { return false; }

	//control_->SetAspectRatioMode(MFVideoARMode_None);

	control_->SetVideoWindow(hwnd);

	render_->QueryInterface(IID_IQualProp, (void**)&prop_);
	if (!prop_) { return false; }

	HRESULT hr = hr = graph->AddFilter(render_, L"Enhanced Video Renderer");
	if (FAILED(hr)) { return false; }

	return true;
}

bool EVR::repaint(HDC /*hdc*/)
{
	if (!control_) { return false; }

	return SUCCEEDED(control_->RepaintVideo());
}

bool EVR::setVideoPosition(RECT& dst)
{
	if (!control_) { return false; }
	MFVideoNormalizedRect nrcSrc;
	nrcSrc.left = 0.0;
	nrcSrc.right = 1.0;
	nrcSrc.top = 0.0;
	nrcSrc.bottom = 1.0;
	return SUCCEEDED(control_->SetVideoPosition(&nrcSrc, &dst));
}

bool EVR::displayModeChanged()
{
	return true;
}


bool EVR::getNativeVideoSize(SIZE& size)
{
    if (!control_) { return false; }
	SIZE asize;
	return SUCCEEDED(control_->GetNativeVideoSize(&size, &asize));
}


template<typename T>
bool EVR::getService(const CComPtr<IBaseFilter>& evr, REFGUID guidService, CComPtr<T>& dst)
{
	if (!evr) { return false; }

	CComPtr<IMFGetService> service;
	evr->QueryInterface<IMFGetService>(&service);

	if (!service) { return false; }


	service->GetService(
		guidService,
		__uuidof(T),
		(void**)&dst);

	return true;
}

//******************************************************************************


bool EVRCustom::addToGraph(CComPtr<IGraphBuilder>& graph, HWND hwnd)
{
	if (graph == NULL || hwnd == NULL) { return false; }

	hwnd_ = hwnd;

	render_.CoCreateInstance(CLSID_EnhancedVideoRenderer, 0, CLSCTX_INPROC_SERVER);
	if (!render_) { return false; }

	CComPtr<IMFVideoPresenter> vp;
	EVRCustomPresenter::CreateInstance(&vp);
	if (!vp) { return false; }

	//------------------------------------------------------

	CComQIPtr<IMFVideoRenderer, &IID_IMFVideoRenderer> vr(render_);
	if (!vr) { return false; }

	HRESULT hr = vr->InitializeRenderer(NULL, vp);
	if (FAILED(hr)) { return false; }

	//------------------------------------------------------

	vp->QueryInterface(IID_ICustomRenderProp, (void**)&custom_);

	if (!custom_) { return false; }

	custom_->SetDXVA2(dxva2_ ? TRUE : FALSE);

	//------------------------------------------------------

	getService(render_, MR_VIDEO_RENDER_SERVICE, control_);
	if (!control_) { return false; }

	hr = control_->SetAspectRatioMode(MFVideoARMode_None);
	if (FAILED(hr)) { return false; }

	hr = control_->SetVideoWindow(hwnd);
	if (FAILED(hr)) { return false; }

	//------------------------------------------------------

	getService(render_, MR_VIDEO_MIXER_SERVICE, proc_);
	if (!proc_) { return false; }

	//------------------------------------------------------

	vp->QueryInterface(IID_IQualProp, (void**)&prop_);
	if (!prop_) { return false; }

	//------------------------------------------------------

	hr = graph->AddFilter(render_, L"Enhanced Video Renderer Custom");
	if (FAILED(hr)) { return false; }

	//custom_->SetResizer(d3dutl::RESIZER_SHADER_2PASS_LANCZOS3);
	return true;
}




bool EVRCustom::repaint(HDC hdc)
{
	if (!control_) { return false; }

	return SUCCEEDED(control_->RepaintVideo());
}

bool EVRCustom::setVideoPosition(RECT& dst)
{
	if (!control_) { return false; }

	//if (dst.bottom <= dst.top)
	//{
	//	dst.bottom = dst.top + 1;
	//}

	//if (dst.right <= dst.left)
	//{
	//	dst.right = dst.left + 1;
	//}

	MFVideoNormalizedRect nrcSrc = {0.0, 0.0, 1.0, 1.0};
	return SUCCEEDED(control_->SetVideoPosition(&nrcSrc, &dst));
}

bool EVRCustom::displayModeChanged()
{
	return true;
}

bool EVRCustom::getNativeVideoSize(SIZE& size)
{
    if (!control_) { return false; }
	SIZE asize = {};
	bool res =  SUCCEEDED(control_->GetNativeVideoSize(&size, &asize));
	return res;
}

bool EVRCustom::saveCurrentImage(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit)
{
	if (control_)
	{
		BITMAPINFO info = {0};
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		LPBYTE dib = NULL;
		control_->GetCurrentImage(&info.bmiHeader, &dib, NULL, NULL);

		if (dib)
		{
			utl::Snapshot snap(&info, dib);
			snap.save(path, format, jpegQuality, width, height, fit);

			CoTaskMemFree(dib);
			return true;
			//HDC dc = GetDC(hwnd_);

			//HBITMAP hbm = CreateDIBitmap(dc,
			//	&info.bmiHeader,
			//	CBM_INIT,
			//	dib,
			//	&info,
			//	DIB_RGB_COLORS);

			//CImage img;
			//img.Attach(hbm);
			//img.Save(path.c_str());
			//img.Detach();

			//DeleteObject(hbm);
			//ReleaseDC(hwnd_, dc);
			//CoTaskMemFree(dib);
			//return true;
		}
	}
	return false;
}

bool EVRCustom::getSampleTime(LONGLONG& time)
{
	if (custom_)
	{
		return SUCCEEDED(custom_->GetSampleTime(&time));
	}
	return false;
}

bool EVRCustom::getProcAmpRange(ProcAmpRangeGroup& group)
{
	if (getProcAmpRange(DXVA2_ProcAmp_Brightness, group.brightness) &&
		getProcAmpRange(DXVA2_ProcAmp_Contrast, group.contrast) &&
		getProcAmpRange(DXVA2_ProcAmp_Hue, group.hue) &&
		getProcAmpRange(DXVA2_ProcAmp_Saturation, group.saturation))
	{
		return true;
	}
	return false;
}

bool EVRCustom::setProcAmp(const ProcAmpValue& value)
{
	if (!value.init) { return false; }
	if (proc_)
	{
		DXVA2_ProcAmpValues v = {};
		v.Brightness =	DXVA2FloatToFixed((float)value.brightness / 100.0f);
		v.Contrast =	DXVA2FloatToFixed((float)value.contrast / 100.0f);
		v.Hue =			DXVA2FloatToFixed((float)value.hue / 100.0f);
		v.Saturation =	DXVA2FloatToFixed((float)value.saturation / 100.0f);

		return SUCCEEDED(proc_->SetProcAmpValues(
			DXVA2_ProcAmp_Brightness|
			DXVA2_ProcAmp_Contrast|
			DXVA2_ProcAmp_Hue|
			DXVA2_ProcAmp_Saturation,
			&v));
	}
	return false;
}

bool EVRCustom::getProcAmp(ProcAmpValue& value)
{
	if (proc_)
	{
		DXVA2_ProcAmpValues v = {};

		if (SUCCEEDED(proc_->GetProcAmpValues(
			DXVA2_ProcAmp_Brightness|
			DXVA2_ProcAmp_Contrast|
			DXVA2_ProcAmp_Hue|
			DXVA2_ProcAmp_Saturation,
			&v)))
		{
			value.brightness =	(int)ceil(DXVA2FixedToFloat(v.Brightness) * 100.0f);
			value.contrast =	(int)ceil(DXVA2FixedToFloat(v.Contrast) * 100.0f);
			value.hue =			(int)ceil(DXVA2FixedToFloat(v.Hue) * 100.0f);
			value.saturation =	(int)ceil(DXVA2FixedToFloat(v.Saturation) * 100.0f);
			value.init = true;
			return true;
		}
	}
	return false;
}

bool EVRCustom::setNominalRange(int value)
{
	if (custom_)
	{
		int v = MFNominalRange_Unknown;
		switch(value)
		{
		case NOMINAL_RANGE_0_255: v = MFNominalRange_0_255; break;
		case NOMINAL_RANGE_16_235: v = MFNominalRange_16_235; break;
		case NOMINAL_RANGE_48_208: v = MFNominalRange_48_208; break;
		case NOMINAL_RANGE_64_127: v = MFNominalRange_64_127; break;

		default: v = MFNominalRange_Unknown; break;
		}

		return SUCCEEDED(custom_->SetNominalRange(v));
	}
	return false;
}

bool EVRCustom::setResizer(int type)
{
	if (custom_)
	{
		return SUCCEEDED(custom_->SetResizer(type));
	}
	return false;
}


//-----------------------------------------------------------------------------
bool EVRCustom::getProcAmpRange(DWORD prop, ProcAmpRange& range)
{
	if (proc_)
	{
		DXVA2_ValueRange vr = {};
		if (SUCCEEDED(proc_->GetProcAmpRange(prop, &vr)))
		{
			range.min =		(int)ceil(DXVA2FixedToFloat(vr.MinValue) * 100.0f);
			range.max =		(int)ceil(DXVA2FixedToFloat(vr.MaxValue) * 100.0f);
			range.def =		(int)ceil(DXVA2FixedToFloat(vr.DefaultValue) * 100.0f);
			range.step =	(int)ceil(DXVA2FixedToFloat(vr.StepSize) * 100.0f);
			return true;
		}
	}
	return false;
}


template<typename T>
bool EVRCustom::getService(const CComPtr<IBaseFilter>& evr, REFGUID guidService, CComPtr<T>& dst)
{
	if (!evr) { return false; }

	CComPtr<IMFGetService> service;
	evr->QueryInterface<IMFGetService>(&service);

	if (!service) { return false; }


	service->GetService(
		guidService,
		__uuidof(T),
		(void**)&dst);

	return true;
}


}