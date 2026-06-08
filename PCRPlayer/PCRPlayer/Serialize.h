#pragma once

#include "SerializeDisplay.h"
#include "SerializeOperation.h"
#include "SerializeVideo.h"
#include "SerializeDetail.h"


namespace sl {

// パス情報
struct PathInfo {
	PathInfo() : commandline(false) {}

	std::wstring path; // URLパス/ファイルパス
	std::wstring name; // チャンネル名
	std::wstring contact; // コンタクトURL
	bool commandline; // コマンドライン引数か
};

// アスペクト比
typedef enum {
	WINDOW_ASPECT_DEFAULT = 0, // デフォルト
	WINDOW_ASPECT_NONE = 1, // ウィンドウに合わせる
	WINDOW_ASPECT_CUSTOM = 2, // ユーザー設定
} WINDOW_ASPECT;

// 実行優先度
typedef enum {
	PROCESS_PRIORITY_IDLE, // 低
	PROCESS_PRIORITY_BELOW_NOMAL, // 通常以下
	PROCESS_PRIORITY_NORMAL, // 通常
	PROCESS_PRIORITY_ABOVE_NORMAL, // 通常以上
	PROCESS_PRIORITY_HIGH, // 高
	PROCESS_PRIORITY_REALTIME, // リアルタイム
} PROCESS_PRIORITY;

// ウィンドウ設定
struct WindowControl {
	// 表示
	struct Show {
		Show()
			: topmost(false)
			, title(true)
			, frame(true)
			, bbs(true)
			, seek(true)
			, status(true)
			, edit(false)
			, browser(false)
			, side(CLayoutCtrl::SIDE_BOTTOM)
			, idealX(300)
			, idealY(100)
		{}

		bool topmost; // 最前面表示
		bool title; // タイトルバー表示
		bool frame; // フレーム表示
		bool bbs; // BBSバー表示
		bool seek; // シークバー表示
		bool status; // ステータスバー表示
		bool edit; // BBSバー書き込み覧表示
		bool browser; // 内蔵BBSブラウザ表示
		int side; // 内蔵BBSブラウザ表示位置
		int idealX; // ブラウザサイズX
		int idealY; // ブラウザサイズY
	};

	// 比率適用
	struct Ratio {
		Ratio() : fixed(true), play(false), height(true), inscribe(true) {}

		bool fixed; // サイズ変更時に比率固定
		bool play; // 再生開始時に自動的に比率適用
		bool height; // 比率適用するとき高さを基準に
		bool inscribe; // 比率適用するとき元のサイズ内に収める
	};

	// スナップ
	struct Snap {
		Snap() : screen(true), window(true) {}

		bool screen; // スクリーンスナップ
		bool window; // ウィンドウスナップ
	};

	// 全画面
	struct Fullscreen {
		Fullscreen() : hide(false), floating(false), mouse(true), topmost(true) {}

		bool hide; // バーを自動的に隠す
		bool floating; // バーを自動的に隠すときにフローティング表示
		bool mouse; // 自動的にマウスカーソルを消す
		bool topmost; // 最前面表示
	};

	// 最大化
	struct Maximize {
		Maximize() : fullscreen(false), taskbar(true) {}

		bool fullscreen; // 全画面化
		bool taskbar; // タスクバー領域を隠さない
	};

	// タイトルバー
	struct Title {
		Title() : player(false), size(false), source(false) {}

		bool player; // プレイヤー名表示
		bool size; // 画面サイズ表示
		bool source; // ソースサイズ表示
	};

	// ステータスバー
	struct Status {
		// チャンネル情報
		struct Channel {
			Channel() : name(true), type(true), genre(true), comment(true), playing(true), relay(true) {}

			bool name; // チャンネル名
			bool type; // タイプ
			bool genre; // ジャンル/詳細
			bool comment; // コメント
			bool playing; // アーティスト/タイトル/アルバム
			bool relay; // リスナー/リレー
		};

		Status() : fps(true), size(false), time(true), play(true) {}

		bool fps; // fps表示(オフライン)
		bool size; // 画面サイズ表示
		bool time; // 時間を可変表示(オンライン)
		bool play; // 再生情報を優先表示
		Channel channel;
	};

