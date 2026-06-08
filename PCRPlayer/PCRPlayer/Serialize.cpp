#include "stdafx.h"


namespace sl {

// LOGFONTèoóÕ
void put(XmlParser& p, const LOGFONT& font)
{
	p.put(L"point", utl::heightToPoint(font.lfHeight));

	p.put(L"lfHeight", font.lfHeight);
	p.put(L"lfWidth", font.lfWidth);
	p.put(L"lfEscapement", font.lfEscapement);
	p.put(L"lfOrientation", font.lfOrientation);
	p.put(L"lfWeight", font.lfWeight);
	p.put(L"lfItalic", font.lfItalic);
	p.put(L"lfUnderline", font.lfUnderline);
	p.put(L"lfStrikeOut", font.lfStrikeOut);
	p.put(L"lfCharSet", font.lfCharSet);
	p.put(L"lfOutPrecision", font.lfOutPrecision);
	p.put(L"lfClipPrecision", font.lfClipPrecision);
	p.put(L"lfQuality", font.lfQuality);
	p.put(L"lfPitchAndFamily", font.lfPitchAndFamily);
	p.put(L"lfFaceName", font.lfFaceName);
}

// LOGFONTì¸óÕ
void get(const XmlParser& p, LOGFONT& font)
{
	LOGFONT def = font;
	try
	{
		int point = 9;
		if (p.get(L"point", point))
		{
			font.lfHeight = utl::pointToHeight(point);
		}
		else
		{
			p.getx(L"lfHeight", font.lfHeight);
		}
		p.getx(L"lfWidth", font.lfWidth);
		p.getx(L"lfEscapement", font.lfEscapement);
		p.getx(L"lfOrientation", font.lfOrientation);
		p.getx(L"lfWeight", font.lfWeight);
		p.getx(L"lfItalic", font.lfItalic);
		p.getx(L"lfUnderline", font.lfUnderline);
		p.getx(L"lfStrikeOut", font.lfStrikeOut);
		p.getx(L"lfCharSet", font.lfCharSet);
		p.getx(L"lfOutPrecision", font.lfOutPrecision);
		p.getx(L"lfClipPrecision", font.lfClipPrecision);
		p.getx(L"lfQuality", font.lfQuality);
		p.getx(L"lfPitchAndFamily", font.lfPitchAndFamily);

		std::wstring facename;
		p.getx(L"lfFaceName", facename);

		wcsncpy_s(font.lfFaceName, facename.c_str(), LF_FACESIZE);
		font.lfFaceName[LF_FACESIZE - 1] = L'\0';
	}
	catch (...)
	{
		font = def;
	}
}

void put(XmlParser& p, const std::vector<utl::ReplaceElement>& data)
{
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"enable", it->enable);
		item.put(L"desc", it->desc);
		item.put(L"regex", it->regex);
		item.put(L"format", it->format);
		item.put(L"icase", it->icase);
	}
}

void get(const XmlParser& p, std::vector<utl::ReplaceElement>& data)
{
	if (p)
	{
		data.clear();
		p.foreach(L"item", [&](const XmlParser& item) {
			utl::ReplaceElement tmp;
			item.get(L"enable", tmp.enable);
			item.get(L"desc", tmp.desc);
			item.get(L"regex", tmp.regex);
			item.get(L"format", tmp.format);
			item.get(L"icase", tmp.icase);
			data.push_back(tmp);
		});
	}
}



