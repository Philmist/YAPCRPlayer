#include "stdafx.h"




namespace bbs{

bool extractURL(const std::wstring& text, std::vector<std::wstring>& url)
{
	using namespace boost::xpressive;

	try {
		wsregex http = wsregex::compile(L"(?:http|ttp|tp)(s)?$");
		wsregex rx = wsregex::compile(L"([a-zA-Z0-9_]*)(://[a-zA-Z0-9_#$@&!?~:;.,=%/*+-]+)");
		for (wsregex_iterator it(text.begin(), text.end(), rx), end; it != end; ++it)
		{
			std::wstring scheme = it->str(1);
			wsmatch what;
			if (regex_search(scheme, what, http))
			{
				url.push_back(L"http" + what.str(1) + it->str(2));
			}
		}
		return !url.empty();
	} catch (...) {}
	return false;
}


BBSOperator::BBSOperator(utl::SyncLog& log)
	: Logger(log, utl::Log::LOGTYPE_BBS)
	, once_(false)
	, block_(true)
{
}

BBSOperator::~BBSOperator()
{
	//terminate();
}

void BBSOperator::terminate()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.execute);

	thread_.reload.interrupt();
	thread_.reload.join();
	thread_.dat.interrupt();
	thread_.dat.join();
	thread_.create.interrupt();
	thread_.create.join();
	thread_.subject.interrupt();
	thread_.subject.join();
	thread_.change.interrupt();
	thread_.change.join();
	thread_.post.interrupt();
	thread_.post.join();
}

void BBSOperator::initialize()
{
	thread_.reload.interrupt();
	thread_.dat.interrupt();
	thread_.create.interrupt();
	thread_.subject.interrupt();
	thread_.change.interrupt();
	thread_.post.interrupt();

	bbs_.uninit();

	dat_.clear();
	partial_.clear();
	init_.clear();
	extract_.clear();
	subject_.clear();
	title_.clear();
	status_.clear();
	post_.clear();
	counter_.clear();
	links_.clear();

	callback_.update.get()(UPDATE_ALL);
}

bool BBSOperator::init(const std::wstring path)
{
	if (path_.get() == path) { return false; }
	path_ = path;
	source_.apply([](std::wstring& source){ source.clear(); });
	once_ = true;

	initialize();

	create();
	return true;
}

// 同じURLならスレ一覧取得
// URL変更でスレッド初期化
void BBSOperator::execute(const std::wstring& board, const BoardControl& cfg, bool move, bool once)
{
	boost::thread thr(&BBSOperator::executeThread, this, board, cfg, move, once);
}

bool BBSOperator::refresh()
{
	thread_.reload.timed_join(0);
	if (thread_.reload.joinable()) { return false; }

	// 最短更新間隔ブロック
	//if (time_)
	//{
	//	SYSTEMTIME t1, t2;
	//	if (time_.get(t1))
	//	{
	//		GetLocalTime(&t2);
	//		CTime time1(t1), time2(t2);
	//		if ((time2.GetTime() - time1.GetTime()) < DEFAULT_BBS_INTERVAL)
	//		{
	//			return false;
	//		}
	//	}
	//}

	ReloadControl ctrl(counter_, false, false, false);
	boost::thread thr(&BBSOperator::reloadThread, this, ctrl);
	thread_.reload.swap(thr);
	return true;
}


bool BBSOperator::reload()
{
	thread_.reload.timed_join(0);
	if (thread_.reload.joinable()) { return false; }

	ReloadControl ctrl(true, false, false, false);
	boost::thread thr(&BBSOperator::reloadThread, this, ctrl);
	thread_.reload.swap(thr);
	return true;
}

void BBSOperator::interrupt()
{
	thread_.dat.interrupt();
}

