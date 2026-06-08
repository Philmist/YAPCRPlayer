#pragma once

#include "BBSManager.h"
#include "BBSSkin.h"

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

#define DEFAULT_BBS_USERAGENT	L"Monazilla/1.00 (PCRPlayer/1.00)"
#define DEFAULT_BBS_TIMEOUT		10000
#define DEFAULT_BBS_SHITARABA	L"jbbs\\.shitaraba\\.net"
#define DEFAULT_BBS_INTERVAL	7
#define DEFAULT_BBS_BLOCK		true
#define DEFAULT_BBS_BROWSER		false
#define DEFAULT_BBS_LATEST		false
#define DEFAULT_BBS_RANGE		5
#define DEFAULT_BBS_HISTORY		30
#define DEFAULT_BBS_RES			10
#define DEFAULT_BBS_WRITE		true

namespace bbs {

// 履歴
#define HISTORY_MAX 100

struct Link {
	std::wstring title;
	std::wstring url;
};

typedef std::vector<Link> Links;

// 掲示板設定
struct BoardControl {
	BoardControl()
		: userAgent(DEFAULT_BBS_USERAGENT)
		, timeout(DEFAULT_BBS_TIMEOUT)
		, shitaraba(DEFAULT_BBS_SHITARABA)
		, block(DEFAULT_BBS_BLOCK)
		, reload(true)
		, interval(DEFAULT_BBS_INTERVAL)
		, thread(true)
		, board(true)
		, move(true)
		, browser(DEFAULT_BBS_BROWSER)
		, latest(DEFAULT_BBS_LATEST)
		, range(DEFAULT_BBS_RANGE)
		, res(DEFAULT_BBS_RES)
		, write(DEFAULT_BBS_WRITE)
	{
		typedef utl::ReplaceElement RE;
		url.push_back(RE(true, L"したらば旧ドメインを置換", L"jbbs\\.livedoor\\.jp/", L"jbbs.shitaraba.net/", false));
		url.push_back(RE(true, L"したらばLite版アドレスを置換", L"jbbs\\.shitaraba\\.net/bbs/lite/", L"jbbs.shitaraba.net/bbs/", false));
		url.push_back(RE(true, L"したらばスレッド一覧を掲示板URLに置換", L"jbbs\\.shitaraba\\.net/bbs/subject\\.cgi/", L"jbbs.shitaraba.net/", false));
		url.push_back(RE(false, L"空URLを置換", L"^$", L"http://", false));

		message.push_back(RE(true, L"空白に置換", L"&#160;", L"&nbsp;", false));
		message.push_back(RE(true, L"文頭の改行を削除", L"^([ 　]*<br>)+", L"", false));
		message.push_back(RE(true, L"文末の改行を削除", L"(<br>[ 　]*)+$", L"", false));
		message.push_back(RE(true, L"行頭の空白を削除", L"^[ 　]|(<br>)[ 　]", L"$1", false));
		message.push_back(RE(true, L"行末の空白を削除", L"[ 　](<br>)|[ 　]$", L"$1", false));
		message.push_back(RE(true, L"二行以上の空行を省略", L"([ 　]*<br>){2,}", L"<br><br>", false));
		message.push_back(RE(false, L"空行を削除", L"([ 　]*<br>)+", L"<br>", false));
		message.push_back(RE(false, L"10行より多い行を省略", L"((.*?<br>){10})(.*)", L"$1（省略されました）", false));
		message.push_back(RE(false, L"連続するｗを省略", L"[wWｗＷ]{2,}(?![A-Za-z0-9_%&\\--/=])", L"ｗ", false));
	}

	struct History {
		History()
			: board(DEFAULT_BBS_HISTORY)
			, thread(DEFAULT_BBS_HISTORY)
		{}

		int board; // 掲示板履歴保存数
		int thread; // スレッド履歴保存数
	};

