#pragma once

namespace sl {

////////////////////////////////////////////////////////////////////////
// 表示設定

// 倍率情報
struct WindowZoom {
	WindowZoom()
		: enable(true), zoom(100)
	{}

	WindowZoom(bool enable, int zoom)
		: enable(enable), zoom(zoom)
	{}

	bool enable; // 有効
	int zoom; // 倍率
};

// サイズ情報
struct WindowSize {
	WindowSize()
		: enable(true), width(800), height(600)
	{}

	WindowSize(bool enable, int width, int height)
		: enable(enable), width(width), height(height)
	{}

	bool enable; // 有効
	int width; // ウィンドウ幅
	int height; // ウィンドウ高さ
};

// アスペクト比情報
struct WindowAspect {
	WindowAspect()
		: ax(4), ay(3)
	{}

	WindowAspect(int ax, int ay)
		: ax(ax), ay(ay)
	{}

	int ax; // 幅比率
	int ay; // 高さ比率
};

// ウィンドウ
struct WindowConfig {
	WindowConfig()
	{
		zoom.push_back(WindowZoom(true, 25));
		zoom.push_back(WindowZoom(true, 50));
		zoom.push_back(WindowZoom(true, 75));
		zoom.push_back(WindowZoom(true, 100));
		zoom.push_back(WindowZoom(true, 125));
		zoom.push_back(WindowZoom(true, 150));
		zoom.push_back(WindowZoom(true, 200));


		size.push_back(WindowSize(false, 320, 180));
		size.push_back(WindowSize(false, 352, 200));
		size.push_back(WindowSize(false, 480, 270));
		size.push_back(WindowSize(true, 640, 360));
		size.push_back(WindowSize(true, 800, 450));
		size.push_back(WindowSize(true, 960, 540));
		size.push_back(WindowSize(false, 1024, 576));
		size.push_back(WindowSize(true, 1280, 720));
		size.push_back(WindowSize(false, 1440, 810));
		size.push_back(WindowSize(false, 1600, 900));
		size.push_back(WindowSize(false, 1920, 1080));

		size.push_back(WindowSize(true, 320, 240));
		size.push_back(WindowSize(true, 480, 360));
		size.push_back(WindowSize(false, 512, 384));
		size.push_back(WindowSize(true, 640, 480));
		size.push_back(WindowSize(true, 800, 600));
		size.push_back(WindowSize(false, 960, 720));
		size.push_back(WindowSize(false, 1024, 768));
		size.push_back(WindowSize(false, 1280, 960));
		size.push_back(WindowSize(false, 1440, 1080));
		size.push_back(WindowSize(false, 1600, 1200));
		size.push_back(WindowSize(false, 1920, 1440));


		aspect.push_back(WindowAspect(16, 9));
		aspect.push_back(WindowAspect(4, 3));
		aspect.push_back(WindowAspect(5, 4));
		aspect.push_back(WindowAspect(235, 100));
		aspect.push_back(WindowAspect(185, 100));
	}
	std::vector<WindowZoom> zoom; // 倍率情報
	std::vector<WindowSize> size; // サイズ情報
	std::vector<WindowAspect> aspect; // アスペクト比情報
};


// 日時取得方法
enum DATETIME_TYPE {
	DATETIME_TYPE_NONE = 0, // 取得しない
	DATETIME_TYPE_FILETIME = 1, // ファイル作成日を取得
	DATETIME_TYPE_FILENAME = 2, // ファイル名から正規表現で取得
};

enum DATETIME_DATE_FORMAT {
	DATETIME_DATE_FORMAT_NONE = 0,
	DATETIME_DATE_FORMAT_ENGLISH = 1,
	DATETIME_DATE_FORMAT_KANJI = 2,
};

enum DATETIME_TIME_FORMAT {
	DATETIME_TIME_FORMAT_24 = 0,
	DATETIME_TIME_FORMAT_12_ENGLISH = 1,
	DATETIME_TIME_FORMAT_12_KANJI = 2,
};

// 日時表示
struct DatetimeConfig {
	DatetimeConfig()
		: type(DATETIME_TYPE_FILENAME)
		, exp(L"(\\d{4})(\\d{2})(\\d{2})\\D*(\\d{2})(\\d{2})(\\d{2})")
		, dateFormat(DATETIME_DATE_FORMAT_KANJI)
		, timeFormat(DATETIME_TIME_FORMAT_24)
	{
		sub[0] = 1;
		sub[1] = 2;
		sub[2] = 3;
		sub[3] = 4;
		sub[4] = 5;
		sub[5] = 6;
	}

