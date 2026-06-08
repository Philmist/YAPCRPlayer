#pragma once

// 50222	JIS
// 51932	EUC-JP
// 932		Shift-JIS
// 65001	UTF-8N
// 1200		UTF-16 Unicode(Little-Endian)
// 1201		UTF-16 Unicode(Big-Endian)

HMODULE GetMlangDll();

class Mlang {
	typedef HRESULT (WINAPI * ConvertINetStringPtr) (
		LPDWORD lpdwMode,
		DWORD dwSrcEncoding,
		DWORD dwDstEncoding,
		LPCSTR lpSrcStr,
		LPINT lpnSrcSize,
		LPBYTE lpDstStr,
		LPINT lpnDstSize);

	static ConvertINetStringPtr ConvertINetString_;

	Mlang();
	virtual ~Mlang();
public:
	static void Init()
	{
		static Mlang mlang;
	}

	static HRESULT WINAPI ConvertINetString(
		LPDWORD lpdwMode,
		DWORD dwSrcEncoding,
		DWORD dwDstEncoding,
		LPCSTR lpSrcStr,
		LPINT lpnSrcSize,
		LPBYTE lpDstStr,
		LPINT lpnDstSize)
	{
		if (ConvertINetString_)
		{
			return ConvertINetString_(
				lpdwMode,
				dwSrcEncoding,
				dwDstEncoding,
				lpSrcStr,
				lpnSrcSize,
				lpDstStr,
				lpnDstSize);
		}
		return E_NOTIMPL;
	}


public:
// 50222	JIS
// 51932	EUC-JP
// 932		Shift-JIS
// 65001	UTF-8N
// 1200		UTF-16 Unicode(Little-Endian)
// 1201		UTF-16 Unicode(Big-Endian)

	enum CODE {
		SHIFT_JIS = 932,
		EUC_JP = 51932
	};

	static std::wstring decode(const char* dat, int len, CODE code);
	static std::wstring decode(const std::string& str, CODE code);
	static std::wstring decode(const std::vector<char>& str, CODE code);

	static std::string decode(const wchar_t* dat, int len, CODE code);
	static std::string decode(const std::wstring& str, CODE code);
	static std::string decode(const std::vector<wchar_t>& str, CODE code);
};