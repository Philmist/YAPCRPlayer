//////////////////////////////////////////////////////////////////////////
//
// PresentEngine.h: Defines the D3DPresentEngine object.
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
// D3DPresentEngine class
//
// This class creates the Direct3D device, allocates Direct3D surfaces for
// rendering, and presents the surfaces. This class also owns the Direct3D
// device manager and provides the IDirect3DDeviceManager9 interface via
// GetService.
//
// The goal of this class is to isolate the EVRCustomPresenter class from
// the details of Direct3D as much as possible.
//-----------------------------------------------------------------------------

class D3DPresentEngine : public SchedulerCallback
{
public:

    // State of the Direct3D device.
    enum DeviceState
    {
        DeviceOK,
        DeviceReset,    // The device was reset OR re-created.
        DeviceRemoved,  // The device was removed.
    };

    D3DPresentEngine(HRESULT& hr);
    virtual ~D3DPresentEngine();

    // GetService: Returns the IDirect3DDeviceManager9 interface.
    // (The signature is identical to IMFGetService::GetService but 
    // this object does not derive from IUnknown.)
    virtual HRESULT GetService(REFGUID guidService, REFIID riid, void** ppv);
    virtual HRESULT CheckFormat(D3DFORMAT format);

    // Video window / destination rectangle:
    // This object implements a sub-set of the functions defined by the 
    // IMFVideoDisplayControl interface. However, some of the method signatures 
    // are different. The presenter's implementation of IMFVideoDisplayControl 
    // calls these methods.
    HRESULT SetVideoWindow(HWND hwnd);
    HWND    GetVideoWindow() const { return m_hwnd; }
    HRESULT SetDestinationRect(const RECT& rcDest);
    RECT    GetDestinationRect() const { return m_rcDestRect; };
    HRESULT SetSourceRect(const RECT& rcSrc);
    RECT    GetSourceRect() const { return m_rcSrcRect; };

    HRESULT CreateVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue);
	HRESULT CreateSourceVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue);
	HRESULT CreateVideoSample(IMFMediaType *pFormat, IMFSample** pSample);
    void    ReleaseResources();

    HRESULT CheckDeviceState(DeviceState *pState);
    HRESULT PresentSample(IMFSample* pSample, LONGLONG llTarget); 

    UINT    RefreshRate() const { return m_DisplayMode.RefreshRate; }

protected:
    HRESULT InitializeD3D();
    HRESULT GetSwapChainPresentParameters(IMFMediaType *pType, D3DPRESENT_PARAMETERS* pPP);
    HRESULT CreateD3DDevice();
    HRESULT CreateD3DSample(IDirect3DSwapChain9 *pSwapChain, IMFSample **ppVideoSample);
    HRESULT UpdateDestRect();

    // A derived class can override these handlers to allocate any additional D3D resources.
    virtual HRESULT OnCreateVideoSamples(D3DPRESENT_PARAMETERS& pp) { return S_OK; }
    virtual void    OnReleaseResources() { }

    virtual HRESULT PresentSwapChain(IDirect3DSwapChain9* pSwapChain, IDirect3DSurface9* pSurface);

    virtual void    PaintFrameWithGDI();

protected:
    UINT                        m_DeviceResetToken;     // Reset token for the D3D device manager.

    HWND                        m_hwnd;                 // Application-provided destination window.
    RECT                        m_rcDestRect;           // Destination rectangle.
	RECT                        m_rcSrcRect;
    D3DDISPLAYMODE              m_DisplayMode;          // Adapter's display mode.

    CritSec                     m_ObjectLock;           // Thread lock for the D3D device.
	
	CritSec                     m_SurfaceRepaintLock; 

    // COM interfaces
    IDirect3D9Ex                *m_pD3D9;
    IDirect3DDevice9Ex          *m_pDevice;
    IDirect3DDeviceManager9     *m_pDeviceManager;        // Direct3D device manager.
    IDirect3DSurface9           *m_pSurfaceRepaint;       // Surface for repaint requests.

	boost::shared_ptr<d3dutl::D3DResizer> resizer_;
	int							m_iResizer;

public:
	HRESULT copySurface(IMFSample* pInput, IMFSample* pOutput, D3DTEXTUREFILTERTYPE filter);
	HRESULT stretchSurface(IMFSample* pInput, IMFSample* pOutput, D3DTEXTUREFILTERTYPE filter);

	HRESULT GetCurrentImage(
		IDirect3DSurface9* pSource,
		BITMAPINFOHEADER *pBih,
		BYTE             **pDib,
		DWORD            *pcbDib,
		LONGLONG         *pTimeStamp
	);

	HRESULT SetResizer(int iResizer);
	HRESULT GetResizer(int* pResizer);
};