	int type; // 日時取得方法
	std::wstring exp; // 日時取得正規表現
	std::array<int, 6> sub; // 日時取得正規表現部分式

	int dateFormat; // 日付書式
	int timeFormat; // 時間書式
};


// チャンネル情報
struct InformationConfig : public network::InformationControl {
};


// ステータスバーレイアウト
struct StatusConfig {
	StatusConfig()
		: marginX(4)
		, marginY(2)
	{
		CFont f;
		if (utl::createFont(f))
		{
			f.GetLogFont(&font);
			f.DeleteObject();
		}

		text[0] = RGB(255, 255, 255);

		color[0] = RGB(32, 32, 32);
		color[1] = RGB(0, 0, 0);
		color[2] = RGB(32, 32, 32);
		color[3] = RGB(0, 0, 0);
	}

	LOGFONT font; // フォント

	int marginX; // マージン横
	int marginY; // マージン縦
	std::array<COLORREF, 1> text; // 文字色
	std::array<COLORREF, 4> color; // 配色
};

// シークバーレイアウト
struct SeekConfig {
	SeekConfig()
		: marginX(4)
		, marginY(2)
		, knob(5)
	{
		CFont f;
		if (utl::createFont(f))
		{
			f.GetLogFont(&font);
			f.DeleteObject();
		}

		text[0] = RGB(255, 255, 255);
		text[1] = RGB(180, 180, 180);

		color[0] = RGB(0, 0, 0);
		color[1] = RGB(128, 128, 128);
		color[2] = RGB(0, 0, 0);
		color[3] = RGB(32, 32, 32);
		color[4] = RGB(0, 0, 0);
		color[5] = RGB(255, 255, 255);
		color[6] = RGB(96, 96, 96);
		color[7] = RGB(32, 32, 32);
		color[8] = RGB(0, 0, 0);
		color[9] = RGB(0, 0, 0);
	}

	LOGFONT font; // フォント

	int marginX; // マージン横
	int marginY; // マージン縦
	int knob; // ノブ幅
	std::array<COLORREF, 2> text; // 文字色
	std::array<COLORREF, 10> color; // 配色
};

// シークヒントレイアウト
struct HintConfig {
	HintConfig()
		: marginX(4)
		, marginY(2)
	{
		CFont f;
		if (utl::createFont(f, true))
		{
			f.GetLogFont(&font);
			f.DeleteObject();
		}

		text[0] = RGB(255, 255, 255);

		color[0] = RGB(64, 64, 64);
		color[1] = RGB(0, 0, 0);
		color[2] = RGB(64, 64, 64);
		color[3] = RGB(96, 96, 96);
		color[4] = RGB(64, 64, 64);
		color[5] = RGB(0, 0, 0);
	}

	struct Opaque {
		Opaque()
			: enable(true)
			, text(224)
			, back(192)
		{}

		bool enable;
		int text;
		int back;
	};

	LOGFONT font; // フォント

	int marginX; // マージン横
	int marginY; // マージン縦
	Opaque opaque; // 不透明度
	std::array<COLORREF, 1> text; // 文字色
	std::array<COLORREF, 6> color; // 配色
};

// チャンネル情報レイアウト
struct ChannelConfig {
	ChannelConfig()
		: marginX(4)
		, marginY(2)
	{
		CFont f;
		if (utl::createFont(f, true))
		{
			f.GetLogFont(&font);
			f.DeleteObject();
		}

		text[0] = RGB(255, 255, 255);

		color[0] = RGB(64, 64, 64);
		color[1] = RGB(0, 0, 0);
		color[2] = RGB(64, 64, 64);
		color[3] = RGB(96, 96, 96);
		color[4] = RGB(64, 64, 64);
		color[5] = RGB(0, 0, 0);
	}

	struct Opaque {
		Opaque()
			: enable(true)
			, text(224)
			, back(192)
		{}

		bool enable;
		int text;
		int back;
	};

	LOGFONT font; // フォント
	