bool BBSOperator::create()
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	boost::thread thr(&BBSOperator::createThread, this, 0, false, false);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::reset()
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	boost::thread thr(&BBSOperator::createThread, this, 0, true, false);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::scroll(int diff)
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	boost::thread thr(&BBSOperator::createThread, this, diff, false, true);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::extract()
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	typedef void(BBSOperator::*func)();
	boost::thread thr((func)(&BBSOperator::extractThread), this);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::extract(const std::wstring& text, bool convert, int size, bool head)
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	typedef void(BBSOperator::*func)(const std::wstring&, bool, int, bool);
	boost::thread thr((func)(&BBSOperator::extractThread), this, text, convert, size, head);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::extract(const bbs::Range& range, int size, bool head)
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	typedef void(BBSOperator::*func)(const bbs::Range&, int, bool);
	boost::thread thr((func)(&BBSOperator::extractThread), this, range, size, head);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::extract(const std::wstring& id, int size, bool head)
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	typedef void(BBSOperator::*func)(const std::wstring&, int, bool);
	boost::thread thr((func)(&BBSOperator::extractThread), this, id, size, head);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::extract(int ref, int size, bool head)
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	typedef void(BBSOperator::*func)(int, int, bool);
	boost::thread thr((func)(&BBSOperator::extractThread), this, ref, size, head);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::extract(const std::vector<int>& vec, int size, bool head)
{
	thread_.create.timed_join(0);
	if (thread_.create.joinable()) { return false; }

	typedef void(BBSOperator::*func)(const std::vector<int>&, int, bool);
	boost::thread thr((func)(&BBSOperator::extractThread), this, vec, size, head);
	thread_.create.swap(thr);
	return true;
}

bool BBSOperator::subject()
{
	thread_.subject.timed_join(0);
	if (thread_.subject.joinable()) { return false; }

	boost::thread thr(&BBSOperator::subjectThread, this);
	thread_.subject.swap(thr);
	return true;
}

bool BBSOperator::change(const std::wstring& key)
{
	thread_.change.timed_join(0);
	if (thread_.change.joinable()) { return false; }

	ReloadControl ctrl(cfg_.func([](const BoardControl& cfg) { return cfg.reload; }), true, false, false);
	boost::thread thr(&BBSOperator::changeThread, this, key, ctrl);
	thread_.change.swap(thr);
	return true;
}

bool BBSOperator::post(const std::wstring& name, const std::wstring& mail, const std::wstring& message)
{
	thread_.post.timed_join(0);
	if (thread_.post.joinable()) { return false; }

	boost::thread thr(&BBSOperator::postThread, this, name, mail, message);
	thread_.post.swap(thr);
	return true;
}

//---------------------------------------------------------------------------------

void BBSOperator::addHistory(bool thread)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.history);

	std::wstring board;
	if (getBoardTitle(board) &&
		board.empty() &&
		subject_.func([&](const std::vector<ThreadInfo>& subject) { return subject.empty(); }))
	{
		return;
	}

	if (thread)
	{
		std::wstring title, url;
		if (getThreadTitle(title) && getThreadURL(url))
		{
			for (auto it = history_.thread.begin(); it != history_.thread.end(); ++it)
			{
				if (it->url == url)
				{
					history_.thread.erase(it);
					break;
				}
			}

			Link link = { L"【" + board + L"】 " + title, url };
			history_.thread.insert(history_.thread.begin(), link);

			resizeHistory(thread);
		}
	}
	else
	{
		std::wstring url;
		if (getBoardURL(url))
		{
			for (auto it = history_.board.begin(); it != history_.board.end(); ++it)
			{
				if (it->url == url)
				{
					history_.board.erase(it);
					break;
				}
			}

			Link link = { board, url };
			history_.board.insert(history_.board.begin(), link);

			resizeHistory(thread);
		}
	}
}

void BBSOperator::resizeHistory(bool thread)
{
	if (thread)
	{
		int max = DEFAULT_BBS_HISTORY;
		cfg_.apply([&](const BoardControl& cfg){
			max = cfg.history.thread;
		});

		boost::recursive_mutex::scoped_lock lock(mutex_.history);
		if ((int)history_.thread.size() > max)
		{
			history_.thread.resize(max);
		}
	}
	else
	{
		int max = DEFAULT_BBS_HISTORY;
		cfg_.apply([&](const BoardControl& cfg){
			max = cfg.history.board;
		});

		boost::recursive_mutex::scoped_lock lock(mutex_.history);
		if ((int)history_.board.size() > max)
		{
			history_.board.resize(max);
		}
	}
}

void BBSOperator::getHistory(bool thread, Links& links)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.history);

	if (thread)
	{
		links = history_.thread;
	}
	else
	{
		links = history_.board;
	}
}

void BBSOperator::setHistory(bool thread, const Links& links)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.history);
	if (thread)
	{
		history_.thread = links;
	}
	else
	{
		history_.board = links;
	}
}

void BBSOperator::clearHistory(bool thread)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.history);
	if (thread)
	{
		history_.thread.clear();
	}
	else
	{
		history_.board.clear();
	}
}

