//////////////////////////////////////////////////////////////////////////
//
// trace.h : Functions to return the names of constants.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "logging.h"
#include <mfidl.h>

#define NAMEX(x) case x: return L#x

namespace MediaFoundationSamples
{

    // IMPORTANT: No function here can return a NULL pointer - caller assumes
    // the return value is a valid null-terminated string. You should only
    // use these functions for debugging purposes.

    // Media Foundation event names (subset)
    inline const WCHAR* EventName(MediaEventType met)
    {
        switch (met)
        {
            NAMEX(MEError);
            NAMEX(MEExtendedType);
            NAMEX(MESessionTopologySet);
            NAMEX(MESessionTopologiesCleared);
            NAMEX(MESessionStarted);
            NAMEX(MESessionPaused);
            NAMEX(MESessionStopped);
            NAMEX(MESessionClosed);
            NAMEX(MESessionEnded);
            NAMEX(MESessionRateChanged);
            NAMEX(MESessionScrubSampleComplete);
            NAMEX(MESessionCapabilitiesChanged);
            NAMEX(MESessionTopologyStatus);
            NAMEX(MESessionNotifyPresentationTime);
            NAMEX(MENewPresentation);
            NAMEX(MELicenseAcquisitionStart);
            NAMEX(MELicenseAcquisitionCompleted);
            NAMEX(MEIndividualizationStart);
            NAMEX(MEIndividualizationCompleted);
            NAMEX(MEEnablerProgress);
            NAMEX(MEEnablerCompleted);
            NAMEX(MEPolicyError);
            NAMEX(MEPolicyReport);
            NAMEX(MEBufferingStarted);
            NAMEX(MEBufferingStopped);
            NAMEX(MEConnectStart);
            NAMEX(MEConnectEnd);
            NAMEX(MEReconnectStart);
            NAMEX(MEReconnectEnd);
            NAMEX(MERendererEvent);
            NAMEX(MESessionStreamSinkFormatChanged);
            NAMEX(MESourceStarted);
            NAMEX(MEStreamStarted);
            NAMEX(MESourceSeeked);
            NAMEX(MEStreamSeeked);
            NAMEX(MENewStream);
            NAMEX(MEUpdatedStream);
            NAMEX(MESourceStopped);
            NAMEX(MEStreamStopped);
            NAMEX(MESourcePaused);
            NAMEX(MEStreamPaused);
            NAMEX(MEEndOfPresentation);
            NAMEX(MEEndOfStream);
            NAMEX(MEMediaSample);
            NAMEX(MEStreamTick);
            NAMEX(MEStreamThinMode);
            NAMEX(MEStreamFormatChanged);
            NAMEX(MESourceRateChanged);
            NAMEX(MEEndOfPresentationSegment);
            NAMEX(MESourceCharacteristicsChanged);
            NAMEX(MESourceRateChangeRequested);
            NAMEX(MESourceMetadataChanged);
            NAMEX(MESequencerSourceTopologyUpdated);
            NAMEX(MEStreamSinkStarted);
            NAMEX(MEStreamSinkStopped);
            NAMEX(MEStreamSinkPaused);
            NAMEX(MEStreamSinkRateChanged);
            NAMEX(MEStreamSinkRequestSample);
            NAMEX(MEStreamSinkMarker);
            NAMEX(MEStreamSinkPrerolled);
            NAMEX(MEStreamSinkScrubSampleComplete);
            NAMEX(MEStreamSinkFormatChanged);
            NAMEX(MEStreamSinkDeviceChanged);
            NAMEX(MEQualityNotify);
            NAMEX(MESinkInvalidated);
            NAMEX(MEAudioSessionNameChanged);
            NAMEX(MEAudioSessionVolumeChanged);
            NAMEX(MEAudioSessionDeviceRemoved);
            NAMEX(MEAudioSessionServerShutdown);
            NAMEX(MEAudioSessionGroupingParamChanged);
            NAMEX(MEAudioSessionIconChanged);
            NAMEX(MEAudioSessionFormatChanged);
            NAMEX(MEAudioSessionDisconnected);
            NAMEX(MEAudioSessionExclusiveModeOverride);
            NAMEX(MEPolicyChanged);
            NAMEX(MEContentProtectionMessage);
            NAMEX(MEPolicySet);

        default:
            return L"Unknown event";
        }
    }

    // Names of VARIANT data types. 
    inline const WCHAR* VariantTypeName(const PROPVARIANT& prop)
    {
        switch (prop.vt & VT_TYPEMASK)
        {
            NAMEX(VT_EMPTY);
            NAMEX(VT_NULL);
            NAMEX(VT_I2);
            NAMEX(VT_I4);
            NAMEX(VT_R4);
            NAMEX(VT_R8);
            NAMEX(VT_CY);
            NAMEX(VT_DATE);
            NAMEX(VT_BSTR);
            NAMEX(VT_DISPATCH);
            NAMEX(VT_ERROR);
            NAMEX(VT_BOOL);
            NAMEX(VT_VARIANT);
            NAMEX(VT_UNKNOWN);
            NAMEX(VT_DECIMAL);
            NAMEX(VT_I1);
            NAMEX(VT_UI1);
            NAMEX(VT_UI2);
            NAMEX(VT_UI4);
            NAMEX(VT_I8);
            NAMEX(VT_UI8);
            NAMEX(VT_INT);
            NAMEX(VT_UINT);
            NAMEX(VT_VOID);
            NAMEX(VT_HRESULT);
            NAMEX(VT_PTR);
            NAMEX(VT_SAFEARRAY);
            NAMEX(VT_CARRAY);
            NAMEX(VT_USERDEFINED);
            NAMEX(VT_LPSTR);
            NAMEX(VT_LPWSTR);
            NAMEX(VT_RECORD);
            NAMEX(VT_INT_PTR);
            NAMEX(VT_UINT_PTR);
            NAMEX(VT_FILETIME);
            NAMEX(VT_BLOB);
            NAMEX(VT_STREAM);
            NAMEX(VT_STORAGE);
            NAMEX(VT_STREAMED_OBJECT);
            NAMEX(VT_STORED_OBJECT);
            NAMEX(VT_BLOB_OBJECT);
            NAMEX(VT_CF);
            NAMEX(VT_CLSID);
            NAMEX(VT_VERSIONED_STREAM);
        default:
            return L"Unknown VARIANT type";
        }
    }

    // Names of topology node types.
    inline const WCHAR* TopologyNodeTypeName(MF_TOPOLOGY_TYPE nodeType)
    {
        switch (nodeType)
        {
            NAMEX(MF_TOPOLOGY_OUTPUT_NODE);
            NAMEX(MF_TOPOLOGY_SOURCESTREAM_NODE);
            NAMEX(MF_TOPOLOGY_TRANSFORM_NODE);
            NAMEX(MF_TOPOLOGY_TEE_NODE);
        default:
            return L"Unknown node type";
        }
    }

    inline const WCHAR* MFTMessageName(MFT_MESSAGE_TYPE msg)
    {
        switch (msg)
        {
            NAMEX(MFT_MESSAGE_COMMAND_FLUSH);
            NAMEX(MFT_MESSAGE_COMMAND_DRAIN);
            NAMEX(MFT_MESSAGE_SET_D3D_MANAGER);
            NAMEX(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING);
            NAMEX(MFT_MESSAGE_NOTIFY_END_STREAMING);
            NAMEX(MFT_MESSAGE_NOTIFY_END_OF_STREAM);
            NAMEX(MFT_MESSAGE_NOTIFY_START_OF_STREAM);
        default:
            return L"Unknown message";
        }
    }

}; // namespace MediaFoundationSamples
