#include "stdafx.h"

namespace dsp {


bool IBaseAudioRenderer::getBalance(int& balance)
{
	if (!control_) { return false; }
	
	long bal = 0;
	if (SUCCEEDED(control_->get_Balance(&bal)))
	{
		balance = (int)bal;
		return true;
	}
	return false;
}

bool IBaseAudioRenderer::getVolume(int& volume)
{
	if (!control_) { return false; }

	long vol = 0;
	if (SUCCEEDED(control_->get_Volume(&vol)))
	{
		volume = (int)(pow(10, (((double)vol / 100.0) / 20.0)) * 100.0);
		return true;
	}
	return false;
}

// 値は -10,000 ～ 10,000 の範囲で指定できる。
// 値が -10,000 の場合、右チャンネルは 100 dB 減衰され、無音となることを意味している。
bool IBaseAudioRenderer::setBalance(int balance)
{
	if (!control_) { return false; }
	return SUCCEEDED(control_->put_Balance(((long)balance)*100));
}

// ボリュームを -10,000 ～ 0 の数値で指定する。
// 最大ボリュームは 0、無音は -10,000。
bool IBaseAudioRenderer::setVolume(int volume)
{
	if (!control_) { return false; }

	long vol = (volume == 0) ? -10000 : (long)((20.0 * log10((double)volume / 100.0)) * 100.0);

	return SUCCEEDED(control_->put_Volume(vol));
}

bool IBaseAudioRenderer::isRunning(DWORD timeout)
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

bool IBaseAudioRenderer::findPin(const CComPtr<IBaseFilter>& filter, CComPtr<IPin>& pin, PIN_DIRECTION dir)
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


////////////////////////////////////////////////////////////////////////////

bool DSR::addToGraph(CComPtr<IGraphBuilder>& graph)
{
	if (graph == NULL) { return false; }

	render_.CoCreateInstance(CLSID_DSoundRender, 0, CLSCTX_INPROC_SERVER);
	if (!render_) { return false; }

	render_->QueryInterface(IID_IBasicAudio, (void**)&control_);
	if (!control_) { return false; }

	HRESULT hr = graph->AddFilter(render_, L"DirectSound Renderer");
	if (FAILED(hr)) { return false; }

	findPin(render_, pin_, PINDIR_INPUT);

	return true;
}


}