	std::wstring userAgent; // userAgent
	int timeout; // タイムアウト(ms)
	std::wstring shitaraba; // したらば掲示板ドメイン
	bool block; // レスポップアップを表示しているときだけ自動更新する
	bool reload; // スレッド自動更新
	int interval; // スレッド自動更新間隔
	bool thread; // スレッド移動/新規スレッド終了時
	bool board; // スレッド移動/新規掲示板
	bool move; // スレッド移動/スレッド終了時
	bool browser; // ブラウザモード
	bool latest; // 新規読み込み時のレスに新着フラグを立てる
	int range; // 参照レス範囲
	History history; // 履歴保存数
	int res; // レス表示数(res < 0ですべてのレス)
	bool write; // 書き込みが成功したらスレッド更新する
	network::Proxy proxy; // プロクシ
	std::vector<utl::ReplaceElement> url; // URL置換
	std::vector<utl::ReplaceElement> message; // レス本文置換
};

struct BoardControlEx : public BoardControl {
	BoardControlEx()
	{
		block = false; // レスポップアップを表示しているときだけ自動更新する
		browser = true; // ブラウザモード
	}
};

// 更新コールバックコマンド
enum UPDATE {
	UPDATE_ALL,
	UPDATE_COUNTER,
	UPDATE_TITLE,
	UPDATE_STATUS,
	UPDATE_POST,
	//UPDATE_DELAY_TITLE,
	UPDATE_DELAY_STATUS,
	UPDATE_DELAY_POST,
};

class BBSOperator : protected utl::Logger {
private:
	struct Callback {
		Callback()
			: update([](long){})
			, post([](bool){})
			, subject([](bool){})
			, dat([](bool){})
			, extract([](){})
		{}

		utl::SyncValue<boost::function<void(long)> > update; // ステータス更新(TITLE/STATUS/POST/ALL)
		utl::SyncValue<boost::function<void(bool)> > post; // 投稿結果(SUCCEEDED/FAILED)
		utl::SyncValue<boost::function<void(bool)> > subject; // スレッド一覧取得(START/END)
		utl::SyncValue<boost::function<void(bool)> > dat; // dat取得(SCROLL/UPDATE)
		utl::SyncValue<boost::function<void()> > extract; // extract取得
	};

	// スレッド多重起動制限
	struct Thread {
		utl::SyncThread reload;
		utl::SyncThread dat;
		utl::SyncThread create;
		utl::SyncThread subject;
		utl::SyncThread change;
		utl::SyncThread post;
	};

	// スレッド多重実行制限
	struct Mutex {
		boost::recursive_mutex execute;
		boost::recursive_mutex reload;
		boost::recursive_mutex dat;
		boost::recursive_mutex create;
		boost::recursive_mutex subject;
		boost::recursive_mutex change;
		boost::recursive_mutex post;

		boost::recursive_mutex history;
	};

	utl::SyncValue<BoardControl> cfg_;

	utl::SyncValue<std::wstring> path_; // 再生している動画のパス 変更されたらリセット
	utl::SyncValue<std::wstring> source_; // スレッド/掲示板URL 同じURLならスレッド一覧更新
	utl::SyncValue<bool> once_; // path_ 変更ごとに有効化
	utl::SyncValue<bool> block_; // 自動更新ブロック

	Thread thread_;
	Mutex mutex_;
	BBSManager bbs_;

	utl::SyncOptional<std::vector<ResInfo> > dat_;
	utl::SyncOptional<std::vector<ResInfo> > partial_;
	utl::SyncOptional<bool> init_;
	utl::SyncOptional<std::vector<ResInfo> > extract_;
	utl::SyncOptional<std::vector<ThreadInfo> > subject_;

	utl::SyncOptional<std::wstring> title_;
	utl::SyncOptional<std::wstring> status_;
	utl::SyncOptional<std::wstring> post_;

	utl::SyncOptional<SYSTEMTIME> time_;
	utl::SyncOptional<int> counter_;
	utl::SyncOptional<std::vector<std::wstring> > links_;

	Callback callback_;

	struct History {
		Links board;
		Links thread;
	};

	History history_;

	void initialize();

public:
	BBSOperator(utl::SyncLog& log);
	virtual ~BBSOperator();

	operator bool() { return bbs_; }

	void terminate();

public:
	void setConfig(const BoardControl& cfg) { cfg_ = cfg; reset(); }

	void setUpdateCallback(const boost::function<void(long)>& callback) { callback_.update = callback; }
	void setPostCallback(const boost::function<void(bool)>& callback) { callback_.post = callback; }
	void setSubjectCallback(const boost::function<void(bool)>& callback) { callback_.subject = callback; }
	void setDatCallback(const boost::function<void(bool)>& callback) { callback_.dat = callback; }
	void setExtractCallback(const boost::function<void()>& callback) { callback_.extract = callback; }

