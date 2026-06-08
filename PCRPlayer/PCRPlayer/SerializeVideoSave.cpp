#include "stdafx.h"

namespace sl {

void put(XmlParser& root, const VideoConfig& video)
{
	auto p = root.put(L"video");

	p = root.put(L"video.procamp");
	p.put(L"init", video.procamp.init);
	p.put(L"brightness", video.procamp.brightness);
	p.put(L"contrast", video.procamp.contrast);
	p.put(L"hue", video.procamp.hue);
	p.put(L"saturation", video.procamp.saturation);

	p = root.put(L"video.color");
	p.put(L"nominal", video.nominal);

	p = root.put(L"video.resizer");
	p.put(L"type", video.resizer);
}

} // namespace sl
