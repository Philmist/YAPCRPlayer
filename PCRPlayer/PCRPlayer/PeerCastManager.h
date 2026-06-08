#pragma once

#include "PeerCast.h"

#include <boost/thread.hpp>
#include <boost/functional.hpp>

namespace network {

struct InformationControl {
	InformationControl()
	{
		typedef utl::ReplaceElement RE;
		genre.push_back(RE(true, L"YPåfç⁄ê›íËÇçÌèú", L"^(sp|tp)(:?)(\\??)(@*)(\\+?)(#?)", L"", false));
	}

	struct Channel {
		Channel()
			: name(true)
			, type(true)
			, genre(true)
			, desc(true)
			, comment(true)
			, artist(true)
			, title(true)
			, album(true)
		{}

		bool name;
		bool type;
		bool genre;
		bool desc;
		bool comment;
		bool artist;
		bool title;
		bool album;
	};

	Channel status;
	Channel panel;
	std::vector<utl::ReplaceElement> name;
	std::vector<utl::ReplaceElement> type;
	std::vector<utl::ReplaceElement> genre;
	std::vector<utl::ReplaceElement> desc;
	std::vector<utl::ReplaceElement> comment;
	std::vector<utl::ReplaceElement> artist;
	std::vector<utl::ReplaceElement> title;
	std::vector<utl::ReplaceElement> album;
};

class PeerCastManager {
	struct Callback {
		Callback()
			: title([](){})
			, bbs([](){})
			, channel([](bool){})
		{}

		utl::SyncValue<boost::function<void()> > title;
		utl::SyncValue<boost::function<void()> > bbs;
		utl::SyncValue<boost::function<void(bool)> > channel;
	};

	PeerCast peca_;

	boost::thread_group group_;
	utl::SyncThread thread_;

	utl::SyncOptional<ChannelInfo> org_;
	ChannelInfo info_;
	std::wstring status_;
	std::wstring panel_;
	std::vector<std::wstring> links_;

	Callback callback_;

public:
	PeerCastManager();
	virtual ~PeerCastManager();

	operator bool() { return peca_; }
	operator std::wstring() { return peca_; }

	bool init(const std::wstring& path);
	void bump(bool relay);
	void stop(bool relay);
	void info();

	void setTitleCallback(const boost::function<void()>& callback);
	void setBBSCallback(const boost::function<void()>& callback);
	void setChannelCallback(const boost::function<void(bool)>& callback);

private:
	void prepareInfo(ChannelInfo& info);
	void searchLinks(const std::wstring& text, std::vector<std::wstring>& links);
	bool replaceScheme(std::wstring& text);

public:
	bool createInfo(const InformationControl& cfg);
	bool createStatus(const InformationControl::Channel& channel, bool name, bool type, bool genre, bool comment, bool playing, bool relay);
	bool createPanel(const InformationControl::Channel& channel);

	bool getInfo(ChannelInfo& info);
	bool getStatus(std::wstring& text);
	bool getPanel(std::wstring& text);
	bool getLinks(std::vector<std::wstring>& links);

	bool getContactURL(std::wstring& url);
	bool getBitrate(int& bitrate);

private:
	void bumpThread(const PeerCast& peca, bool relay);
	void stopThread(const PeerCast& peca, bool relay);
	void infoThread(const PeerCast& peca);
};

} // namespace network