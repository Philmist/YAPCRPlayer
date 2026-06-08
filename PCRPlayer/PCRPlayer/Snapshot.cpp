
#include "stdafx.h"

namespace utl {

std::wstring createSnapshotFilename(int format)
{
	SYSTEMTIME time;
	::memset(&time, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&time);

	CString tmp;
	tmp.Format(L"%04d%02d%02d_%02d%02d%02d_%03d",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	switch (format)
	{
	case GRAPHIC_FORMAT_JPG:
		tmp += L".jpg";
		break;

	case GRAPHIC_FORMAT_PNG:
		tmp += L".png";
		break;

	case GRAPHIC_FORMAT_BMP:
	default:
		tmp += L".bmp";
		break;
	}
	return std::wstring((LPCWSTR)tmp);
}

bool Snapshot::save(const std::wstring& path, int format, int jpegQuality, UINT width, UINT height, bool fit)
{
	using namespace Gdiplus;

	if (!info_ || !dib_) { return false; }

	Bitmap* src = Bitmap::FromBITMAPINFO(info_, dib_);
	if (!src) { return false; }

	if (width == 0 || height == 0 ||
		(fit && (width < src->GetWidth() || height < src->GetHeight())))
	{
		width = src->GetWidth();
		height = src->GetHeight();
	}

	Bitmap bmp(width, height, src->GetPixelFormat());
	Graphics g(&bmp);

	g.SetSmoothingMode(SmoothingModeNone);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	g.DrawImage(src, Rect(0, 0, width, height), 0, 0, src->GetWidth(), src->GetHeight(), UnitPixel);

	// EncoderParametersとEncoderParameterで5個のパラメーターをあらかじめ確保
	EncoderParameters *eps = static_cast<EncoderParameters*>(
			malloc(sizeof(EncoderParameters) + (sizeof(EncoderParameter) * 4)));
	if (!eps) { return false; }

	// ImageFormatPNG
	CLSID clsid = CLSID_NULL;
	ULONG quality = jpegQuality; // 0～100
	ULONG compress = 100; // 0～100
	ULONG colordepth = 32; // 1, 4, 8, 24, 32

	switch (format)
	{
	case GRAPHIC_FORMAT_JPG:
		eps->Count = 1;

		eps->Parameter[0].Guid = EncoderQuality;
		eps->Parameter[0].Type = EncoderParameterValueTypeLong;
		eps->Parameter[0].NumberOfValues = 1;
		eps->Parameter[0].Value = &quality;

		getEncoderClsid(L"image/jpeg", &clsid);
		break;

	case GRAPHIC_FORMAT_PNG:
		eps->Count = 2;

		eps->Parameter[0].Guid = EncoderCompression;
		eps->Parameter[0].Type = EncoderParameterValueTypeLong;
		eps->Parameter[0].NumberOfValues = 1;
		eps->Parameter[0].Value = &compress;

		eps->Parameter[1].Guid = EncoderColorDepth;
		eps->Parameter[1].Type = EncoderParameterValueTypeLong;
		eps->Parameter[1].NumberOfValues = 1;
		eps->Parameter[1].Value = &colordepth;

		getEncoderClsid(L"image/png", &clsid);
		break;

	case GRAPHIC_FORMAT_BMP:
	default:
		eps->Count = 1;

		eps->Parameter[0].Guid = EncoderColorDepth;
		eps->Parameter[0].Type = EncoderParameterValueTypeLong;
		eps->Parameter[0].NumberOfValues = 1;
		eps->Parameter[0].Value = &colordepth;

		getEncoderClsid(L"image/bmp", &clsid);
		break;
	}

	bmp.Save(path.c_str(), &clsid, eps);
	free(eps);
	delete src;
	return true;
}

bool Snapshot::getEncoderClsid(const std::wstring& format, CLSID* clsid)
{
	using namespace Gdiplus;
	Status stat;
    UINT num = 0, size = 0;

    stat = GetImageEncodersSize(&num, &size);
    if (stat != Ok || size == 0) { return false; }

    ImageCodecInfo* info = (ImageCodecInfo*)malloc(size);
    if (!info) { return false; }

    stat = GetImageEncoders(num, size, info);
	if (stat != Ok)
	{
		free(info);
		return false;
	}

    for(UINT i = 0; i < num; ++i)
    {
		if (wcscmp(info[i].MimeType, format.c_str()) == 0)
        {
            *clsid = info[i].Clsid;
            free(info);
            return true;
        }
    }

    free(info);
    return false;
}


} // namespace utl
