#include "stdafx.h"



namespace dsp {

AsyncBufferingLoader::AsyncBufferingLoader()
: timeout_(10000)
, terminated_(false)
, total_(0)
, consume_(false)
{
		
}

AsyncBufferingLoader::~AsyncBufferingLoader()
{
	// 自動的に停止できないので
	// 何処かで停止する必要がある
	terminate();
}


// バッファリングを開始して初期化
// データの先頭を読んでタイプの判別をする
bool AsyncBufferingLoader::execute(const std::wstring& url, const std::wstring& userAgent, int timeout)
{
	terminate();


	// 初期化
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		
		timeout_ = timeout;
		terminated_ = false;
		total_ = 0;
		lead_.clear();
		buffer_.clear();
	}

	{
		boost::recursive_mutex::scoped_lock lock(mutexClient_);
		client_.close();
		if (!client_.open(url, userAgent, network::Proxy(), timeout)) return false;
	}


	// 先頭データを同期で読み込んでおく
	size_t size = 0;
	while (size < 1024)// 数値以上を読み込み
	{
		std::vector<char> tmp;
		int state = network::HttpClient::STATE_ERROR;
		{
			boost::recursive_mutex::scoped_lock lock(mutexClient_);
			state = client_.recv(tmp);
		}

		// 正常終了もしくは終端でデータが空でなければバッファに追加する
		if ((state == network::HttpClient::STATE_OK || state == network::HttpClient::STATE_EOF) && !tmp.empty())
		{
			boost::recursive_mutex::scoped_lock lock(mutex_);
			lead_.insert(lead_.end(), tmp.begin(), tmp.end());
			size += lead_.size();
		}

		// 正常終了でなければ終了
		if (state != network::HttpClient::STATE_OK)
		{
			boost::recursive_mutex::scoped_lock lock(mutexClient_);
			client_.close();
			return true;
		}
	}

	// スレッド生成 ここから非同期
	boost::thread thr(&AsyncBufferingLoader::buffering, this);
	mainloop_.detach();
	mainloop_.swap(thr);
	return true;
}

// バッファリングを停止する
void AsyncBufferingLoader::terminate()
{
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		if (terminated_) { return; }
		terminated_ = true;
	}

	if (mainloop_.joinable())
	{
		mainloop_.interrupt(); // スレッド中断命令
		//DebugText(L"AsyncBufferingLoader::thread_.interrupt();\n");
		mainloop_.join(); // 中断されるまで待機
		//DebugText(L"AsyncBufferingLoader::thread_.join();\n");
	}
}

// バッファリングが停止している
bool AsyncBufferingLoader::terminated()
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	return terminated_;
}

// 指定した長さ以上のデータをうけとるまで待機する
// タイムアウトするか停止命令をうけたら中断する
bool AsyncBufferingLoader::fill(LONGLONG length)
{
	if (!client_) { return false; }

	clock_t time = clock();
	while (size() < length)
	{
		Sleep(1);

		if (terminated_)
		{
			return false;
		}

		if ((clock() - time) > timeout_)
		{
			return false;
		}
	}
	return true;
}

LONGLONG AsyncBufferingLoader::size()
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	return total_;
}


// 内部バッファを消費せずにコピーする
size_t AsyncBufferingLoader::lead(void *dst, size_t size)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	if (lead_.size() < size)
	{
		size = lead_.size();
	}
		
	CopyMemory(dst, lead_.data(), size);
	return size;
}

// 内部バッファを消費せずにコピーする
size_t AsyncBufferingLoader::copy(void *dst, LONGLONG pos, LONG length)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	if (!consume_)
	{
		if ((int)buffer_.size() < pos + length)
		{
			CopyMemory(dst, buffer_.data(), buffer_.size());
			return buffer_.size();
		}

		CopyMemory(dst, buffer_.data() + pos, length);
		return length;
	}
	else
	{
		// 全体から現在のバッファ長を引いた位置が先頭
		// コピー位置から先頭を引くとバッファ上のコピー位置になる
		LONG current = (LONG)(pos - (total_ - buffer_.size()));

		// コピー位置がバッファより前
		if (current < 0)
		{
			//DebugText(L"AsyncBufferingLoader::copy(fail current)\n");
			return 0;
		}

		// コピー長がバッファより大きい
		if (buffer_.size() < (size_t)(current + length))
		{
			// 長さをバッファのサイズに切り詰め
			length = (LONG)buffer_.size() - current;
		}

		// コピー位置がバッファサイズを超えてる場合長さがマイナスになることがあるが
		// 通常はfillされてから呼ばれるのでマイナスにならない
		if (length < 0)
		{
			//DebugText(L"AsyncBufferingLoader::copy(fail length)\n");
			return 0;
		}
		
		CopyMemory(dst, buffer_.data() + current, length);
		
		// 先頭からコピーした範囲までを削除する
		std::vector<char> tmp(buffer_.begin() + current + length, buffer_.end());
		std::swap(buffer_, std::move(tmp));

		//DebugText(L"buffer_.size:%d\n", buffer_.size());
		return length;
	}
}

// 消費コピーに切り替え
// MemSourceFilter::CompleteConnect
// メディアタイプチェックをして接続完了したら
// 消費コピーに切り替える
void AsyncBufferingLoader::consume(bool enable)
{
	boost::recursive_mutex::scoped_lock lock(mutex_);
	consume_ = enable;
}

// バッファリング本体
void AsyncBufferingLoader::buffering()
{
	{
		// 先頭データからコピー
		boost::recursive_mutex::scoped_lock lock(mutex_);
		total_ = lead_.size();
		buffer_ = lead_;
	}

	try {
		while (true)
		{
			boost::this_thread::interruption_point();

			// データ読み込み
			std::vector<char> tmp;
			int state = network::HttpClient::STATE_ERROR;
			{
				boost::recursive_mutex::scoped_lock lock(mutexClient_);
				state = client_.recv(tmp);
			}

			boost::this_thread::interruption_point();
			
			
			// 正常終了もしくは終端でデータが空でなければバッファに追加する
			if ((state == network::HttpClient::STATE_OK || state == network::HttpClient::STATE_EOF) && !tmp.empty())
			{
				boost::recursive_mutex::scoped_lock lock(mutex_);
				
				total_ += tmp.size();
				buffer_.insert(buffer_.end(), tmp.begin(), tmp.end());
			}

			// 正常終了でなければ終了
			if (state != network::HttpClient::STATE_OK)
			{
				break;
			}
		}
	}
	catch(...){}

	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		terminated_ = true;
	}
	{
		boost::recursive_mutex::scoped_lock lock(mutexClient_);
		client_.close();
	}
}


} // namespace dsp

