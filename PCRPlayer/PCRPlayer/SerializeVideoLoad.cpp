#include "stdafx.h"

namespace sl {

void get(const XmlParser& root, VideoConfig& video)
{
	auto p = root.get(L"video");

	p = root.get(L"video.procamp");
	p.get(L"init", video.procamp.init);
	p.get(L"brightness", video.procamp.brightness);
	p.get(L"contrast", video.procamp.contrast);
	p.get(L"hue", video.procamp.hue);
	p.get(L"saturation", video.procamp.saturation);

	p = root.get(L"video.color");
	p.get(L"nominal", video.nominal);

	p = root.get(L"video.resizer");
	p.get(L"type", video.resizer);
}

} // namespace sl
