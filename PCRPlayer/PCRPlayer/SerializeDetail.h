#pragma once

namespace sl {

////////////////////////////////////////////////////////////////////////
// 詳細設定
struct NetworkConfig : public dsp::NetworkControl {
	NetworkConfig()
		: stop(false)
	{}
	bool stop; // プレイヤー終了時チャンネル切断
};

// タイプ別設定
struct TypeConfig : public dsp::TypeControl {
};

// デコーダ情報
struct DecoderInfo {
	DecoderInfo()
		: enable(false)
		, name()
		, id()
	{}

	bool enable; // 有効
	std::wstring name; // フィルタ名
	GUID id; // フィルタID
};

// ブラウザ
struct BrowserConfig {
	BrowserConfig()
		: def(true)
		, bbs(L"PCRBrowser.exe")
		, close(false)
	{}

	bool def; // 既定のブラウザを使う
	std::wstring web; // URLを開くWebブラウザ
	std::wstring bbs; // BBSブラウザ
	bool close; // 終了時にBBSブラウザを終了する
};

// スナップショット
struct SnapshotConfig {
	SnapshotConfig()
		: folder(L"snapshot")
		, format(utl::GRAPHIC_FORMAT_PNG)
		, jpegQuality(100)
		, display(false)
		, fit(false)
	{}

	std::wstring folder; // 保存フォルダ
	int format; // 保存形式
	int jpegQuality; // JPEG品質
	bool display; // 表示解像度で保存する
	bool fit; // 元の解像度より縮小しない
};

// 起動時
struct StartConfig {
	StartConfig()
		: recent(true)
	{}

	bool recent; // 最後に再生したファイルを再生する
};

// 終了時
struct EndConfig {
	EndConfig()
		: pos(true)
		, size(true)
		, aspect(false)
		, volume(true)
		, balance(false)
		, mute(true)
	{}

	bool pos; // ウィンドウ位置を保存
	bool size; // ウィンドウサイズを保存
	bool aspect; // アスペクト比を保存
	bool volume; // 音量を保存
	bool balance; // 音量バランスを保存
	bool mute; // ミュート状態を保存
};

// BBSバー
struct BoardConfig : public bbs::BoardControl {
	// レスポップアップ位置
	enum POPUP_POSITION {
		POPUP_POSITION_CENTER, // カーソル中央
		POPUP_POSITION_LEFT, // カーソル左
		POPUP_POSITION_RIGHT, // カーソル右
		POPUP_POSITION_BOTH, // カーソル左右
		POPUP_POSITION_COUNT, // enum総数
	};

	enum EDIT_SHORTCUT {
		EDIT_SHORTCUT_SHIFT_ENTER, // Shift + Enter
		EDIT_SHORTCUT_CTRL_ENTER, // Ctrl + Enter
		EDIT_SHORTCUT_ALT_ENTER, // Alt + Enter
	};

	struct Movable {
		struct Pair {
			Pair()
				: board(true)
				, thread(false)
			{}

			bool board; // 掲示板
			bool thread; // スレッド
		};

		Movable()
			: contact(true)
			, commandline(true)
			, manual(false)
		{}

		bool contact; // コンタクトURL
		bool commandline; // コマンドライン引数
		bool manual; // アドレスバーから入力
		Pair favorite; // お気に入り
		Pair history; // 履歴
	};

	BoardConfig()
		: contact(true)
		, change(true)
		, retry(false)
		, delay(50)
		, position(POPUP_POSITION_CENTER)
		, under(false)
		, align(true)
		, order(false)
		, scroll(false)
		, shortcut(EDIT_SHORTCUT_SHIFT_ENTER)
		, hide(false)
		, log(false)
		, path(L"log.txt")
		, focus(false)
		, newly(true)
		, def(50)
		, partial(1010)
	{}

