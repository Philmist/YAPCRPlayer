#include "stdafx.h"

namespace bbs {

std::string urlencode(const std::wstring& src, Mlang::CODE code)
{
	return utl::urlencode(Mlang::decode(src, code));
}

double calcSpeed(int count, const std::wstring& key)
{
	time_t start = wcstoul(key.c_str(), NULL, 10);
	start = time(NULL) - start;
	double day = start > 0 ? (double)(start) / (60.0 * 60.0 * 24.0) : 0.0;

	return day > 0.0 ? (double)count / day : 0.0;
}

double levenshteinDistance(const std::wstring& str1, const std::wstring& str2)
{
	if (str1.empty()) { return 0.0; }
	if (str2.empty()) { return 0.0; }

	int x = (int)str1.size();
	int y = (int)str2.size();

	std::vector< std::vector<int> > d;
	d.resize(x + 1);
	for (auto it = d.begin(); it != d.end(); ++it)
	{
		it->resize(y + 1);
	}

	for (int i = 0; i <= x; ++i)
	{
		d[i][0] = i;
	}

	for (int i = 0; i <= y; ++i)
	{
		d[0][i] = i;
	}

	for (int i = 1; i <= x; ++i)
	{
		for (int j = 1; j <= y; ++j)
		{
			const int cost = str1[i - 1] == str2[j - 1] ? 0 : 1;

			d[i][j] = min(min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);

			if (i > 1 && j > 1
				&& str1[i - 1] == str2[j - 2]
				&& str1[i - 2] == str2[j - 1])
				{
					d[i][j] = min(d[i][j], d[i - 2][j - 2] + cost);
				}
		}
	}

	int big = max(x, y);
	return (double)d[x][y] / (double)big;
}


std::wstring makeScheme(const std::wstring& text)
{
	using namespace boost::xpressive;
	try {
		const static wsregex rx = wsregex::compile(L"(?:http|ttp|tp)(s)?$");
		wsmatch what;
		if (regex_search(text, what, rx))
		{
			return std::wstring(L"http" + what[1]);
		}
	} catch(...) {}
	return std::wstring(L"http");
}

//==============================================================================

BaseBBS::BaseBBS()
	: valid_(false)
	, removeATag_(L"a")
{}

BaseBBS::BaseBBS(const std::wstring& url, const std::wstring& userAgent, const network::Proxy& proxy, int timeout)
	: valid_(false)
	, removeATag_(L"a")
{
	using namespace boost::xpressive;

	try {
		wsregex rx;
	
		if (url.find(L"test/read.cgi") != std::wstring::npos)
		{
			const static wsregex r = wsregex::compile(L"(.*)://(([^/]+).*)/test/read\\.cgi/([^/]+)/([^/]+)");
			rx = r;
		}
		else
		{
			const static wsregex r = wsregex::compile(L"(.*)://(([^/]+).*)/([^/]+)");
			rx = r;
		}

		wsmatch what;
		if (regex_search(url, what, rx))
		{
			bbs_.board.scheme = makeScheme(what[1]);
			bbs_.board.base = what[2];
			bbs_.board.host = what[3];
			bbs_.board.board = what[4];
			bbs_.thread.key = what[5];

			bbs_.board.code = Mlang::SHIFT_JIS;
			valid_ = true;

			bbs_.board.url = board();
			bbs_.thread.url = thread();

			DWORD code = 0;
			if (loadSetting(userAgent, proxy, timeout, code) >= STATE_OK)
			{
				auto it = setting_.find(L"BBS_TITLE");
				if (it != setting_.end())
				{
					bbs_.board.title = it->second;
				}

				it = setting_.find(L"BBS_NONAME_NAME");
				if (it != setting_.end())
				{
					bbs_.board.noname = it->second;
				}
			}
		}
	}
	catch (...) {}
}

void BaseBBS::getInfo(BBSInfo& info)
{
	info = bbs_;
}

bool BaseBBS::change(const std::wstring& key)
{
	if (!valid_) { return false; }

	for (auto it = subject_.begin(); it != subject_.end(); ++it)
	{
		if (it->key == key)
		{
			bbs_.thread.key = key;
			bbs_.thread.title = it->title;
			bbs_.thread.count = it->count;
			bbs_.thread.url = thread();
			dat_ = Dat();
			raw_.clear();
			modDat_.clear();
			return true;
		}
	}
	return false;
}

int BaseBBS::loadSetting(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	using namespace boost::xpressive;

	if (!valid_) { return STATE_ERROR_INVALID; }

	std::vector<char> raw;
	int stat = download(raw, modSetting_, setting(), userAgent, proxy, timeout, code);
	if (stat != STATE_OK) { return stat; }
	std::wstring str = Mlang::decode(raw, bbs_.board.code);

	setting_.clear();

	try {
		for (wsregex_token_iterator it(str.begin(), str.end(), _n, -1), end; it != end; ++it)
		{
			std::wstring name, value;
			if (parser(name, value, it->str()))
			{
				setting_.insert(std::make_pair(name, value));
			}
		}
	} catch (...) {}

	return STATE_OK;
}

int BaseBBS::loadSubject(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	using namespace boost::xpressive;

	if (!valid_) { return STATE_ERROR_INVALID; }

	std::vector<char> raw;
	int stat = download(raw, modSubject_, subject(), userAgent, proxy, timeout, code);
	if (stat != STATE_OK) { return stat; }
	std::wstring str = Mlang::decode(raw, bbs_.board.code);

	auto duplicate = [&](const std::wstring& key)->bool{
		for (auto it = subject_.begin(); it != subject_.end(); ++it)
		{
			if (it->key == key) { return true; }
		}
		return false;
	};

	subject_.clear();

	utl::HTMLEscape esc;
	int i = 1;
	bool key = false;
	try {
		for (wsregex_token_iterator it(str.begin(), str.end(), _n, -1), end; it != end; ++it)
		{
			ThreadInfo thr;
			if (parser(thr, it->str()))
			{
				// 重複スレッドは加えない
				if (duplicate(thr.key)) { continue; }

				thr.title = esc(thr.title);

				if (thr.key == bbs_.thread.key)
				{
					bbs_.thread.title = thr.title;
					bbs_.thread.count = thr.count;
					key = true;
				}

				thr.number =  (boost::wformat(L"%d") % i).str();
				subject_.push_back(thr);
				++i;
			}
		}
	} catch(...) {}

	if (!key)
	{// URLのキーとsubjectのキーが一致していない
		// 掲示板URLは取得できるがkeyが不正
		bbs_.thread.key.clear();
		bbs_.thread.title.clear();
		bbs_.thread.url.clear();
		bbs_.thread.count = 0;
	}
	return STATE_OK;
}

int BaseBBS::loadDat(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	using namespace boost::xpressive;

	if (!valid_) { return STATE_ERROR_INVALID; }
	if (bbs_.thread.key.empty()) { return STATE_ERROR_NOKEY; }

	int stat = partial(userAgent, proxy, timeout, code);
	if (stat != STATE_OK) { return stat; }
	std::wstring str = Mlang::decode(raw_.dat, bbs_.board.code);

	int i = static_cast<int>(dat_.res.size() + 1);
	try {
		for (wsregex_token_iterator it(str.begin(), str.end(), _n, -1), end; it != end; ++it)
		{
			ResInfo res;
			if (parser(res, it->str()))
			{
				if (res.number.empty())
				{// レス番号が無いときは読み込み順に格納する
					res.number = (boost::wformat(L"%d") % i).str();
					dat_.res[i] = res;
				}
				else
				{// レス番号があるならレス番号で格納する
				 // レス番号が0なら読み込み順に格納する
					int n = wcstol(res.number.c_str(), NULL, 10);
					res.number = (boost::wformat(L"%d") % n).str();
					dat_.res[n != 0 ? n : i] = res;

					if (n > i)
					{// 読み込み順よりレス番号が大きい（削除で番号が飛んでいる）
					 // 削除レスを格納して読み込み順を進める
						ResInfo del;
						del.del = true;
						for (int x = i; x < n; ++x)
						{
							del.number = (boost::wformat(L"%d") % x).str();
							dat_.res[x] = del;
						}
						i = n;
					}
				}
				++i;
			}
		}
	} catch (...) {}
	bbs_.thread.count = (int)dat_.res.size();

	return STATE_OK;
}

bool BaseBBS::getSubject(std::vector<ThreadInfo>& data)
{
	if (!valid_) { return false; }
	data = subject_;
	return true;
}

bool BaseBBS::getDat(std::vector<ResInfo>& res, int& pos)
{
	if (!valid_) { return false; }

	if (dat_.init)
	{
		for (auto it = dat_.res.begin(); it != dat_.res.end(); ++it)
		{
			res.push_back(it->second);
		}
		dat_.init = false;
		pos = 0;
	}
	else
	{
		for (auto it = dat_.res.find(pos); it != dat_.res.end(); ++it)
		{
			res.push_back(it->second);
		}
	}
	return true;
}

int BaseBBS::post(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, const std::wstring& name, const std::wstring& mail, const std::wstring& message,
	DWORD& code, std::wstring& error)
{
	if (!valid_) { return STATE_ERROR_INVALID; }
	if (bbs_.thread.key.empty()) { return STATE_ERROR_NOKEY; }

	network::HttpClient client;
	std::string data = query(name, mail, message);
	std::wstring header =
		L"Content-Type: application/x-www-form-urlencoded\r\n"
		L"Referer: " + thread() + L"\r\n"
		L"Accept-Encoding: gzip\r\n"
		L"Cookie: " + cookie(name, mail);

	if (!client.post(write(), userAgent, proxy, data, header, timeout)) { return STATE_ERROR_CONNECT; }

	DWORD c = 0;
	if (client.queryStatus(c))
	{
		code = c;
		// エラー内容を受け取るためここでreturnしない
		//if (code != 200) { return STATE_ERROR_CODE; }
	}

	std::vector<char> buff;
	if (client.get(buff) != network::HttpClient::STATE_EOF) { return STATE_ERROR_RECIEVE; }


	std::wstring text;
	if (client.queryContentEncoding(text))
	{
		if (text.find(L"gzip") != std::wstring::npos)
		{
			std::vector<char> src(buff.begin(), buff.end());
			utl::decompress(buff, src);
		}
	}

	// エラー確認
	std::wstring str = Mlang::decode(buff, bbs_.board.code);
	std::wstring err;
	if (!check(str, err))
	{
		error = err;
		return STATE_ERROR_POST;
	}

	return STATE_OK;
}

bool BaseBBS::check(const std::wstring& str, std::wstring& error)
{
	//gl_.detailLog(L"str", L"%s", str.c_str());

	std::wstring half;
	if (!utl::LCMapStr(LCMAP_HALFWIDTH, str, half))
	{
		half = str;
	}

	//gl_.detailLog(L"half", L"%s", half.c_str());

	using namespace boost::xpressive;
	try {
		const static wsregex rx2chx = wsregex::compile(L"<!--\\s*2ch_X:(.*?)-->", icase);
		const static wsregex rxTitle = wsregex::compile(L"<title>(.*?)</title>", icase);

		const static wsregex err = wsregex::compile(L"error", icase);
		const static wsregex errCookie = wsregex::compile(L"cookie", icase);
		const static wsregex errAngel = wsregex::compile(L"You just summoned a Ruthless Angel.", icase);

		wsmatch what;
		if (regex_search(half, what, rx2chx))
		{
			std::wstring status = what[1];
			if (regex_search(status, what, err))
			{
				error = what[0];
				return false;
			}

			if (regex_search(status, what, errCookie))
			{
				error = what[0];
				return false;
			}
		}

		if (regex_search(half, what, rxTitle))
		{
			std::wstring status = what[1];
			if (regex_search(status, what, err))
			{
				error = what[0];
				return false;
			}

			if (regex_search(status, what, errAngel))
			{
				error = what[0];
				return false;
			}
		}
	} catch (...) {}
	return true;
}

bool BaseBBS::datetimeid(ResInfo& res, const std::wstring& datetime)
{
	using namespace boost::xpressive;
	try {
		std::wstring rx;
		wsmatch what;

		const static wsregex rxDatetime = wsregex::compile(L"([0-9]+/[0-9]+/[0-9]+.*?)\\s*([0-9]+:[0-9]+:[0-9]+\\S*)");
		if (!regex_search(datetime, what, rxDatetime)) { return false; }
		res.datetime = what[1] + L" " + what[2];
	

		const static wsregex rxID = wsregex::compile(L"ID:(\\S*)");
		if (regex_search(datetime, what, rxID))
		{
			res.id = what[1];
		}
		else
		{
			res.id = L"";
		}
	} catch (...) {}
	return true;
}


int BaseBBS::download(std::vector<char>& raw, std::wstring& modified, const std::wstring& url, const std::wstring& userAgent, const network::Proxy& proxy, int timeout,
	DWORD& code)
{
	std::wstring header = L"Accept-Encoding: gzip";
	if (!modified.empty())
	{
		header += L"\r\nIf-Modified-Since: " + modified;
	}

	network::HttpClient client;
	if (!client.open(url, userAgent, proxy, header, timeout)) { return STATE_ERROR_CONNECT; }

	DWORD c = 0;
	if (client.queryStatus(c))
	{
		code = c;
		switch (code)
		{
		case 304: return STATE_NOT_MODIFIED; // 更新されていない
		case 200: break; // 成功
		default: // エラー
			client.close();
			return STATE_ERROR_CODE;
		}
	}

	std::vector<char> buff;
	if (client.get(buff) != network::HttpClient::STATE_EOF) { return STATE_ERROR_RECIEVE; }

	std::wstring mod;
	if (client.queryLastModified(mod))
	{
		modified = mod;
	}

	std::wstring text;
	if (client.queryContentEncoding(text))
	{
		if (text.find(L"gzip") != std::wstring::npos)
		{
			std::vector<char> src(buff.begin(), buff.end());
			utl::decompress(buff, src);
		}
	}

	raw = buff;
	return STATE_OK;
}

int BaseBBS::partial(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	auto reread = [&]() {
		int stat = download(raw_.dat, modDat_, dat(), userAgent, proxy, timeout, code);
		if (stat == STATE_OK)
		{
			dat_ = Dat();
			raw_.size = raw_.dat.size();
		}
		return stat;
	};

	if (raw_.size == 0)
	{
		return reread();
	}

	size_t size = raw_.size - 1; // 削除された時の検証用に1バイト前の改行を読む

	// gzipは差分取得時には使えない
	std::wstring header = L"Range: bytes=" + (boost::wformat(L"%u-") % size).str();
	if (!modDat_.empty())
	{
		header += L"\r\nIf-Modified-Since: " + modDat_;
	}

	network::HttpClient client;
	if (!client.open(dat(), userAgent, proxy, header, timeout)) { return STATE_ERROR_CONNECT; }

	DWORD c = 0;
	if (client.queryStatus(c))
	{
		// 200 OK
		// 206 Partial Content
		// 304 Not Modified
		// 416 Requested Range Not Satisfiable
		code = c;
		switch (c)
		{
		case 206: break; // 部分取得成功
		case 304: return STATE_NOT_MODIFIED; // 更新されていない
		default: // 読み直し
			client.close();
			return reread();
		}
	}

	std::vector<char> buff;
	if (client.get(buff) != network::HttpClient::STATE_EOF) { return STATE_ERROR_RECIEVE; }

	if (buff.empty() || buff[0] != L'\n')
	{// 読み直し
		client.close();
		return reread();
	}

	if (buff.size() == 1)
	{
		return STATE_NOT_MODIFIED;
	}

	std::wstring mod;
	if (client.queryLastModified(mod))
	{
		modDat_ = mod;
	}

	raw_.dat.assign(buff.begin() + 1, buff.end());
	raw_.size += raw_.dat.size();
	return STATE_OK;
}


bool BaseBBS::parser(std::wstring& name, std::wstring& value, const std::wstring& line)
{
	using namespace boost::xpressive;

	try {
		//const static wsregex rx = wsregex::compile(L"(.+?)=(.+)");
		const static wsregex rx = (s1 = -+_) >> L'=' >> (s2 = +_);
		wsmatch what;
		if (regex_match(line, what, rx))
		{
			name = what[1];
			value = what[2];
			return true;
		}
	} catch (...) {}
	return false;
}

bool BaseBBS::parser(ThreadInfo& thr, const std::wstring& line)
{
	using namespace boost::xpressive;
	
	try {
		int i = 0;
		const static wsregex rx = as_xpr(L"<>");
		for (wsregex_token_iterator it(line.begin(), line.end(), rx, -1), end; it != end; ++it)
		{
			std::wstring str = it->str();
			switch(i)
			{
			case 0:
				{
					const static wsregex rx = +digit;
					wsmatch what;
					if (!regex_search(str, what, rx)) { return false; }
					thr.key = what.str();
				break;
				}
			case 1:
				{
					// 5ch
					// 1234567890.dat<>タイトル \t (999)
					// ぜろちゃんねるプラス
					// 1234567890.dat<>タイトル (999)
					const static wsregex rx = (s1 = -*_) >> *_s >> L'(' >> (s2 = +digit) >> L')' >> *_s >> eos;
					wsmatch what;
					if (!regex_search(str, what, rx)) { return false; }

					thr.title = what[1];
					thr.count = wcstol(what.str(2).c_str(), NULL, 10);
					thr.speed = calcSpeed(thr.count, thr.key);
				break;
				}
			default: break;
			}
			++i;
		}
		if (i < 2) { return false; }
		return true;
	} catch (...) {}
	return false;
}

bool BaseBBS::parser(ResInfo& res, const std::wstring& line)
{
	using namespace boost::xpressive;

	try {
		int i = 0;
		const static wsregex rx = as_xpr(L"<>");
		for (wsregex_token_iterator it(line.begin(), line.end(), rx, -1), end; it != end; ++it)
		{
			switch(i)
			{
			case 0: res.name = it->str(); break;
			case 1: res.mail = it->str(); break;
			case 2:
				{
					if (!datetimeid(res, it->str()))
					{
						res.datetime = it->str();
					}
					break;
				}
			case 3: res.message = removeATag_(it->str()); break;
			case 4: res.title = it->str(); break;
			default: break;
			}
			++i;
		}
		return true;
	} catch (...) {}
	return false;
}



std::wstring BaseBBS::board()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/"
		+ bbs_.board.board + L"/";
}

