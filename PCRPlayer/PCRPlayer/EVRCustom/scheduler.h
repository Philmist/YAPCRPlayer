//////////////////////////////////////////////////////////////////////////
//
// Scheduler.h: Schedules when video frames are presented.
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

#include <math.h>

struct SchedulerCallback;

//-----------------------------------------------------------------------------
// Scheduler class
//
// Schedules when a sample should be displayed.
//
// Note: Presentation of each sample is performed by another object which
// must implement SchedulerCallback::PresentSample.
//
// General design:
// The scheduler generally receives samples before their presentation time. It
// puts the samples on a queue and presents them in FIFO order on a worker 
// thread. The scheduler communicates with the worker thread by posting thread
// messages.
//
// The caller has the option of presenting samples immediately (for example,
// for repaints). 
//-----------------------------------------------------------------------------

class Scheduler
{
public:
    Scheduler();
    virtual ~Scheduler();

    void SetCallback(SchedulerCallback *pCB)
    {
        m_pCB = pCB;
    }

    void SetFrameRate(const MFRatio& fps);
    void SetClockRate(float fRate) { m_fRate = fRate; }

    const LONGLONG& LastSampleTime() const { return m_LastSampleTime; }
    const LONGLONG& FrameDuration() const { return m_PerFrameInterval; }

    HRESULT StartScheduler(IMFClock *pClock);
    HRESULT StopScheduler();

    HRESULT ScheduleSample(IMFSample *pSample, BOOL bPresentNow);
    HRESULT ProcessSamplesInQueue(LONG *plNextSleep);
    HRESULT ProcessSample(IMFSample *pSample, LONG *plNextSleep);
    HRESULT Flush();

    // ThreadProc for the scheduler thread.
    static DWORD WINAPI SchedulerThreadProc(LPVOID lpParameter);


	void OnFrameDraw(LONGLONG hnsDelta, MFTIME hnsSystemTime);

	// IQualProp(EVRCustomPresenter)
	int GetAvgFrameRate() { AutoLock lock(propLock_); return prop_.avgFrameRate; }
	int GetAvgSyncOffset() { AutoLock lock(propLock_); return prop_.avgSyncOffset; }
	int GetDevSyncOffset() { AutoLock lock(propLock_); return prop_.devSyncOffset; }
	int GetFramesDrawn() { AutoLock lock(propLock_); return prop_.framesDrawn; }
	int GetFramesDroppedInRenderer() { AutoLock lock(propLock_); return prop_.framesDroppedInRenderer; }
	int GetJitter() { AutoLock lock(propLock_); return prop_.jitter; }

	// ICustomRenderProp(EVRCustomPresenter)
	LONGLONG GetSampleTime() { AutoLock lock(propLock_); return prop_.sampleTime; }

private: 
    // non-static version of SchedulerThreadProc.
    DWORD SchedulerThreadProcPrivate();


private:
    ThreadSafeQueue<IMFSample>  m_ScheduledSamples;     // Samples waiting to be presented.

    IMFClock            *m_pClock;  // Presentation clock. Can be NULL.
    SchedulerCallback   *m_pCB;     // Weak reference; do not delete.

    DWORD               m_dwThreadID;
    HANDLE              m_hSchedulerThread;
    HANDLE              m_hThreadReadyEvent;
    HANDLE              m_hFlushEvent;

    float               m_fRate;                // Playback rate.
    MFTIME              m_PerFrameInterval;     // Duration of each frame.
    LONGLONG            m_PerFrame_1_4th;       // 1/4th of the frame duration.
    MFTIME              m_LastSampleTime;       // Most recent sample time.

	CritSec propLock_;

	struct Prop {
		Prop()
			: avgFrameRate(0)
			, avgSyncOffset(0)
			, devSyncOffset(0)
			, framesDrawn(0)
			, framesDroppedInRenderer(0)
			, jitter(0)
			, sampleTime(0)
			, sumCount(0)
			, recentSystemTime(0)
			, recentJitter(0)
			, sumDelta(0)
			, sumvDelta(0)
			, sumJitter(0)
			, sumvJitter(0)
		{}

		// IQualProp(EVRCustomPresenter)
		int avgFrameRate;
		int	avgSyncOffset;
		int	devSyncOffset;
		int framesDrawn;
		int framesDroppedInRenderer;
		int jitter;

		// ICustomRenderProp(EVRCustomPresenter)
		LONGLONG sampleTime;

		LONGLONG sumCount;
		LONGLONG recentSystemTime;
		LONGLONG recentJitter;
		LONGLONG sumDelta;
		LONGLONG sumvDelta;
		LONGLONG sumJitter;
		LONGLONG sumvJitter;
	} prop_;
};


//-----------------------------------------------------------------------------
// SchedulerCallback
//
// Defines the callback method to present samples. 
//-----------------------------------------------------------------------------

struct SchedulerCallback
{
    virtual HRESULT PresentSample(IMFSample *pSample, LONGLONG llTarget) = 0;
};
