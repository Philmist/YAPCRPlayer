#pragma once

namespace gl {

struct Html {
	std::wstring header;
	std::wstring res;
	std::wstring newRes;
	std::wstring popupHeader;
	std::wstring popupRes;
};

struct Callback {
	Callback();
	bbs::BBSMultiCallback<bool> dat;
	bbs::BBSCallback<void> extract;
};

struct ScreenStyle {
	ScreenStyle()
		: full(false)
		, maximize(false)
	{}

	bool full;
	bool maximize;
	WINDOWPLACEMENT restore;
};


struct GlobalValues : public utl::Logger {
	GlobalValues()
		: Logger(log, utl::Log::LOGTYPE_NONE)
		, player(log)
		, bbs(log)
	{}

	sl::PathInfo latest; // 最後に再生したパス情報
	utl::SyncLog log; // ログ
	dsp::DSPlayer player; // プレイヤー本体
	bbs::BBSOperator bbs; // BBS操作
	Callback callback; // BBSコールバック
	Html html; // ブラウザHTML
	network::PeerCastManager peca; // PeerCast操作
	std::wstring current; // カレントパス
	
	ScreenStyle screen; // 全画面/フルスクリーン状態

	////////////////////////////////////////////
	// 保存するデータ
	sl::SerializeValues value;


	void getAspect(SIZE& aspect);
	void createChannelInfo();
};

}

// グローバルデータ本体を参照
extern gl::GlobalValues gl_;

// シリアル化
extern sl::SerializeValues sl_;