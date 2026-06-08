#include "stdafx.h"

namespace sl {

void put(XmlParser& p, const bbs::SkinConfig::Color::Set& data)
{
	p.put(L"enable", data.enable);
	p.put(L"color", data.color);
}

void put(XmlParser& p, const bbs::Link& link)
{
	p.put(L"title", link.title);
	p.put(L"url", link.url);
}

void put(XmlParser& p, const FontConfig::Font& data)
{
	p.put(L"name", data.name);
	p.put(L"point", data.point);
}

void put(XmlParser& p, const std::vector<DecoderInfo>& data)
{
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		auto item = p.add(L"item");
		item.put(L"enable", it->enable);
		item.put(L"name", it->name);

		std::vector<OLECHAR> id;
		id.resize(1024, 0);
		StringFromGUID2(it->id, id.data(), (int)id.size());
		item.put(L"id", std::wstring(id.data()));
	}
}

void put(XmlParser& p, const PopupBrowserConfig::Popup& data)
{
	p.put(L"enable", data.enable);
	p.put(L"limit", data.limit);
	p.put(L"head", data.head);
}

void put(XmlParser& p, const bbs::SkinConfig::TextColor& data)
{
	p.put(L"text", data.text);
	p.put(L"link", data.link);
	p.put(L"sage", data.sage);
}

void put(XmlParser& p, const bbs::SkinConfig::ColorSet& data)
{
	p.put(L"first", data.first);
	p.put(L"second", data.second);
	p.put(L"third", data.third);
}

void put(XmlParser& p, const bbs::SkinConfig& data)
{
	p.put(L"skin", data.skin);

	auto x = p.put(L"font");
	x.put(L"name", data.font.name);
	x.put(L"point", data.font.point);
	x.put(L"pixel", data.font.pixel);

	x = p.put(L"color.text");
	put(x, data.color.text);

	x = p.put(L"color.message");
	put(x, data.color.message);

	x = p.put(L"color.link");
	put(x, data.color.link);

	x = p.put(L"color.back");
	put(x, data.color.back);

	x = p.put(L"color.newly");
	put(x, data.color.newly);

	x = p.put(L"color.border");
	put(x, data.color.border);

	x = p.put(L"color.scroll");
	put(x, data.color.scroll);

	x = p.put(L"name");
	x.put(L"format", data.name.format);
	x.put(L"bold", data.name.bold);

	x = p.put(L"name.color");
	put(x, data.name.color);

	x = p.put(L"mail.color");
	put(x, data.mail.color);

	x = p.put(L"time.color");
	put(x, data.time.color);

	x = p.put(L"number");
	x.put(L"threshold", data.number.threshold);

	x = p.put(L"number.color");
	put(x, data.number.color);

	x = p.put(L"id");
	x.put(L"enable", data.id.enable);
	x.put(L"threshold", data.id.threshold);

	x = p.put(L"id.color");
	put(x, data.id.color);
}