std::wstring BaseBBS::thread()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/test/read.cgi/"
		+ bbs_.board.board + L"/"
		+ bbs_.thread.key + L"/";
}

std::wstring BaseBBS::setting()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base	+ L"/"
		+ bbs_.board.board + L"/SETTING.TXT";
}

std::wstring BaseBBS::subject()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/"
		+ bbs_.board.board + L"/subject.txt";
}

std::wstring BaseBBS::dat()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base	+ L"/"
		+ bbs_.board.board + L"/dat/"
		+ bbs_.thread.key + L".dat";
}

std::wstring BaseBBS::write()
{
	return bbs_.board.scheme + L"://" + bbs_.board.base + L"/test/bbs.cgi";
}



std::string BaseBBS::query(const std::wstring& name, const std::wstring& mail, const std::wstring& message)
{
//bbs=[BOARD]&time=[POST_TIME]&FROM=[POST_NAME]&mail=[POST_MAIL]&MESSAGE=[POST_MESSAGE]
//&key=[THREAD]&submit=書き込む

	auto fm = [&](const std::string& n, const std::wstring& v) {
		return n + urlencode(v, bbs_.board.code);
	};
	
	return
		fm("submit=", L"書き込む") +
		fm("&FROM=", name) +
		fm("&mail=", mail) +
		fm("&MESSAGE=", message) +
		fm("&bbs=", bbs_.board.board) +
		fm("&key=", bbs_.thread.key) +
		fm("&time=", (boost::wformat(L"%lld") % time(NULL)).str());
}

