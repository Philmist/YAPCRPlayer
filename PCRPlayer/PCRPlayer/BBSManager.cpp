#include "stdafx.h"


namespace bbs {

BBSManager::BBSManager()
{
}

BBSManager::~BBSManager()
{
}

BBSManager::operator bool()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	return bbs_ ? *bbs_ : false;
}

bool BBSManager::init(const std::wstring& url, const std::wstring& shitaraba, const std::wstring& userAgent, const network::Proxy& proxy, int timeout)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);

	network::URLParser u(url);

	if (match(u.host(), shitaraba))
	{
		bbs_.reset(new ShitarabaBBS(url, userAgent, proxy, timeout));
	}
	else
	{
		bbs_.reset(new BaseBBS(url, userAgent, proxy, timeout));
	}

	boost::recursive_mutex::scoped_lock lock2(mutex_.value);
	info_.reset();
	subject_.reset();
	dat_.reset();
	pos_.reset();

	if (bbs_ && *bbs_)
	{
		info_.emplace();
		bbs_->getInfo(*info_);
		return true;
	}
	return false;
}

void BBSManager::uninit()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	bbs_.reset();

	boost::recursive_mutex::scoped_lock lock2(mutex_.value);
	info_.reset();
	subject_.reset();
	dat_.reset();
	pos_.reset();
}

bool BBSManager::change(const std::wstring& key)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	if (bbs_ && *bbs_ && bbs_->change(key))
	{
		boost::recursive_mutex::scoped_lock lock(mutex_.value);
		dat_.reset();
		pos_.reset();
		info_.emplace();
		bbs_->getInfo(*info_);
		return true;
	}
	return false;
}

int BBSManager::loadSetting(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	if (bbs_ && *bbs_)
	{
		int res = bbs_->loadSetting(userAgent, proxy, timeout, code);
		if (res == STATE_OK)
		{
			boost::recursive_mutex::scoped_lock lock(mutex_.value);
			info_.emplace();
			bbs_->getInfo(*info_);
		}
		return res;
	}
	return STATE_ERROR_INVALID;
}

int BBSManager::loadSubject(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	if (bbs_ && *bbs_)
	{
		int res = bbs_->loadSubject(userAgent, proxy, timeout, code);

		if (res == STATE_OK)
		{
			boost::recursive_mutex::scoped_lock lock(mutex_.value);
			info_.emplace();
			bbs_->getInfo(*info_);

			subject_.emplace();
			bbs_->getSubject(*subject_);
		}
		return res;
	}
	return STATE_ERROR_INVALID;
}

int BBSManager::loadDat(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, DWORD& code)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	if (bbs_ && *bbs_)
	{
		int res = bbs_->loadDat(userAgent, proxy, timeout, code);

		boost::recursive_mutex::scoped_lock lock(mutex_.value);
		if (res == STATE_OK)
		{
			info_.emplace();
			bbs_->getInfo(*info_);

			// 現在位置が下端にあるなら更新時に下端に移動する
			bool equal = pos_ && dat_ ? pos_.get() == (int)dat_->res.size() : false;
			bool init = !pos_ || !dat_ || equal;
			
			if (!dat_) { dat_.emplace(); }
			std::vector<bbs::ResInfo> tmp;
			int pos = (int)(dat_->res.size() + 1);
			if (bbs_->getDat(tmp, pos))
			{
				if (pos == 0)
				{// 初期化
					dat_.emplace();
					init = true;
				}
				else
				{// 新着消去
					for (auto it = dat_->res.rbegin(); it != dat_->res.rend(); ++it)
					{
						if (it->latest == false) { break; }
						it->latest = false;
					}
				}
				dat_->tmp = tmp;
			}

			if (init)
			{
				if (dat_)
				{
					pos_ = (int)(dat_->res.size() + dat_->tmp.size());
				}
				else
				{
					pos_.reset();
				}
			}
		}
		return res;
	}
	return STATE_ERROR_INVALID;
}

