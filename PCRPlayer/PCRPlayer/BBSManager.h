#pragma once

#include "BaseBBS.h"
#include "BBSRegex.h"

#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace bbs {


class BBSManager {
private:
	struct Mutex {
		boost::recursive_mutex bbs;
		boost::recursive_mutex value;
	};
	Mutex mutex_;

	boost::shared_ptr<BaseBBS> bbs_;

	boost::optional<BBSInfo> info_;
	boost::optional<std::vector<ThreadInfo>> subject_;

	struct Dat {
		Dat() : init(true) {}
		bool init;
		std::vector<ResInfo> tmp;
		std::vector<ResInfo> res;
		std::unordered_map<std::wstring, std::vector<int>> id;
		std::unordered_map<std::wstring, int> identifier;
		std::map<int, std::vector<bbs::Range>> anchor;
	};

	boost::optional<Dat> dat_;
	boost::optional<int> pos_;

public:
	BBSManager();
	virtual ~BBSManager();

	operator bool();

	bool init(const std::wstring& url, const std::wstring& shitaraba, const std::wstring& userAgent, const network::Proxy& proxy, int timeout);
	void uninit();

	bool change(const std::wstring& key);

	int loadSetting(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);
	int loadSubject(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);
	int loadDat(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);

	void prepareDat(bool browser, bool latest, int range, const std::vector<utl::ReplaceElement>& message);
	bool cleanDat();
	void resetDat();

	bool getSubject(std::vector<ThreadInfo>& data);
	bool getDat(std::vector<ResInfo>& data, bool& init);
	bool getDat(std::vector<ResInfo>& data, int diff, int len);
	bool getExtract(std::vector<ResInfo>& data);
	bool getExtract(std::vector<ResInfo>& data, const std::wstring& text, bool convert);
	bool getExtract(std::vector<ResInfo>& data, const bbs::Range& range);
	bool getExtract(std::vector<ResInfo>& data, const std::wstring& id);
	bool getExtract(std::vector<ResInfo>& data, int ref);
	bool getExtract(std::vector<ResInfo>& data, const std::vector<int>& vec);
	bool getPos(int& pos);

	int post(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, const std::wstring& name, const std::wstring& mail, const std::wstring& message,
		DWORD& code, std::wstring& error);

public:
	bool getURL(std::wstring& url);
	bool getBoardURL(std::wstring& url);
	bool getThreadURL(std::wstring& url);
	bool getBoardTitle(std::wstring& title);
	bool getThreadTitle(std::wstring& title);	
	bool getNoName(std::wstring& noname);	
	bool getCount(int& count);
	bool getKey(std::wstring& key);
	bool hasKey();
	bool getStop(int& stop);
	bool isStop();

private:
	bool match(const std::wstring& str, const std::wstring& rx);
};


} // namespace bbs