	bool contact; // コンタクトURLを読み込む
	bool change; // URL変更を反映
	bool retry; // リトライ終了時にスレッド更新停止
	int delay; // ポップアップ時間
	int position; // ポップアップ位置
	bool under; // バーの下を基準にする
	bool align; // スクリーンから縦にはみ出さないように表示
	bool order; // レス順序反転
	bool scroll; // スレッドスクロール反転
	int shortcut; // 書き込みショートカットキー
	bool hide; // 書き込みが成功したら書き込み欄を非表示にする
	bool log; // 書き込み成功時に書き込みログを作成する
	std::wstring path; // 書き込みログファイルパス
	bool focus; // 書き込み欄表示中は書き込み欄にフォーカスを固定する
	Movable movable; // スレッド移動する動作
	bool newly; // 新着表示をする
	int def; // 新規表示レス数
	int partial; // 分割読み込みレス数
	bbs::Links links; // 掲示板履歴
	bbs::Links threads; // スレッド履歴
};

struct FontConfig {
	struct Font {
		Font()
			: name(L"MS UI Gothic")
			, point(9)
			//, pixel(12)
		{}

		void set(const LOGFONT& lf)
		{
			name = lf.lfFaceName;
			point = abs(utl::heightToPoint(lf.lfHeight));
			//pixel = abs(lf.lfHeight);
		}

		void get(LOGFONT& lf)
		{
			CFont cf;
			cf.CreatePointFont(point * 10, name.c_str());

			memset(&lf, 0, sizeof(LOGFONT));
			cf.GetLogFont(&lf);
			cf.DeleteObject();
		}

		std::wstring name; // フォント名
		int point; // フォントポイント
		//int pixel; // フォントピクセル
	};

	Font thread;
	Font search;
	Font extract;
};

// スクロール
struct ScrollConfig {
	ScrollConfig()
		: init(false)
		, latest(true)
		, refresh(false)
		, manual(true)
		, last(true)
		, delta(1)
		, interval(10)
	{}

	struct Position {
		Position()
			: init(true)
			, latest(false)
		{}

		bool init; // 新規読み込み時に末尾へジャンプ
		bool latest; // 新着レス取得時に末尾を維持
	};

	bool init; // 新規読み込み時に開始
	bool latest; // 新着レス取得時に開始
	bool refresh; // 手動更新時に開始
	bool manual; // 手動スクロールで停止
	bool last; // 末尾で停止
	int delta; // スクロール量
	int interval; // スクロール間隔(ミリ秒)
	Position position;
};

// スプリッタ
struct SplitterConfig {
	SplitterConfig()
		: split(6)
		, edge(true)
		, face(RGB(240, 240, 240))
		, border(RGB(208, 208, 208))
	{}

	int split;
	bool edge;
	COLORREF face;
	COLORREF border;
};

#define DEFAULT_SKIN_THREAD L"chat"
#define DEFAULT_SKIN_POPUP L"default"

// レイアウト
struct SkinConfig {
	SkinConfig()
		: thread(DEFAULT_SKIN_THREAD, RGB(240, 240, 240), bbs::SKIN_NAME_FORMAT_NODEFAULT, false)
		, popup(DEFAULT_SKIN_POPUP, RGB(255, 255, 224), bbs::SKIN_NAME_FORMAT_NORMAL, true)
	{}
	bbs::SkinConfig thread; // スレッドスキン
	bbs::SkinConfig popup; // ポップアップスキン
};

// 詳細設定
struct DetailConfig {
	NetworkConfig network; // ネットワーク
	TypeConfig type; // タイプ別設定
	dsp::DecoderControl decoder; // デコーダ
	std::vector<DecoderInfo> video; // ビデオデコーダ情報
	std::vector<DecoderInfo> audio; // オーディオデコーダ情報
	BrowserConfig browser; // ブラウザ
	SnapshotConfig snapshot; // スナップショット
	StartConfig start; // 起動時
	EndConfig end; // 終了時

	BoardConfig board; // BBSバー
	FontConfig font; // フォント
	PopupBrowserConfig popup; // 内蔵BBSブラウザポップアップ
	ScrollConfig scroll; // 内蔵BBSブラウザスクロール
	SplitterConfig splitter; // スプリッタ
	SkinConfig skin; // 内蔵BBSブラウザスキン
};

void put(XmlParser& p, const bbs::Link& link);
void get(const XmlParser& p, bbs::Link& link);

} // namespace sl