#include "stdafx.h"




namespace dsp
{

namespace
{


WCHAR const MemSourceFilter_OutputPinName[] = L"Output";


HRESULT WINAPI MyCopyMediaType(
	AM_MEDIA_TYPE       *pmtTarget,
	const AM_MEDIA_TYPE *pmtSource
){
	CopyMemory(pmtTarget, pmtSource, sizeof(*pmtTarget));
	if (pmtTarget->cbFormat != 0){
		pmtTarget->pbFormat = (BYTE*)CoTaskMemAlloc(pmtTarget->cbFormat);
		if(! pmtTarget->pbFormat){
			ZeroMemory(pmtTarget, sizeof(*pmtTarget));
			return E_OUTOFMEMORY;
		}

		CopyMemory(pmtTarget->pbFormat, pmtSource->pbFormat, pmtTarget->cbFormat);
	}
	if(pmtTarget->pUnk){
		pmtTarget->pUnk->AddRef();
	}

	return S_OK;
}

void MyFreeMediaType(AM_MEDIA_TYPE& mt)
{
	if(mt.cbFormat != 0){
		CoTaskMemFree(mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = nullptr;
	}
	if(mt.pUnk){
		mt.pUnk->Release();
		mt.pUnk = nullptr;
	}
}

AM_MEDIA_TYPE* WINAPI MyCreateMediaType(
	AM_MEDIA_TYPE const *pSrc
){
	if(! pSrc) return nullptr;

	auto pDest = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(*pSrc));
	if(! pDest) return nullptr;

	if(FAILED(MyCopyMediaType(pDest, pSrc))){
		CoTaskMemFree(pDest);
		return nullptr;
	}

	return pDest;		
}

void MyDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if(pmt){
		MyFreeMediaType(*pmt); 
		CoTaskMemFree(pmt);
	}
}


class MemEnumPins
	: public IEnumPins
{
public:
	friend class MemSourceFilter;

private:
	MemEnumPins(MemSourceFilter* pFilter, ULONG index = 0)
		: pFilter_(pFilter), refCount_(1), index_(index)
	{
		pFilter_->AddRef();
	}

	virtual ~MemEnumPins()
	{
		pFilter_->Release();
	}

	MemSourceFilter* pFilter_;

//
// IUnknown
//
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

private:
	ULONG            refCount_;

//
// IEnumPins
//
public:
	virtual HRESULT STDMETHODCALLTYPE Next( 
		/* [in] */ ULONG cPins,
		/* [annotation][size_is][out] */ 
		__out_ecount_part(cPins, *pcFetched)  IPin **ppPins,
		/* [annotation][out] */ 
		__out_opt  ULONG *pcFetched);
        
	virtual HRESULT STDMETHODCALLTYPE Skip( 
		/* [in] */ ULONG cPins);
        
	virtual HRESULT STDMETHODCALLTYPE Reset( void);
        
	virtual HRESULT STDMETHODCALLTYPE Clone( 
		/* [annotation][out] */ 
		__out  IEnumPins **ppEnum);

private:
	ULONG            index_;
};


class MemEnumMediaTypes
	: public IEnumMediaTypes
{
public:
	friend class MemOutputPin;

private:
	MemEnumMediaTypes(MemOutputPin* pPin, ULONG index = 0)
		: pPin_(pPin), refCount_(1), index_(index)
	{
		pPin_->AddRef();
	}

	virtual ~MemEnumMediaTypes()
	{
		pPin_->Release();
	}

	MemOutputPin* pPin_;

//
// IUnknown
//
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

private:
	ULONG            refCount_;

//
// IEnumMediaTypes
//
public:
	virtual HRESULT STDMETHODCALLTYPE Next( 
		/* [in] */ ULONG cMediaTypes,
		/* [annotation][size_is][out] */ 
		AM_MEDIA_TYPE **ppMediaTypes,
		/* [annotation][out] */ 
		_Out_opt_  ULONG *pcFetched);
        
	virtual HRESULT STDMETHODCALLTYPE Skip( 
		/* [in] */ ULONG cMediaTypes);
        
	virtual HRESULT STDMETHODCALLTYPE Reset( void);
        
	virtual HRESULT STDMETHODCALLTYPE Clone( 
		/* [annotation][out] */ 
		_Out_  IEnumMediaTypes **ppEnum);

private:
	ULONG            index_;
};


bool FindMediaTypeInRegistry(
	BYTE const* data, std::size_t size, AM_MEDIA_TYPE* output
);

} // namespace anonymous


/////
///// ここから実装部
/////

//
// MemOutputPin の実装
//

//
// IUnknown
//
HRESULT STDMETHODCALLTYPE MemOutputPin::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(! ppvObject){
		return E_POINTER;
	}

#define ADDBRANCH(interf__) \
	if(riid == __uuidof(interf__)){ \
		(	*reinterpret_cast<interf__**>(ppvObject) \
				= static_cast<interf__*>(this) \
		)->AddRef(); \
		return S_OK; \
	}

	ADDBRANCH(IPin);
	ADDBRANCH(IUnknown);

#undef ADDBRANCH

#define ADDBRANCH(interf__, getter__, ...) \
	if(riid == __uuidof(interf__)){ \
		*reinterpret_cast<interf__**>(ppvObject) = pFilter_->getter__(); \
		__VA_ARGS__; \
		return S_OK; \
	}

	ADDBRANCH(IAMAsyncReaderTimestampScaling, GetAMAsyncReaderTimestampScaling);
	ADDBRANCH(IAsyncReader, GetAsyncReader, this->bAsyncReaderQueried_ = true);

#undef ADDBRANCH

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE MemOutputPin::AddRef( void)
{
	// このオブジェクトはフィルタのサブオブジェクトとして使うので、参照カウンタが共通
	return pFilter_->AddRef();
}


ULONG STDMETHODCALLTYPE MemOutputPin::Release( void)
{
	return pFilter_->Release();
}


//
// IPin
//

