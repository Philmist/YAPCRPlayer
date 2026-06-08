
#pragma once

#include "XmlParser.h"
#include "Network.h"

#include <boost/thread.hpp>

namespace network {

struct ChannelInfo {
	struct Relay {
		std::wstring listeners;
		std::wstring relays;
		std::wstring hosts;
		std::wstring status;
		std::wstring firewalled;
	};

	struct Track {
		std::wstring title;
		std::wstring artist;
		std::wstring album;
		std::wstring genre;
		std::wstring contact;
	};

	struct Convert {
		Convert()
			: bitrate(0)
			, listeners(0)
			, relays(0)
		{}

		int bitrate;
		int listeners;
		int relays;
		std::wstring genreDesc;
		std::wstring playing;
	};

	std::wstring name;
	std::wstring id;
	std::wstring bitrate;
	std::wstring type;
	std::wstring genre;
	std::wstring desc;
	std::wstring url;
	std::wstring uptime;
	std::wstring comment;
	std::wstring skips;
	std::wstring age;
	std::wstring bcflags;

	Relay relay;
	Track track;

	Convert cnv;
};

class PeerCast {
protected:
	bool valid_;
	std::wstring url_;
	std::wstring host_;
	std::wstring port_;
	std::wstring id_;

private:
	boost::recursive_mutex& mutex() const;
	DWORD timer(bool update = false, bool reset = false) const;

public:
	PeerCast();
	PeerCast(const std::wstring& url);
	virtual ~PeerCast(){};

	operator bool() const { return valid_; }
	operator std::wstring() const { return url_; }

	// Ú‘±‚µ‚Ästream URL‚ğæ“¾‚·‚é
	bool connect(std::wstring& stream, int timeout) const;

	bool command(const std::wstring& cmd) const;

	bool stop() const;
	bool stop(bool relay) const;
	bool bump() const;
	bool bump(bool relay) const;

	bool check(bool relay) const;
	bool check(int interval, int& remain) const;
	bool check(bool relay, int interval, int& remain) const;

	void reset() { timer(false, true); }

	// Ú‘±‚µ‚ÄXML‚©‚çƒ`ƒƒƒ“ƒlƒ‹î•ñ‚ğæ“¾‚·‚é
	bool info(ChannelInfo& info) const;

private:
	bool analyse(const std::wstring& playlist, std::wstring& stream) const;
	std::wstring toUTF16(const std::string& src) const;
};

} // namespace network