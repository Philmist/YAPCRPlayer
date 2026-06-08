#include "stdafx.h"

namespace sl {

void get(const XmlParser& p, sl::MouseOperation::Screen& data)
{
	p.get(L"invert", data.invert);
	p.get(L"wheel", data.wheel);
	p.get(L"mclick", data.mclick);
	p.get(L"ldblclick", data.ldblclick);
}

void get(const XmlParser& p, sl::MouseOperation::Bar& data)
{
	p.get(L"transfer", data.transfer);
	p.get(L"invert", data.invert);
	p.get(L"wheel", data.wheel);
	p.get(L"mclick", data.mclick);
}

void get(const XmlParser& p, sl::Variation::Value& data)
{
	p.get(L"normal", data.normal);
	p.get(L"low", data.low);
	p.get(L"high", data.high);
}


void get(const XmlParser& root, OperationConfig& operation)
{
	auto p = root.get(L"operation");

	p = root.get(L"operation.shortcut");
	p.get(L"movement", operation.shortcut.movement);
	p.foreach(L"item", [&](const XmlParser& item){
		utl::Shortcut tmp;
		item.get(L"id", tmp.id, 0UL);
		item.get(L"key", tmp.key, 0UL);
		item.get(L"mod", tmp.mod, 0UL);
		item.get(L"gesture", tmp.gesture, L"");

		for (auto it = operation.shortcut.data.begin(); it != operation.shortcut.data.end(); ++it)
		{
			if (it->id == tmp.id)
			{
				operation.shortcut.reset(tmp.key, tmp.mod, tmp.gesture);
				*it = tmp;
			}
		}
	});

	p = root.get(L"operation.mouse.online");
	get(p, operation.mouse.online);

	p = root.get(L"operation.mouse.offline");
	get(p, operation.mouse.offline);

	p = root.get(L"operation.mouse.status");
	get(p, operation.mouse.status);

	p = root.get(L"operation.mouse.seek");
	get(p, operation.mouse.seek);

	p = root.get(L"operation.mouse.bbs");
	get(p, operation.mouse.bbs);

	p = root.get(L"operation.variation.volume");
	get(p, operation.variation.volume);

	p = root.get(L"operation.variation.seek");
	get(p, operation.variation.seek);

	p = root.get(L"operation.variation.scroll");
	get(p, operation.variation.scroll);

	p = root.get(L"operation.window");
	p.get(L"frame", operation.window.frame);

	p = root.get(L"operation.window.snap");
	p.get(L"screen", operation.window.snap.screen);
	p.get(L"window", operation.window.snap.window);
}

} // namespace sl
