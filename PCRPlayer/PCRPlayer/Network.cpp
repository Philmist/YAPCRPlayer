
#include "stdafx.h"




namespace network {

URLParser::URLParser()
	: valid_(false)
{}

URLParser::URLParser(const std::wstring& url)
	: valid_(false)
	, url_(url)
{
	std::wstring::const_iterator it = url.begin();
	std::wstring::const_iterator end = url.end();

	if ((it = parseScheme(scheme_, it, end)) == end) { return; }
	if ((it = parseHost(host_, it, end)) == end) { return; }
	
	std::wstring::const_iterator tmp = it;
	if ((it = parsePort(port_, it, end)) == end) { it = tmp; }
	it = parsePath(path_, it, end);

	std::wstring::const_reverse_iterator file_begin = std::find(path_.rbegin(), path_.rend(), '/');
	if (file_begin != path_.rend())
	{
		file_.assign(file_begin.base(), path_.end());
	}

	it = parseQuery(query_, it, url.end());

	fullpath_ = path_ + query_;
	valid_ = true;
}

WORD URLParser::port()
{
	if (port_.empty())
	{
		if (scheme_ == L"https")
		{
			return 443;
		}
		else
		{
			return 80;
		}
	}
	return _wtoi(port_.c_str());
}

const std::wstring::const_iterator URLParser::parseScheme(
	std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
	const std::wstring marker(L"://");
	std::wstring::const_iterator last =
		std::search(begin, end, marker.begin(), marker.end());

	if (last == end) { return end; }

	dst.clear();
	dst.reserve(std::distance(begin, last));
	std::transform(begin, last, std::back_inserter(dst), tolower);
	return last + 3;
}

const std::wstring::const_iterator URLParser::parseHost(
	std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
	const std::wstring marker(L":/");
	std::wstring::const_iterator last =
		std::find_first_of(begin, end, marker.begin(), marker.end());

	if (last == end) { return end; }

	dst.clear();
	dst.reserve(std::distance(begin, last));
	std::transform(begin, last, std::back_inserter(dst), tolower);
	return last;
}

const std::wstring::const_iterator URLParser::parsePort(
	std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
	if (begin == end || begin[0] != L':')
	{
		return end;
	}

	std::wstring::const_iterator last = std::find(begin + 1, end, L'/');

	dst.assign(begin + 1, last);
	return last;
}

const std::wstring::const_iterator URLParser::parsePath(
	std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
	if (begin == end || begin[0] != L'/')
	{
		return end;
	}

	std::wstring::const_iterator last = std::find(begin + 1, end, L'?');

	dst.assign(begin, last);
	return last;
}

const std::wstring::const_iterator URLParser::parseQuery(
	std::wstring& dst, std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
	if (begin == end || begin[0] != L'?')
	{
		return end;
	}

	dst.assign(begin, end);
	return end;
}

/*
HttpUrl::HttpUrl()
	: valid_(false)
{
	memset(&url_, 0, sizeof(URL_COMPONENTS));
	url_.dwStructSize = sizeof(URL_COMPONENTS);
}

HttpUrl::HttpUrl(const std::wstring& url)
	: valid_(false)
{
	WCHAR scheme[256] = {'\0'};
	WCHAR host[256] = {'\0'};
	WCHAR user[256] = {'\0'};
	WCHAR password[256] = {'\0'};
	WCHAR path[2048] = {'\0'};
	WCHAR extra[2048] = {'\0'};

	memset(&url_, 0, sizeof(URL_COMPONENTS));
	url_.dwStructSize = sizeof(URL_COMPONENTS);

	url_.lpszScheme = scheme;
	url_.dwSchemeLength = sizeof(scheme) / sizeof(WCHAR);

	url_.lpszHostName = host;
	url_.dwHostNameLength = sizeof(host) / sizeof(WCHAR);

	url_.lpszUserName = user;
	url_.dwUserNameLength = sizeof(user) / sizeof(WCHAR);

	url_.lpszPassword = password;
	url_.dwPasswordLength = sizeof(password) / sizeof(WCHAR);

	url_.lpszUrlPath = path;
	url_.dwUrlPathLength = sizeof(path) / sizeof(WCHAR);

	url_.lpszExtraInfo = extra;
	url_.dwExtraInfoLength = sizeof(extra) / sizeof(WCHAR);


	if (!WinHttpCrackUrl(url.c_str(), url.size(), 0, &url_))
	{
		return;
	}

	scheme_ = scheme;
	host_ = host;
	user_ = user;
	password_ = password;
	path_ = path;
	extra_ = extra;

	auto it = std::find(path_.rbegin(), path_.rend(), '/');
	if (it != path_.rend())
	{
		file_.assign(it.base(), path_.end());
	}

	valid_ = true;
}

std::wstring HttpUrl::scheme()
{
	return scheme_;
}

int HttpUrl::schemeType()
{
	return url_.nScheme;
}

std::wstring HttpUrl::host()
{
	return host_;
}

WORD HttpUrl::port()
{
	return url_.nPort;
}

std::wstring HttpUrl::user()
{
	return user_;
}

std::wstring HttpUrl::password()
{
	return password_;
}

std::wstring HttpUrl::path()
{
	return path_;
}

std::wstring HttpUrl::fullpath()
{
	return path_ + extra_;
}

std::wstring HttpUrl::extra()
{
	return extra_;
}

std::wstring HttpUrl::file()
{
	return file_;
}
*/
/////////////////////////////////////////////////////////////

HttpClient::~HttpClient()
{
	close();
}

bool HttpClient::open(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::wstring& header, int timeout)
{
	close();
	URLParser urlx(url);

	if (proxy.get.enable && match(urlx.host(), proxy.white, proxy.black))
	{
		// バイパスリスト
		// プロクシを介さずアクセスするサーバーリスト
		// <local>でローカル通信すべて
		// ; で区切って複数設定可

		// セッションハンドルを取得
		session_ = WinHttpOpen(
			userAgent.c_str(), 
			WINHTTP_ACCESS_TYPE_NAMED_PROXY,
			proxy.get.name.c_str(),
			L"<local>",
			0);
	}
	else
	{
		session_ = WinHttpOpen(
			userAgent.c_str(),
			WINHTTP_ACCESS_TYPE_NO_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);
	}

	if (session_)
	{
		WinHttpSetTimeouts(session_, timeout, timeout, timeout, timeout);

		connect_ = WinHttpConnect(
			session_,
			urlx.host().c_str(),
			urlx.port(),
			0);
	}

	if (connect_)
	{
		request_ = WinHttpOpenRequest(
			connect_,
			L"GET",
			urlx.fullpath().c_str(), // path
			NULL, // the HTTP version. NULL, the function uses HTTP/1.1.
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			urlx.scheme() == L"https" ? WINHTTP_FLAG_SECURE : 0);
	}

	BOOL result = FALSE;

	if (request_)
	{
		result = WinHttpSendRequest(
			request_,
			header.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : header.c_str(),
			header.empty() ? 0 : -1,
			WINHTTP_NO_REQUEST_DATA,
			0, 
			0,
			0);
	}

	if (result)
	{
		result = WinHttpReceiveResponse(request_, NULL);
	}

	if (!result)
	{
		close();
		return false;
	}

	valid_ = true;
	return true;
}

bool HttpClient::post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const char* data, DWORD size, const std::wstring& header, int timeout)
{
	close();
	URLParser urlx(url);

	if (proxy.post.enable && match(urlx.host(), proxy.white, proxy.black))
	{
		// バイパスリスト
		// プロクシを介さずアクセスするサーバーリスト
		// <local>でローカル通信すべて
		// ; で区切って複数設定可

		// セッションハンドルを取得
		session_ = WinHttpOpen(
			userAgent.c_str(), 
			WINHTTP_ACCESS_TYPE_NAMED_PROXY,
			proxy.post.name.c_str(),
			L"<local>",
			0);
	}
	else
	{
		session_ = WinHttpOpen(
			userAgent.c_str(),
			WINHTTP_ACCESS_TYPE_NO_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);
	}

	if (session_)
	{
		WinHttpSetTimeouts(session_, timeout, timeout, timeout, timeout);

		connect_ = WinHttpConnect(
			session_,
			urlx.host().c_str(),
			urlx.port(),
			0);
	}

	if (connect_)
	{
		request_ = WinHttpOpenRequest(
			connect_,
			L"POST",
			urlx.fullpath().c_str(), // path
			NULL, // the HTTP version. NULL, the function uses HTTP/1.1.
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			urlx.scheme() == L"https" ? WINHTTP_FLAG_SECURE : 0);
	}

	BOOL result = FALSE;

	if (request_)
	{
		result = WinHttpSendRequest(
			request_,
			header.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : header.c_str(),
			header.empty() ? 0 : -1,
			WINHTTP_NO_REQUEST_DATA,
			0, 
			size,
			0);
	}

	if (result)
	{
		DWORD written = 0;
		result = WinHttpWriteData(request_, data, size, &written);
	}

	if (result)
	{
		result = WinHttpReceiveResponse(request_, NULL);
	}

	if (!result)
	{
		close();
		return false;
	}

	valid_ = true;
	return true;
}


