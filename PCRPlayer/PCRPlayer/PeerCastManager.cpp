#include "stdafx.h"


namespace network {

PeerCastManager::PeerCastManager()
{

}

PeerCastManager::~PeerCastManager()
{
	thread_.join();
	group_.join_all();
}

bool PeerCastManager::init(const std::wstring& path)
{
	peca_ = PeerCast(path);
	peca_.reset();
	org_.clear();
	info_ = ChannelInfo();
	status_.clear();
	panel_.clear();
	links_.clear();
	return peca_;
}

void PeerCastManager::bump(bool relay)
{
	group_.create_thread(boost::bind(&PeerCastManager::bumpThread, this, peca_, relay));
}

void PeerCastManager::stop(bool relay)
{
	group_.create_thread(boost::bind(&PeerCastManager::stopThread, this, peca_, relay));
}

void PeerCastManager::info()
{
	thread_.timed_join(0);
	if (thread_.joinable()) { return; }

	boost::thread thr(&PeerCastManager::infoThread, this, peca_);
	thread_.swap(thr);
}

void PeerCastManager::setTitleCallback(const boost::function<void()>& callback)
{
	callback_.title = callback;
}

void PeerCastManager::setBBSCallback(const boost::function<void()>& callback)
{
	callback_.bbs = callback;
}

void PeerCastManager::setChannelCallback(const boost::function<void(bool)>& callback)
{
	callback_.channel = callback;
}


// ===================================================================


void PeerCastManager::prepareInfo(ChannelInfo& info)
{
	// genre_desc
	if (!info.genre.empty() && !info.desc.empty())
	{
		info.cnv.genreDesc = info.genre + L"-" + info.desc;
	}
	else if (!info.genre.empty())
	{
		info.cnv.genreDesc = info.genre;
	}
	else if (!info.desc.empty())
	{
		info.cnv.genreDesc = info.desc;
	}

	// track
	if (!info.track.artist.empty())
	{
		info.cnv.playing = info.track.artist;
	}

	if (!info.track.title.empty())
	{
		if (!info.cnv.playing.empty()) { info.cnv.playing += L"/"; }
		info.cnv.playing += info.track.title;
	}

	if (!info.track.album.empty())
	{
		if (!info.cnv.playing.empty()) { info.cnv.playing += L"/"; }
		info.cnv.playing += info.track.album;
	}
}


void PeerCastManager::searchLinks(const std::wstring& text, std::vector<std::wstring>& links)
{
	using namespace boost::xpressive;
	try {
		std::wstring str;
		const static wsregex rx = (s1 = wsregex::compile(L"[a-zA-Z0-9_]*:")) >> (s2 = wsregex::compile(L"//[a-zA-Z0-9_#$@&!?~:;.,=%/*+-]+"));
		for (wsregex_iterator it(text.begin(), text.end(), rx), end; it != end; ++it)
		{
			std::wstring s = (*it)[s1];
			if (replaceScheme(s))
			{
				links.push_back(s + (*it)[s2]);
			}
		}
	} catch (...) {}
}

bool PeerCastManager::replaceScheme(std::wstring& text)
{
	using namespace boost::xpressive;
	try {
		const static wsregex rx = wsregex::compile(L"(?:http|ttp|tp)(s)?:$");
		wsmatch what;
		if (regex_search(text, what, rx))
		{
			text = L"http" + what[1] + L":";
			return true;
		}
	} catch (...) {}
	return false;
}




bool PeerCastManager::createInfo(const InformationControl& cfg)
{
	ChannelInfo info;
	if (!org_.get(info)) { return false; }

	std::vector<std::wstring> links;
	auto rx = [this](const std::vector<utl::ReplaceElement>& elem, std::wstring& str, std::vector<std::wstring>& links) {
		searchLinks(str, links);
		
		for (auto it = elem.begin(); it != elem.end(); ++it)
		{
			(*it)(str);
		}
	};

	rx(cfg.name, info.name, links);
	rx(cfg.type, info.type, links);
	rx(cfg.genre, info.genre, links);
	rx(cfg.desc, info.desc, links);
	rx(cfg.comment, info.comment, links);
	rx(cfg.artist, info.track.artist, links);
	rx(cfg.title, info.track.title, links);
	rx(cfg.album, info.track.album, links);

	info_ = info;
	links_ = links;

	return true;
}

bool PeerCastManager::createStatus(const InformationControl::Channel& channel, bool name, bool type, bool genre, bool comment, bool playing, bool relay)
{
	ChannelInfo info;

	if (!org_.get(info)) { return false; }
	
	if (channel.name)	{ info.name = info_.name; }
	if (channel.type)	{ info.type = info_.type; }
	if (channel.genre)	{ info.genre = info_.genre; }
	if (channel.desc)	{ info.desc = info_.desc; }
	if (channel.comment){ info.comment = info_.comment; }
	if (channel.artist)	{ info.track.artist = info_.track.artist; }
	if (channel.title)	{ info.track.title = info_.track.title; }
	if (channel.album)	{ info.track.album = info_.track.album; }

	prepareInfo(info);

	struct {
		std::wstring name;
		std::wstring type;
		std::wstring genre;
		std::wstring comment;
		std::wstring playing;
		CString relay;
	} text;

	if (name)
	{
		if (!info.name.empty())
		{
			text.name = info.name + L" ";
		}
	}

	if (type)
	{
		if (!info.type.empty())
		{
			text.type = L"(" + info.type + L") ";
		}
	}

	if (genre)
	{
		if (!info.cnv.genreDesc.empty())
		{
			text.genre = L"[" + info.cnv.genreDesc + L"]";
		}
	}

	if (comment)
	{
		if (!info.comment.empty())
		{
			text.comment = L"「" + info.comment + L"」";
		}
	}

	if (playing)
	{
		if (!info.cnv.playing.empty())
		{
			text.playing = info.cnv.playing;
		}
	}

	if (relay)
	{
		text.relay.Format(L"[%d/%d]", info.cnv.listeners, info.cnv.relays);
	}

	std::wstring tmp = text.genre + text.comment + text.playing + std::wstring(text.relay);
	if (!tmp.empty()) { tmp += L" "; };
	status_ = text.name + text.type + tmp;
	return true;
}

bool PeerCastManager::createPanel(const InformationControl::Channel& channel)
{
	ChannelInfo info;

	if (!org_.get(info)) { return false; }

	if (channel.name)	{ info.name = info_.name; }
	if (channel.type)	{ info.type = info_.type; }
	if (channel.genre)	{ info.genre = info_.genre; }
	if (channel.desc)	{ info.desc = info_.desc; }
	if (channel.comment){ info.comment = info_.comment; }
	if (channel.artist)	{ info.track.artist = info_.track.artist; }
	if (channel.title)	{ info.track.title = info_.track.title; }
	if (channel.album)	{ info.track.album = info_.track.album; }

	prepareInfo(info);

	std::wstring text;

	if (!info.name.empty())
	{
		text = info.name;
	}

	if (!info.type.empty())
	{
		text += L" (" + info.type + L")";
	}

	CString bitrate;
	bitrate.Format(L" %d kbps", info.cnv.bitrate);
	text += bitrate;

	CString relay;
	relay.Format(L" [%d/%d]", info.cnv.listeners, info.cnv.relays);
	text += std::wstring(relay);

	if (!info.cnv.genreDesc.empty())
	{
		text += L"\r\n  [" + info.cnv.genreDesc + L"]";
	}

	if (!info.comment.empty())
	{
		text += L"\r\n  「" + info.comment + L"」";
	}

	if (!info.cnv.playing.empty())
	{
		text += L"\r\n  " + info.cnv.playing;
	}

	panel_ = text;
	return true;
}


bool PeerCastManager::getInfo(ChannelInfo& info)
{
	if (org_)
	{
		info = info_;
		return true;
	}
	return false;
}

bool PeerCastManager::getStatus(std::wstring& text)
{
	if (org_)
	{
		text = status_;
		return true;
	}
	return false;
}

bool PeerCastManager::getPanel(std::wstring& text)
{
	if (org_)
	{
		text = panel_;
		return true;
	}
	return false;
}

bool PeerCastManager::getLinks(std::vector<std::wstring>& links)
{
	if (org_)
	{
		links = links_;
		return true;
	}
	return false;
}



bool PeerCastManager::getContactURL(std::wstring& url)
{
	return org_.apply([&](const ChannelInfo& info) { url = info.url; });
}

bool PeerCastManager::getBitrate(int& bitrate)
{
	return org_.apply([&](const ChannelInfo& info) { bitrate = info.cnv.bitrate; });
}




// ===================================================================
void PeerCastManager::bumpThread(const PeerCast& peca, bool relay)
{
	if (peca) { peca.bump(relay); }

	//if (peca)
	//{
	//	int remain = 0;
	//	if (peca.check(relay, 60000, remain))
	//	{
	//		gl_.addLog(L"true (remain:%dms)", remain);
	//		peca.bump();
	//	}
	//	else
	//	{
	//		gl_.addLog(L"false (remain:%dms)", remain);
	//	}
	//}
}

void PeerCastManager::stopThread(const PeerCast& peca, bool relay)
{
	if (peca) { peca.stop(relay); }
}

void PeerCastManager::infoThread(const PeerCast& peca)
{
	callback_.channel.get()(true);

	if (peca)
	{
		ChannelInfo info;
		if (peca.info(info))
		{
			org_ = info;

			callback_.title.get();
			callback_.bbs.get()();
		}
	}

	callback_.channel.get()(false);
}

} // namespace network