//---------------------------------------------------------------------------------

bool BBSOperator::isValid()
{
	std::wstring title;
	if (!bbs_.getBoardTitle(title)) { return false; }

	if (title.empty())
	{
		if (subject_.func([&](const std::vector<ThreadInfo>& subject) { return !subject.empty(); }))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool BBSOperator::isBrowser()
{
	return cfg_.func([](const BoardControl& cfg) { return cfg.browser; });
}

std::wstring BBSOperator::getSourceTitle()
{
	std::wstring url = source_.get();
	if (url.empty())
	{
		return L"URL未設定";
	}

	if (isBrowser())
	{
		url = L" - " + url;
	}
	else
	{
		url = L" [ " + url + L" ]";
	}
	return L"未対応URL" + url;
}

std::wstring BBSOperator::getBoardTitle()
{
	std::wstring title;
	if (!bbs_.getBoardTitle(title))
	{
		return getSourceTitle();
	}

	if (title.empty())
	{
		if (subject_.func([&](const std::vector<ThreadInfo>& subject) { return !subject.empty(); }))
		{
			title = L"掲示板タイトル取得失敗";
		}
		else
		{
			title = getSourceTitle();
		}
	}
	else
	{
		if (!isBrowser())
		{
			title = L"[ " + title + L" ]";
			//title = L"掲示板 [ " + title + L" ]";
		}
		else
		{
			title = L"【" + title + L"】";
		}
	}
	return title;
}

std::wstring BBSOperator::getThreadTitle()
{
	std::wstring title;
	if (!bbs_.getThreadTitle(title))
	{
		return getSourceTitle();
	}

	if (!title.empty())
	{
		if (!isBrowser())
		{
			title = L"[ " + title + L" ]";
		}
	}
	else
	{
		title = L"スレッドタイトル取得失敗";
	}

	int count = 0;
	if (bbs_.getCount(count))
	{
		std::wstring str;
		int pos = 0;
		if (bbs_.getPos(pos) && pos != 0 && pos != count)
		{
			str = (boost::wformat(L"%d/") % pos).str();
		}

		title += (boost::wformat(L" (%s%d)") % str.c_str() % count).str();
	}
	return title;
}

bool BBSOperator::getFastest(ThreadInfo& info)
{
	time_t org = 0;
	std::wstring key;
	if (getKey(key))
	{
		org = wcstoul(key.c_str(), NULL, 10);
	}

	int stop = 0;
	if (bbs_.getStop(stop))
	{
		return subject_.func([&](const std::vector<ThreadInfo>& subject)->bool {
			ThreadInfo x;
			for (auto it = subject.begin(); it != subject.end(); ++it)
			{
				if (it->count >= stop) { continue; }

				time_t time = wcstoul(it->key.c_str(), NULL, 10);
				if (time > org && it->speed > x.speed) { x = *it; }
			}

			if (x.key.empty())
			{
				return false;
			}
			info = x;
			return true;
		});
	}
	return false;
}

void BBSOperator::cut(std::vector<ResInfo>& dat, int size, bool head)
{
	if (size > 0 && (int)dat.size() > size)
	{
		std::vector<ResInfo> tmp;
		if (head)
		{
			tmp.assign((dat.rbegin() + size).base(), dat.rbegin().base());
		}
		else
		{
			tmp.assign(dat.begin(), dat.begin() + size);
		}
		dat = tmp;
	}
}

//---------------------------------------------------------------------------------

void BBSOperator::executeThread(const std::wstring& board, const BoardControl& cfg, bool move, bool once)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.execute);

	// 一度だけ読み込み
	if (once)
	{
		if (!once_.get()) { return; }
		once_ = false;
	}

	// URL置換
	std::wstring url = board;
	for (auto it = cfg.url.begin(); it != cfg.url.end(); ++it)
	{
		(*it)(url);
	}

	cfg_ = cfg;

	// URLが同じならスレッド一覧だけ更新
	if (!url.empty() && source_.func([&](const std::wstring& source) { return source == url; }))
	{
		if (bbs_)
		{
			subject();
			return;
		}
	}

	source_ = url;

	initialize();

	title_ = L"通信中";
	callback_.update.get()(UPDATE_TITLE);
	
	std::wstring shitaraba = DEFAULT_BBS_SHITARABA;
	std::wstring userAgent = DEFAULT_BBS_USERAGENT;
	network::Proxy proxy;
	int timeout = DEFAULT_BBS_TIMEOUT;
	cfg_.apply([&](const BoardControl& cfg){
		shitaraba = cfg.shitaraba;
		userAgent = cfg.userAgent;
		proxy = cfg.proxy;
		timeout = cfg.timeout;
	});
	if (bbs_.init(url, shitaraba, userAgent, proxy, timeout) && subjectThread())
	{// 掲示板形式と掲示板設定読み込み && スレッド一覧読み込み
		ReloadControl ctrl(cfg_.func([](const BoardControl& cfg) { return cfg.reload; }), true, move, false);

		addHistory(false);

		if (bbs_.hasKey())
		{// スレッドキーがある
			addHistory(true);

			if (move && cfg_.func([](const BoardControl& cfg) { return cfg.thread; }))
			{// スレッド移動
				if (bbs_.isStop())
				{
					ThreadInfo info;
					if (getFastest(info))
					{
						changeThread(info.key, ctrl);
						return;
					}
				}
			}

			reloadThread(ctrl);
			callback_.subject.get()(false);
			return;
		}

		if (move && cfg_.func([](const BoardControl& cfg) { return cfg.board; }))
		{// スレッド移動
			ThreadInfo info;
			if (getFastest(info))
			{
				changeThread(info.key, ctrl);
				return;
			}
		}

		// 掲示板タイトル
		title_ = getBoardTitle();
	}
	else
	{
		title_ = getSourceTitle();
	}

	if (!isValid())
	{
		bbs_.uninit();
	}

	callback_.dat.get()(false);
	callback_.subject.get()(false);
	callback_.update.get()(UPDATE_TITLE);
}