	// シークバー
	struct Seek {
		Seek() : filename(true) {}

		bool filename; // ファイル名を表示する
	};

	// BBSバー
	struct BBS {
		BBS() : counter(false) {}

		bool counter; // 自動更新カウンタを表示する
	};

public:
	WindowControl()
		: aspect(WINDOW_ASPECT_DEFAULT)
		, priority(PROCESS_PRIORITY_NORMAL)
		, stop(true)
		, zoom(false)
	{}

	int aspect; // アスペクト比のタイプ
	int priority; // 実行優先度
	bool stop; // 再生中モニター電源OFFとスタンバイ抑止
	bool zoom; // 起動時に倍率100%

	Show show; // 表示
	Ratio ratio; // 比率適用
	Snap snap; // スナップ
	Fullscreen fullscreen; // 全画面化
	Maximize maximize; // 最大化
	Title title; // タイトルバー
	Status status; // ステータスバー
	Seek seek; // シークバー
	BBS bbs; // BBSバー

	DWORD getPriority()
	{
		switch(priority)
		{
		case PROCESS_PRIORITY_IDLE:			return IDLE_PRIORITY_CLASS;
		case PROCESS_PRIORITY_BELOW_NOMAL:	return BELOW_NORMAL_PRIORITY_CLASS;
		case PROCESS_PRIORITY_NORMAL:		return NORMAL_PRIORITY_CLASS;
		case PROCESS_PRIORITY_ABOVE_NORMAL:	return ABOVE_NORMAL_PRIORITY_CLASS;
		case PROCESS_PRIORITY_HIGH:			return HIGH_PRIORITY_CLASS;
		case PROCESS_PRIORITY_REALTIME:		return REALTIME_PRIORITY_CLASS;
		default: break;	
		}
		return NORMAL_PRIORITY_CLASS;
	}
};

// オフライン再生モード
typedef enum {
	PLAY_MODE_NORMAL,
	PLAY_MODE_REPEAT,
	PLAY_MODE_FOLDER,
	PLAY_MODE_FOLDER_LOOP,
} PLAY_MODE;

// オフライン再生
struct PlayControl {
	PlayControl()
		: mode(PLAY_MODE_NORMAL)
	{}

	int mode; // オフライン再生モード
};

// 各ダイアログ設定
struct DialogConfig {
	// BBSバーメニュー
	struct BBS {
		BBS()
			: sage(true)
		{}

		bool sage; // sageで書き込む
	};

	// スレッド選択
	struct Thread {
		Thread()
			: width(600)
			, height(300)
			, stop(false)
			, stretch(true)
			, datetime(true)
			, close(true)
		{
			column[0] = 0; // ダミー
			column[1] = 40; // No.
			column[2] = 300; // タイトル
			column[3] = 50; // レス
			column[4] = 60; // 勢い
			column[5] = 110; // Since
		}

		int width; // ウィンドウ幅
		int height; // ウィンドウ高さ
		std::array<int, 6> column; // カラム幅
		bool stop; // 終了したスレッドを表示
		bool stretch; // カラム自動伸縮
		bool datetime; // 日時表示
		bool close; // スレッド選択時にウィンドウを閉じる
		bbs::Links links; // お気に入り掲示板
		bbs::Links threads; // お気に入りスレッド
	};

	// ファイル/URLを開く
	struct File {
		File()
			: width(600)
			, height(150)
		{}

		int width; // ウィンドウ幅
		int height; // ウィンドウ高さ
	};

	// 検索
	struct Search {
		Search()
			: width(400)
			, height(300)
		{}

		int width; // ウィンドウ幅
		int height; // ウィンドウ高さ
	};

	// 抽出
	struct Extract {
		Extract()
			: width(400)
			, height(300)
		{}

		int width; // ウィンドウ幅
		int height; // ウィンドウ高さ
	};

	// ログ
	struct Log {
		Log()
			: graph(true)
			, filter(true)
			, bbs(true)
			, peercast(true)
			, none(true)
		{}
		
