#include "stdafx.h"

HMODULE GetMlangDll()
{
	static HMODULE dll = NULL;
	if (!dll) { dll = LoadLibrary(L"mlang.dll"); }
	return dll;
}

Mlang::ConvertINetStringPtr Mlang::ConvertINetString_ = NULL;

Mlang::Mlang()
{
	HMODULE dll = GetMlangDll();
	
	if (dll)
	{
		ConvertINetString_ = (ConvertINetStringPtr)GetProcAddress(dll, "ConvertINetString");
	}
}

Mlang::~Mlang()
{
}

std::wstring Mlang::decode(const char* dat, int len, CODE code)
{
	DWORD mode = 0;
	int srclen = len;
	int dstlen = (srclen * 2) + 1;
	std::vector<wchar_t> dst;
	dst.resize(dstlen, L'\0');

	HRESULT hr = ConvertINetString(&mode, code, 1200, dat, &srclen, (LPBYTE)dst.data(), &dstlen);
	if (SUCCEEDED(hr))
	{
		return std::wstring(dst.data());
	}
	return std::wstring();
}

std::wstring Mlang::decode(const std::string& str, CODE code)
{
	return decode(str.c_str(), (int)str.size(), code);
}

std::wstring Mlang::decode(const std::vector<char>& str, CODE code)
{
	return decode(str.data(), (int)str.size(), code);
}


std::string Mlang::decode(const wchar_t* dat, int len, CODE code)
{
	DWORD mode = 0;
	int srclen = len * 2;
	int dstlen = (srclen * 2) + 1;
	std::vector<char> dst;
	dst.resize(dstlen, L'\0');

	HRESULT hr = ConvertINetString(&mode, 1200, code, (LPCSTR)dat, &srclen, (LPBYTE)dst.data(), &dstlen);
	if (SUCCEEDED(hr))
	{
		return std::string(dst.data());
	}
	return std::string();
}

std::string Mlang::decode(const std::wstring& str, CODE code)
{
	return decode(str.c_str(), (int)str.size(), code);
}

std::string Mlang::decode(const std::vector<wchar_t>& str, CODE code)
{
	return decode(str.data(), (int)str.size(), code);
}