void BBSOperator::reloadThread(const ReloadControl& ctrl)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.reload);

	if (!bbs_.hasKey())
	{
		status_ = L"スレッドを選択してください";
		callback_.update.get()(UPDATE_DELAY_STATUS);
		return;
	}

	thread_.dat.interrupt();
	thread_.dat.join();

	boost::thread thr(&BBSOperator::datThread, this, ctrl);
	thread_.dat.swap(thr);
}

// reload // 自動更新
// stop // レス数上限なら停止
// move // スレッド移動を有効
void BBSOperator::datThread(const ReloadControl& ctrl)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.dat);

	if (ctrl.reload)
	{
		counter_ = 0;
		callback_.update.get()(UPDATE_COUNTER);
	}

	title_ = getThreadTitle();
	callback_.update.get()(UPDATE_TITLE);

	bool once = ctrl.once;
	bool first = true;
	bool flag = false; // スレッド上限以外で true
	while (true)
	{
		if (once)
		{
			once = false;
		}
		else
		{
			bool block = DEFAULT_BBS_BLOCK;
			cfg_.apply([&](const BoardControl& cfg){ block = cfg.block; });
			if (block && block_.get())
			{// 更新ブロック
				if (counter_)
				{
					counter_ = 0;
					callback_.update.get()(UPDATE_COUNTER);
				}

				while (true)
				{
					if (!block_.get()) { break; }

					try { boost::this_thread::sleep(boost::posix_time::milliseconds(1)); }
					catch (...) { goto exit_loop; }
				}
			}
		}

		if (first)
		{
			first = false;
			status_ = L"通信中";
			callback_.update.get()(UPDATE_STATUS);
		}

		std::wstring userAgent = DEFAULT_BBS_USERAGENT;
		network::Proxy proxy;
		int timeout = DEFAULT_BBS_TIMEOUT;
		std::vector<utl::ReplaceElement> message;
		cfg_.apply([&](const BoardControl& cfg){
			userAgent = cfg.userAgent;
			proxy = cfg.proxy;
			timeout = cfg.timeout;
			message = cfg.message;
		});
		DWORD code = 0;
		switch (bbs_.loadDat(userAgent, proxy, timeout, code))
		{
		case STATE_OK:
			{
				create();
				title_ = getThreadTitle();
				status_.clear();
				break;
			}
		case STATE_NOT_MODIFIED:
			{
				if (bbs_.cleanDat())
				{// 新着フラグが消去されたときだけ更新する
					create();
				}
				status_.clear();
				break;
			}

		case STATE_ERROR_CODE:
			{
				status_ = L"通信失敗" + (boost::wformat(L" (%d)") % code).str();
				break;
			}
		case STATE_ERROR_CONNECT:
			{
				status_ = L"接続失敗";
				break;
			}
		case STATE_ERROR_RECIEVE:
			{
				status_ = L"読み込み失敗" + (boost::wformat(L" (%d)") % code).str();
				break;
			}
		case STATE_ERROR_INVALID:
		case STATE_ERROR_NOKEY:
			{
				status_ = L"スレッドを選択してください";
				goto exit_loop;
				break;
			}
		default:
			{
				status_.clear();
				break;
			}
		}

		{// 最終通信時間
			SYSTEMTIME time;
			GetLocalTime(&time);
			time_ = time;
		}

		callback_.update.get()(UPDATE_DELAY_STATUS);

		if (!bbs_.isStop())
		{
			flag = true;
		}
		else
		{
			if ((ctrl.move || flag) && cfg_.func([](const BoardControl& cfg) { return cfg.move; }))
			{// スレッド移動
				if (subjectThread())
				{
					ThreadInfo info;
					if (getFastest(info))
					{
						counter_.clear();
						callback_.update.get()(UPDATE_COUNTER);
						callback_.update.get()(UPDATE_DELAY_STATUS);
						change(info.key);
						return;
					}
				}
			}
			else if (ctrl.stop)
			{
				break;
			}
		}

		// 自動更新
		if (!ctrl.reload) { break; }
		
		int interval = DEFAULT_BBS_INTERVAL;
		cfg_.apply([&](const BoardControl& cfg){ interval = cfg.interval; });
		for (int i = 0; i < interval; ++i)
		{
			counter_ = interval - i;
			callback_.update.get()(UPDATE_COUNTER);

			try { boost::this_thread::sleep(boost::posix_time::seconds(1)); }
			catch (...) { goto exit_loop; }
		}
	}