bool HttpClient::post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::vector<char>& data, const std::wstring& header, int timeout)
{
	return post(url, userAgent, proxy, data.data(), (DWORD)data.size(), header, timeout);
}

bool HttpClient::post(const std::wstring& url, const std::wstring& userAgent, const Proxy& proxy, const std::string& data, const std::wstring& header, int timeout)
{
	return post(url, userAgent, proxy, data.data(), (DWORD)data.size(), header, timeout);
}

int HttpClient::recv(std::vector<char>& dst)
{
	if (!valid_) { return false; }

	std::vector<char> tmp;
	DWORD size = 0, downloaded = 0;

	if (!WinHttpQueryDataAvailable(
		request_,
		&size))
	{
		DWORD error = GetLastError();
		if (error == ERROR_WINHTTP_TIMEOUT)
		{
			return STATE_TIMEOUT;
		}
		return STATE_ERROR;
	}

	if (size == 0) { return STATE_EOF; }

	tmp.resize(size);

	downloaded = 0;
	if (!WinHttpReadData(
		request_,
		tmp.data(), 
		size,
		&downloaded))
	{
		DWORD error = GetLastError();
		if (error == ERROR_WINHTTP_TIMEOUT)
		{
			return STATE_TIMEOUT;
		}
		return STATE_ERROR;
	}
	std::swap(dst, tmp);
	return STATE_OK;
}



