
#include "stdafx.h"

namespace network {

boost::recursive_mutex& PeerCast::mutex() const
{
	static boost::recursive_mutex mutex;
	return mutex;
}

DWORD PeerCast::timer(bool update, bool reset) const
{
	boost::recursive_mutex::scoped_lock lock(mutex());
	static DWORD time = 0;
	if (reset)
	{
		time = 0;
	}
	else if (update)
	{
		time = timeGetTime();
	}
	return time;
}

PeerCast::PeerCast()
	: valid_(false)
{
}


PeerCast::PeerCast(const std::wstring& url)
	: valid_(false)
{
	using namespace boost::xpressive;

	try {
		const static wsregex rx = wsregex::compile(L"http:\\/\\/([^\\:]+):([^\\/]+)\\/pls\\/([0-9A-Za-z]{32})");
		wsmatch what;
		if (regex_search(url, what, rx))
		{
			url_ = url;
			host_ = what[1];
			port_ = what[2];
			id_ = what[3];
			valid_ = true;
		}
	} catch (...) {}
}

bool PeerCast::connect(std::wstring& stream, int timeout) const
{
	HttpClient client(url_, DEFAULT_USERAGENT, network::Proxy(), timeout);
	if (!client) { return false; }

	std::vector<char> buf(1024*32, 0);
	int state = client.read(buf.data(), (unsigned int)buf.size());
	if (state == HttpClient::STATE_ERROR || state == HttpClient::STATE_TIMEOUT)
	{
		return false;
	}

	analyse(toUTF16(std::string(buf.begin(), buf.end())), stream);

	client.close();
	return true;
}

bool PeerCast::command(const std::wstring& cmd) const
{
	if (id_ == L"") { return false; }
	std::wstring url = L"http://" + host_ + L":" + port_ + L"/admin?cmd=" + cmd + L"&id=" + id_;

	HttpClient client(url, DEFAULT_USERAGENT, network::Proxy(), DEFAULT_TIMEOUT);
	if (client)
	{
		client.close();
		return true;
	}
	return false;
}

bool PeerCast::stop() const
{
	return command(L"stop");
}

bool PeerCast::stop(bool relay) const
{
	return check(relay) && stop();
}

bool PeerCast::bump() const
{
	if (command(L"bump"))
	{
		timer(true);
		return true;
	}
	return false;
}

bool PeerCast::bump(bool relay) const
{
	return check(relay) && bump();
}

bool PeerCast::check(bool relay) const
{
	if (relay)
	{
		ChannelInfo ci;
		return info(ci) && (ci.cnv.relays == 0);
	}
	return true;
}

bool PeerCast::check(int interval, int& remain) const
{
	if (timer() == 0)
	{
		remain = 0;
		return true;
	}

	int diff = timeGetTime() - timer();
	if (diff >= interval)
	{
		remain = 0;
		return true;
	}
	remain = interval - diff;
	return false;
}

bool PeerCast::check(bool relay, int interval, int& remain) const
{
	if (check(relay))
	{
		return check(interval, remain);
	}
	return false;
}


bool PeerCast::info(ChannelInfo& info) const
{
	std::wstring url = L"http://" + host_ + L":" + port_ + L"/admin?cmd=viewxml";

	HttpClient client(url, DEFAULT_USERAGENT, network::Proxy(), DEFAULT_TIMEOUT);
	std::vector<char> data;
	if (client.get(data))
	{
		client.close();

		data.push_back(L'\0');

		bool exist = false;

		try {
			using namespace boost::property_tree;
			// xml“Ç‚Ýž‚Ý
			wptree org;
			boost::property_tree::read_xml(std::wstringstream(toUTF16(data.data())), org);

			XmlParser pt(org);
			auto p = pt.get(L"peercast.channels_relayed");
			p.search(L"channel", [&](const XmlParser& x)->bool{

				std::wstring id;
				x.get(L"id", id);

				if (id.empty()) { return false; }

				if(_wcsicmp(id.c_str(), id_.c_str()) == 0)
				{
					info.id = id;

					x.get(L"name", info.name);

					if (info.name.empty()) { return false; }

					x.get(L"bitrate", info.bitrate);
					x.get(L"type", info.type);
					x.get(L"genre", info.genre);
					x.get(L"desc", info.desc);
					x.get(L"url", info.url);
					x.get(L"uptime", info.uptime);
					x.get(L"comment", info.comment);
					x.get(L"skips", info.skips);
					x.get(L"age", info.age);
					x.get(L"bcflags", info.bcflags);

					auto relay = x.get(L"relay");
					relay.get(L"listeners", info.relay.listeners);
					relay.get(L"relays", info.relay.relays);
					relay.get(L"hosts", info.relay.hosts);
					relay.get(L"status", info.relay.status);
					relay.get(L"firewalled", info.relay.firewalled);

					auto track = x.get(L"track");
					track.get(L"title", info.track.title);
					track.get(L"artist", info.track.artist);
					track.get(L"album", info.track.album);
					track.get(L"genre", info.track.genre);
					track.get(L"contact", info.track.contact);

					info.cnv.bitrate = wcstol(info.bitrate.c_str(), NULL, 10);
					info.cnv.listeners = wcstol(info.relay.listeners.c_str(), NULL, 10);
					info.cnv.relays = wcstol(info.relay.relays.c_str(), NULL, 10);

					exist = true;
					return true;
				}
				return false;
			});
		} catch (...) { return false; }
		return exist;
	}
	return false;
}


bool PeerCast::analyse(const std::wstring& playlist, std::wstring& stream) const
{
	using namespace boost::xpressive;

	try {
		//const static wsregex rx = wsregex::compile(L"(mms|https?):\\/\\/[^\\/]+\\/stream\\/[0-9a-zA-Z]{32}(\\.[_0-9a-zA-Z]+)?");
		//const static wsregex rx = wsregex::compile(L"(mmsh?|https?):\\/\\/[^\\/]+\\/stream\\/[0-9a-zA-Z]{32}(\\.[_0-9a-zA-Z]+)?");
		const static wsregex rx = wsregex::compile(L"[0-9a-z]+:\\/\\/[^\\/]+\\/stream\\/[0-9a-zA-Z]{32}(\\.[_0-9a-zA-Z]+)?");

		wsmatch what;
		if (regex_search(playlist, what, rx))
		{
			stream = what[0];
			return !stream.empty();
		}
	} catch (...) {}
	return false;
}

std::wstring PeerCast::toUTF16(const std::string& src) const
{
	const int size = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, NULL, 0);

	if (size < 0) { return std::wstring(); }

	std::vector<wchar_t> buff(size);
	
	int result = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, buff.data(), size);

	if (result < 0) { return std::wstring(); }

	return buff.data();
}

} // namespace network