	int marginX; // マージン横
	int marginY; // マージン縦
	Opaque opaque; // 不透明度
	std::array<COLORREF, 1> text; // 文字色
	std::array<COLORREF, 6> color; // 配色
};

// BBSバーレイアウト
struct BBSConfig {
	// 掲示板/スレッドタイトル
	struct Title {
		Title()
			: bold(true)
		{
			text[0] = RGB(255, 0, 0);
		}

		bool bold; // 太字
		std::array<COLORREF, 1> text; // スレッド終了時の文字色
	};

	// 書き込み欄
	struct Edit {
		Edit()
		{
			CFont f;
			if (utl::createFont(f))
			{
				f.GetLogFont(&font);
				f.DeleteObject();
			}
		}

		LOGFONT font; // フォント
	};

	BBSConfig()
		: marginX(4)
		, marginY(2)
	{
		CFont f;
		if (utl::createFont(f))
		{
			f.GetLogFont(&font);
			f.DeleteObject();
		}

		text[0] = RGB(255, 255, 255);

		color[0] = RGB(0, 0, 0);
		color[1] = RGB(32, 32, 32);
		color[2] = RGB(32, 32, 32);
		color[3] = RGB(0, 0, 0);
	}

	LOGFONT font; // フォント

	int marginX; // マージン横
	int marginY; // マージン縦
	std::array<COLORREF, 1> text; // 文字色
	std::array<COLORREF, 4> color; // 配色

	Title title; // 掲示板/スレッドタイトル
	Edit edit; // 書き込み欄
};

// レスポップアップレイアウト
struct ResConfig {
	// レス書式
	enum FORMAT {
		FORMAT_NORMAL1, // ノーマル
		FORMAT_NORMAL2, // ノーマル（非デフォルト名のみ）
		FORMAT_SIMPLE1, // シンプル
		FORMAT_SIMPLE2, // シンプル（非デフォルト名のみ）
		FORMAT_COMPACT1, // コンパクト
		FORMAT_COMPACT2, // コンパクト（非デフォルト名のみ）
		FORMAT_COUNT, // enum総数
	};

	// 区切り文字
	enum PUNCT {
		PUNCT_NONE, // なし
		PUNCT_SPACE, // スペース
		PUNCT_COLON, // コロン
		PUNCT_COUNT, // enum総数
	};

	// 書式
	struct Format {
		Format()
			: type(FORMAT_NORMAL1)
			, space(2)
			, width(300)
			, punct(PUNCT_COLON)
			, second(false)
		{
			CFont f;
			if (utl::createFont(f, false))
			{
				f.GetLogFont(&font);
				f.DeleteObject();
			}
		}

		int type; // レス書式
		int space; // 行間
		int width; // 本文の最大幅（0で無制限）
		int punct; // 区切り文字
		bool second; // 時間（シンプル）の秒数表示
		LOGFONT font; // フォント
	};

	// ID
	struct ID {
		ID()
			: enable(true)
			, threshold(5)
		{}

		bool enable; // IDの横に発言回数を表示
		int threshold; // 発言が多いIDのしきい値
	};

	// 配色
	struct Color {
		Color()
		{
			text[0] = RGB(0, 0, 0);
			text[1] = RGB(0, 128, 0);
			text[2] = RGB(0, 128, 0);
			text[3] = RGB(0, 0, 0);
			text[4] = RGB(0, 0, 0);
			text[5] = RGB(0, 0, 0);
			text[6] = RGB(0, 0, 0);
			text[7] = RGB(0, 128, 0);
			text[8] = RGB(0, 0, 0);
			text[9] = RGB(0, 0, 0);
			text[10] = RGB(0, 0, 0);

			link[0] = RGB(0, 0, 0);
			link[1] = RGB(0, 0, 255);
			link[2] = RGB(0, 0, 255);
			link[3] = RGB(0, 0, 0);
			link[4] = RGB(0, 0, 0);
			link[5] = RGB(0, 0, 0);
			link[6] = RGB(0, 0, 0);
			link[7] = RGB(0, 0, 255);
			link[8] = RGB(0, 0, 0);
			link[9] = RGB(0, 0, 0);
			link[10] = RGB(0, 0, 255);

			sage[0] = RGB(0, 0, 0);
			sage[1] = RGB(128, 0, 128);
			sage[2] = RGB(128, 0, 128);
			sage[3] = RGB(0, 0, 0);
			sage[4] = RGB(0, 0, 0);
			sage[5] = RGB(0, 0, 0);
			sage[6] = RGB(0, 0, 0);
			sage[7] = RGB(128, 0, 128);
			sage[8] = RGB(0, 0, 0);
			sage[9] = RGB(0, 0, 0);
			sage[10] = RGB(255, 0, 0);

			bold[0] = FALSE;
			bold[1] = TRUE;
			bold[2] = FALSE;
			bold[3] = FALSE;
			bold[4] = FALSE;
			bold[5] = FALSE;
			bold[6] = FALSE;
			bold[7] = FALSE;
			bold[8] = FALSE;
			bold[9] = FALSE;
			bold[10] = FALSE;
		}