std::wstring BaseBBS::cookie(const std::wstring& name, const std::wstring& mail)
{
	auto fm = [&](const std::wstring& n, const std::wstring& v) {
		return n + utl::toUTF16(urlencode(v, bbs_.board.code));
	};
	
	return fm(L"NAME=", name) + fm(L"; MAIL=", mail);
}


//=============================================================================

ShitarabaBBS::ShitarabaBBS(const std::wstring& url, const std::wstring& userAgent, const network::Proxy& proxy, int timeout)
	: BaseBBS()
{
	using namespace boost::xpressive;

	try {
		wsregex rx;
	
		if (url.find(L"bbs/read.cgi") != std::wstring::npos)
		{
			const static wsregex r = wsregex::compile(L"(.*)://([^/]+)/bbs/read\\.cgi/([^/]+)/([^/]+)/([^/]+)");
			rx = r;
		}
		else
		{
			const static wsregex r = wsregex::compile(L"(.*)://([^/]+)/([^/]+)/([^/]+)");
			rx = r;
		}

		wsmatch what;
		if (regex_search(url, what, rx))
		{
			bbs_.board.scheme = makeScheme(what[1]);
			bbs_.board.host = what[2];
			bbs_.board.base = what[2];
			bbs_.board.board = what[3];
			bbs_.board.number = what[4];
			bbs_.thread.key = what[5];

			bbs_.board.code = Mlang::EUC_JP;
			valid_ = true;

			bbs_.board.url = board();
			bbs_.thread.url = thread();

			DWORD code = 0;
			if (loadSetting(userAgent, proxy, timeout, code) >= STATE_OK)
			{
				auto it = setting_.find(L"BBS_TITLE");
				if (it != setting_.end())
				{
					bbs_.board.title = it->second;
				}

				it = setting_.find(L"BBS_NONAME_NAME");
				if (it != setting_.end())
				{
					bbs_.board.noname = it->second;
				}

				it = setting_.find(L"BBS_THREAD_STOP");
				if (it != setting_.end())
				{
					bbs_.thread.stop = wcstol(it->second.c_str(), NULL, 10);
				}
			}
		}
	} catch (...) {}
}

