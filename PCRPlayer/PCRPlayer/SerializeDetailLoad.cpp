#include "stdafx.h"

namespace sl {

void get(const XmlParser& p, bbs::SkinConfig::Color::Set& data)
{
	p.get(L"enable", data.enable);
	p.get(L"color", data.color);
}

void get(const XmlParser& p, bbs::Link& link)
{
	p.get(L"title", link.title);
	p.get(L"url", link.url);
}

void get(const XmlParser& p, FontConfig::Font& data)
{
	p.get(L"name", data.name);
	p.get(L"point", data.point);
}

void get(const XmlParser& p, std::vector<DecoderInfo>& data)
{
	if (p)
	{
		data.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			DecoderInfo tmp;
			std::wstring id;
			item.get(L"enable", tmp.enable);
			item.get(L"name", tmp.name);
			item.get(L"id", id);

			GUID guid = {};
			CLSIDFromString(id.c_str(), &guid);
			tmp.id = guid;

			data.push_back(tmp);
		});
	}
}

void get(const XmlParser& p, PopupBrowserConfig::Popup& data)
{
	p.get(L"enable", data.enable);
	p.get(L"limit", data.limit);
	p.get(L"head", data.head);
}

void get(const XmlParser& p, bbs::SkinConfig::TextColor& data)
{
	p.get(L"text", data.text);
	p.get(L"link", data.link);
	p.get(L"sage", data.sage);
}

void get(const XmlParser& p, bbs::SkinConfig::ColorSet& data)
{
	p.get(L"first", data.first);
	p.get(L"second", data.second);
	p.get(L"third", data.third);
}

void get(const XmlParser& p, bbs::SkinConfig& data)
{
	p.get(L"skin", data.skin);

	auto x = p.get(L"font");
	x.get(L"name", data.font.name);
	x.get(L"point", data.font.point);
	x.get(L"pixel", data.font.pixel);

	x = p.get(L"color.text");
	get(x, data.color.text);

	x = p.get(L"color.message");
	get(x, data.color.message);

	x = p.get(L"color.link");
	get(x, data.color.link);

	x = p.get(L"color.back");
	get(x, data.color.back);

	x = p.get(L"color.newly");
	get(x, data.color.newly);

	x = p.get(L"color.border");
	get(x, data.color.border);

	x = p.get(L"color.scroll");
	get(x, data.color.scroll);

	x = p.get(L"name");
	x.get(L"format", data.name.format);
	x.get(L"bold", data.name.bold);

	x = p.get(L"name.color");
	get(x, data.name.color);

	x = p.get(L"mail.color");
	get(x, data.mail.color);

	x = p.get(L"time.color");
	get(x, data.time.color);

	x = p.get(L"number");
	x.get(L"threshold", data.number.threshold);

	x = p.get(L"number.color");
	get(x, data.number.color);

	x = p.get(L"id");
	x.get(L"enable", data.id.enable);
	x.get(L"threshold", data.id.threshold);

	x = p.get(L"id.color");
	get(x, data.id.color);
}

