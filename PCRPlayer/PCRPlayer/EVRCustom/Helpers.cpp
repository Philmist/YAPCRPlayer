//////////////////////////////////////////////////////////////////////////
//
// Helpers.cpp : Miscellaneous helpers.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//////////////////////////////////////////////////////////////////////////

#include "../stdafx.h"

#include "EVRPresenter.h"

//-----------------------------------------------------------------------------
// SamplePool class
//-----------------------------------------------------------------------------

SamplePool::SamplePool() : m_bInitialized(FALSE), m_cPending(0)
{
}

SamplePool::~SamplePool()
{
}


//-----------------------------------------------------------------------------
// GetSample
//
// Gets a sample from the pool. If no samples are available, the method
// returns MF_E_SAMPLEALLOCATOR_EMPTY.
//-----------------------------------------------------------------------------

HRESULT SamplePool::GetSample(IMFSample **ppSample)
{
    AutoLock lock(m_lock);

    if (!m_bInitialized)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (m_VideoSampleQueue.IsEmpty())
    {
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    // Get a sample from the allocated queue.

    // It doesn't matter if we pull them from the head or tail of the list,
    // but when we get it back, we want to re-insert it onto the opposite end.
    // (see ReturnSample)

    CHECK_HR(hr = m_VideoSampleQueue.RemoveFront(&pSample));

    m_cPending++;

    // Give the sample to the caller.
    *ppSample = pSample;
    (*ppSample)->AddRef();

done:
    SAFE_RELEASE(pSample);
    return hr;
}

//-----------------------------------------------------------------------------
// ReturnSample
//
// Returns a sample to the pool.
//-----------------------------------------------------------------------------

HRESULT SamplePool::ReturnSample(IMFSample *pSample) 
{
    AutoLock lock(m_lock);

    if (!m_bInitialized)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;

    CHECK_HR(hr = m_VideoSampleQueue.InsertBack(pSample));

    m_cPending--;

done:
    return hr;
}

//-----------------------------------------------------------------------------
// AreSamplesPending
//
// Returns TRUE if any samples are in use.
//-----------------------------------------------------------------------------

BOOL SamplePool::AreSamplesPending()
{
    AutoLock lock(m_lock);

    if (!m_bInitialized)
    {
        return FALSE;
    }

    return (m_cPending > 0);
}


//-----------------------------------------------------------------------------
// Initialize
//
// Initializes the pool with a list of samples.
//-----------------------------------------------------------------------------

HRESULT SamplePool::Initialize(VideoSampleList& samples)
{
    AutoLock lock(m_lock);

    if (m_bInitialized)
    {
        return MF_E_INVALIDREQUEST;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    // Move these samples into our allocated queue.
    VideoSampleList::POSITION pos = samples.FrontPosition();
    while (pos != samples.EndPosition())
    {
        CHECK_HR(hr = samples.GetItemPos(pos, &pSample));
        CHECK_HR(hr = m_VideoSampleQueue.InsertBack(pSample));

        pos = samples.Next(pos);
        SAFE_RELEASE(pSample);
    }

    m_bInitialized = TRUE;

done:
    samples.Clear();

    SAFE_RELEASE(pSample);
    return hr;
}


//-----------------------------------------------------------------------------
// Clear
//
// Releases all samples.
//-----------------------------------------------------------------------------

HRESULT SamplePool::Clear()
{
    HRESULT hr = S_OK;

    AutoLock lock(m_lock);

    m_VideoSampleQueue.Clear();
    m_bInitialized = FALSE;
    m_cPending = 0;
    return S_OK;
}









//-----------------------------------------------------------------------------
// ProcessOutputBuffer class
//-----------------------------------------------------------------------------


ProcessOutputBuffer::ProcessOutputBuffer() : init_(FALSE), flush_(FALSE)
{
}

ProcessOutputBuffer::~ProcessOutputBuffer()
{
}

HRESULT ProcessOutputBuffer::Initialize(VideoSampleList& samples)
{
    AutoLock lock(lock_);

    if (init_)
    {
        return MF_E_INVALIDREQUEST;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    // Move these samples into our allocated queue.
    VideoSampleList::POSITION pos = samples.FrontPosition();
    while (pos != samples.EndPosition())
    {
        CHECK_HR(hr = samples.GetItemPos(pos, &pSample));
        CHECK_HR(hr = queue_.InsertBack(pSample));

        pos = samples.Next(pos);
        SAFE_RELEASE(pSample);
    }

    init_ = TRUE;

done:
    samples.Clear();

    SAFE_RELEASE(pSample);
    return hr;
}


HRESULT ProcessOutputBuffer::Clear()
{
    HRESULT hr = S_OK;

    AutoLock lock(lock_);

    queue_.Clear();
	pos_ = queue_.EndPosition();
    init_ = FALSE;
    return S_OK;
}

HRESULT ProcessOutputBuffer::GetSample(IMFSample **ppSample, BOOL* bFlush)
{
	AutoLock lock(lock_);

    if (!init_)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (queue_.IsEmpty())
    {
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

//#ifdef _DEBUG
//	static int test = 0;
//#endif

	if (pos_ != queue_.EndPosition())
	{
//#ifdef _DEBUG
//		test++;
//		DebugText(L"test:%d count:%d\n", test, queue_.GetCount());
//#endif
		*bFlush = TRUE;
		hr = queue_.GetItemPos(pos_, &pSample);
	}
	else
	{
//#ifdef _DEBUG
//		test = 0;
//#endif
		*bFlush = FALSE;
		hr = queue_.RemoveFront(&pSample);
	}

	if (FAILED(hr))
	{
		SAFE_RELEASE(pSample);
		return hr;
	}

	*ppSample = pSample;
	(*ppSample)->AddRef();
    SAFE_RELEASE(pSample);
    return hr;
}


HRESULT ProcessOutputBuffer::ReturnSample(IMFSample *pSample, BOOL bBack) 
{
	AutoLock lock(lock_);

    if (!init_)
    {
        return MF_E_NOT_INITIALIZED;
    }

	if (pSample == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr = S_OK;

	if (pos_ != queue_.EndPosition()) 
	{
		pos_ = queue_.Next(pos_);
		return S_OK;
	}

	if (bBack)
	{// 成功したら後方に追加する
		CHECK_HR(hr = queue_.InsertBack(pSample));
		// 一度成功したらフラッシュを有効にする
		flush_ = TRUE;
	}
	else
	{// 失敗したら前方に戻す
		CHECK_HR(hr = queue_.InsertFront(pSample));
	}

done:
    return hr;
}


HRESULT ProcessOutputBuffer::LockSample(IMFSample **ppSample, BOOL* bFlush)
{
	lock_.Lock();

    if (!init_)
    {
		lock_.Unlock();
        return MF_E_NOT_INITIALIZED;
    }

    if (queue_.IsEmpty())
    {
		lock_.Unlock();
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }

    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;
//#ifdef _DEBUG
//	static int test = 0;
//#endif
	if (pos_ != queue_.EndPosition())
	{
//#ifdef _DEBUG
//		test++;
//		DebugText(L"test:%d count:%d\n", test, queue_.GetCount());
//#endif
		*bFlush = TRUE;
		//hr = queue_.GetItemPos(pos_, &pSample);
		hr = queue_.GetBack(&pSample);
	}
	else
	{
//#ifdef _DEBUG
//		test = 0;
//#endif
		*bFlush = FALSE;
		hr = queue_.RemoveFront(&pSample);
	}

	if (FAILED(hr))
	{
		SAFE_RELEASE(pSample);
		lock_.Unlock();
		return hr;
	}

	*ppSample = pSample;
	(*ppSample)->AddRef();
    SAFE_RELEASE(pSample);
    return hr;
}


HRESULT ProcessOutputBuffer::UnlockSample(IMFSample *pSample, BOOL bBack) 
{
    if (!init_)
    {
		lock_.Unlock();
        return MF_E_NOT_INITIALIZED;
    }

	if (pSample == NULL)
	{
		lock_.Unlock();
		return E_POINTER;
	}

	HRESULT hr = S_OK;

	if (pos_ != queue_.EndPosition()) 
	{
		pos_ = queue_.Next(pos_);
		lock_.Unlock();
		return S_OK;
	}

	if (bBack)
	{// 成功したら後方に追加する
		CHECK_HR(hr = queue_.InsertBack(pSample));
		// 一度成功したらフラッシュを有効にする
		flush_ = TRUE;
	}
	else
	{// 失敗したら前方に戻す
		CHECK_HR(hr = queue_.InsertFront(pSample));
	}

done:
	lock_.Unlock();
    return hr;
}

HRESULT ProcessOutputBuffer::GetBack(IMFSample **ppSample)
{
	AutoLock lock(lock_);

    if (!init_)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (queue_.IsEmpty())
    {
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }

	return queue_.GetBack(ppSample);
}

HRESULT ProcessOutputBuffer::Flush()
{
	AutoLock lock(lock_);

    if (!init_)
    {
        return MF_E_NOT_INITIALIZED;
    }

	if (!flush_)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (queue_.IsEmpty())
    {
        return MF_E_SAMPLEALLOCATOR_EMPTY;
    }
	pos_ = queue_.FrontPosition();

    return S_OK;
}

BOOL ProcessOutputBuffer::IsInitialize()
{
	AutoLock lock(lock_);
	return init_;
}













ProcessOutputWrapper::ProcessOutputWrapper() : flush_(FALSE), count_(0)
{
}

ProcessOutputWrapper::~ProcessOutputWrapper()
{
}

HRESULT ProcessOutputWrapper::Initialize(IMFSample *pWorker, IMFSample *pStore)
{
	if (pWorker == NULL || pStore == NULL) { return E_POINTER; }

	AutoLock lock(lock_);
	worker_ = pWorker;
	store_ = pStore;
	return S_OK;
}

BOOL ProcessOutputWrapper::IsInitialize()
{
	AutoLock lock(lock_);
	if (worker_ && store_) { return TRUE; }
	return FALSE;
}

HRESULT ProcessOutputWrapper::Clear()
{
	AutoLock lock(lock_);
    if (!worker_ || !store_)
    {
        return MF_E_NOT_INITIALIZED;
    }
	worker_.Release();
	store_.Release();
	flush_ = FALSE;
	count_ = 0;
	return S_OK;
}

HRESULT ProcessOutputWrapper::Flush(int count)
{
	AutoLock lock(lock_);
    if (!worker_ || !store_)
    {
        return MF_E_NOT_INITIALIZED;
    }

	if (!flush_)
    {
        return MF_E_NOT_INITIALIZED;
    }

	count_ = 2;
	return S_OK;
}



HRESULT ProcessOutputWrapper::LockSample(IMFSample **ppSample, BOOL* bFlush)
{
	lock_.Lock();

    if (!worker_ || !store_)
    {
		lock_.Unlock();
        return MF_E_NOT_INITIALIZED;
    }

	if (bFlush)
	{
//#ifdef _DEBUG
//		static int test = 0;
//#endif
		if (count_ > 0)
		{
//#ifdef _DEBUG
//			test++;
//			DebugText(L"test:%d count:%d\n", test, count_);
//#endif
			*bFlush = TRUE;
			*ppSample = store_.p;
		}
		else
		{
//#ifdef _DEBUG
//			test = 0;
//#endif
			*bFlush = FALSE;
			*ppSample = worker_.p;
		}
	}
	else
	{
		*ppSample = store_.p;
	}

    return S_OK;
}


HRESULT ProcessOutputWrapper::UnlockSample(BOOL bStore) 
{
    if (!worker_ || !store_)
    {
		lock_.Unlock();
        return MF_E_NOT_INITIALIZED;
    }

	if (count_ > 0) 
	{
		count_--;
	}

	if (bStore)
	{
		IMFSample* w = worker_.Detach();
		IMFSample* s = store_.Detach();
		
		worker_.Attach(s);
		store_.Attach(w);

		flush_ = TRUE;
	}

	lock_.Unlock();
    return S_OK;
}