int ShitarabaBBS::partial(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	if (raw_.size == 0)
	{
		int stat = download(raw_.dat, modDat_, dat(), userAgent, proxy, timeout, code);
		if (stat == STATE_OK)
		{
			dat_ = Dat();
			raw_.size = raw_.dat.size();
		}
		return stat;
	}

	int stat = download(raw_.dat, modDat_, dat() + (boost::wformat(L"%u-") % (dat_.res.size() + 1)).str(), userAgent, proxy, timeout, code);
	if (stat == STATE_OK)
	{
		raw_.size += raw_.dat.size();
	}
	return stat;
}

bool ShitarabaBBS::parser(ThreadInfo& thr, const std::wstring& line)
{
	using namespace boost::xpressive;

	try {
		//const static wsregex rx = wsregex::compile(L"([0-9]+)\\.cgi,(.*)\\(([0-9]+)\\)");
		const static wsregex rx = (s1 = +digit) >> L".cgi," >> (s2 = *_) >> L'(' >> (s3 = +digit) >> L')';
		wsmatch what;
		if (regex_match(line, what, rx))
		{
			thr.key = what[1];
			thr.title = what[2];
			thr.count = wcstol(what.str(3).c_str(), NULL, 10);
			thr.speed = calcSpeed(thr.count, thr.key);
			return true;
		}
	} catch (...) {}
	return false;
}

