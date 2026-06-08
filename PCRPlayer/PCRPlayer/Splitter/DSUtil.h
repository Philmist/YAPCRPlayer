#pragma once

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC // include Microsoft memory leak detection procedures

	#if 0
		#include <crtdbg.h>
		#define DNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#else
		#define DNew new(__FILE__, __LINE__)
	#endif

#else

	#define DNew new

#endif

#define countof(array) (sizeof(array)/sizeof(array[0]))

#define QI(i) (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) :

#define BeginEnumPins(pBaseFilter, pEnumPins, pPin) \
	{CComPtr<IEnumPins> pEnumPins; \
	if(pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins))) \
	{ \
		for(CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
		{ \

#define EndEnumPins }}}


class CPinInfo : public PIN_INFO
{
public:
	CPinInfo() {
		pFilter = NULL;
	}
	~CPinInfo() {
		if(pFilter) pFilter->Release();
	}
};

class CFilterInfo : public FILTER_INFO
{
public:
	CFilterInfo() {
		pGraph = NULL;
	}
	~CFilterInfo() {
		if(pGraph) pGraph->Release();
	}
};


IBaseFilter* GetFilterFromPin(IPin* pPin);
IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir = PINDIR_INPUT);
IBaseFilter* GetRendererFilterFromOutputPin(IPin* pin);
bool IsFilterRunning(IBaseFilter* filter, DWORD timeout = 0);

CLSID GetCLSID(IBaseFilter* pBF);
CLSID GetCLSID(IPin* pPin);
GUID GUIDFromCString(CString str);
HRESULT GUIDFromCString(CString str, GUID& guid);
void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);

int MakeAACInitData(BYTE* pData, int profile, int freq, int channels);