void BBSManager::prepareDat(bool browser, bool latest, int range, const std::vector<utl::ReplaceElement>& message)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (dat_ && !dat_->tmp.empty())
	{
		// 参照レス判定
		auto within = [](int pos, int range, ResInfo& info, const std::map<int, std::vector<bbs::Range>>& anchor)->bool {
			bool result = false;
			for (auto itx = anchor.begin(); itx != anchor.end(); ++itx)
			{
				const int& ref = itx->first;
				const std::vector<bbs::Range>& ranges = itx->second;

				for (auto it = ranges.begin(); it != ranges.end(); ++it)
				{
					if ((it->range() <= range) && it->within(pos))
					{
						info.ref.push_back(ref);
						result = true;
						break;
					}
				}
			}
			return result;
		};

		// 置換正規表現コンパイル
		std::vector<std::pair<boost::xpressive::wsregex, std::wstring>> rx;
		for (auto it = message.begin(); it != message.end(); ++it)
		{
			if (it->enable)
			{
				rx.push_back(std::make_pair(it->compile(), it->format));
			}
		}

		bbs::BBSReplace replace;
		std::map<int, std::vector<bbs::Range>> anchor;
		//utl::HTMLEscape esc;
		//utl::NameSplitter split;

		bool init = !latest && dat_->res.empty();
		int pos = (int)(dat_->res.size() + 1);
		for (auto it = dat_->tmp.begin(); it != dat_->tmp.end(); ++it)
		{
			// 新着フラグ消去
			if (init)
			{
				it->latest = false;
			}

			// 本文置換
			for (auto rt = rx.begin(); rt != rx.end(); ++rt)
			{
				it->message = boost::xpressive::regex_replace(it->message, rt->first, rt->second);
			}

			// リンクの置換と抽出
			std::vector<std::wstring> urls;
			std::vector<bbs::Range> ranges;
			replace(it->name, it->message, urls, ranges);

			if (!urls.empty())
			{
				it->link = urls;
			}

			if (!ranges.empty())
			{
				anchor[pos] = ranges;
			}

			// IDカウント
			if (!it->id.empty() && it->id.find(L"???") != 0)
			{
				dat_->id[it->id].push_back(pos);
				it->count = (int)dat_->id[it->id].size();

				auto identifier = dat_->identifier.find(it->id);
				if (identifier == dat_->identifier.end())
				{
					it->identifier = (int)dat_->identifier.size() + 1;
					dat_->identifier.insert(std::make_pair(it->id, it->identifier));
				}
				else
				{
					it->identifier = identifier->second;
				}
			}

			within(pos, range, *it, dat_->anchor);

			// エスケープ処理
			//ResInfo info = *it;
			//if (!browser)
			//{
			//	split(info.name, info.split);//エスケープする前に分割
			//	esc.replace(info.name);
			//	esc.replace(info.mail);
			//	esc.replace(info.datetime);
			//	esc.replace(info.message);
			//	esc.replace(info.title);
			//	esc.replace(info.id);
			//}

			//dat_->res.push_back(info);

			dat_->res.push_back(*it);

			++pos;
		}

		// 追加分の参照レスを全体に反映
		pos = 1;
		for (auto it = dat_->res.begin(); it != dat_->res.end(); ++it)
		{
			auto id = dat_->id.find(it->id);
			if (id != dat_->id.end())
			{
				it->total = (int)id->second.size();
			}

			within(pos, range, *it, anchor);

			++pos;
		}

		dat_->anchor.insert(anchor.begin(), anchor.end());

		dat_->tmp.clear();
	}
}

// 更新されていなかった時の更新フラグ消し
bool BBSManager::cleanDat()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	bool res = false;
	if (dat_)
	{
		for (auto it = dat_->res.rbegin(); it != dat_->res.rend(); ++it)
		{
			if (it->latest == false) { break; }
			it->latest = false;
			res = true;
		}
	}
	return res;
}

// 設定変更時の読み込み直し用
void BBSManager::resetDat()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	if (bbs_ && *bbs_)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_.value);
		if (dat_)
		{
			dat_.emplace();
			int pos = 1;
			if (bbs_->getDat(dat_->tmp, pos))
			{
				for (auto it = dat_->tmp.begin(); it != dat_->tmp.end(); ++it)
				{
					it->latest = false;
				}
			}
		}
	}
}


bool BBSManager::getSubject(std::vector<ThreadInfo>& data)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!subject_) { return false; }
	data = *subject_;
	return true;
}

bool BBSManager::getDat(std::vector<ResInfo>& data, bool& init)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }

	data.clear();
	for (auto it = dat_->res.begin(); it != dat_->res.end(); ++it)
	{
		if (it->del) { continue; }
		data.push_back(*it);
	}

	init = dat_->init;
	dat_->init = false;
	return true;
}

bool BBSManager::getDat(std::vector<ResInfo>& data, int diff, int len)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }

	if (len < 0)
	{
		bool init = false;
		return getDat(data, init);
	}

	int size = (int)dat_->res.size();
	if (size <= 0) { return false; }

	int org = 0;
	if (pos_) { org = *pos_; }
	if (org <= 0) { return false; }

	int pos = size;
	if (diff != INT_MAX)
	{
		len = min(size, len);
		pos = max(min(org + diff, size), len);
	}

	if (pos <= 0) { return false; }
	if (diff != 0 && org == pos) { return false; }

	pos_ = pos;

	//---------------------------------------------

	auto first = dat_->res.begin();

	if (diff <= 0)
	{
		first += pos;
	}
	else
	{
		first += (pos - 1);
		for (; first != dat_->res.end(); ++first, ++pos)
		{
			if (!first->del) { break; }
		}
		first++;
	}

	int i = 1;
	data.clear();
	std::deque<ResInfo> d;
	bool flag = true;
	for (std::vector<ResInfo>::const_reverse_iterator it(first); it != dat_->res.rend(); ++it)
	{
		if (i > len) { break; }
		int now = pos--;
		if (it->del) { continue; }

		if (flag) { flag = false; pos_ = now; }

		++i;
		d.push_front(*it);
	}

	for (auto it = d.begin(); it != d.end(); ++it)
	{
		data.push_back(*it);
	}
	return true;
}

