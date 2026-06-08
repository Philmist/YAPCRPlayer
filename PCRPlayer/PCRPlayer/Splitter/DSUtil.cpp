#include "../stdafx.h"

IBaseFilter* GetFilterFromPin(IPin* pPin)
{
	if(!pPin) return NULL;
	IBaseFilter* pBF = NULL;
	CPinInfo pi;
	if(pPin && SUCCEEDED(pPin->QueryPinInfo(&pi)))
		pBF = pi.pFilter;
	return(pBF);
}

IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
	if(!pBF) return(NULL);

	BeginEnumPins(pBF, pEP, pPin)
	{
		PIN_DIRECTION dir2;
		pPin->QueryDirection(&dir2);
		if(dir == dir2)
		{
			IPin* pRet = pPin.Detach();
			pRet->Release();
			return(pRet);
		}
	}
	EndEnumPins

	return(NULL);
}

IBaseFilter* GetRendererFilterFromOutputPin(IPin* pin)
{
	if (!pin) { return NULL; }

	PIN_DIRECTION dir = PINDIR_INPUT;
	if (FAILED(pin->QueryDirection(&dir)) || dir != PINDIR_OUTPUT) { return NULL; }

	CComPtr<IBaseFilter> filter;
	CComPtr<IPin> out(pin);
	while (out)
	{
		CComPtr<IPin> in;
		if (FAILED(out->ConnectedTo(&in)) || !in) { break; }

		out.Release();
		filter.Release();
		filter = GetFilterFromPin(in);
		if (!filter) { break; }

		out = GetFirstPin(filter, PINDIR_OUTPUT);
	}
	
	return filter;
}

bool IsFilterRunning(IBaseFilter* filter, DWORD timeout)
{
	if (!filter) { return false; }

	FILTER_STATE state = State_Stopped;
	if (SUCCEEDED(filter->GetState(timeout, &state)))
	{
		if (state == State_Running)
		{
			return true;
		}
	}
	return false;
}


CLSID GetCLSID(IBaseFilter* pBF)
{
	CLSID clsid = GUID_NULL;
	if(pBF) pBF->GetClassID(&clsid);
	return(clsid);
}

CLSID GetCLSID(IPin* pPin)
{
	return(GetCLSID(GetFilterFromPin(pPin)));
}

GUID GUIDFromCString(CString str)
{
	GUID guid = GUID_NULL;
	HRESULT hr = CLSIDFromString(CComBSTR(str), &guid);
	ASSERT(SUCCEEDED(hr));
	return guid;
}

HRESULT GUIDFromCString(CString str, GUID& guid)
{
	guid = GUID_NULL;
	return CLSIDFromString(CComBSTR(str), &guid);
}


//
// Usage: SetThreadName (-1, "MainThread");
//
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

int MakeAACInitData(BYTE* pData, int profile, int freq, int channels)
{
	int srate_idx;

	if(92017 <= freq) srate_idx = 0;
	else if(75132 <= freq) srate_idx = 1;
	else if(55426 <= freq) srate_idx = 2;
	else if(46009 <= freq) srate_idx = 3;
	else if(37566 <= freq) srate_idx = 4;
	else if(27713 <= freq) srate_idx = 5;
	else if(23004 <= freq) srate_idx = 6;
	else if(18783 <= freq) srate_idx = 7;
	else if(13856 <= freq) srate_idx = 8;
	else if(11502 <= freq) srate_idx = 9;
	else if(9391 <= freq) srate_idx = 10;
	else srate_idx = 11;

	pData[0] = ((abs(profile) + 1) << 3) | ((srate_idx & 0xe) >> 1);
	pData[1] = ((srate_idx & 0x1) << 7) | (channels << 3);

	int ret = 2;

	if(profile < 0)
	{
		freq *= 2;

		if(92017 <= freq) srate_idx = 0;
		else if(75132 <= freq) srate_idx = 1;
		else if(55426 <= freq) srate_idx = 2;
		else if(46009 <= freq) srate_idx = 3;
		else if(37566 <= freq) srate_idx = 4;
		else if(27713 <= freq) srate_idx = 5;
		else if(23004 <= freq) srate_idx = 6;
		else if(18783 <= freq) srate_idx = 7;
		else if(13856 <= freq) srate_idx = 8;
		else if(11502 <= freq) srate_idx = 9;
		else if(9391 <= freq) srate_idx = 10;
		else srate_idx = 11;

		pData[2] = 0x2B7>>3;
		pData[3] = (BYTE)((0x2B7<<5) | 5);
		pData[4] = (1<<7) | (srate_idx<<3);

		ret = 5;
	}

	return(ret);
}
