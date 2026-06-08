#pragma once

#pragma warning(disable : 4003)
#pragma warning(disable : 4005)


#include <Shlwapi.h> // StrCmpLogicalW
#pragma comment(lib, "Shlwapi.lib") // StrCmpLogicalW

#include <Imm.h>
#pragma comment(lib, "imm32.lib")

#include <Dwmapi.h>

#include <vector>
#include <boost/xpressive/xpressive.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <boost/function.hpp>

#include <boost/algorithm/string/replace.hpp>


// オリジナルの ON_NOTIFY_REFLECT の記述だとインテリセンスがバグでエラーを吐く
// WM_NOTIFY+WM_REFLECT_BASE の + の左右にスペースが無いのが原因
#undef ON_NOTIFY_REFLECT
#define ON_NOTIFY_REFLECT(wNotifyCode, memberFxn) \
	{ WM_NOTIFY + WM_REFLECT_BASE, (WORD)(int)wNotifyCode, 0, 0, AfxSigNotify_v, \
		(AFX_PMSG) \
		(static_cast<void (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		(memberFxn)) },

namespace utl {

////////////////////////////////////////
// OSバージョン
bool getOSVersion(DWORD& major, DWORD& minor, DWORD& build);
bool isWindows10();

////////////////////////////////////////
// コマンドライン
class CommandLine {
	static LPWSTR* line_;
	static int size_;

	CommandLine();
	~CommandLine();

public:
	static void init();
	static int size();
	static std::wstring str(int i);
};

////////////////////////////////////////
// カレントフォルダ
bool getModuleFolder(std::wstring &path);

////////////////////////////////////////
// フォルダパスブラウザ
bool folderBrowser(HWND hwnd, const std::wstring& src, std::wstring& dst);

////////////////////////////////////////
// ファイル関連
std::wstring toFullpath(const std::wstring& current, const std::wstring& path);
std::wstring pathToFile(const std::wstring& path);
bool getNextFilePath(const std::wstring& src, std::wstring& dst, bool loop, bool prev);
boost::posix_time::ptime getFileTime(const std::wstring& path);
bool selectExecute(bool def, const std::wstring& path, const std::wstring& command);
bool processExecute(const std::wstring& path, const std::wstring& command, DWORD& id);
bool processExecute(const std::wstring& path, const std::wstring& command);
bool getFolderPath(const std::wstring& src, std::vector<std::wstring>& path);


////////////////////////////////////////
// 時間関連
class DateTime {
	boost::posix_time::ptime time_;
public:
	bool set(const boost::posix_time::ptime& time);
	bool set(
		const std::wstring& text,
		const std::wstring& rx = L"(\\d{4})(\\d{2})(\\d{2})\\D*(\\d{2})(\\d{2})(\\d{2})",
		int year = 1, int month = 2, int day = 3,
		int hour = 4, int min = 5, int sec = 6);
	boost::posix_time::ptime get(LONGLONG nanodiff = 0);
	void clear();
	bool valid();
};


double round(double r);
long lround(double r);
void convertTimes(LONGLONG time, int& hour, int& min, int& sec);
std::wstring convertTimes(LONGLONG time, bool variable = true);
std::wstring convertDate(const boost::posix_time::ptime& time, int dateFormat = 0, int timeFormat = 0);

const wchar_t* convertWeek(WORD wDayOfWeek);
const wchar_t* convertWeekEn(WORD wDayOfWeek);

////////////////////////////////////////////////////////////////////
// アスペクト比適用

// aspect比率でsrcに内接するdstを返す
bool getAspectRect(const SIZE& aspect, const RECT& src, RECT& dst);

////////////////////////////////////////////////////////////////////
// ウィンドウ枠
// srcに枠を足す
void inflateWindowRect(RECT& src, bool title, bool frame);
// srcから枠を引く
void deflateWindowRect(RECT& src, bool title, bool frame);




typedef enum {
	FRAMEPOS_NONE = -1,
	FRAMEPOS_TOP = 0,
	FRAMEPOS_TOPLEFT,
	FRAMEPOS_TOPRIGHT,
	FRAMEPOS_LEFT,
	FRAMEPOS_RIGHT,
	FRAMEPOS_BOTTOM,
	FRAMEPOS_BOTTOMLEFT,
	FRAMEPOS_BOTTOMRIGHT,
} FRAMEPOS;

int windowFramePos(const RECT& src, CPoint &pt, int size);

////////////////////////////////////////
// ウィンドウデフォルトサイズ指定
RECT defWindowRect(int width, int height);
WINDOWPLACEMENT defWindowPlacement(int width, int height);

////////////////////////////////////////
// デスクトップ関数
BOOL getDesktopRectFromPoint(const POINT pt, MONITORINFO &mi);
BOOL getDesktopRectFromRect(const RECT &rcFrom, MONITORINFO &mi);
BOOL getDesktopRectFromWindow(const HWND hWnd, MONITORINFO &mi);

////////////////////////////////////////
// ウィンドウスナップ
//void setWindowSnap(HWND hwnd, LPPOINT lppt, int distance = 10);

////////////////////////////////////////
// Desktop Window Manager関数ローダー
class DWMLoader {
	static HMODULE module_;

	typedef HRESULT (WINAPI* PDwmIsCompositionEnabled)(BOOL*);
	typedef HRESULT (WINAPI* PDwmGetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);

	static PDwmIsCompositionEnabled fpDwmIsCompositionEnabled;
	static PDwmGetWindowAttribute fpDwmGetWindowAttribute;

	DWMLoader();
	~DWMLoader();

public:
	static void Init(); // 下の関数を使うには事前にこれで初期化すること

	static HRESULT WINAPI DwmIsCompositionEnabled(_Out_ BOOL *pfEnabled);
	static HRESULT WINAPI DwmGetWindowAttribute(HWND hwnd, DWORD dwAttribute, _Out_ PVOID pvAttribute, DWORD cbAttribute);
};

////////////////////////////////////////
// DWM関数を使用して見かけ上のウィンドウ外形を得る
BOOL getWindowShapeRect(HWND hwnd, LPRECT lprc);

////////////////////////////////////////
// ドラッグ＆ドロップ
void dragAcceptFiles(HWND hwnd);
bool dragQueryFiles(HDROP drop, std::vector<std::wstring>& files);


// 2色のグラデーションを描画する関数です
BOOL twoColorsGradient(
    HDC hdc,            // 描画先のデバイスコンテキスト・ハンドルです
    const RECT *pRect,  // 描画する範囲の矩形です
    COLORREF Color1,    // 描画する一つ目の色です
    COLORREF Color2,    // 描画する二つ目の色です
    BOOL fHorizontal = FALSE    // 水平のグラデーションを描画する場合は TRUE にします
);

COLORREF mixColor(COLORREF Color1, COLORREF Color2, BYTE Ratio = 128);

// テカったグラデーションを描画する関数
void glossyGradient(
    HDC hdc,                   // 描画先のデバイスコンテキスト・ハンドルです
    const RECT *pRect,         // 描画する範囲の矩形です
    COLORREF Color1,           // 描画する一つ目の色です
    COLORREF Color2,           // 描画する二つ目の色です
    BOOL fHorizontal = FALSE,  // 水平のグラデーションを描画する場合は TRUE にします
    BYTE GlossRatio1 = 96,     // 端のテカりの強さです
    BYTE GlossRatio2 = 48     // 中央のテカりの強さです
);

void drawFrameRect(CDC& dc, const CRect& rc,
		COLORREF color1 = RGB(63, 63, 63),
		COLORREF color2 = RGB(0, 0, 0),
		COLORREF color3 = RGB(63, 63, 63),
		COLORREF color4 = RGB(95, 95, 95),
		COLORREF color5 = RGB(63, 63, 63),
		COLORREF color6 = RGB(0, 0, 0));

void drawFrameRect(CDC& dc, const CRect& rc, const std::array<COLORREF, 6>& color);

void drawStatusBar(CDC& dc, const CRect& rc,
	COLORREF color1 = RGB(0, 0, 0),
	COLORREF color2 = RGB(0, 0, 0),
	COLORREF color3 = RGB(0, 0, 0),
	COLORREF color4 = RGB(0, 0, 0));

void drawStatusBar(CDC& dc, const CRect& rc, const std::array<COLORREF, 4>& color);

void drawSeekBar(CDC& dc, const CRect& rc, int pos, int knob,
	COLORREF color1 = RGB(0, 0, 0),
	COLORREF color2 = RGB(127, 127, 127),
	COLORREF color3 = RGB(0, 0, 0),
	COLORREF color4 = RGB(31, 31, 31),
	COLORREF color5 = RGB(0, 0, 0),
	COLORREF color6 = RGB(255, 255, 255),
	COLORREF color7 = RGB(95, 95, 95),
	COLORREF color8 = RGB(31, 31, 31),
	COLORREF color9 = RGB(0, 0, 0),
	COLORREF color10 = RGB(0, 0, 0));

void drawSeekBar(CDC& dc, const CRect& rc, int pos, int knob, const std::array<COLORREF, 10>& color);

void drawSlider(CDC& dc, const CRect& rc, int pos);
void drawCross(CDC& dc, const CRect& rc);

class CCBitmap {
	CDC memDC_;
	CBitmap bmp_;
	CBitmap* oldBmp_;

public:
	CCBitmap(CDC& dc, const CRect& rc);
	virtual ~CCBitmap();

	operator CDC*() { return &memDC_; }

	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
		int xSrc, int ySrc, DWORD dwRop);

	BOOL AlphaBlend(
		HDC hdcDest,
		int xoriginDest,
		int yoriginDest,
		int wDest,
		int hDest,
		int xoriginSrc,
		int yoriginSrc,
		BYTE byteAlpha);
};


/////////////////////////////////////////////////////////////
// フォント

bool createFont(CFont& font, bool bold = false, bool underline = false, int point = 90, const std::wstring& name = L"MS UI Gothic");
int getFontHeight(HWND hwnd, CFont& font);
int pointToHeight(int point);
int heightToPoint(int height);
int getEditHeight(HWND hwnd, CFont& font);

/////////////////////////////////////////////////////////////
// 文字列関連


void replaceAll(std::wstring& input, const std::wstring& search, const std::wstring& format);
std::wstring replaceAllCopy(const std::wstring& input, const std::wstring& search, const std::wstring& format);

void getLines(const std::wstring& text, bool trim, std::vector<std::wstring>& lines);
void getLines(const std::wstring& text, bool trim, boost::function<void(const std::wstring&)> func);

bool decompress(std::vector<char>& dst, const std::vector<char>& src);
bool LCMapStr(DWORD flag, const std::wstring& src, std::wstring& dst);
bool unencode(char x);
std::string urlencode(const std::string& src);
std::string toUTF8(const std::wstring& src);
std::wstring toUTF16(const std::string& src);

/////////////////////////////////////////////////////////////
// IME
void IMEDisable();
void IMEDisable(HWND hwnd);

/////////////////////////////////////////////////////////////
// その他
inline bool toggle(bool& var)
{
	var = var ? false : true;
	return var;
}

} // namespace utl

