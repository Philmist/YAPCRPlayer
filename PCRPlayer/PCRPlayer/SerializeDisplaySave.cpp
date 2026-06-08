#include "stdafx.h"

namespace sl {

void put(XmlParser& p, const StatusConfig& data)
{
	p.put(L"marginX", data.marginX);
	p.put(L"marginY", data.marginY);
	p.put(L"text", data.text);
	p.put(L"color", data.color);

	auto f = p.put(L"font");
	put(f, data.font);
}

void put(XmlParser& p, const SeekConfig& data)
{
	p.put(L"marginX", data.marginX);
	p.put(L"marginY", data.marginY);
	p.put(L"knob", data.knob);
	p.put(L"text", data.text);
	p.put(L"color", data.color);

	auto f = p.put(L"font");
	put(f, data.font);
}

void put(XmlParser& p, const HintConfig& data)
{
	p.put(L"marginX", data.marginX);
	p.put(L"marginY", data.marginY);

	auto o = p.put(L"opaque");
	o.put(L"enable", data.opaque.enable);
	o.put(L"text", data.opaque.text);
	o.put(L"back", data.opaque.back);

	p.put(L"text", data.text);
	p.put(L"color", data.color);

	auto f = p.put(L"font");
	put(f, data.font);
}

void put(XmlParser& p, const ChannelConfig& data)
{
	p.put(L"marginX", data.marginX);
	p.put(L"marginY", data.marginY);

	auto o = p.put(L"opaque");
	o.put(L"enable", data.opaque.enable);
	o.put(L"text", data.opaque.text);
	o.put(L"back", data.opaque.back);

	p.put(L"text", data.text);
	p.put(L"color", data.color);

	auto f = p.put(L"font");
	put(f, data.font);
}

void put(XmlParser& p, const BBSConfig& data)
{
	p.put(L"marginX", data.marginX);
	p.put(L"marginY", data.marginY);
	p.put(L"text", data.text);
	p.put(L"color", data.color);

	auto f = p.put(L"font");
	put(f, data.font);

	auto t = p.put(L"title");
	t.put(L"bold", data.title.bold);
	t.put(L"text", data.title.text);

	auto e = p.put(L"edit.font");
	put(e, data.edit.font);
}

void put(XmlParser& p, const ResConfig& data)
{
	auto x = p.put(L"format");
	x.put(L"type", data.format.type);
	x.put(L"space", data.format.space);
	x.put(L"width", data.format.width);
	x.put(L"punct", data.format.punct);
	x.put(L"second", data.format.second);

	auto f = x.put(L"font");
	put(f, data.format.font);

	x = p.put(L"id");
	x.put(L"enable", data.id.enable);
	x.put(L"threshold", data.id.threshold);

	x = p.put(L"color");
	x.put(L"text", data.color.text);
	x.put(L"link", data.color.link);
	x.put(L"sage", data.color.sage);
	x.put(L"bold", data.color.bold);

	x = p.put(L"box");
	x.put(L"marginX", data.box.marginX);
	x.put(L"marginY", data.box.marginY);
	x.put(L"space", data.box.space);
	x.put(L"punct", data.box.punct);
	x.put(L"separate", data.box.separate);
	x.put(L"color", data.box.color);
	x.put(L"latest", data.box.latest);

	x = p.put(L"back");
	x.put(L"marginX", data.back.marginX);
	x.put(L"marginY", data.back.marginY);

	auto o = x.put(L"opaque");
	o.put(L"enable", data.back.opaque.enable);
	o.put(L"text", data.back.opaque.text);
	o.put(L"back", data.back.opaque.back);

	x.put(L"color", data.back.color);
}

void put(XmlParser& p, const InformationConfig::Channel& data)
{
	p.put(L"name", data.name);
	p.put(L"type", data.type);
	p.put(L"genre", data.genre);
	p.put(L"desc", data.desc);
	p.put(L"comment", data.comment);
	p.put(L"artist", data.artist);
	p.put(L"title", data.title);
	p.put(L"album", data.album);
}



// •\Ž¦Ý’è

void put(XmlParser& root, const DisplayConfig& display)
{
	auto p = root.put(L"display");

	p = root.put(L"display.window.zoom");
	for (auto it = display.window.zoom.begin(); it != display.window.zoom.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"enable", it->enable);
		item.put(L"zoom", it->zoom);
	}

	p = root.put(L"display.window.size");
	for (auto it = display.window.size.begin(); it != display.window.size.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"enable", it->enable);
		item.put(L"width", it->width);
		item.put(L"height", it->height);
	}

	p = root.put(L"display.window.aspect");
	for (auto it = display.window.aspect.begin(); it != display.window.aspect.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"ax", it->ax);
		item.put(L"ay", it->ay);
	}

	p = root.put(L"display.datetime");
	p.put(L"type", display.datetime.type);
	p.put(L"exp", display.datetime.exp);
	p.put(L"sub", display.datetime.sub);

	p = root.put(L"display.datetime.format");
	p.put(L"date", display.datetime.dateFormat);
	p.put(L"time", display.datetime.timeFormat);


	p = root.put(L"display.information.status");
	put(p, display.information.status);

	p = root.put(L"display.information.panel");
	put(p, display.information.panel);

	p = root.put(L"display.information.name");
	put(p, display.information.name);

	p = root.put(L"display.information.type");
	put(p, display.information.type);

	p = root.put(L"display.information.genre");
	put(p, display.information.genre);

	p = root.put(L"display.information.desc");
	put(p, display.information.desc);

	p = root.put(L"display.information.comment");
	put(p, display.information.comment);

	p = root.put(L"display.information.artist");
	put(p, display.information.artist);

	p = root.put(L"display.information.title");
	put(p, display.information.title);

	p = root.put(L"display.information.album");
	put(p, display.information.album);


	p = root.put(L"display.status");
	put(p, display.status);

	p = root.put(L"display.seek");
	put(p, display.seek);

	p = root.put(L"display.hint");
	put(p, display.hint);

	p = root.put(L"display.channel");
	put(p, display.channel);

	p = root.put(L"display.bbs");
	put(p, display.bbs);

	p = root.put(L"display.res");
	put(p, display.res);


	p = root.put(L"display.custom.status");
	for (auto it = display.custom.status.begin(); it != display.custom.status.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"name", it->first);
		put(item, it->second);
	}

	p = root.put(L"display.custom.seek");
	for (auto it = display.custom.seek.begin(); it != display.custom.seek.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"name", it->first);
		put(item, it->second);
	}

	p = root.put(L"display.custom.hint");
	for (auto it = display.custom.hint.begin(); it != display.custom.hint.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"name", it->first);
		put(item, it->second);
	}

	p = root.put(L"display.custom.channel");
	for (auto it = display.custom.channel.begin(); it != display.custom.channel.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"name", it->first);
		put(item, it->second);
	}

	p = root.put(L"display.custom.bbs");
	for (auto it = display.custom.bbs.begin(); it != display.custom.bbs.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"name", it->first);
		put(item, it->second);
	}

	p = root.put(L"display.custom.res");
	for (auto it = display.custom.res.begin(); it != display.custom.res.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"name", it->first);
		put(item, it->second);
	}
}

} // namespace sl