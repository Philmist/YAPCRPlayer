#pragma once

#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")

namespace utl {

// GDI+èâä˙âª/èIóπ
class GdiplusInit {
private:
	ULONG_PTR gdiplusToken;

public:
	GdiplusInit()
	{
		using namespace Gdiplus;
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}
	virtual ~GdiplusInit()
	{
		using namespace Gdiplus;
		GdiplusShutdown(gdiplusToken);
	}
};

enum GRAPHIC_FORMAT {
	GRAPHIC_FORMAT_BMP = 0,
	GRAPHIC_FORMAT_JPG = 1,
	GRAPHIC_FORMAT_PNG = 2,
};

std::wstring createSnapshotFilename(int format);

class Snapshot {
	BITMAPINFO* info_;
	BYTE* dib_;
public:
	Snapshot(BITMAPINFO* info, BYTE* dib)
		: info_(info), dib_(dib)
	{}
	virtual ~Snapshot(){}

	bool save(const std::wstring& path, int format, int jpegQuality, UINT width = 0, UINT height = 0, bool fit = false);

private:
	bool getEncoderClsid(const std::wstring& format, CLSID* clsid);
};

} // namespace utl