void SerializeValues::save(const std::wstring& current)
{
	XmlParser xml;
	XmlParser p;

	XmlParser root = xml.put(L"root");

	p = root.put(L"recent");
	p.put(L"path", recent.path);
	p.put(L"name", recent.name);
	p.put(L"contact", recent.contact);
	p.put(L"commandline", recent.commandline);

	//-----------------------------------------------

	p = root.put(L"placement");
	p.put(L"left", placement.left);
	p.put(L"top", placement.top);
	p.put(L"right", placement.right);
	p.put(L"bottom", placement.bottom);

	//-----------------------------------------------

	p = root.put(L"window");
	p.put(L"aspect", window.aspect);
	p.put(L"priority", window.priority);
	p.put(L"stop", window.stop);
	p.put(L"zoom", window.zoom);

	p = root.put(L"window.show");
	p.put(L"topmost", window.show.topmost);
	p.put(L"title", window.show.title);
	p.put(L"frame", window.show.frame);
	p.put(L"bbs", window.show.bbs);
	p.put(L"seek", window.show.seek);
	p.put(L"status", window.show.status);
	p.put(L"edit", window.show.edit);
	p.put(L"browser", window.show.browser);
	p.put(L"side", window.show.side);
	p.put(L"idealX", window.show.idealX);
	p.put(L"idealY", window.show.idealY);

	p = root.put(L"window.ratio");
	p.put(L"fixed", window.ratio.fixed);
	p.put(L"play", window.ratio.play);
	p.put(L"height", window.ratio.height);
	p.put(L"inscribe", window.ratio.inscribe);

	p = root.put(L"window.snap");
	p.put(L"screen", window.snap.screen);
	p.put(L"window", window.snap.window);

	p = root.put(L"window.fullscreen");
	p.put(L"hide", window.fullscreen.hide);
	p.put(L"floating", window.fullscreen.floating);
	p.put(L"mouse", window.fullscreen.mouse);
	p.put(L"topmost", window.fullscreen.topmost);

	p = root.put(L"window.maximize");
	p.put(L"fullscreen", window.maximize.fullscreen);
	p.put(L"taskbar", window.maximize.taskbar);

	p = root.put(L"window.title");
	p.put(L"player", window.title.player);
	p.put(L"size", window.title.size);
	p.put(L"source", window.title.source);

	p = root.put(L"window.status");
	p.put(L"fps", window.status.fps);
	p.put(L"size", window.status.size);
	p.put(L"time", window.status.time);
	p.put(L"play", window.status.play);

	p = root.put(L"window.status.channel");
	p.put(L"name", window.status.channel.name);
	p.put(L"type", window.status.channel.type);
	p.put(L"genre", window.status.channel.genre);
	p.put(L"comment", window.status.channel.comment);
	p.put(L"playing", window.status.channel.playing);
	p.put(L"relay", window.status.channel.relay);

	p = root.put(L"window.seek");
	p.put(L"filename", window.seek.filename);

	p = root.put(L"window.bbs");
	p.put(L"counter", window.bbs.counter);


	p = root.put(L"play");
	p.put(L"mode", play.mode);

	p = root.put(L"audio");
	p.put(L"volume", audio.volume);
	p.put(L"balance", audio.balance);
	p.put(L"mute", audio.mute);

	p = root.put(L"format.enable");
	p.put(L"name", format.name.enable);
	p.put(L"bitrate", format.bitrate.enable);
	p.put(L"fps", format.fps.enable);
	p.put(L"size", format.size.enable);
	p.put(L"time", format.time.enable);
	p.put(L"volume", format.volume.enable);

	p = root.put(L"format.type");
	p.put(L"name", format.name.type);
	p.put(L"bitrate", format.bitrate.type);
	p.put(L"fps", format.fps.type);
	p.put(L"size", format.size.type);
	p.put(L"time", format.time.type);
	p.put(L"volume", format.volume.type);


	p = root.put(L"dialog.bbs");
	p.put(L"sage", dialog.bbs.sage);

	p = root.put(L"dialog.thread");
	p.put(L"width", dialog.thread.width);
	p.put(L"height", dialog.thread.height);
	p.put(L"column", dialog.thread.column);
	p.put(L"stop", dialog.thread.stop);
	p.put(L"stretch", dialog.thread.stretch);
	p.put(L"datetime", dialog.thread.datetime);
	p.put(L"close", dialog.thread.close);

	p = root.put(L"dialog.thread.links");
	for (auto it = dialog.thread.links.begin(); it != dialog.thread.links.end(); ++it)
	{
		auto item = p.add(L"item");
		put(item, *it);
	}

	p = root.put(L"dialog.thread.threads");
	for (auto it = dialog.thread.threads.begin(); it != dialog.thread.threads.end(); ++it)
	{
		auto item = p.add(L"item");
		put(item, *it);
	}

	p = root.put(L"dialog.file");
	p.put(L"width", dialog.file.width);
	p.put(L"height", dialog.file.height);

	p = root.put(L"dialog.search");
	p.put(L"width", dialog.search.width);
	p.put(L"height", dialog.search.height);

	p = root.put(L"dialog.extract");
	p.put(L"width", dialog.extract.width);
	p.put(L"height", dialog.extract.height);

	p = root.put(L"dialog.log");
	p.put(L"graph", dialog.log.graph);
	p.put(L"filter", dialog.log.filter);
	p.put(L"bbs", dialog.log.bbs);
	p.put(L"peercast", dialog.log.peercast);
	p.put(L"none", dialog.log.none);

	//-----------------------------------------------

	p = root.put(L"option");
	auto x = p.put(L"select");
	x.put(L"option", option.select.option);
	x.put(L"network", option.select.network);
	x.put(L"type", option.select.type);
	x.put(L"information", option.select.information);
	x.put(L"res", option.select.res);

	x = p.put(L"select.bbs");
	x.put(L"common", option.select.bbs.common);
	x.put(L"bar", option.select.bbs.bar);
	x.put(L"browser", option.select.bbs.browser);

	put(root, video);
	put(root, detail);
	put(root, operation);
	put(root, display);

	root.put(L"initial", false);

	xml.write(current + L"PCRPlayer.xml");
}