		// 00:レス番号
		// 01:名前
		// 02:メール
		// 03:日時
		// 04:ID
		// 05:本文
		// 06:レス番号(シンプル/コンパクト)
		// 07:時間(シンプル/コンパクト)
		// 08:区切り文字
		// 09:メール欄括弧
		// 10:IDヘッダ

		std::array<COLORREF, 11> text; // メール欄なし文字色
		std::array<COLORREF, 11> link; // メール欄あり文字色
		std::array<COLORREF, 11> sage; // メール欄sage文字色
		std::array<BOOL, 11> bold; // 太字
	};

	// レス区分
	struct Box {
		Box()
			: marginX(4)
			, marginY(4)
			, space(1)
			, punct(false)
			, separate(0)
		{
			color[0] = RGB(248, 248, 248);
			color[1] = RGB(248, 248, 248);
			color[2] = RGB(248, 248, 248);
			color[3] = RGB(248, 248, 248);
			color[4] = RGB(248, 248, 248);
			color[5] = RGB(248, 248, 248);

			latest[0] = RGB(224, 224, 255);
			latest[1] = RGB(224, 224, 255);
			latest[2] = RGB(224, 224, 255);
			latest[3] = RGB(224, 224, 255);
			latest[4] = RGB(224, 224, 255);
			latest[5] = RGB(224, 224, 255);
		}

		int marginX; // マージン横
		int marginY; // マージン縦
		int space; // 区間の余白
		bool punct; // 分割表示する
		int separate; // 分割幅
		std::array<COLORREF, 6> color; // 配色
		std::array<COLORREF, 6> latest; // 新着配色
	};

	// 背景
	struct Back {
		Back()
			: marginX(1)
			, marginY(1)
		{
			color[0] = RGB(192, 192, 192);
			color[1] = RGB(192, 192, 192);
			color[2] = RGB(50, 50, 50);
			color[3] = RGB(50, 50, 50);
			color[4] = RGB(50, 50, 50);
			color[5] = RGB(50, 50, 50);
		}

		struct Opaque {
			Opaque()
				: enable(false)
				, text(255)
				, back(224)
			{}

			bool enable;
			int text;
			int back;
		};

		int marginX; // マージン横
		int marginY; // マージン縦
		Opaque opaque; // 不透明度
		std::array<COLORREF, 6> color; // 配色
	};

public:
	Format format; // レス書式
	ID id; // ID
	Color color; // 配色
	Box box; // レス区分
	Back back; // 背景
};



typedef std::pair<std::wstring, StatusConfig> StatusPair;
typedef std::pair<std::wstring, SeekConfig>  SeekPair;
typedef std::pair<std::wstring, HintConfig> HintPair;
typedef std::pair<std::wstring, ChannelConfig> ChannelPair;
typedef std::pair<std::wstring, BBSConfig> BBSPair;
typedef std::pair<std::wstring, ResConfig> ResPair;

// カスタムレイアウト
struct CustomDisplayConfig {
	std::vector<StatusPair> status;
	std::vector<SeekPair> seek;
	std::vector<HintPair> hint;
	std::vector<ChannelPair> channel;
	std::vector<BBSPair> bbs;
	std::vector<ResPair> res;
};



struct DisplayConfig {
	CustomDisplayConfig custom; // カスタムレイアウト

	WindowConfig window; // ウィンドウ
	DatetimeConfig datetime; // 日時表示
	InformationConfig information; // チャンネル情報

	StatusConfig status; // ステータスバーレイアウト
	SeekConfig seek; // シークバーレイアウト
	HintConfig hint; // シークヒントレイアウト
	ChannelConfig channel; // チャンネル情報レイアウト
	BBSConfig bbs; // BBSバーレイアウト
	ResConfig res; // レスポップアップレイアウト
};

} // namespace sl