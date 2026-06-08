#pragma once

#define DEFAULT_USERAGENT	L"PCRPlayer/1.00"
#define DEFAULT_TIMEOUT		10000

#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace network {


class URLParser {
private:
	bool valid_;
	std::wstring url_;
	std::wstring scheme_;
	std::wstring host_;
	std::wstring port_;
	std::wstring path_;
	std::wstring fullpath_;
	std::wstring file_;
	std::wstring query_;

public:
	URLParser();
	URLParser(const std::wstring& url);

	operator bool() { return valid_; }

	const std::wstring& url() { return url_; }
	const std::wstring& scheme() { return scheme_; }
	const std::wstring& host() { return host_; }
	//const std::wstring& port() { return port_; }
	WORD port();

	// /path/file?query
	const std::wstring& path() { return path_; }
	const std::wstring& fullpath() { return fullpath_; }
	const std::wstring& file() { return file_; }
	const std::wstring& query() { return query_; }

private:
	const std::wstring::const_iterator parseScheme(std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end);
	const std::wstring::const_iterator parseHost(std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end);
	const std::wstring::const_iterator parsePort(std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end);
	const std::wstring::const_iterator parsePath(std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end);
	const std::wstring::const_iterator parseQuery(std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end);
};


struct Proxy {
	struct Server {
		Server()
			: enable(false)
			, name(L"localhost:8080")
		{}
		bool enable;
		std::wstring name;
	};

	Proxy()
		: white(L".*")
		, black(L"")
	{}

	Server get;
	Server post;
	std::wstring white;
	std::wstring black;
};

class HttpClient {
private:
	bool valid_;
	HINTERNET session_;
	HINTERNET connect_;
	HINTERNET request_;
	std::vector<char> buffer_;
	int timeout_;

public:
	HttpClient::HttpClient()
			: valid_(false)
			, session_(NULL)
			, connect_(NULL)
			, request_(NULL)
			, timeout_(DEFAULT_TIMEOUT)
	{}


	HttpClient::HttpClient(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::wstring& header, int timeout)
			: valid_(false)
			, session_(NULL)
			, connect_(NULL)
			, request_(NULL)
			, timeout_(timeout)
	{
		open(url, userAgent, proxy, header, timeout);
	}

	HttpClient::HttpClient(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, int timeout)
			: valid_(false)
			, session_(NULL)
			, connect_(NULL)
			, request_(NULL)
			, timeout_(timeout)
	{
		open(url, userAgent, proxy, timeout);
	}

	virtual ~HttpClient();

	enum STATE {
		STATE_ERROR = -1,
		STATE_OK = 0,
		STATE_EOF = 1,
		STATE_TIMEOUT = 2,
	};

	operator bool() { return valid_; }

	bool open(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::wstring& header, int timeout);
	bool open(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, int timeout)
	{
		return open(url, userAgent, proxy, std::wstring(), timeout);
	}

	bool post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const char* data, DWORD size, const std::wstring& header, int timeout);
	bool post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const char* data, DWORD size, int timeout)
	{
		return post(url, userAgent, proxy, data, size, std::wstring(), timeout);
	}

	bool post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::vector<char>& data, const std::wstring& header, int timeout);
	bool post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::vector<char>& data, int timeout)
	{
		return post(url, userAgent, proxy, data.data(), (DWORD)data.size(), std::wstring(), timeout);
	}

	bool post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::string& data, const std::wstring& header, int timeout);
	bool post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::string& data, int timeout)
	{
		return post(url, userAgent, proxy, data.data(), (DWORD)data.size(), std::wstring(), timeout);
	}

	int recv(std::vector<char>& dst);// 一回分の返信だけ受け取る
	int get(std::vector<char>& dst);// ファイルの最後まで受け取る
	int read(void* dst, unsigned int size);// 指定サイズまで受け取る

	bool queryStatus(DWORD& code);
	bool queryInfo(DWORD infoLevel, std::wstring& text);
	bool queryContentEncoding(std::wstring& text);
	bool queryLastModified(std::wstring& text);
	bool querySetCookie(std::wstring& text);

	void close();

private:
	void closeHandle(HINTERNET& handle)
	{
		if (handle) { WinHttpCloseHandle(handle); handle = NULL; }
	}

	bool match(const std::wstring& host, const std::wstring& rx);
	bool match(const std::wstring& host, const std::wstring& white, const std::wstring& black);
};

} // namespace network