//==================================================================================

void SerializeValues::load(const std::wstring& current)
{
	XmlParser xml(current + L"PCRPlayer.xml");
	XmlParser p;

	XmlParser root = xml.get(L"root");

	p = root.get(L"recent");
	p.get(L"path", recent.path);
	p.get(L"name", recent.name);
	p.get(L"contact", recent.contact);
	p.get(L"commandline", recent.commandline);

	//-----------------------------------------------

	try {
		p = root.get(L"placement");
		p.getx(L"left", placement.left);
		p.getx(L"top", placement.top);
		p.getx(L"right", placement.right);
		p.getx(L"bottom", placement.bottom);
	}
	catch (...) {
		placement = utl::defWindowRect(480, 360);
	}

	//-----------------------------------------------

	p = root.get(L"window");
	p.get(L"aspect", window.aspect);
	p.get(L"priority", window.priority);
	p.get(L"stop", window.stop);
	p.get(L"zoom", window.zoom);

	p = root.get(L"window.show");
	p.get(L"topmost", window.show.topmost);
	p.get(L"title", window.show.title);
	p.get(L"frame", window.show.frame);
	p.get(L"bbs", window.show.bbs);
	p.get(L"seek", window.show.seek);
	p.get(L"status", window.show.status);
	p.get(L"edit", window.show.edit);
	p.get(L"browser", window.show.browser);
	p.get(L"side", window.show.side);
	p.get(L"idealX", window.show.idealX);
	p.get(L"idealY", window.show.idealY);

	p = root.get(L"window.ratio");
	p.get(L"fixed", window.ratio.fixed);
	p.get(L"play", window.ratio.play);
	p.get(L"height", window.ratio.height);
	p.get(L"inscribe", window.ratio.inscribe);

	p = root.get(L"window.snap");
	p.get(L"screen", window.snap.screen);
	p.get(L"window", window.snap.window);

	p = root.get(L"window.fullscreen");
	p.get(L"hide", window.fullscreen.hide);
	p.get(L"floating", window.fullscreen.floating);
	p.get(L"mouse", window.fullscreen.mouse);
	p.get(L"topmost", window.fullscreen.topmost);

	p = root.get(L"window.maximize");
	p.get(L"fullscreen", window.maximize.fullscreen);
	p.get(L"taskbar", window.maximize.taskbar);

	p = root.get(L"window.title");
	p.get(L"player", window.title.player);
	p.get(L"size", window.title.size);
	p.get(L"source", window.title.source);

	p = root.get(L"window.status");
	p.get(L"fps", window.status.fps);
	p.get(L"size", window.status.size);
	p.get(L"time", window.status.time);
	p.get(L"play", window.status.play);

	p = root.get(L"window.status.channel");
	p.get(L"name", window.status.channel.name);
	p.get(L"type", window.status.channel.type);
	p.get(L"genre", window.status.channel.genre);
	p.get(L"comment", window.status.channel.comment);
	p.get(L"playing", window.status.channel.playing);
	p.get(L"relay", window.status.channel.relay);

	p = root.get(L"window.seek");
	p.get(L"filename", window.seek.filename);

	p = root.get(L"window.bbs");
	p.get(L"counter", window.bbs.counter);


	p = root.get(L"play");
	p.get(L"mode", play.mode);

	p = root.get(L"audio");
	p.get(L"volume", audio.volume);
	p.get(L"balance", audio.balance);
	p.get(L"mute", audio.mute);

	p = root.get(L"format.enable");
	p.get(L"name", format.name.enable);
	p.get(L"bitrate", format.bitrate.enable);
	p.get(L"fps", format.fps.enable);
	p.get(L"size", format.size.enable);
	p.get(L"time", format.time.enable);
	p.get(L"volume", format.volume.enable);

	p = root.get(L"format.type");
	p.get(L"name", format.name.type);
	p.get(L"bitrate", format.bitrate.type);
	p.get(L"fps", format.fps.type);
	p.get(L"size", format.size.type);
	p.get(L"time", format.time.type);
	p.get(L"volume", format.volume.type);

	p = root.get(L"dialog.bbs");
	p.get(L"sage", dialog.bbs.sage);

	p = root.get(L"dialog.thread");
	p.get(L"width", dialog.thread.width);
	p.get(L"height", dialog.thread.height);
	p.get(L"column", dialog.thread.column);
	p.get(L"stop", dialog.thread.stop);
	p.get(L"stretch", dialog.thread.stretch);
	p.get(L"datetime", dialog.thread.datetime);
	p.get(L"close", dialog.thread.close);

	p = root.get(L"dialog.thread.links");
	if (p)
	{
		dialog.thread.links.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			bbs::Link link;
			get(item, link);

			dialog.thread.links.push_back(link);
		});
	}

	p = root.get(L"dialog.thread.threads");
	if (p)
	{
		dialog.thread.threads.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			bbs::Link link;
			get(item, link);

			dialog.thread.threads.push_back(link);
		});
	}

	p = root.get(L"dialog.file");
	p.get(L"width", dialog.file.width);
	p.get(L"height", dialog.file.height);

	p = root.get(L"dialog.search");
	p.get(L"width", dialog.search.width);
	p.get(L"height", dialog.search.height);

	p = root.get(L"dialog.extract");
	p.get(L"width", dialog.extract.width);
	p.get(L"height", dialog.extract.height);

	p = root.get(L"dialog.log");
	p.get(L"graph", dialog.log.graph);
	p.get(L"filter", dialog.log.filter);
	p.get(L"bbs", dialog.log.bbs);
	p.get(L"peercast", dialog.log.peercast);
	p.get(L"none", dialog.log.none);

	//-----------------------------------------------

	p = root.get(L"option");
	auto x = p.get(L"select");
	x.get(L"option", option.select.option);
	x.get(L"network", option.select.network);
	x.get(L"type", option.select.type);
	x.get(L"information", option.select.information);
	x.get(L"res", option.select.res);

	x = p.get(L"select.bbs");
	x.get(L"common", option.select.bbs.common);
	x.get(L"bar", option.select.bbs.bar);
	x.get(L"browser", option.select.bbs.browser);

	get(root, video);
	get(root, detail);
	get(root, operation);
	get(root, display);

	bool initial = true;
	root.get(L"initial", initial);
	if (initial)
	{
		detail.network = NetworkConfig();
		detail.type = TypeConfig();
	}
}

} // namespace sl