HRESULT STDMETHODCALLTYPE MemOutputPin::Connect( 
	/* [in] */ IPin *pReceivePin,
	/* [annotation][in] */ 
	_In_opt_  const AM_MEDIA_TYPE *pmt)
{
	if(! pReceivePin) return E_POINTER;
	if(pPinTo_)       return VFW_E_ALREADY_CONNECTED;

	if(State_Stopped != pFilter_->GetState()){
		return VFW_E_NOT_STOPPED;
	}

	bool bCallerSpecifiedMajorType
		= (pmt && pmt->majortype != GUID_NULL);

	bool bCallerSpecifiedMinorType
		= (pmt && pmt->subtype != GUID_NULL);

	auto AttemptConnection = [&](AM_MEDIA_TYPE* pNewOne) -> HRESULT
	{
		// ReceiveConnection する前にこの二つを設定する
		this->pPinTo_     = pReceivePin;
		this->pMediaType_ = pNewOne;

		// 相手から IAsyncReader をクエリされたか監視する
		this->bAsyncReaderQueried_ = false;
		auto hr = pReceivePin->ReceiveConnection(this, pNewOne);
		if(SUCCEEDED(hr)){
			if(this->bAsyncReaderQueried_){
				return S_OK;
			}
			else{
				// 相手は接続に成功したと思ってるので、切断しておく
				pReceivePin->Disconnect();
				hr = E_FAIL;
			}
		}

		// 失敗
		MyDeleteMediaType(pNewOne);
		this->pPinTo_ = nullptr;
		this->pMediaType_ = nullptr;

		return hr;
	};

	// pmt が完全型のときは、それで接続を試みる
	if(	bCallerSpecifiedMajorType
	&& bCallerSpecifiedMinorType
	){
		HRESULT hr = AttemptConnection(MyCreateMediaType(pmt));
		if(SUCCEEDED(hr))
		{
			//DebugText(L"AttemptConnection(0)\n");
			pFilter_->CompleteConnect();
		}
		return hr;
	}


	// 下流のフィルタの受け付けるメディアタイプを使ってみる
	IEnumMediaTypes* pEnumMediaTypes = nullptr;
	if(SUCCEEDED(pReceivePin->EnumMediaTypes(&pEnumMediaTypes))){
		AM_MEDIA_TYPE* pReceiversType = nullptr;
		ULONG          cFetched = 0;

		while(S_OK == pEnumMediaTypes->Next(1, &pReceiversType, &cFetched)){
			bool bMatch = false;

			if((! bCallerSpecifiedMajorType || pmt->majortype == pReceiversType->majortype)
			&& (! bCallerSpecifiedMinorType || pmt->subtype == pReceiversType->subtype)
			){
				auto hr = AttemptConnection(pReceiversType);
				if(SUCCEEDED(hr))
				{
					pEnumMediaTypes->Release();
					//DebugText(L"AttemptConnection(1)\n");
					pFilter_->CompleteConnect();
					return S_OK;
				}
			}
			else{
				// AttemptConnection を呼ばない場合は
				// 自分で pReceiversType を殺す必要がある
				MyDeleteMediaType(pReceiversType);	
			}
			pReceiversType = nullptr;
		}

		pEnumMediaTypes->Release();
	}


	// 自分の型をプロポーズしてみる
	for(ULONG i = 0; i < this->GetNumMyMediaType(); ++i){
		auto theType = this->GetMyMediaType(i);

		if((! bCallerSpecifiedMajorType || pmt->majortype == theType->majortype)
		&& (! bCallerSpecifiedMinorType || pmt->subtype == theType->subtype)
		){
			auto hr = AttemptConnection(MyCreateMediaType(theType));
			if(SUCCEEDED(hr))
			{
				//DebugText(L"AttemptConnection(2)\n");
				pFilter_->CompleteConnect();
				return S_OK;
			}
		}
	}


	return VFW_E_NO_ACCEPTABLE_TYPES;
}


