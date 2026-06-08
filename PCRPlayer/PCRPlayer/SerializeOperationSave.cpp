#include "stdafx.h"

namespace sl {

void put(XmlParser& p, const sl::MouseOperation::Screen& data)
{
	p.put(L"invert", data.invert);
	p.put(L"wheel", data.wheel);
	p.put(L"mclick", data.mclick);
	p.put(L"ldblclick", data.ldblclick);
}

void put(XmlParser& p, const sl::MouseOperation::Bar& data)
{
	p.put(L"transfer", data.transfer);
	p.put(L"invert", data.invert);
	p.put(L"wheel", data.wheel);
	p.put(L"mclick", data.mclick);
}

void put(XmlParser& p, const sl::Variation::Value& data)
{
	p.put(L"normal", data.normal);
	p.put(L"low", data.low);
	p.put(L"high", data.high);
}


void put(XmlParser& root, const OperationConfig& operation)
{
	auto p = root.put(L"operation");

	p = root.put(L"operation.shortcut");
	p.put(L"movement", operation.shortcut.movement);
	for (auto it = operation.shortcut.data.begin(); it != operation.shortcut.data.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"id", it->id);
		item.put(L"key", it->key);
		item.put(L"mod", it->mod);
		item.put(L"gesture", it->gesture);
	}

	p = root.put(L"operation.mouse.online");
	put(p, operation.mouse.online);

	p = root.put(L"operation.mouse.offline");
	put(p, operation.mouse.offline);

	p = root.put(L"operation.mouse.status");
	put(p, operation.mouse.status);

	p = root.put(L"operation.mouse.seek");
	put(p, operation.mouse.seek);

	p = root.put(L"operation.mouse.bbs");
	put(p, operation.mouse.bbs);

	p = root.put(L"operation.variation.volume");
	put(p, operation.variation.volume);

	p = root.put(L"operation.variation.seek");
	put(p, operation.variation.seek);

	p = root.put(L"operation.variation.scroll");
	put(p, operation.variation.scroll);

	p = root.put(L"operation.window");
	p.put(L"frame", operation.window.frame);

	p = root.put(L"operation.window.snap");
	p.put(L"screen", operation.window.snap.screen);
	p.put(L"window", operation.window.snap.window);
}

} // namespace sl