exit_loop:
	counter_.clear();
	callback_.update.get()(UPDATE_COUNTER);
	callback_.update.get()(UPDATE_DELAY_STATUS);
	return;
}

void BBSOperator::createThread(int diff, bool reset, bool scroll)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	int res = DEFAULT_BBS_RES;
	bool browser = DEFAULT_BBS_BROWSER;
	bool latest = DEFAULT_BBS_LATEST;
	int range = DEFAULT_BBS_RANGE;
	std::vector<utl::ReplaceElement> message;
	cfg_.apply([&](const BoardControl& cfg) {
		res = cfg.res;
		browser = cfg.browser;
		latest = cfg.latest;
		range = cfg.range;
		message = cfg.message;
	});

	if (reset) { bbs_.resetDat(); }
	bbs_.prepareDat(browser, latest, range, message);

	if (diff == 0)
	{
		std::vector<ResInfo> dat;
		bool init = true;
		if (bbs_.getDat(dat, init))
		{
			dat_ = dat;
			if (init || !init_) { init_ = true; }
		}
	}

	std::vector<ResInfo> partial;
	if (bbs_.getDat(partial, diff, res))
	{
		std::vector<std::wstring> links;
		for (auto it = partial.begin(); it != partial.end(); ++it)
		{
			links.insert(links.end(), it->link.begin(), it->link.end());
		}
		partial_ = partial;
		links_ = links;
	}

	if (bbs_.hasKey())
	{
		title_ = getThreadTitle();
	}
	else
	{
		title_ = getBoardTitle();
	}
	callback_.update.get()(UPDATE_TITLE);
	callback_.dat.get()(scroll);
}

void BBSOperator::extractThread()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	std::vector<ResInfo> dat;
	if (bbs_.getExtract(dat))
	{
		extract_ = dat;
	}
	else
	{
		extract_.clear();
	}

	callback_.extract.get()();
}

void BBSOperator::extractThread(const std::wstring& text, bool convert, int size, bool head)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	std::vector<ResInfo> dat;
	if (bbs_.getExtract(dat, text, convert))
	{
		cut(dat, size, head);
		extract_ = dat;
	}
	else
	{
		extract_.clear();
	}

	callback_.extract.get()();
}

void BBSOperator::extractThread(const bbs::Range& range, int size, bool head)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	std::vector<ResInfo> dat;
	if (bbs_.getExtract(dat, range))
	{
		cut(dat, size, head);
		extract_ = dat;
	}
	else
	{
		extract_.clear();
	}

	callback_.extract.get()();
}

