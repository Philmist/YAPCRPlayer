#include "stdafx.h"

namespace sl {

void get(const XmlParser& p, StatusConfig& data)
{
	p.get(L"marginX", data.marginX);
	p.get(L"marginY", data.marginY);
	p.get(L"text", data.text);
	p.get(L"color", data.color);

	auto f = p.get(L"font");
	get(f, data.font);
}

void get(const XmlParser& p, SeekConfig& data)
{
	p.get(L"marginX", data.marginX);
	p.get(L"marginY", data.marginY);
	p.get(L"knob", data.knob);
	p.get(L"text", data.text);
	p.get(L"color", data.color);

	auto f = p.get(L"font");
	get(f, data.font);
}

void get(const XmlParser& p, HintConfig& data)
{
	p.get(L"marginX", data.marginX);
	p.get(L"marginY", data.marginY);

	auto o = p.get(L"opaque");
	o.get(L"enable", data.opaque.enable);
	o.get(L"text", data.opaque.text);
	o.get(L"back", data.opaque.back);

	p.get(L"text", data.text);
	p.get(L"color", data.color);

	auto f = p.get(L"font");
	get(f, data.font);
}

void get(const XmlParser& p, ChannelConfig& data)
{
	p.get(L"marginX", data.marginX);
	p.get(L"marginY", data.marginY);

	auto o = p.get(L"opaque");
	o.get(L"enable", data.opaque.enable);
	o.get(L"text", data.opaque.text);
	o.get(L"back", data.opaque.back);

	p.get(L"text", data.text);
	p.get(L"color", data.color);

	auto f = p.get(L"font");
	get(f, data.font);
}

void get(const XmlParser& p, BBSConfig& data)
{
	p.get(L"marginX", data.marginX);
	p.get(L"marginY", data.marginY);
	p.get(L"text", data.text);
	p.get(L"color", data.color);

	auto f = p.get(L"font");
	get(f, data.font);

	auto t = p.get(L"title");
	t.get(L"bold", data.title.bold);
	t.get(L"text", data.title.text);

	auto e = p.get(L"edit.font");
	get(e, data.edit.font);
}

void get(const XmlParser& p, ResConfig& data)
{
	auto x = p.get(L"format");
	x.get(L"type", data.format.type);
	x.get(L"space", data.format.space);
	x.get(L"width", data.format.width);
	x.get(L"punct", data.format.punct);
	x.get(L"second", data.format.second);

	auto f = x.get(L"font");
	get(f, data.format.font);

	x = p.get(L"id");
	x.get(L"enable", data.id.enable);
	x.get(L"threshold", data.id.threshold);

	x = p.get(L"color");
	x.get(L"text", data.color.text);
	x.get(L"link", data.color.link);
	x.get(L"sage", data.color.sage);
	x.get(L"bold", data.color.bold);

	x = p.get(L"box");
	x.get(L"marginX", data.box.marginX);
	x.get(L"marginY", data.box.marginY);
	x.get(L"space", data.box.space);
	x.get(L"punct", data.box.punct);
	x.get(L"separate", data.box.separate);
	x.get(L"color", data.box.color);
	x.get(L"latest", data.box.latest);

	x = p.get(L"back");
	x.get(L"marginX", data.back.marginX);
	x.get(L"marginY", data.back.marginY);

	auto o = x.get(L"opaque");
	o.get(L"enable", data.back.opaque.enable);
	o.get(L"text", data.back.opaque.text);
	o.get(L"back", data.back.opaque.back);

	x.get(L"color", data.back.color);
}

void get(const XmlParser& p, InformationConfig::Channel& data)
{
	p.get(L"name", data.name);
	p.get(L"type", data.type);
	p.get(L"genre", data.genre);
	p.get(L"desc", data.desc);
	p.get(L"comment", data.comment);
	p.get(L"artist", data.artist);
	p.get(L"title", data.title);
	p.get(L"album", data.album);
}

void get(const XmlParser& root, DisplayConfig& display)
{
	auto p = root.get(L"display");

	p = root.get(L"display.window.zoom");
	if (p)
	{
		display.window.zoom.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			WindowZoom tmp;
			item.get(L"enable", tmp.enable);
			item.get(L"zoom", tmp.zoom);

			display.window.zoom.push_back(tmp);
		});
	}

	p = root.get(L"display.window.size");
	if (p)
	{
		display.window.size.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			WindowSize tmp;
			item.get(L"enable", tmp.enable);
			item.get(L"width", tmp.width);
			item.get(L"height", tmp.height);

			display.window.size.push_back(tmp);
		});
	}

	p = root.get(L"display.window.aspect");
	if (p)
	{
		display.window.aspect.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			WindowAspect tmp;
			item.get(L"ax", tmp.ax);
			item.get(L"ay", tmp.ay);

			display.window.aspect.push_back(tmp);
		});
	}

	p = root.get(L"display.datetime");
	p.get(L"type", display.datetime.type);
	p.get(L"exp", display.datetime.exp);
	p.get(L"sub", display.datetime.sub);

	p = root.get(L"display.datetime.format");
	p.get(L"date", display.datetime.dateFormat);
	p.get(L"time", display.datetime.timeFormat);


	p = root.get(L"display.information.status");
	get(p, display.information.status);

	p = root.get(L"display.information.panel");
	get(p, display.information.panel);

	p = root.get(L"display.information.name");
	get(p, display.information.name);

	p = root.get(L"display.information.type");
	get(p, display.information.type);

	p = root.get(L"display.information.genre");
	get(p, display.information.genre);

	p = root.get(L"display.information.desc");
	get(p, display.information.desc);

	p = root.get(L"display.information.comment");
	get(p, display.information.comment);

	p = root.get(L"display.information.artist");
	get(p, display.information.artist);

	p = root.get(L"display.information.title");
	get(p, display.information.title);

	p = root.get(L"display.information.album");
	get(p, display.information.album);


	p = root.get(L"display.status");
	get(p, display.status);

	p = root.get(L"display.seek");
	get(p, display.seek);

	p = root.get(L"display.hint");
	get(p, display.hint);

	p = root.get(L"display.channel");
	get(p, display.channel);

	p = root.get(L"display.bbs");
	get(p, display.bbs);

	p = root.get(L"display.res");
	get(p, display.res);



	p = root.get(L"display.custom.status");
	if (p)
	{
		display.custom.status.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			std::wstring first;
			StatusConfig second;
			item.get(L"name", first, L"カスタム");
			get(item, second);

			display.custom.status.push_back(std::make_pair(first, second));
		});
	}

	p = root.get(L"display.custom.seek");
	if (p)
	{
		display.custom.seek.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			std::wstring first;
			SeekConfig second;
			item.get(L"name", first, L"カスタム");
			get(item, second);

			display.custom.seek.push_back(std::make_pair(first, second));
		});
	}

	p = root.get(L"display.custom.hint");
	if (p)
	{
		display.custom.hint.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			std::wstring first;
			HintConfig second;
			item.get(L"name", first, L"カスタム");
			get(item, second);

			display.custom.hint.push_back(std::make_pair(first, second));
		});
	}

	p = root.get(L"display.custom.channel");
	if (p)
	{
		display.custom.channel.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			std::wstring first;
			ChannelConfig second;
			item.get(L"name", first, L"カスタム");
			get(item, second);

			display.custom.channel.push_back(std::make_pair(first, second));
		});
	}

	p = root.get(L"display.custom.bbs");
	if (p)
	{
		display.custom.bbs.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			std::wstring first;
			BBSConfig second;
			item.get(L"name", first, L"カスタム");
			get(item, second);

			display.custom.bbs.push_back(std::make_pair(first, second));
		});
	}

	p = root.get(L"display.custom.res");
	if (p)
	{
		display.custom.res.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			std::wstring first;
			ResConfig second;
			item.get(L"name", first, L"カスタム");
			get(item, second);

			display.custom.res.push_back(std::make_pair(first, second));
		});
	}
}


} // namespace sl