HRESULT STDMETHODCALLTYPE MemOutputPin::ReceiveConnection( 
	/* [in] */ IPin *pConnector,
	/* [in] */ const AM_MEDIA_TYPE *pmt)
{
	return E_NOTIMPL;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::Disconnect( void)
{
	if(! pPinTo_) return S_FALSE;

	if(State_Stopped != pFilter_->GetState()){
		return VFW_E_NOT_STOPPED;
	}

	MyDeleteMediaType(this->pMediaType_);

	this->pPinTo_     = nullptr;
	this->pMediaType_ = nullptr;

	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::ConnectedTo( 
	/* [annotation][out] */ 
	_Out_  IPin **pPin)
{
	if(! pPin) return E_POINTER;

	*pPin = this->pPinTo_;
	if(! *pPin) return VFW_E_NOT_CONNECTED; 

	(*pPin)->AddRef();
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::ConnectionMediaType( 
	/* [annotation][out] */ 
	_Out_  AM_MEDIA_TYPE *pmt)
{
	if(! pmt) return E_POINTER;
	if(! pPinTo_) return VFW_E_NOT_CONNECTED; 

	return MyCopyMediaType(pmt, this->pMediaType_);
}
        

HRESULT STDMETHODCALLTYPE MemOutputPin::QueryPinInfo( 
	/* [annotation][out] */ 
	_Out_  PIN_INFO *pInfo)
{
	if(! pInfo) return E_POINTER;

	(pInfo->pFilter = this->pFilter_)->AddRef();
	pInfo->dir      = PINDIR_OUTPUT;

	lstrcpynW(pInfo->achName, MemSourceFilter_OutputPinName, NUMELMS(pInfo->achName));
	pInfo->achName[NUMELMS(pInfo->achName) - 1] = L'\0';

	return S_OK;
}
    

HRESULT STDMETHODCALLTYPE MemOutputPin::QueryDirection( 
	/* [annotation][out] */ 
	_Out_  PIN_DIRECTION *pPinDir)
{
	if(! pPinDir) return E_POINTER;

	*pPinDir = PINDIR_OUTPUT;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::QueryId( 
	/* [annotation][out] */ 
	_Out_  LPWSTR *Id)
{
	if(! Id) return E_POINTER;

	*Id = (LPWSTR)CoTaskMemAlloc(sizeof(MemSourceFilter_OutputPinName));
	if(! *Id) return E_OUTOFMEMORY;

	CopyMemory(*Id, MemSourceFilter_OutputPinName, sizeof(MemSourceFilter_OutputPinName));
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::QueryAccept( 
	/* [in] */ const AM_MEDIA_TYPE *pmt)
{
	return E_NOTIMPL;
}
        

HRESULT STDMETHODCALLTYPE MemOutputPin::EnumMediaTypes( 
	/* [annotation][out] */ 
	_Out_  IEnumMediaTypes **ppEnum)
{
	if(! ppEnum) return E_POINTER;

	*ppEnum = new(std::nothrow) MemEnumMediaTypes(this);
	if(! *ppEnum){
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE MemOutputPin::QueryInternalConnections( 
	/* [annotation][out] */ 
	IPin **apPin,
	/* [out][in] */ ULONG *nPin)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE MemOutputPin::EndOfStream( void)
{
	return E_UNEXPECTED;
}


HRESULT STDMETHODCALLTYPE MemOutputPin::BeginFlush( void)
{
	return E_UNEXPECTED;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::EndFlush( void)
{
	return E_UNEXPECTED;
}

        
HRESULT STDMETHODCALLTYPE MemOutputPin::NewSegment( 
	/* [in] */ REFERENCE_TIME tStart,
	/* [in] */ REFERENCE_TIME tStop,
	/* [in] */ double dRate)
{
	return E_UNEXPECTED;
}


void MemOutputPin::SetMyMediaType(std::vector<unsigned char> const& mem)
{
	if(FindMediaTypeInRegistry(mem.data(), mem.size(), &myMediaTypes_[0])){
		numMyMediaTypes_ = 2;
		//DebugText(L"TRUE MemOutputPin::SetMyMediaType\n");
	}
	else{
		myMediaTypes_[0] = myMediaTypes_[1];
		numMyMediaTypes_ = 1;
		//DebugText(L"FALSE MemOutputPin::SetMyMediaType\n");
	}
}


//
// MemAMAsyncReaderTimestampScaling の実装
//

//
// IUnknown
//
HRESULT STDMETHODCALLTYPE MemAMAsyncReaderTimestampScaling::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(! ppvObject){
		return E_POINTER;
	}

#define ADDBRANCH(interf__) \
	if(riid == __uuidof(interf__)){ \
		(	*reinterpret_cast<interf__**>(ppvObject) \
				= static_cast<interf__*>(this) \
		)->AddRef(); \
		return S_OK; \
	}

	ADDBRANCH(IAMAsyncReaderTimestampScaling);

#undef ADDBRANCH

#define ADDBRANCH(interf__, getter__) \
	if(riid == __uuidof(interf__)){ \
		*reinterpret_cast<interf__**>(ppvObject) = pFilter_->getter__(); \
		return S_OK; \
	}

	ADDBRANCH(IAsyncReader, GetAsyncReader);
	ADDBRANCH(IPin, GetOutputPin);

#undef ADDBRANCH

	// IUnknown のアドレスは、IPin と共通
	if(riid == __uuidof(IUnknown)){
		*reinterpret_cast<IUnknown**>(ppvObject) = pFilter_->GetOutputPin(); \
		return S_OK; \
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE MemAMAsyncReaderTimestampScaling::AddRef( void)
{
	// このオブジェクトはフィルタのサブオブジェクトとして使うので、参照カウンタが共通
	return pFilter_->AddRef();
}


ULONG STDMETHODCALLTYPE MemAMAsyncReaderTimestampScaling::Release( void)
{
	return pFilter_->Release();
}

//
// IAMAsyncReaderTimestampScaling
//
HRESULT STDMETHODCALLTYPE MemAMAsyncReaderTimestampScaling::GetTimestampMode( 
	/* [annotation] */ 
	_Out_  BOOL *pfRaw)
{
	if(! pfRaw) return E_POINTER;

	*pfRaw = this->fRaw_;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemAMAsyncReaderTimestampScaling::SetTimestampMode( 
	BOOL fRaw)
{
	this->fRaw_ = fRaw;
	return S_OK;
}


//
// MemAsyncReader の実装
//

//
// IUnknown
//
HRESULT STDMETHODCALLTYPE MemAsyncReader::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(! ppvObject){
		return E_POINTER;
	}

#define ADDBRANCH(interf__) \
	if(riid == __uuidof(interf__)){ \
		(	*reinterpret_cast<interf__**>(ppvObject) \
				= static_cast<interf__*>(this) \
		)->AddRef(); \
		return S_OK; \
	}

	ADDBRANCH(IAsyncReader);

#undef ADDBRANCH

#define ADDBRANCH(interf__, getter__) \
	if(riid == __uuidof(interf__)){ \
		*reinterpret_cast<interf__**>(ppvObject) = pFilter_->getter__(); \
		return S_OK; \
	}

	ADDBRANCH(IPin, GetOutputPin);
	ADDBRANCH(IAMAsyncReaderTimestampScaling, GetAMAsyncReaderTimestampScaling);

#undef ADDBRANCH

	// IUnknown のアドレスは、IPin と共通
	if(riid == __uuidof(IUnknown)){
		*reinterpret_cast<IUnknown**>(ppvObject) = pFilter_->GetOutputPin(); \
		return S_OK; \
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE MemAsyncReader::AddRef( void)
{
	// このオブジェクトはフィルタのサブオブジェクトとして使うので、参照カウンタが共通
	return pFilter_->AddRef();
}


ULONG STDMETHODCALLTYPE MemAsyncReader::Release( void)
{
	return pFilter_->Release();
}


//
// IAsyncReader
//
HRESULT STDMETHODCALLTYPE MemAsyncReader::RequestAllocator( 
	/* [in] */ IMemAllocator *pPreferred,
	/* [annotation][in] */ 
	_In_  ALLOCATOR_PROPERTIES *pProps,
	/* [annotation][out] */ 
	_Out_  IMemAllocator **ppActual)
{
	if(! ppActual) return E_POINTER;

	if(pPreferred){
		pPreferred->AddRef();
	}
	else{
		// 下流のフィルタから指定のアロケータがないときは
		// こちらで用意する
		auto hr = ::CoCreateInstance(
			CLSID_MemoryAllocator,
			nullptr,
			CLSCTX_INPROC,
			__uuidof(IMemAllocator),
			(LPVOID *)&pPreferred
		);
		if(FAILED(hr)) return hr;
	}

	ALLOCATOR_PROPERTIES propIn = *pProps;
	if(propIn.cBuffers == 0) propIn.cBuffers = 4;
	if(propIn.cbBuffer == 0) propIn.cbBuffer = 0x1000;
	if(propIn.cbAlign  == 0) propIn.cbAlign  = 1;

	ALLOCATOR_PROPERTIES propOut;
	auto hr = pPreferred->SetProperties(&propIn, &propOut);
	if(FAILED(hr)){
		pPreferred->Release();
		return hr;
	}

	*ppActual = pPreferred;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemAsyncReader::Request( 
	/* [in] */ IMediaSample *pSample,
	/* [in] */ DWORD_PTR dwUser)
{
	if(! pSample) return E_POINTER;

	REFERENCE_TIME start, last;
	if(pSample->GetTime(&start, &last) != S_OK){
		return VFW_E_SAMPLE_TIME_NOT_SET;
	}

	if(! pFilter_->IsTimestampScalingRaw()){
		start /= 10000000;
		// 使わない: last  /= 10000000;
	}

	if(REFERENCE_TIME(this->memory_.size()) <= start){
		return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	}

	// キューの状態を変更している間は、
	// フラッシュ状態が変わってはいけない
	::EnterCriticalSection(&csQueue_);

	if(WAIT_OBJECT_0 == ::WaitForSingleObject(hEvFlushing_, 0)){
		// フラッシュ状態なのでなにもしない
		::LeaveCriticalSection(&csQueue_);
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = S_OK;
	try{
		queue_.emplace(std::pair<IMediaSample*, DWORD_PTR>(pSample, dwUser));
		::ReleaseSemaphore(hSemNumQueue_, 1, nullptr);
	}
	catch(...){
		hr = E_OUTOFMEMORY;
	}

	::LeaveCriticalSection(&csQueue_);
	return hr;
}


HRESULT STDMETHODCALLTYPE MemAsyncReader::WaitForNext( 
	/* [in] */ DWORD dwTimeout,
	/* [annotation][out] */ 
	_Out_opt_  IMediaSample **ppSample,
	/* [annotation][out] */ 
	_Out_  DWORD_PTR *pdwUser)
{
	if(! ppSample || ! pdwUser) return E_POINTER;

	HANDLE handles[] = { hSemNumQueue_, hEvFlushing_ };

	::EnterCriticalSection(&csEndFlush_);
	bool bTimeout = (
		WAIT_TIMEOUT == ::WaitForMultipleObjects(NUMELMS(handles), handles, FALSE, dwTimeout)
	);

	::EnterCriticalSection(&csQueue_);

	if(bTimeout || queue_.empty()){
		*ppSample = nullptr;
		*pdwUser  = 0;
	}
	else{
		auto& front = queue_.front();
		*ppSample = front.first;
		*pdwUser  = front.second;
		queue_.pop();
	}

	bool bFlushing = (
		WAIT_OBJECT_0 == ::WaitForSingleObject(hEvFlushing_, 0)
	);

	HRESULT hr;

	if(bFlushing || bTimeout){
		if(*ppSample){
			// *ppSample の長さをゼロに設定する
			REFERENCE_TIME start, last;
			if((*ppSample)->GetTime(&start, &last) == S_OK){
				(*ppSample)->SetTime(&start, &start);
			}
			(*ppSample)->SetActualDataLength(0);
		}
		hr = VFW_E_TIMEOUT;
	}
	else{
		hr = this->SyncReadAligned(*ppSample);
	}

	::LeaveCriticalSection(&csQueue_);
	::LeaveCriticalSection(&csEndFlush_);
	return hr;
}

        
HRESULT STDMETHODCALLTYPE MemAsyncReader::SyncReadAligned( 
	/* [in] */ IMediaSample *pSample)
{
// MSDN の IAsyncReader::Request の説明には、IMediaSample の GetTime が返すのは
//      first and last **inclusive**
// と明記されている。
//
// しかし実際の挙動としては、last は exclusive なのが慣習のよう（？）
//
#define OBEY_MSDN_VERBATIM 0

#if OBEY_MSDN_VERBATIM
	const int endSubLast = 1;
#else
	const int endSubLast = 0;
#endif

	if(! pSample) return E_POINTER;

	REFERENCE_TIME start, last;
	if(pSample->GetTime(&start, &last) != S_OK){
		return E_FAIL;
	}

	if(! pFilter_->IsTimestampScalingRaw()){
		start /= 10000000;
		last  /= 10000000;
	}

	// end: exclusive <=> last: inclusive
	auto end  = last + endSubLast;
	bool bFew = false;

	if(REFERENCE_TIME(this->memory_.size()) < end){
		end  = this->memory_.size();
		bFew = true;
	}

	if(end <= start){
		return E_FAIL;
	}

	auto copiedSize = end - start;
	if(copiedSize > pSample->GetSize()){
		return E_FAIL;
	}

	BYTE* pDest = nullptr;
	auto hr = pSample->GetPointer(&pDest);
	if(FAILED(hr)) return hr;

	//this->memory_.consume(pDest, std::size_t(copiedSize));
	this->memory_.copy(pDest, start, (LONG)std::size_t(copiedSize));
	//CopyMemory(pDest, this->memory_.data() + start, std::size_t(copiedSize));

	if(bFew){
		last = end - endSubLast;

		if(! pFilter_->IsTimestampScalingRaw()){
			start *= 10000000;
			last  *= 10000000;
		}

		hr = pSample->SetTime(&start, &last);
		if(FAILED(hr)) return hr;
	}

	hr = pSample->SetActualDataLength(long(copiedSize));
	if(FAILED(hr)) return hr;

	pSample->SetSyncPoint(TRUE);

	if(bFew){
		return S_FALSE;
	}
	else{
		return S_OK;
	}
}

        
HRESULT STDMETHODCALLTYPE MemAsyncReader::SyncRead( 
	/* [in] */ LONGLONG llPosition,
	/* [in] */ LONG lLength,
	/* [annotation][size_is][out] */ 
	BYTE *pBuffer)
{
	if(! pBuffer) return E_POINTER;

	if(llPosition < 0 || lLength < 0) return E_FAIL;

	// 2016/07/19
	// 読み込み量が少なくて再生前に停止してしまう不具合
	// すぐ下の条件でバッファ待機する
	//if(REFERENCE_TIME(this->memory_.size()) <= llPosition)
	//{
	//	//DebugText(L"SyncRead this->memory_.size():%lld <= llPosition:%lld\n", this->memory_.size(), llPosition);
	//	return E_FAIL;
	//}

	bool bFew = false;

	if(REFERENCE_TIME(this->memory_.size()) < llPosition + lLength)
	{
		//lLength = LONG(this->memory_.size() - llPosition);
		//bFew = true;

		if (!this->memory_.fill(llPosition + lLength))
		{
			// S_FALSE
			// 取得したバイトの数が要求されたバイトの数より少ない
			// (ファイルの末尾に達したと考えられる)
			// 読み込みをやめる

			//DebugText(L"SyncRead !this->memory_.fill bFew lLength(%d)\n", lLength);

			this->memory_.terminate();
			timeout_ = true;
			return S_FALSE;
			//lLength = min(lLength, LONG(this->memory_.size() - llPosition));
			//bFew = true;
		}

		//
		//DebugText(L"SyncRead pos:%lld len:%d size:%lld %lld\n", llPosition, lLength, this->memory_.size(),
		//		(llPosition + lLength) - this->memory_.size());
	}
	

	this->memory_.copy(pBuffer, llPosition, lLength);
	//this->memory_.consume(pBuffer, lLength);
	//CopyMemory(pBuffer, this->memory_.data() + llPosition, lLength);

	if(bFew){
		return S_FALSE;
	}
	else{
		return S_OK;
	}
}

        
HRESULT STDMETHODCALLTYPE MemAsyncReader::Length( 
	/* [annotation][out] */ 
	_Out_  LONGLONG *pTotal,
	/* [annotation][out] */ 
	_Out_  LONGLONG *pAvailable)
{
	if(! pTotal || ! pAvailable) return E_POINTER;

	//*pTotal     = 0;
	//*pAvailable = this->memory_.size();

	// ストリーミング時は0
	*pTotal     = 0;

	// バッファリングが停止しているときはマイナスを返す
	*pAvailable = this->memory_.terminated() ? -1 : this->memory_.size();// 現在の長さ
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemAsyncReader::BeginFlush( void)
{
	::EnterCriticalSection(&csQueue_);

	HRESULT hr;

	if(WAIT_OBJECT_0 == ::WaitForSingleObject(hEvFlushing_, 0)){
		// すでにフラッシュ状態
		hr = S_FALSE;
	}
	else{
		::SetEvent(hEvFlushing_);
		hr = S_OK;
	}

	::LeaveCriticalSection(&csQueue_);
	return hr;
}

        
HRESULT STDMETHODCALLTYPE MemAsyncReader::EndFlush( void)
{
	::EnterCriticalSection(&csEndFlush_);
	::EnterCriticalSection(&csQueue_);

	HRESULT hr;

	if(WAIT_OBJECT_0 == ::WaitForSingleObject(hEvFlushing_, 0)){
		::ResetEvent(hEvFlushing_);
		hr = S_OK;
	}
	else{
		// すでに非フラッシュ状態
		hr = S_FALSE;
	}

	::LeaveCriticalSection(&csQueue_);
	::LeaveCriticalSection(&csEndFlush_);
	return hr;
}


//
// MemSourceFilter の実装
//


//
// IUnknown
//
HRESULT STDMETHODCALLTYPE MemSourceFilter::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(! ppvObject){
		return E_POINTER;
	}

#define ADDBRANCH(interf__) \
	if(riid == __uuidof(interf__)){ \
		(	*reinterpret_cast<interf__**>(ppvObject) \
				= static_cast<interf__*>(this) \
		)->AddRef(); \
		return S_OK; \
	}

	ADDBRANCH(IBaseFilter);
	ADDBRANCH(IMediaFilter);
	ADDBRANCH(IPersist);
	ADDBRANCH(IUnknown);

#undef ADDBRANCH

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE MemSourceFilter::AddRef( void)
{
	EnterCriticalSection(&this->csRefCount_);
	auto newRefCount = ++(this->refCount_);

	//DebugText(L">>>newRefCount(%d) AddRef\n", newRefCount);

	LeaveCriticalSection(&this->csRefCount_);
	return newRefCount;
}


ULONG STDMETHODCALLTYPE MemSourceFilter::Release( void)
{
	EnterCriticalSection(&this->csRefCount_);
	auto newRefCount = --(this->refCount_);

	//DebugText(L">>>newRefCount(%d) Release\n", newRefCount);

	LeaveCriticalSection(&this->csRefCount_);

	if(newRefCount == 0){
		delete this;
	}

	return newRefCount;
}


//
// IPersist
//
HRESULT STDMETHODCALLTYPE MemSourceFilter::GetClassID( 
    /* [out] */ __RPC__out CLSID *pClassID)
{
	// 乱数で生成した独自の GUID
	//static GUID const guid = { 0xfd2d2e86, 0xd456, 0x4876, { 0x94, 0x45, 0x24, 0xd6, 0x6c, 0x98, 0x40, 0xc8 }};

	// {AFDF4396-34FE-4cb3-9C63-D3A2AB247DBA}
	static const GUID guid = 
	{ 0xafdf4396, 0x34fe, 0x4cb3, { 0x9c, 0x63, 0xd3, 0xa2, 0xab, 0x24, 0x7d, 0xba } };


	if(! pClassID){
		return E_POINTER;
	}

	*pClassID = guid;
	return S_OK;
}


//
// IMediaFilter
//
HRESULT STDMETHODCALLTYPE MemSourceFilter::Stop( void)
{
	this->filterState_ = State_Stopped;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemSourceFilter::Pause( void)
{
	this->filterState_ = State_Paused;
	return S_OK;
}
        

HRESULT STDMETHODCALLTYPE MemSourceFilter::Run( 
	REFERENCE_TIME tStart)
{
	this->filterState_ = State_Running;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemSourceFilter::GetState( 
	/* [in] */ DWORD dwMilliSecsTimeout,
	/* [annotation][out] */ 
	__out  FILTER_STATE *State)
{
	if(! State){
		return E_POINTER;
	}

	*State = this->filterState_;
	return S_OK; 
}


HRESULT STDMETHODCALLTYPE MemSourceFilter::SetSyncSource( 
	/* [annotation][in] */ 
	__in_opt  IReferenceClock *pClock)
{
	if(this->pClock_){
		this->pClock_->Release();
	}

	this->pClock_ = pClock;
	if(this->pClock_) this->pClock_->AddRef();

	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemSourceFilter::GetSyncSource( 
	/* [annotation][out] */ 
	__deref_out_opt  IReferenceClock **pClock)
{
	if(! pClock) return E_POINTER;

	if(this->pClock_){
		this->pClock_->AddRef();
	}

	*pClock = this->pClock_;
	return S_OK;
}
 

//
// IBaseFilter
//
HRESULT STDMETHODCALLTYPE MemSourceFilter::EnumPins( 
	/* [annotation][out] */ 
	__out  IEnumPins **ppEnum)
{
	if(! ppEnum) return E_POINTER;

	*ppEnum = new(std::nothrow) MemEnumPins(this);
	if(! *ppEnum){
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemSourceFilter::FindPin( 
	/* [string][in] */ LPCWSTR Id,
	/* [annotation][out] */ 
	__out  IPin **ppPin)
{
	if(! ppPin) return E_POINTER;

	if(0 == lstrcmpW(Id, MemSourceFilter_OutputPinName)){
		*ppPin = this->GetOutputPin();
		return S_OK;
	}

	*ppPin = nullptr;
	return VFW_E_NOT_FOUND;
}


HRESULT STDMETHODCALLTYPE MemSourceFilter::QueryFilterInfo( 
	/* [annotation][out] */ 
	__out  FILTER_INFO *pInfo)
{
	if(! pInfo) return E_POINTER;

	*pInfo = this->filterInfo_;
	if(pInfo->pGraph){
		pInfo->pGraph->AddRef();
	}

	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemSourceFilter::JoinFilterGraph( 
	/* [annotation][in] */ 
	__in_opt  IFilterGraph *pGraph,
	/* [annotation][string][in] */ 
	__in_opt  LPCWSTR pName)
{
	// 循環参照を避けるため、addref しない
	// 古い this->filterInfo_.pGraph も addref していないので
	// release する必要もない
	this->filterInfo_.pGraph = pGraph;

	if(pGraph){
		lstrcpynW(this->filterInfo_.achName, pName, NUMELMS(this->filterInfo_.achName));
		this->filterInfo_.achName[NUMELMS(this->filterInfo_.achName) - 1] = L'\0';
	}
	else{
		this->filterInfo_.achName[0] = L'\0';
	}

	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemSourceFilter::QueryVendorInfo( 
	/* [annotation][string][out] */ 
	__out  LPWSTR *pVendorInfo)
{
	return E_NOTIMPL;
}


namespace
{
//
// MemEnumPins の実装
//

//
// IUnknown
//

HRESULT STDMETHODCALLTYPE MemEnumPins::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(! ppvObject){
		return E_POINTER;
	}

#define ADDBRANCH(interf__) \
	if(riid == __uuidof(interf__)){ \
		(	*reinterpret_cast<interf__**>(ppvObject) \
				= static_cast<interf__*>(this) \
		)->AddRef(); \
		return S_OK; \
	}

	ADDBRANCH(IEnumPins);
	ADDBRANCH(IUnknown);

#undef ADDBRANCH

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}



ULONG STDMETHODCALLTYPE MemEnumPins::AddRef( void)
{
	return ++(this->refCount_);
}


ULONG STDMETHODCALLTYPE MemEnumPins::Release( void)
{
	auto newRefCount = --(this->refCount_);
	if(newRefCount == 0){
		delete this;
	}

	return newRefCount;
}

//
// IEnumPins
//
HRESULT STDMETHODCALLTYPE MemEnumPins::Next( 
	/* [in] */ ULONG cPins,
	/* [annotation][size_is][out] */ 
	__out_ecount_part(cPins, *pcFetched)  IPin **ppPins,
	/* [annotation][out] */ 
	__out_opt  ULONG *pcFetched)
{
	if(! ppPins) return E_POINTER;
	if(! pcFetched && cPins > 1) return E_INVALIDARG;

	ULONG dummy;
	if(! pcFetched) pcFetched = &dummy;

	*pcFetched = 0;

	if(this->index_ == 0 && cPins > 0){
		*ppPins = pFilter_->GetOutputPin();
		if(pcFetched) *pcFetched = 1;

		++(this->index_);
		--cPins;
	}

	if(cPins == 0) return S_OK;
	else           return S_FALSE;
}

        
HRESULT STDMETHODCALLTYPE MemEnumPins::Skip( 
	/* [in] */ ULONG cPins)
{
	if(this->index_ == 0 && cPins > 0){
		++(this->index_);
		--cPins;
	}

	if(cPins == 0) return S_OK;
	else           return S_FALSE;
}

        
HRESULT STDMETHODCALLTYPE MemEnumPins::Reset( void)
{
	this->index_ = 0;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemEnumPins::Clone( 
	/* [annotation][out] */ 
	__out  IEnumPins **ppEnum)
{
	if(! ppEnum) return E_POINTER;

	*ppEnum = new(std::nothrow) MemEnumPins(this->pFilter_, this->index_);
	if(! *ppEnum){
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


//
// MemEnumMediaTypes の実装
//

//
// IUnknown
//

HRESULT STDMETHODCALLTYPE MemEnumMediaTypes::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(! ppvObject){
		return E_POINTER;
	}

#define ADDBRANCH(interf__) \
	if(riid == __uuidof(interf__)){ \
		(	*reinterpret_cast<interf__**>(ppvObject) \
				= static_cast<interf__*>(this) \
		)->AddRef(); \
		return S_OK; \
	}

	ADDBRANCH(IEnumMediaTypes);
	ADDBRANCH(IUnknown);

#undef ADDBRANCH

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}



ULONG STDMETHODCALLTYPE MemEnumMediaTypes::AddRef( void)
{
	return ++(this->refCount_);
}


ULONG STDMETHODCALLTYPE MemEnumMediaTypes::Release( void)
{
	auto newRefCount = --(this->refCount_);
	if(newRefCount == 0){
		delete this;
	}

	return newRefCount;
}

//
// IEnumMediaTypes
//
HRESULT STDMETHODCALLTYPE MemEnumMediaTypes::Next( 
	/* [in] */ ULONG cMediaTypes,
	/* [annotation][size_is][out] */ 
	AM_MEDIA_TYPE **ppMediaTypes,
	/* [annotation][out] */ 
	_Out_opt_  ULONG *pcFetched)
{
	if(! ppMediaTypes) return E_POINTER;
	if(! pcFetched && cMediaTypes > 1) return E_INVALIDARG;

	ULONG dummy;
	if(! pcFetched) pcFetched = &dummy;

	*pcFetched = 0;

	while(this->index_ < pPin_->GetNumMyMediaType()
		&& cMediaTypes > 0
	){
		*ppMediaTypes = MyCreateMediaType(pPin_->GetMyMediaType(this->index_));
		++ppMediaTypes;
		++(*pcFetched);

		++(this->index_);
		--cMediaTypes;
	}

	if(cMediaTypes == 0) return S_OK;
	else           return S_FALSE;
}

        
HRESULT STDMETHODCALLTYPE MemEnumMediaTypes::Skip( 
	/* [in] */ ULONG cMediaTypes)
{
	while(this->index_ < pPin_->GetNumMyMediaType()
		&& cMediaTypes > 0
	){
		++(this->index_);
		--cMediaTypes;
	}

	if(cMediaTypes == 0) return S_OK;
	else                 return S_FALSE;
}

        
HRESULT STDMETHODCALLTYPE MemEnumMediaTypes::Reset( void)
{
	this->index_ = 0;
	return S_OK;
}

        
HRESULT STDMETHODCALLTYPE MemEnumMediaTypes::Clone( 
	/* [annotation][out] */ 
	_Out_  IEnumMediaTypes **ppEnum)
{
	if(! ppEnum) return E_POINTER;

	*ppEnum = new(std::nothrow) MemEnumMediaTypes(this->pPin_, this->index_);
	if(! *ppEnum){
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


//
// レジストリを検索して、data にふさわしいメディアタイプを見つける
// 関数群
//

bool MatchMediaType(
	LPCWSTR szCheck, BYTE const* data, std::size_t size
){
	// szCheck は offset,cb,mask,val の繰り返し

	LPCWSTR pOffset; // GetNextQuad 呼び出しで更新される
	LPCWSTR pCb;	 // GetNextQuad 呼び出しで更新される
	LPCWSTR pMask;   // GetNextQuad 呼び出しで更新される
	LPCWSTR pVal;    // GetNextQuad 呼び出しで更新される

	const int Found = 0; // GetNextQuad の戻り値
	const int End   = 1; // GetNextQuad の戻り値
	const int Error = 2; // GetNextQuad の戻り値

	auto GetNextQuad = [&]() -> int {
		while(*szCheck == L' ') ++szCheck;
		if(*szCheck == L'\0') return End;

		pOffset = szCheck;

		while(*szCheck != L','){
			if(*szCheck == L'\0') return Error;
			++szCheck;
		}
		++szCheck; // skip ','
		while(*szCheck == L' ') ++szCheck;

		pCb = szCheck;

		while(*szCheck != L','){
			if(*szCheck == L'\0') return Error;
			++szCheck;
		}
		++szCheck; // skip ','
		while(*szCheck == L' ') ++szCheck;

		pMask = szCheck;

		while(*szCheck != L','){
			if(*szCheck == L'\0') return Error;
			++szCheck;
		}
		++szCheck; // skip ','
		while(*szCheck == L' ') ++szCheck;

		pVal = szCheck;

		while(*szCheck != L','){
			if(*szCheck == L'\0') return Found;
			++szCheck;
		}
		++szCheck; // skip ','

		return Found;
	};


	unsigned char const InvalidDigit = (unsigned char)(-1);
	auto CharToHexDigit = [=](WCHAR c) -> unsigned char
	{
		if(L'0' <= c && c <= L'9') return (unsigned char)(c - L'0');
		if(L'A' <= c && c <= L'F') return (unsigned char)(c - L'A' + 10);
		if(L'a' <= c && c <= L'f') return (unsigned char)(c - L'a' + 10);

		return InvalidDigit;
	};


	LONG                       offset; // ConvertQuad で更新される
	LONG                       cb;	   // ConvertQuad で更新される
	std::vector<unsigned char> mask;   // ConvertQuad で更新される
	std::vector<unsigned char> val;	   // ConvertQuad で更新される

	auto ConvertQuad = [&]() -> bool
	{
		offset = StrToInt(pOffset);
		cb     = StrToInt(pCb);

		mask.clear();
		val.clear();

		for(LONG i = 0; i < cb; ++i){
			unsigned char digitH = CharToHexDigit(*(pMask++));
			if(digitH == InvalidDigit) break;
			unsigned char digitL = CharToHexDigit(*(pMask++));
			if(digitL == InvalidDigit) return false;

			mask.push_back((digitH << 4) | digitL);
		}
			
		for(LONG i = 0; i < cb; ++i){
			unsigned char digitH = CharToHexDigit(*(pVal++));
			if(digitH == InvalidDigit) break;
			unsigned char digitL = CharToHexDigit(*(pVal++));
			if(digitL == InvalidDigit) return false;
			
			val.push_back((digitH << 4) | digitL);
		}

		return true;
	};


	std::vector<unsigned char> maskedData;

	mask.reserve(256);
	val.reserve(256);
	maskedData.reserve(256);

	int errorCode;
	while(errorCode = GetNextQuad(), errorCode == Found){
		if(! ConvertQuad()) return false;

		if(offset < 0) offset += (LONG)size;
		if((std::size_t)(offset + cb) > size) return false;

		if((std::size_t)cb != val.size()) return false;

		if(mask.empty()){
			// 比較
			if(! std::equal(val.begin(), val.end(), data + offset)){
				return false;
			}
		}
		else{
			if((std::size_t)cb != mask.size()) return false;
			maskedData.resize(cb);

			for(LONG i = 0; i < cb; ++i){
				maskedData[i] = (data[i + offset] & mask[i]);
			}
			// 比較
			if(! std::equal(val.begin(), val.end(), maskedData.begin())){
				return false;
			}
		}

		// next quad
	}

	return errorCode == End;
}


bool MatchMediaTypeInRegistry(
	HKEY hSubtype, BYTE const* data, std::size_t size
){
	// hSubType の中には
	//    0    "0,4,,52494646,8,4,,524D4944"
	//    1    "0,4,,4D546864"
	// みたいなデータと
	//    Source Filter  "{CLSID}"
	// が入っている。
	// このうち必要なのは、 Source Filter 以外。
	//
	DWORD cbMaxValueLen = 512;
	DWORD cchMaxNameLen = 256;
	DWORD numValues = 0;

	::RegQueryInfoKey(
		hSubtype,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&numValues,
		&cchMaxNameLen,
		&cbMaxValueLen,
		nullptr,
		nullptr
	);

	DWORD cchMaxValueLen = (cbMaxValueLen + sizeof(WCHAR) - 1) / sizeof(WCHAR) + 1;

	cbMaxValueLen = cchMaxValueLen * sizeof(WCHAR);
	++cchMaxNameLen; // 末尾のナル文字分

	// 以下の二つはただのメモリとして使う。
	// クラスとして何かしようとしないこと (operator== とか)
	std::wstring value(cchMaxValueLen, L'\0');
	std::wstring name(cchMaxNameLen, L'\0');

	for(DWORD valueIndex = 0; valueIndex < numValues; ++valueIndex){
		DWORD cbValueLen = cbMaxValueLen;
		DWORD cchNameLen = cchMaxNameLen;
		DWORD type;

		if(ERROR_SUCCESS != ::RegEnumValue(
			hSubtype,
			valueIndex,
			const_cast<LPWSTR>(name.data()),
			&cchNameLen,
			nullptr,
			&type,
			(BYTE*)const_cast<LPWSTR>(value.data()),
			&cbValueLen
		)){
			continue;
		}

		if(type != REG_SZ) continue;

		if(0 == ::lstrcmpi(name.data(), L"Source Filter")){
			continue;
		}
		
		if(MatchMediaType(value.data(), data, size)) return true;
	}
	return false;
}


bool FindMediaTypeInRegistry(
	BYTE const* data, std::size_t size, AM_MEDIA_TYPE* output
){
	// sz <= "Media Type\{MEDIATYPE_Stream}"
	#define STR_MEDIA_TYPE   L"Media Type\\"

	WCHAR sz[256] = STR_MEDIA_TYPE;
	::StringFromGUID2(
		MEDIATYPE_Stream,
		sz + NUMELMS(STR_MEDIA_TYPE) - 1,
		NUMELMS(sz) - NUMELMS(STR_MEDIA_TYPE)
	);

	#undef STR_MEDIA_TYPE

	// "Media Type\{MEDIATYPE_Stream}" を開く
	HKEY hMajor = nullptr;
	if(ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, KEY_READ, &hMajor)){
		return false;
	}

	// その下には、Subtype の CLSID が列挙されている

	DWORD minorIndex = 0;
	DWORD nameLen = NUMELMS(sz) - 1;
	while(ERROR_SUCCESS == RegEnumKeyEx(
		hMajor, minorIndex, sz, &nameLen, nullptr, nullptr, nullptr, nullptr
	)){
		HKEY  hMinor = nullptr;
		if(ERROR_SUCCESS == RegOpenKeyEx(hMajor, sz, 0, KEY_READ, &hMinor)){
			// subtype がマッチするかどうか調べる
			CLSID subtype;
			if(NOERROR == CLSIDFromString(sz, &subtype)
			&& MatchMediaTypeInRegistry(hMinor, data, size)
			){
				output->subtype = subtype;
				::RegCloseKey(hMinor);
				::RegCloseKey(hMajor);
				return true;
			}
			::RegCloseKey(hMinor);
		}
		// 次の RegEnumKeyEx 呼び出しのために nameLen を初期化
		nameLen = NUMELMS(sz) - 1;
		++minorIndex;
	}

	::RegCloseKey(hMajor);
	return false;
}


} // namespace anonymous
} // namespace dsp