		bool graph; // グラフ
		bool filter; // フィルタ
		bool bbs; // BBS
		bool peercast; // PeerCast
		bool none; // その他
	};

	BBS bbs; // BBSバーメニュー
	Thread thread; // スレッド選択
	File file; // ファイル/URLを開く
	Search search; // 検索
	Extract extract; // 抽出
	Log log; // ログ
};

// オプションダイアログ
struct OptionConfing {
	// 選択項目/タブ
	struct Select {
		Select()
			: option(0)
			, network(0)
			, type(0)
			, information(0)
			, res(0)
		{}
		int option; // オプション項目
		int network; // 詳細設定/ネットワークタブ
		int type; // 詳細設定/タイプタブ
		int information; // 表示設定/チャンネル情報タブ
		int res; // レイアウト/レスポップアップタブ

		struct BBS {
			BBS()
				: common(0)
				, bar(0)
				, browser(0)
			{}

			int common; // BBS設定/共通タブ
			int bar; // BBS設定/BBSバータブ
			int browser; // BBS設定/内蔵BBSブラウザタブ
		} bbs;
	};

	// 詳細設定/プロパティグリッド
	struct Detail {
		struct Network {
			utl::PropSetting general;
			utl::PropSetting peercast;
		} network;

		struct Type {
			utl::PropSetting wmv;
			utl::PropSetting flv;
		} type;

		utl::PropSetting browser;
		utl::PropSetting snapshot;
		utl::PropSetting start;
	};

	// BBS設定/プロパティグリッド
	struct BBS {
		struct Common {
			utl::PropSetting general;
			utl::PropSetting network;
			utl::PropSetting thread;
			utl::PropSetting font;
		};
		struct Bar {
			utl::PropSetting thread;
			utl::PropSetting popup;
			utl::PropSetting post;
		};
		struct Browser {
			utl::PropSetting thread;
			utl::PropSetting popup;
			utl::PropSetting scroll;

			struct Skin {
				utl::PropSetting thread;
				utl::PropSetting popup;
			};
			Skin skin;
		};
		Common common;
		Bar bar;
		Browser browser;
	};

	// 操作設定/プロパティグリッド
	struct Operation {
		utl::PropSetting mouse;
		utl::PropSetting variation;
		utl::PropSetting window;
	};

	// 表示設定/プロパティグリッド
	struct Display {
		struct Information {
			utl::PropSetting general;
		};

		Information information;
	};

	OptionConfing()
	{}

	Select select; // 選択項目/タブ
	Detail detail;
	BBS bbs;
	Operation operation;
	Display display;
};

//////////////////////////////////////////////////////////////

// LOGFONT
void put(XmlParser& p, const LOGFONT& font);
void get(const XmlParser& p, LOGFONT& font);

void put(XmlParser& p, const std::vector<utl::ReplaceElement>& data);
void get(const XmlParser& p, std::vector<utl::ReplaceElement>& data);

void put(XmlParser& root, const VideoConfig& video);
void get(const XmlParser& root, VideoConfig& video);

void put(XmlParser& root, const DetailConfig& detail);
void get(const XmlParser& root, DetailConfig& detail);

void put(XmlParser& root, const OperationConfig& operation);
void get(const XmlParser& root, OperationConfig& operation);

void put(XmlParser& root, const DisplayConfig& display);
void get(const XmlParser& root, DisplayConfig& display);







// 保存するデータ
struct SerializeValues {
	PathInfo recent; // 最近のパス情報 

	RECT placement; // ウィンドウ位置

	WindowControl window; // ウィンドウ設定

	PlayControl play; // オフライン再生

	dsp::AudioControl audio; // オーディオ

	utl::StatusFormat format; // ステータスバー書式

	DialogConfig dialog; // 各ダイアログ設定

	OptionConfing option; // オプションダイアログ
	VideoConfig video; // 映像調整
	DetailConfig detail; // 詳細設定
	OperationConfig operation; // 操作設定
	DisplayConfig display; // 表示設定

	////////////////////////////////////////
	void load(const std::wstring& current);
	void save(const std::wstring& current);
};

} // namespace sl