bool BBSManager::getExtract(std::vector<ResInfo>& data)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }
	if (dat_->res.empty()) { return false; }

	data = dat_->res;

	return !data.empty();
}

bool BBSManager::getExtract(std::vector<ResInfo>& data, const std::wstring& text, bool convert)
{
	if (convert)
	{
		bbs::URIConvert cnv;
		bbs::URIConvert::Result result;
		if (cnv(text, result))
		{
			if (result.anchor.matched)
			{
				return getExtract(data, result.anchor.value);
			}
			else if (result.id.matched)
			{
				return getExtract(data, result.id.value);
			}
			else if (result.ref.matched)
			{
				return getExtract(data, result.ref.value);
			}
		}
	}
	else
	{
		return getExtract(data, text);
	}
	return false;
}

bool BBSManager::getExtract(std::vector<ResInfo>& data, const bbs::Range& range)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }
	if (dat_->res.empty()) { return false; }

	bbs::Range r = range;
	if (r.first > r.last) { std::swap(r.first, r.last); }
	if (r.first < 1) { r.first = 1; }
	if (r.last < 1) { r.last = 1; }
	if (r.first > (int)dat_->res.size()) { return false; }
	if (r.last > (int)dat_->res.size()) { r.last = (int)dat_->res.size(); }

	auto begin = dat_->res.begin() + (r.first - 1);
	auto end = dat_->res.begin() + r.last;

	for (auto it = begin; it != end; ++it)
	{
		if (it->del) { continue; }
		data.push_back(*it);
	}
	return !data.empty();
}

bool BBSManager::getExtract(std::vector<ResInfo>& data, const std::wstring& id)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }
	if (dat_->res.empty()) { return false; }

	auto p = dat_->id.find(id);
	if (p == dat_->id.end()) { return false; }
	try {
		for (auto it = p->second.begin(); it != p->second.end(); ++it)
		{
			data.push_back(dat_->res.at((*it) - 1));
		}
		return !data.empty();
	} catch (...) {}
	return false;
}

bool BBSManager::getExtract(std::vector<ResInfo>& data, int ref)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }
	if (dat_->res.empty()) { return false; }

	try {
		std::vector<int>& refs = dat_->res.at(ref - 1).ref;
		if (refs.empty()) { return false; }
		for (auto it = refs.begin(); it != refs.end(); ++it)
		{
			data.push_back(dat_->res.at((*it) - 1));
		}
		return !data.empty();
	} catch (...) {}
	return false;
}

bool BBSManager::getExtract(std::vector<ResInfo>& data, const std::vector<int>& vec)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!dat_) { return false; }
	if (dat_->res.empty()) { return false; }

	try {
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			data.push_back(dat_->res.at((*it) - 1));
		}
		return !data.empty();
	} catch (...) {}
	return false;
}

bool BBSManager::getPos(int& pos)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!pos_) { return false; }
	pos = *pos_;
	return true;
}

int BBSManager::post(const std::wstring& userAgent, const network::Proxy& proxy, int timeout, const std::wstring& name, const std::wstring& mail, const std::wstring& message,
	DWORD& code, std::wstring& error)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.bbs);
	if (bbs_ && *bbs_)
	{
		return bbs_->post(userAgent, proxy, timeout, name, mail, message, code, error);
	}
	return STATE_ERROR_INVALID;
}

//---------------------------------------------------------------------------------

bool BBSManager::getURL(std::wstring& url)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	if (info_->thread.key.empty())
	{
		url = info_->board.url;
	}
	else
	{
		url = info_->thread.url;
	}
	return true;
}

bool BBSManager::getBoardURL(std::wstring& url)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	url = info_->board.url;
	return true;
}

bool BBSManager::getThreadURL(std::wstring& url)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	url = info_->thread.url;
	return true;
}

bool BBSManager::getBoardTitle(std::wstring& title)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	title = info_->board.title;
	return true;
}

bool BBSManager::getThreadTitle(std::wstring& title)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	title = info_->thread.title;
	return true;
}

bool BBSManager::getNoName(std::wstring& noname)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	noname = info_->board.noname;
	return true;
}

bool BBSManager::getCount(int& count)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	count = info_->thread.count;
	return true;
}

bool BBSManager::getKey(std::wstring& key)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	key = info_->thread.key;
	return true;
}

bool BBSManager::hasKey()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	return !info_->thread.key.empty();
}

bool BBSManager::getStop(int& stop)
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	stop = info_->thread.stop;
	return true;
}

bool BBSManager::isStop()
{
	boost::recursive_mutex::scoped_lock lock(mutex_.value);
	if (!info_) { return false; }

	return info_->thread.count >= info_->thread.stop;
}


//---------------------------------------------------------------------------------

bool BBSManager::match(const std::wstring& str, const std::wstring& rx)
{
	using namespace boost::xpressive;
	
	try {
		if (regex_match(str, wsregex::compile(rx)))
		{
			return true;
		}
	} catch (...) {}
	return false;
}

} // namespace bbs