void get(const XmlParser& root, DetailConfig& detail)
{
	auto p = root.get(L"detail");

	p = root.get(L"detail.network");
	p.get(L"stop", detail.network.stop);

	p = root.get(L"detail.network.connect");
	p.get(L"timeout", detail.network.connect.timeout);
	p.get(L"count", detail.network.connect.count);
	p.get(L"wait", detail.network.connect.wait);
	p.get(L"stop", detail.network.connect.stop);

	p = root.get(L"detail.network.reconnect");
	p.get(L"interval", detail.network.reconnect.interval);

	p = root.get(L"detail.network.relay.manual");
	p.get(L"reconnect", detail.network.relay.manual.reconnect);
	p.get(L"stop", detail.network.relay.manual.stop);

	p = root.get(L"detail.network.relay.automatic");
	p.get(L"reconnect", detail.network.relay.automatic.reconnect);
	p.get(L"stop", detail.network.relay.automatic.stop);

	p = root.get(L"detail.network.rebuild");
	p.get(L"wait", detail.network.rebuild.wait);

	p = root.get(L"detail.network.reader");
	p.get(L"userAgent", detail.network.reader.userAgent);
	p.get(L"timeout", detail.network.reader.timeout);

	p = root.get(L"detail.type.wmv.packet");
	p.get(L"enable", detail.type.wmv.packet.enable);
	p.get(L"time", detail.type.wmv.packet.time);

	p = root.get(L"detail.type.wmv.frame");
	p.get(L"enable", detail.type.wmv.frame.enable);
	p.get(L"time", detail.type.wmv.frame.time);

	p = root.get(L"detail.type.flv");
	p.get(L"loop", detail.type.flv.loop);
	p.get(L"keyframe", detail.type.flv.keyframe);
	p.get(L"timestamp", detail.type.flv.timestamp);
	p.get(L"buffer", detail.type.flv.buffer);
	p.get(L"optimize", detail.type.flv.optimize);
	p.get(L"delay", detail.type.flv.delay);
	p.get(L"limit", detail.type.flv.limit);
	p.get(L"interval", detail.type.flv.interval);

	p = root.get(L"detail.decoder");
	p.get(L"dxva2", detail.decoder.dxva2);

	p = root.get(L"detail.video");
	get(p, detail.video);

	p = root.get(L"detail.audio");
	get(p, detail.audio);

	p = root.get(L"detail.browser");
	p.get(L"def", detail.browser.def);
	p.get(L"web", detail.browser.web);
	p.get(L"bbs", detail.browser.bbs);
	p.get(L"close", detail.browser.close);

	p = root.get(L"detail.snapshot");
	p.get(L"folder", detail.snapshot.folder);
	p.get(L"format", detail.snapshot.format);
	p.get(L"jpegQuality", detail.snapshot.jpegQuality);
	p.get(L"display", detail.snapshot.display);
	p.get(L"fit", detail.snapshot.fit);
	
	p = root.get(L"detail.start");
	p.get(L"recent", detail.start.recent);

	p = root.get(L"detail.end");
	p.get(L"pos", detail.end.pos);
	p.get(L"size", detail.end.size);
	p.get(L"aspect", detail.end.aspect);
	p.get(L"volume", detail.end.volume);
	p.get(L"balance", detail.end.balance);
	p.get(L"mute", detail.end.mute);

	p = root.get(L"detail.board");
	p.get(L"contact", detail.board.contact);
	p.get(L"change", detail.board.change);
	p.get(L"userAgent", detail.board.userAgent);
	p.get(L"timeout", detail.board.timeout);
	p.get(L"shitaraba", detail.board.shitaraba);
	p.get(L"block", detail.board.block);
	p.get(L"reload", detail.board.reload);
	p.get(L"interval", detail.board.interval);
	p.get(L"retry", detail.board.retry);
	p.get(L"thread", detail.board.thread);
	p.get(L"board", detail.board.board);
	p.get(L"move", detail.board.move);
	p.get(L"res", detail.board.res);
	p.get(L"delay", detail.board.delay);
	p.get(L"position", detail.board.position);
	p.get(L"under", detail.board.under);
	p.get(L"align", detail.board.align);
	p.get(L"order", detail.board.order);
	p.get(L"scroll", detail.board.scroll);
	p.get(L"shortcut", detail.board.shortcut);
	p.get(L"write", detail.board.write);
	p.get(L"hide", detail.board.hide);
	p.get(L"log", detail.board.log);
	p.get(L"path", detail.board.path);
	p.get(L"focus", detail.board.focus);
	p.get(L"latest", detail.board.latest);
	p.get(L"newly", detail.board.newly);
	p.get(L"def", detail.board.def);
	p.get(L"partial", detail.board.partial);
	p.get(L"range", detail.board.range);

	p = root.get(L"detail.board.history");
	p.get(L"board", detail.board.history.board);
	p.get(L"thread", detail.board.history.thread);

	p = root.get(L"detail.board.movable");
	p.get(L"contact", detail.board.movable.contact);
	p.get(L"commandline", detail.board.movable.commandline);
	p.get(L"manual", detail.board.movable.manual);

	p = root.get(L"detail.board.movable.favorite");
	p.get(L"board", detail.board.movable.favorite.board);
	p.get(L"thread", detail.board.movable.favorite.thread);

	p = root.get(L"detail.board.movable.history");
	p.get(L"board", detail.board.movable.history.board);
	p.get(L"thread", detail.board.movable.history.thread);

	p = root.get(L"detail.board.links");
	if (p)
	{
		detail.board.links.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			bbs::Link link;
			get(item, link);

			detail.board.links.push_back(link);
		});
	}

	p = root.get(L"detail.board.threads");
	if (p)
	{
		detail.board.threads.clear();
		p.foreach(L"item", [&](const XmlParser& item){
			bbs::Link link;
			get(item, link);

			detail.board.threads.push_back(link);
		});
	}

	p = root.get(L"detail.font.thread");
	get(p, detail.font.thread);

	p = root.get(L"detail.font.search");
	get(p, detail.font.search);

	p = root.get(L"detail.font.extract");
	get(p, detail.font.extract);

	p = root.get(L"detail.board.proxy");
	p.get(L"white", detail.board.proxy.white);
	p.get(L"black", detail.board.proxy.black);

	p = root.get(L"detail.board.proxy.get");
	p.get(L"enable", detail.board.proxy.get.enable);
	p.get(L"name", detail.board.proxy.get.name);

	p = root.get(L"detail.board.proxy.post");
	p.get(L"enable", detail.board.proxy.post.enable);
	p.get(L"name", detail.board.proxy.post.name);

	p = root.get(L"detail.board.url");
	get(p, detail.board.url);

	p = root.get(L"detail.board.message");
	get(p, detail.board.message);

	p = root.get(L"detail.popup");
	p.get(L"delay", detail.popup.delay);
	p.get(L"edge", detail.popup.edge);
	p.get(L"border", detail.popup.border);

	p = root.get(L"detail.popup.ref");
	get(p, detail.popup.ref);

	p = root.get(L"detail.popup.anchor");
	get(p, detail.popup.anchor);

	p = root.get(L"detail.popup.id");
	get(p, detail.popup.id);

	p = root.get(L"detail.scroll");
	p.get(L"init", detail.scroll.init);
	p.get(L"latest", detail.scroll.latest);
	p.get(L"refresh", detail.scroll.refresh);
	p.get(L"manual", detail.scroll.manual);
	p.get(L"last", detail.scroll.last);
	p.get(L"delta", detail.scroll.delta);
	p.get(L"interval", detail.scroll.interval);

	p = root.get(L"detail.scroll.position");
	p.get(L"init", detail.scroll.position.init);
	p.get(L"latest", detail.scroll.position.latest);

	p = root.get(L"detail.splitter");
	p.get(L"split", detail.splitter.split);
	p.get(L"edge", detail.splitter.edge);
	p.get(L"face", detail.splitter.face);
	p.get(L"border", detail.splitter.border);

	p = root.get(L"detail.skin.thread");
	get(p, detail.skin.thread);

	p = root.get(L"detail.skin.popup");
	get(p, detail.skin.popup);
}

} // namespace sl
