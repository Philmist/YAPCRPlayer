#pragma once

#include "Network.h"

namespace dsp {

class AsyncBufferingLoader {
	boost::thread mainloop_;
	boost::recursive_mutex mutex_;
	boost::recursive_mutex mutexClient_;

	network::HttpClient client_;
	int timeout_;
	volatile bool terminated_;
	volatile LONGLONG total_;
	std::vector<char> lead_;
	std::vector<char> buffer_;

	bool consume_;

public:
	AsyncBufferingLoader();
	~AsyncBufferingLoader();

	bool execute(const std::wstring& url, const std::wstring& userAgent, int timeout);
	void terminate();
	bool terminated();

	bool fill(LONGLONG length);
	LONGLONG size();

	size_t lead(void *dst, size_t size);
	size_t copy(void *dst, LONGLONG pos, LONG length);
	void consume(bool enable);

private:
	void buffering();
};

} // namespace dsp