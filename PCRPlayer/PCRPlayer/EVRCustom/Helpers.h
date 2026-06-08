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

#pragma once

//-----------------------------------------------------------------------------
// SamplePool class
//
// Manages a list of allocated samples.
//-----------------------------------------------------------------------------

class SamplePool 
{
public:
    SamplePool();
    virtual ~SamplePool();

    HRESULT Initialize(VideoSampleList& samples);
    HRESULT Clear();
   
    HRESULT GetSample(IMFSample **ppSample);    // Does not block.
    HRESULT ReturnSample(IMFSample *pSample);   
    BOOL    AreSamplesPending();

private:
    CritSec                     m_lock;

    VideoSampleList             m_VideoSampleQueue;         // Available queue

    BOOL                        m_bInitialized;
    DWORD                       m_cPending;
};


//-----------------------------------------------------------------------------
// ProcessOutputBuffer class
//-----------------------------------------------------------------------------

class ProcessOutputBuffer 
{
public:
	ProcessOutputBuffer();
	virtual ~ProcessOutputBuffer();

	HRESULT Initialize(VideoSampleList& samples);
	HRESULT Clear();

    HRESULT GetSample(IMFSample **ppSample, BOOL* bFlush);
    HRESULT ReturnSample(IMFSample *pSample, BOOL bBack);

    HRESULT LockSample(IMFSample **ppSample, BOOL* bFlush);
    HRESULT UnlockSample(IMFSample *pSample, BOOL bBack);

	HRESULT GetBack(IMFSample **ppSample);

	CritSec& GetCritSec() { return lock_; }

	HRESULT Flush();

	BOOL IsInitialize();

private:
    CritSec						lock_;
	BOOL						init_;
	BOOL						flush_;
    VideoSampleList				queue_;         // Available queue
	VideoSampleList::POSITION	pos_;
};


class ProcessOutputWrapper {
public:
	ProcessOutputWrapper();
	virtual ~ProcessOutputWrapper();
	
	HRESULT Initialize(IMFSample* pWorker, IMFSample* pStore);
	BOOL IsInitialize();
	HRESULT Clear();

	HRESULT Flush(int count = 2);

    HRESULT LockSample(IMFSample **ppSample, BOOL* bFlush = NULL);
    HRESULT UnlockSample(BOOL bStore = FALSE);
	
private:
	CComPtr<IMFSample>		worker_;
	CComPtr<IMFSample>		store_;
    CritSec					lock_;
	BOOL					flush_;
	int						count_;
};


//-----------------------------------------------------------------------------
// ThreadSafeQueue template
// Thread-safe queue of COM interface pointers.
//
// T: COM interface type.
//
// This class is used by the scheduler. 
//
// Note: This class uses a critical section to protect the state of the queue.
// With a little work, the scheduler could probably use a lock-free queue.
//-----------------------------------------------------------------------------

template <class T>
class ThreadSafeQueue
{
public:
    HRESULT Queue(T *p)
    {
        AutoLock lock(m_lock);
        return m_list.InsertBack(p);
    }

    HRESULT Dequeue(T **pp)
    {
        AutoLock lock(m_lock);

        if (m_list.IsEmpty())
        {
            *pp = NULL;
            return S_FALSE;
        }

        return m_list.RemoveFront(pp);
    }

    HRESULT PutBack(T *p)
    {
        AutoLock lock(m_lock);
        return m_list.InsertFront(p);
    }

    void Clear() 
    {
        AutoLock lock(m_lock);
        m_list.Clear();
    }


private:
    CritSec         m_lock; 
    ComPtrList<T>   m_list;
};

