#pragma once

namespace sl {

////////////////////////////////////////////////////////////////////////
// 操作設定

// マウスホイール
enum MOUSE_WHEEL {
	MOUSE_WHEEL_NONE = -1,
	MOUSE_WHEEL_VOLUME = 0, // 音量
	MOUSE_WHEEL_SEEK, // シーク
	MOUSE_WHEEL_THREAD, // スレッドスクロール
	MOUSE_WHEEL_ZOOM, // ウィンドウ拡大/縮小
};

// マウスミドルクリック
enum MOUSE_MCLICK {
	MOUSE_MCLICK_NONE = -1, // なし
	MOUSE_MCLICK_MUTE = 0, // ミュート
	MOUSE_MCLICK_MINIMIZE_MUTE, // 最小化ミュート
	MOUSE_MCLICK_PAUSE, // 一時停止
	MOUSE_MCLICK_MINIMIZE_PAUSE, // 最小化一時停止
	MOUSE_MCLICK_THREAD_RESET, // スレッドスクロールリセット
	MOUSE_MCLICK_THREAD_RELOAD, // スレッド自動更新
	MOUSE_MCLICK_BBS_EDIT, // BBSバーの書き込み欄表示
};

// マウス左ダブルクリック
enum MOUSE_LDBLCLICK {
	MOUSE_LDBLCLICK_NONE = -1, // なし
	MOUSE_LDBLCLICK_FULLSCREEN = 0, // 全画面化
	MOUSE_LDBLCLICK_MAXIMIZE, // 最大化
	MOUSE_LDBLCLICK_ZOOM100, // 倍率100%
	MOUSE_LDBLCLICK_ZOOM100_FULLSCREEN, // 倍率100%/全画面化
	MOUSE_LDBLCLICK_ZOOM100_MAXIMIZE, // 倍率100%/最大化
};

// マウス操作
struct MouseOperation {
	// 画面上
	struct Screen {
		Screen(bool invert, int wheel, bool rinvert, int rwheel, int mclick, int ldblclick)
			: invert(invert)
			, wheel(wheel)
			, rinvert()
			, rwheel(rwheel)
			, mclick(mclick)
			, ldblclick(ldblclick)
		{}

		bool invert; // マウスホイール増減反転
		int wheel; // マウスホイール
		bool rinvert; // 右クリック+マウスホイール増減反転
		int rwheel; // 右クリック+マウスホイール
		int mclick; // マウスミドルクリック
		int ldblclick; // マウス左ダブルクリック
	};

	// バー上
	struct Bar {
		Bar(bool transfer, bool invert, int wheel, int mclick)
			: transfer(transfer)
			, invert(invert)
			, wheel(wheel)
			, mclick(mclick)
		{}
		bool transfer; // 画面上の同じ操作
		bool invert; // マウスホイール増減反転
		int wheel; // マウスホイール
		int mclick; // マウスミドルクリック
	};

	MouseOperation()
		: online(Screen(false, MOUSE_WHEEL_VOLUME, false, MOUSE_WHEEL_ZOOM, MOUSE_MCLICK_MUTE, MOUSE_LDBLCLICK_FULLSCREEN))
		, offline(Screen(false, MOUSE_WHEEL_SEEK, false, MOUSE_WHEEL_ZOOM, MOUSE_MCLICK_PAUSE, MOUSE_LDBLCLICK_FULLSCREEN))
		, status(Bar(false, false, MOUSE_WHEEL_VOLUME, MOUSE_MCLICK_MUTE))
		, seek(Bar(false, false, MOUSE_WHEEL_SEEK, MOUSE_MCLICK_PAUSE))
		, bbs(Bar(false, false, MOUSE_WHEEL_THREAD, MOUSE_MCLICK_THREAD_RESET))
	{}

	Screen online;
	Screen offline;
	Bar status;
	Bar seek;
	Bar bbs;
};



// ホイール変化量
struct Variation {
	// 変化量
	struct Value {
		Value(int normal, int low, int high)
			: normal(normal)
			, low(low)
			, high(high)
		{}

		int normal; // 通常
		int low; // 小
		int high; // 大
	};

	Variation()
		: volume(Value(5, 1, 10))
		, seek(Value(5000, 1000, 20000))
		, scroll(Value(10, 1, 100))
		, scale(Value(10, 5, 20))
	{}

	Value volume; // 音量変化量
	Value seek; // シーク移動量
	Value scroll; // スレッドスクロール量
	Value scale; // ウィンドウ拡大/縮小量
};

// ウィンドウ操作
struct WindowOperation {
	WindowOperation()
		: frame(30)
	{}

	int frame; // フレームなしウィンドウのサイズ変更枠
	utl::SnapDistance snap; // スナップ距離
};


// 操作設定
struct OperationConfig {
	utl::ShortcutManager shortcut; // ショートカット設定
	MouseOperation mouse; // マウス操作
	Variation variation; // ホイール変化量
	WindowOperation window; // ウィンドウ操作
};

} // namespace sl