bool ShitarabaBBS::parser(ResInfo& res, const std::wstring& line)
{
	using namespace boost::xpressive;

	try {
		int i = 0;
		const static wsregex rx = as_xpr(L"<>");
		for (wsregex_token_iterator it(line.begin(), line.end(), rx, -1), end; it != end; ++it)
		{
			switch(i)
			{
			case 0: res.number = it->str(); break;
			case 1: res.name = it->str(); break;
			case 2: res.mail = it->str(); break;
			case 3: res.datetime = it->str(); break;
			case 4: res.message = removeATag_(it->str()); break;
			case 5: res.title = it->str(); break;
			case 6: res.id = it->str(); break;
			default: break;
			}
			++i;
		}
		return true;
	} catch (...) {}
	return false;
}


std::wstring ShitarabaBBS::board()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/"
		+ bbs_.board.board + L"/"
		+ bbs_.board.number + L"/";
}

std::wstring ShitarabaBBS::thread()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/bbs/read.cgi/"
		+ bbs_.board.board + L"/"
		+ bbs_.board.number + L"/"
		+ bbs_.thread.key + L"/";
}

std::wstring ShitarabaBBS::setting()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/bbs/api/setting.cgi/"
		+ bbs_.board.board + L"/"
		+ bbs_.board.number + L"/";
}

