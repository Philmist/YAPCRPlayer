#pragma once

#include "Network.h"
#include "Mlang.h"

#include "HTMLEscape.h"

#include <unordered_map>

#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>

#pragma warning(push)
#pragma warning(disable:4244)
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#pragma warning(pop)


namespace bbs {

enum BBS_STATE {
	STATE_ERROR_POST = -6,		// 書き込み失敗
	STATE_ERROR_RECIEVE = -5,	// 読み込み失敗
	STATE_ERROR_CODE = -4,		// ステータスコードが200と304以外
	STATE_ERROR_CONNECT = -3,	// 接続失敗
	STATE_ERROR_NOKEY = -2,		// スレッドキーがない
	STATE_ERROR_INVALID = -1,	// 無効/初期化失敗

	STATE_OK = 0,				// 更新成功
	STATE_NOT_MODIFIED = 1,		// 更新なし
};

// 掲示板情報
struct BBSInfo {
	struct Board {
		Board()
			: code(Mlang::SHIFT_JIS)
		{}

		Mlang::CODE code;		// 文字コード

		std::wstring scheme;	// スキーム
		std::wstring host;		// ホスト
		std::wstring base;		// 基底URL
		std::wstring board;		// 掲示板
		std::wstring number;	// 掲示板番号

		std::wstring title;		// タイトル(setting.txt)
		std::wstring noname;	// 名無し名
		std::wstring url;
	};

	struct Thread {
		Thread()
			: count(0)
			, stop(1000)
		{}

		std::wstring key;		// スレッド番号
		std::wstring title;		// タイトル(subject.txt)
		std::wstring url;
		int count;				// スレッドのレス数(subject.txt/dat)
		int stop;				// スレッド終了レス数(setting.txt)
	};

	Thread thread;
	Board board;
};

// スレッド情報
struct ThreadInfo {
	ThreadInfo()
		: count(0)
		, speed(0.0)
	{}

	std::wstring number;	// 番号
	std::wstring key;		// スレッド番号(立てた時のUNIX時間)
	std::wstring title;		// タイトル
	int count;				// レス数
	double speed;			// 勢い(一日あたりのレス数)
};

// レス情報
struct ResInfo {
	ResInfo()
		: del(false)
		, latest(true)
		, count(0)
		, total(0)
		, identifier(0)
	{}

	bool del;				// 削除
	bool latest;			// 最新
	std::wstring number;	// 番号
	std::wstring name;		// 名前
	std::wstring mail;		// メール
	std::wstring datetime;	// 日時
	std::wstring message;	// レス
	std::wstring title;		// スレッドタイトル(>>1以外は空)
	std::wstring id;		// ID
	int count;				// ID数
	int total;				// ID総数
	int identifier;			// ID識別番号

	std::vector<std::wstring> link; // 抽出されたurl
	std::vector<int> ref; // 被参照レス

	std::vector<std::pair<std::wstring, bool> > split; // <b>で区切った名前
};

class BaseBBS {
protected:
	bool valid_;

	BBSInfo bbs_;

	std::unordered_map<std::wstring, std::wstring> setting_;
	std::vector<ThreadInfo> subject_;

	struct Dat {
		Dat() : init(true) {}
		bool init;
		std::map<int, ResInfo> res;
	} dat_;

	std::wstring modSetting_;
	std::wstring modSubject_;
	std::wstring modDat_;

	struct Raw {
		Raw() : size(0) {}

		std::vector<char> dat;
		size_t size;

		void clear() { dat.clear(); size = 0; }
	} raw_;

	struct Regex {
		boost::xpressive::wsregex compile(const std::wstring& text, boost::xpressive::regex_constants::syntax_option_type flag = boost::xpressive::regex_constants::ECMAScript)
		{
			try {
				return boost::xpressive::wsregex::compile(text, flag);
			} catch(...) {}
			return boost::xpressive::wsregex();
		}

		const boost::xpressive::wsregex& scheme()
		{
			const static boost::xpressive::wsregex rx = compile(L".*://(([^/]+).*)/test/read\\.cgi/([^/]+)/([^/]+)");
			return rx;
		}
	} regex_;

public:
	BaseBBS();
	BaseBBS(const std::wstring& url, const std::wstring& userAgent, const network::Proxy& proxy, int timeout);
	virtual ~BaseBBS() {}

	operator bool() { return valid_; }

	void getInfo(BBSInfo& info);

	bool change(const std::wstring& key);

	int loadSetting(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);
	int loadSubject(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);
	int loadDat(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);

	bool getSubject(std::vector<ThreadInfo>& data);
	bool getDat(std::vector<ResInfo>& res, int& pos);

	int post(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, const std::wstring& name, const std::wstring& mail, const std::wstring& message,
		DWORD& code, std::wstring& error);

private:
	bool check(const std::wstring& str, std::wstring& error);
	bool datetimeid(ResInfo& res, const std::wstring& datetime);

protected:
	virtual int download(std::vector<char>& raw, std::wstring& modified, const std::wstring& url, const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);
	virtual int partial(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);

	virtual bool parser(std::wstring& name, std::wstring& value, const std::wstring& line);
	virtual bool parser(ThreadInfo& thr, const std::wstring& line);
	virtual bool parser(ResInfo& res, const std::wstring& line);

	inline virtual std::wstring board();
	inline virtual std::wstring thread();
	inline virtual std::wstring setting();
	inline virtual std::wstring subject();
	inline virtual std::wstring dat();
	inline virtual std::wstring write();

	inline virtual std::string query(const std::wstring& name, const std::wstring& mail, const std::wstring& message);
	inline virtual std::wstring cookie(const std::wstring& name, const std::wstring& mail);

protected:
	utl::TagRemover removeATag_;
};

class ShitarabaBBS : public BaseBBS {
public:
	ShitarabaBBS(const std::wstring& url, const std::wstring& userAgent, const network::Proxy& proxy, int timeout);

protected:
	int partial(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code);

	bool parser(ThreadInfo& thr, const std::wstring& line);
	bool parser(ResInfo& res, const std::wstring& line);

	inline std::wstring board();
	inline std::wstring thread();
	inline std::wstring setting();
	inline std::wstring subject();
	inline std::wstring dat();
	inline std::wstring write();

	inline std::string query(const std::wstring& name, const std::wstring& mail, const std::wstring& message);
};



} // namespace bbs