void put(XmlParser& root, const DetailConfig& detail)
{
	auto p = root.put(L"detail");

	p = root.put(L"detail.network");
	p.put(L"stop", detail.network.stop);

	p = root.put(L"detail.network.connect");
	p.put(L"timeout", detail.network.connect.timeout);
	p.put(L"count", detail.network.connect.count);
	p.put(L"wait", detail.network.connect.wait);
	p.put(L"stop", detail.network.connect.stop);

	p = root.put(L"detail.network.reconnect");
	p.put(L"interval", detail.network.reconnect.interval);

	p = root.put(L"detail.network.relay.manual");
	p.put(L"reconnect", detail.network.relay.manual.reconnect);
	p.put(L"stop", detail.network.relay.manual.stop);

	p = root.put(L"detail.network.relay.automatic");
	p.put(L"reconnect", detail.network.relay.automatic.reconnect);
	p.put(L"stop", detail.network.relay.automatic.stop);

	p = root.put(L"detail.network.rebuild");
	p.put(L"wait", detail.network.rebuild.wait);

	p = root.put(L"detail.network.reader");
	p.put(L"userAgent", detail.network.reader.userAgent);
	p.put(L"timeout", detail.network.reader.timeout);

	p = root.put(L"detail.type.wmv.packet");
	p.put(L"enable", detail.type.wmv.packet.enable);
	p.put(L"time", detail.type.wmv.packet.time);

	p = root.put(L"detail.type.wmv.frame");
	p.put(L"enable", detail.type.wmv.frame.enable);
	p.put(L"time", detail.type.wmv.frame.time);

	p = root.put(L"detail.type.flv");
	p.put(L"loop", detail.type.flv.loop);
	p.put(L"keyframe", detail.type.flv.keyframe);
	p.put(L"timestamp", detail.type.flv.timestamp);
	p.put(L"buffer", detail.type.flv.buffer);
	p.put(L"optimize", detail.type.flv.optimize);
	p.put(L"delay", detail.type.flv.delay);
	p.put(L"limit", detail.type.flv.limit);
	p.put(L"interval", detail.type.flv.interval);

	p = root.put(L"detail.decoder");
	p.put(L"dxva2", detail.decoder.dxva2);

	p = root.put(L"detail.video");
	put(p, detail.video);

	p = root.put(L"detail.audio");
	put(p, detail.audio);

	p = root.put(L"detail.browser");
	p.put(L"def", detail.browser.def);
	p.put(L"web", detail.browser.web);
	p.put(L"bbs", detail.browser.bbs);
	p.put(L"close", detail.browser.close);

	p = root.put(L"detail.snapshot");
	p.put(L"folder", detail.snapshot.folder);
	p.put(L"format", detail.snapshot.format);
	p.put(L"jpegQuality", detail.snapshot.jpegQuality);
	p.put(L"display", detail.snapshot.display);
	p.put(L"fit", detail.snapshot.fit);

	p = root.put(L"detail.start");
	p.put(L"recent", detail.start.recent);

	p = root.put(L"detail.end");
	p.put(L"pos", detail.end.pos);
	p.put(L"size", detail.end.size);
	p.put(L"aspect", detail.end.aspect);
	p.put(L"volume", detail.end.volume);
	p.put(L"balance", detail.end.balance);
	p.put(L"mute", detail.end.mute);

	p = root.put(L"detail.board");
	p.put(L"contact", detail.board.contact);
	p.put(L"change", detail.board.change);
	p.put(L"userAgent", detail.board.userAgent);
	p.put(L"timeout", detail.board.timeout);
	p.put(L"shitaraba", detail.board.shitaraba);
	p.put(L"block", detail.board.block);
	p.put(L"reload", detail.board.reload);
	p.put(L"interval", detail.board.interval);
	p.put(L"retry", detail.board.retry);
	p.put(L"thread", detail.board.thread);
	p.put(L"board", detail.board.board);
	p.put(L"move", detail.board.move);
	p.put(L"res", detail.board.res);
	p.put(L"delay", detail.board.delay);
	p.put(L"position", detail.board.position);
	p.put(L"under", detail.board.under);
	p.put(L"align", detail.board.align);
	p.put(L"order", detail.board.order);
	p.put(L"scroll", detail.board.scroll);
	p.put(L"shortcut", detail.board.shortcut);
	p.put(L"write", detail.board.write);
	p.put(L"hide", detail.board.hide);
	p.put(L"log", detail.board.log);
	p.put(L"path", detail.board.path);
	p.put(L"focus", detail.board.focus);
	p.put(L"latest", detail.board.latest);
	p.put(L"newly", detail.board.newly);
	p.put(L"def", detail.board.def);
	p.put(L"partial", detail.board.partial);
	p.put(L"range", detail.board.range);

	p = root.put(L"detail.board.history");
	p.put(L"board", detail.board.history.board);
	p.put(L"thread", detail.board.history.thread);

	p = root.put(L"detail.board.movable");
	p.put(L"contact", detail.board.movable.contact);
	p.put(L"commandline", detail.board.movable.commandline);
	p.put(L"manual", detail.board.movable.manual);

	p = root.put(L"detail.board.movable.favorite");
	p.put(L"board", detail.board.movable.favorite.board);
	p.put(L"thread", detail.board.movable.favorite.thread);

	p = root.put(L"detail.board.movable.history");
	p.put(L"board", detail.board.movable.history.board);
	p.put(L"thread", detail.board.movable.history.thread);

	p = root.put(L"detail.board.links");
	for (auto it = detail.board.links.begin(); it != detail.board.links.end(); ++it)
	{
		auto item = p.add(L"item");
		put(item, *it);
	}

	p = root.put(L"detail.board.threads");
	for (auto it = detail.board.threads.begin(); it != detail.board.threads.end(); ++it)
	{
		auto item = p.add(L"item");
		put(item, *it);
	}

	p = root.put(L"detail.font.thread");
	put(p, detail.font.thread);

	p = root.put(L"detail.font.search");
	put(p, detail.font.search);

	p = root.put(L"detail.font.extract");
	put(p, detail.font.extract);

	p = root.put(L"detail.board.proxy");
	p.put(L"white", detail.board.proxy.white);
	p.put(L"black", detail.board.proxy.black);

	p = root.put(L"detail.board.proxy.get");
	p.put(L"enable", detail.board.proxy.get.enable);
	p.put(L"name", detail.board.proxy.get.name);

	p = root.put(L"detail.board.proxy.post");
	p.put(L"enable", detail.board.proxy.post.enable);
	p.put(L"name", detail.board.proxy.post.name);

	p = root.put(L"detail.board.url");
	put(p, detail.board.url);

	p = root.put(L"detail.board.message");
	put(p, detail.board.message);

	p = root.put(L"detail.popup");
	p.put(L"delay", detail.popup.delay);
	p.put(L"edge", detail.popup.edge);
	p.put(L"border", detail.popup.border);

	p = root.put(L"detail.popup.ref");
	put(p, detail.popup.ref);

	p = root.put(L"detail.popup.anchor");
	put(p, detail.popup.anchor);

	p = root.put(L"detail.popup.id");
	put(p, detail.popup.id);

	p = root.put(L"detail.scroll");
	p.put(L"init", detail.scroll.init);
	p.put(L"latest", detail.scroll.latest);
	p.put(L"refresh", detail.scroll.refresh);
	p.put(L"manual", detail.scroll.manual);
	p.put(L"last", detail.scroll.last);
	p.put(L"delta", detail.scroll.delta);
	p.put(L"interval", detail.scroll.interval);

	p = root.put(L"detail.scroll.position");
	p.put(L"init", detail.scroll.position.init);
	p.put(L"latest", detail.scroll.position.latest);

	p = root.put(L"detail.splitter");
	p.put(L"split", detail.splitter.split);
	p.put(L"edge", detail.splitter.edge);
	p.put(L"face", detail.splitter.face);
	p.put(L"border", detail.splitter.border);

	p = root.put(L"detail.skin.thread");
	put(p, detail.skin.thread);

	p = root.put(L"detail.skin.popup");
	put(p, detail.skin.popup);
}

} // namespace sl