std::wstring ShitarabaBBS::subject()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/"
		+ bbs_.board.board + L"/"
		+ bbs_.board.number + L"/subject.txt";
}

std::wstring ShitarabaBBS::dat()
{
	return bbs_.board.scheme + L"://"
		+ bbs_.board.base + L"/bbs/rawmode.cgi/"
		+ bbs_.board.board + L"/"
		+ bbs_.board.number + L"/"
		+ bbs_.thread.key + L"/";
}

std::wstring ShitarabaBBS::write()
{
	return bbs_.board.scheme + L"://" + bbs_.board.base + L"/bbs/write.cgi";
}



std::string ShitarabaBBS::query(const std::wstring& name, const std::wstring& mail, const std::wstring& message)
{
//DIR=[BOARD]&BBS=[NUMBER]&TIME=[POST_TIME]&NAME=[POST_NAME]&MAIL=[POST_MAIL]&MESSAGE=[POST_MESSAGE]
//&KEY=[THREAD]&submit=書き込む

	auto form = [&](const std::string& n, const std::wstring& v) {
		return n + urlencode(v, bbs_.board.code);
	};
	
	return
		form("submit=", L"書き込む") +
		form("&NAME=", name) +
		form("&MAIL=", mail) +
		form("&MESSAGE=", message) +
		form("&DIR=", bbs_.board.board) +
		form("&BBS=", bbs_.board.number) +
		form("&KEY=", bbs_.thread.key) +
		form("&TIME=", (boost::wformat(L"%lld") % time(NULL)).str());
}


} // namespace bbs

