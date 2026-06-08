/* 
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <atlcoll.h>
#include "BaseSplitter.h"

enum USERABORT {
	USERABORT_DEFAULT,
	USERABORT_TAG_SIZE,
	USERABORT_DATA_SIZE,
	USERABORT_PREVIOUS_TAG_SIZE,
	USERABORT_STREAM_ID,
	USERABORT_SYNC_ERROR,
	USERABORT_DATA_ERROR,
	USERABORT_TIMESTAMP_AUDIO,
	USERABORT_TIMESTAMP_VIDEO,
};

// FLVMetaData
#include "FLVMetaData.h"

struct FLVConfig {
	FLVConfig()
		: loop(true)
		, keyframe(true)
		, buffer(0)
		, optimize(false)
		, delay(true)
		, limit(10000)
		, interval(10000)
	{}

	bool loop; // ループを検出したらチャンネル再接続
	bool keyframe; // キーフレームから再生
	int buffer; // バッファ(ms)
	bool optimize; // バッファ最適化
	bool delay; // 遅延補正を有効
	int limit; // バッファ上限
	int interval; // 最適化間隔
};

// {779393E2-9D6A-47e3-AEBD-02B7EF0650AD}
DEFINE_GUID(IID_IFLVSplitterProp,
0x779393e2, 0x9d6a, 0x47e3, 0xae, 0xbd, 0x2, 0xb7, 0xef, 0x6, 0x50, 0xad);

DECLARE_INTERFACE_(IFLVSplitterProp, IUnknown)
{
	STDMETHOD(GetEncoder) (std::string* str) PURE;
	STDMETHOD(GetDatarate) (int* rate) PURE;
	STDMETHOD(GetFramerate) (int* rate) PURE;
	STDMETHOD(GetCurrentPos) (LONGLONG* pos) PURE;
	STDMETHOD(GetPositionDiff) (LONGLONG* diff) PURE;
	STDMETHOD(SetConfig) (const FLVConfig& cfg) PURE;
};


//class __declspec(uuid("47E792CF-0BBE-4F7A-859C-194B0768650A"))
class __declspec(uuid("9058F496-FC28-4682-801C-9D0D5A9202C9"))
CFLVSplitterFilter
	: public CBaseSplitterFilter
	, public IFLVSplitterProp
	, protected utl::Logger
{
	UINT32 m_DataOffset;
	bool m_IgnorePrevSizes;

	bool Sync(__int64& pos);

	struct VideoTweak
	{
		BYTE x;
		BYTE y;
	};

	bool ReadTag(VideoTweak& t);
	
	struct Tag
	{
		UINT32 PreviousTagSize;
		BYTE TagType;
		UINT32 DataSize;
		UINT32 TimeStamp;
		UINT32 StreamID;
	};

	bool ReadTag(Tag& t);

	bool WaitForData(LONGLONG length);
	bool ReadTag(Tag& t, UINT32& previousTagSize, bool sync, bool& error);
	bool SyncReadTag(Tag& t, UINT32& previousTagSize);
	void LogTag(const Tag& t, const std::wstring& str);
	void LogTag(const Tag& t, const std::wstring& str, UINT32 previousTagSize);
	void LogTag(const Tag& t, const std::wstring& str, LONGLONG diff);

	struct AudioTag
	{
		BYTE SoundFormat;
		BYTE SoundRate;
		BYTE SoundSize;
		BYTE SoundType;
	};

	bool ReadTag(AudioTag& at);

	struct VideoTag
	{
		BYTE FrameType;
		BYTE CodecID;
	};

	bool ReadTag(VideoTag& vt);

	void NormalSeek(REFERENCE_TIME rt);
	void AlternateSeek(REFERENCE_TIME rt);

protected:
	CAutoPtr<CBaseSplitterFileEx> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CFLVSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr, utl::SyncLog& log);

protected:
	flv::FLVMetaData metadata_;
	LONGLONG currentPos_;
	LONGLONG positionDiff_;
	FLVConfig cfg_;

public:
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IFLVSplitterProp
	STDMETHOD(GetEncoder) (std::string* str);
	STDMETHOD(GetDatarate) (int* rate);
	STDMETHOD(GetFramerate) (int* rate);
	STDMETHOD(GetCurrentPos) (LONGLONG* pos);
	STDMETHOD(GetPositionDiff) (LONGLONG* diff);
	STDMETHOD(SetConfig) (const FLVConfig& cfg);
};



//class __declspec(uuid("C9ECE7B3-1D8E-41F5-9F24-B255DF16C087"))
class __declspec(uuid("C81DF2C6-B904-478c-99D1-498BC76B6340"))
CFLVSourceFilter : public CFLVSplitterFilter
{
public:
	CFLVSourceFilter(LPUNKNOWN pUnk, HRESULT* phr, utl::SyncLog& log);
};

//#include "../../transform/BaseVideoFilter/BaseVideoFilter.h"