	void setBlock(bool block) { block_ = block; }

public:
	// ファイルが変わった時に初期化する
	bool init(const std::wstring path);
	// 設定読み込みとスレッド読み込み
	void execute(const std::wstring& board, const BoardControl& cfg, bool move = true, bool once = false);
	bool refresh();
	bool reload();
	void interrupt();
	bool create();
	bool reset();
	bool scroll(int diff);
	bool extract();
	bool extract(const std::wstring& text, bool convert, int size = 0, bool head = true);
	bool extract(const bbs::Range& range, int size = 0, bool head = true);
	bool extract(const std::wstring& id, int size = 0, bool head = true);
	bool extract(int ref, int size = 0, bool head = true);
	bool extract(const std::vector<int>& vec, int size = 0, bool head = true);
	bool subject(); // 板一覧更新
	bool change(const std::wstring& key); // スレッド変更と読み込み
	bool post(const std::wstring& name, const std::wstring& mail, const std::wstring& message);

public:
	bool getDat(std::vector<ResInfo>& dat, bool& init)
	{
		if (dat_.get(dat) && init_.get(init))
		{
			init_ = false;
			return true;
		}
		return false;
	}
	bool getDat(std::vector<ResInfo>& dat)
	{
		if (partial_.get(dat))
		{
			utl::HTMLEscape esc;
			utl::NameSplitter split;
			for (auto it = dat.begin(); it != dat.end(); ++it)
			{// エスケープ処理
				split(it->name, it->split);//エスケープする前に分割
				esc.replace(it->name);
				esc.replace(it->mail);
				esc.replace(it->datetime);
				esc.replace(it->message);
				esc.replace(it->title);
				esc.replace(it->id);
			}
			return true;
		}
		return false;
	}
	bool getExtract(std::vector<ResInfo>& dat) { return extract_.get(dat); }
	bool getLinks(std::vector<std::wstring>& links) { return links_.get(links); }
	bool getSubject(std::vector<ThreadInfo>& subject) { return subject_.get(subject); }

public:
	void getSourceURL(std::wstring& url) { url = source_.get(); }
	bool getURL(std::wstring& url) { return bbs_.getURL(url); }
	bool getBoardURL(std::wstring& url) { return bbs_.getBoardURL(url); }
	bool getThreadURL(std::wstring& url) { return bbs_.getThreadURL(url); }
	bool getBoardTitle(std::wstring& title) { return bbs_.getBoardTitle(title); }
	bool getThreadTitle(std::wstring& title) { return bbs_.getThreadTitle(title); }
	bool getNoName(std::wstring& noname) { return bbs_.getNoName(noname); }
	bool getCount(int& count) { return bbs_.getCount(count); }
	bool getKey(std::wstring& key) { return bbs_.getKey(key); }
	bool isThread() { return bbs_.hasKey(); }
	bool getStop(int& stop) { return bbs_.getStop(stop); }
	bool isStop() { return bbs_.isStop(); }

public:
	void setTitle(const std::wstring& title) { title_ = title; }
	bool getTitle(std::wstring& title) { return title_.get(title); }
	void setStatus(const std::wstring& status) { status_ = status; }
	bool getStatus(std::wstring& status) { return status_.get(status); }
	void setPost(const std::wstring& post) { post_ = post; }
	bool getPost(std::wstring& post) { return post_.get(post); }
	void clearPost() { post_.clear(); }
	bool getCounter(int& counter) { return counter_.get(counter); }
	bool isReload() { return counter_; }
	bool getTime(SYSTEMTIME& time) { return time_.get(time); }

public:
	void addHistory(bool thread);
	void resizeHistory(bool thread);
	void getHistory(bool thread, Links& links);
	void setHistory(bool thread, const Links& links);
	void clearHistory(bool thread);

private:
	bool isValid();
	bool isBrowser();
	std::wstring getSourceTitle();
	std::wstring getBoardTitle();
	std::wstring getThreadTitle();
	bool getFastest(ThreadInfo& info);
	void cut(std::vector<ResInfo>& dat, int size, bool head = true);

private:
	struct ReloadControl {
		ReloadControl(bool reload, bool stop, bool move, bool once)
			: reload(reload)
			, stop(stop)
			, move(move)
			, once(once)
		{}

		bool reload; // 自動更新
		bool stop; // レス数上限なら停止
		bool move; // レス数上限のときスレッド移動を有効
		bool once; // 初回だけカウンターを無視して即座に更新(書き込み成功時等)
	};

	void executeThread(const std::wstring& board, const BoardControl& cfg, bool move, bool once);
	void reloadThread(const ReloadControl& ctrl);
	void datThread(const ReloadControl& ctrl);
	void createThread(int diff, bool reset, bool scroll);
	void extractThread();
	void extractThread(const std::wstring& text, bool convert, int size, bool head);
	void extractThread(const bbs::Range& range, int size, bool head);
	void extractThread(const std::wstring& id, int size, bool head);
	void extractThread(int ref, int size, bool head);
	void extractThread(const std::vector<int>& vec, int size, bool head);
	bool subjectThread();
	void changeThread(const std::wstring& key, const ReloadControl& ctrl);
	void postThread(const std::wstring& name, const std::wstring& mail, const std::wstring& message);
};


} // namespace bbs