int HttpClient::get(std::vector<char>& dst)
{
	if (!valid_) { return false; }

	int state = STATE_OK;

	while (true)
	{
		std::vector<char> tmp;
		state = recv(tmp);
		if (!tmp.empty())
		{
			dst.insert(dst.end(), tmp.begin(), tmp.end());
		}

		if (state != STATE_OK) { break; }
	}

	return state;
}

int HttpClient::read(void* dst, unsigned int size)
{
	if (!valid_) { return false; }
	
	DWORD lastTime = GetTickCount(); // タイムアウトチェック用
	int state = STATE_OK;
	while (true)
	{
		// 要求サイズを満たしている
		if (buffer_.size() >= size)
		{
			std::memcpy(dst, buffer_.data(), size); // 要求サイズをコピー
			// コピーした分だけバッファから削除する
			std::vector<char> flip(buffer_.begin() + size, buffer_.end());
			std::swap(buffer_, flip);
			break;
		}

		// タイムアウト
		if ((GetTickCount() - lastTime) > (DWORD)timeout_)
		{
			return STATE_TIMEOUT;
		}

		// 読み込み
		std::vector<char> tmp;
		state = recv(tmp);
		// 読み込んだ分をバッファに追加
		buffer_.insert(buffer_.end(), tmp.begin(), tmp.end());

		// 要求サイズを読み込めずに中断
		if (state != STATE_OK)
		{
			///////
			if (state == STATE_EOF && !buffer_.empty() && buffer_.size() <= size)
			{
				std::memcpy(dst, buffer_.data(), buffer_.size());
			}
			///////
			return state;
		}
	}
	return STATE_OK;
}

bool HttpClient::queryStatus(DWORD& code)
{
	if (request_ == NULL) { return false; }

	DWORD size = sizeof(DWORD);
	return WinHttpQueryHeaders( 
		request_, 
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, 
		&code, 
		&size,
		WINHTTP_NO_HEADER_INDEX
		) ? true : false;
}

bool HttpClient::queryInfo(DWORD infoLevel, std::wstring& text)
{
	if (request_ == NULL) { return false; }

	DWORD size = 0;

	WinHttpQueryHeaders(
		request_, 
		infoLevel,
		WINHTTP_HEADER_NAME_BY_INDEX, 
		WINHTTP_NO_OUTPUT_BUFFER, 
		&size,
		WINHTTP_NO_HEADER_INDEX
		);

	DWORD err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
	{
		return false;
	}

	std::vector<WCHAR> buff;
	buff.resize(size / sizeof(WCHAR));

	BOOL result = WinHttpQueryHeaders( 
		request_, 
		infoLevel,
		WINHTTP_HEADER_NAME_BY_INDEX, 
		buff.data(), 
		&size,
		WINHTTP_NO_HEADER_INDEX
		);
	if (result)
	{
		text = buff.data();
		return true;
	}
	return false;
}

bool HttpClient::queryContentEncoding(std::wstring& text)
{
	return queryInfo(WINHTTP_QUERY_CONTENT_ENCODING, text);
}

bool HttpClient::queryLastModified(std::wstring& text)
{
	return queryInfo(WINHTTP_QUERY_LAST_MODIFIED, text);
}

bool HttpClient::querySetCookie(std::wstring& text)
{
	return queryInfo(WINHTTP_QUERY_SET_COOKIE, text);
}


void HttpClient::close()
{
	closeHandle(request_);
	closeHandle(connect_);
	closeHandle(session_);
	valid_ = false;
	buffer_.clear();
}

bool HttpClient::match(const std::wstring& host, const std::wstring& rx)
{
	using namespace boost::xpressive;
	try {
		wsmatch what;
		if (regex_match(host, what, wsregex::compile(rx)))
		{
			return true;
		}
	}
	catch (...) {}
	return false;
}

bool HttpClient::match(const std::wstring& host, const std::wstring& white, const std::wstring& black)
{
	return match(host, white) && !match(host, black);
}

} // namespace network