void BBSOperator::extractThread(const std::wstring& id, int size, bool head)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	std::vector<ResInfo> dat;
	if (bbs_.getExtract(dat, id))
	{
		cut(dat, size, head);
		extract_ = dat;
	}
	else
	{
		extract_.clear();
	}

	callback_.extract.get()();
}

void BBSOperator::extractThread(int ref, int size, bool head)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	std::vector<ResInfo> dat;
	if (bbs_.getExtract(dat, ref))
	{
		cut(dat, size, head);
		extract_ = dat;
	}
	else
	{
		extract_.clear();
	}

	callback_.extract.get()();
}

void BBSOperator::extractThread(const std::vector<int>& vec, int size, bool head)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.create);

	std::vector<ResInfo> dat;
	if (bbs_.getExtract(dat, vec))
	{
		cut(dat, size, head);
		extract_ = dat;
	}
	else
	{
		extract_.clear();
	}

	callback_.extract.get()();
}

bool BBSOperator::subjectThread()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.subject);

	callback_.subject.get()(true);

	std::wstring userAgent = DEFAULT_BBS_USERAGENT;
	network::Proxy proxy;
	int timeout = DEFAULT_BBS_TIMEOUT;
	cfg_.apply([&](const BoardControl& cfg){
		userAgent = cfg.userAgent;
		proxy = cfg.proxy;
		timeout = cfg.timeout;
	});

	DWORD code = 0;
	if (bbs_.loadSubject(userAgent, proxy, timeout, code) >= STATE_OK)
	{
		std::vector<ThreadInfo> subject;
		bbs_.getSubject(subject);
		subject_ = subject;
		callback_.subject.get()(false);
		return true;
	}
	callback_.subject.get()(false);
	return false;
}

void BBSOperator::changeThread(const std::wstring& key, const ReloadControl& ctrl)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.change);

	if (!bbs_.change(key)) { return; }

	addHistory(true);
	
	std::wstring url;
	if (bbs_.getURL(url))
	{
		source_ = url;
	}
	reloadThread(ctrl);
	callback_.subject.get()(false);
}

void BBSOperator::postThread(const std::wstring& name, const std::wstring& mail, const std::wstring& message)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.post);

	DWORD code = 0;
	std::wstring error;

	post_ = L"書き込み中";
	callback_.update.get()(UPDATE_POST);

	std::wstring userAgent = DEFAULT_BBS_USERAGENT;
	network::Proxy proxy;
	int timeout = DEFAULT_BBS_TIMEOUT;
	cfg_.apply([&](const BoardControl& cfg){
		userAgent = cfg.userAgent;
		proxy = cfg.proxy;
		timeout = cfg.timeout;
	});
	switch (bbs_.post(userAgent, proxy, timeout, name, mail, message, code, error))
	{
	case STATE_OK:
	case STATE_NOT_MODIFIED:
		{
			post_ = L"書き込み成功";
			callback_.post.get()(true);
			callback_.update.get()(UPDATE_DELAY_POST);
			if (cfg_.func([](const BoardControl& cfg) { return cfg.write; }))
			{
				ReloadControl ctrl(counter_, true, true, true);
				reloadThread(ctrl);
			}

			//bool block = DEFAULT_BBS_BLOCK;
			//bool write = DEFAULT_BBS_WRITE;
			//cfg_.apply([&](const BoardControl& cfg){
			//	block = cfg.block;
			//	write = cfg.write;
			//});
			//if (block || write)
			//{
			//	ReloadControl ctrl(counter_, true, true, write);
			//	reloadThread(ctrl);
			//}
			return;
		}
	case STATE_ERROR_CODE:
		{
			post_ = L"通信失敗" + (boost::wformat(L" (%d)") % code).str();
			break;
		}
	case STATE_ERROR_CONNECT:
		{
			post_ = L"接続失敗";
			break;
		}
	case STATE_ERROR_RECIEVE:
		{
			post_ = L"読み込み失敗" + (boost::wformat(L" (%d)") % code).str();
			break;
		}
	case STATE_ERROR_POST:
		{
			post_ = L"書き込み失敗" + (boost::wformat(L" (%d %s)") % code % error).str();
			break;
		}
	case STATE_ERROR_INVALID:
	case STATE_ERROR_NOKEY:
		{
			post_ = L"スレッドを選択してください";
			break;
		}
	default:
		{
			post_ = L"書き込み失敗";
			break;
		}
	}

	callback_.post.get()(false);
	callback_.update.get()(UPDATE_DELAY_POST);
}

} // namespace bbs