#include "stdafx.h"

namespace utl {

const static DWORD bindKeyCode[] = {
	VK_BACK,
	VK_TAB,
	VK_CLEAR,
	VK_RETURN,
	VK_SHIFT,
	VK_CONTROL,
	VK_MENU,
	VK_PAUSE,
	VK_CAPITAL,
	VK_ESCAPE,
	VK_SPACE,
	VK_PRIOR,
	VK_NEXT,
	VK_END,
	VK_HOME,
	VK_LEFT,
	VK_UP,
	VK_RIGHT,
	VK_DOWN,
	VK_PRINT,
	VK_INSERT,
	VK_DELETE,
	VK_LWIN,
	VK_RWIN,
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_MULTIPLY,
	VK_ADD,
	VK_SEPARATOR,
	VK_SUBTRACT,
	VK_DECIMAL,
	VK_DIVIDE,
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12,
	VK_F13,
	VK_F14,
	VK_F15,
	VK_F16,
	VK_NUMLOCK,
	VK_SCROLL,
	0xBA,
	0xBB,
	0xBC,
	0xBD,
	0xBE,
	0xBF,
	0xC0,
	0xDB,
	0xDC,
	0xDD,
	0xDE,
	0xE2,
};

const static wchar_t *bindKeyName[] = {
	L"BackSpace",
	L"Tab",
	L"Clear",
	L"Enter",
	L"Shift",
	L"Ctrl",
	L"Alt",
	L"Pause",
	L"CapsLock",
	L"Esc",
	L"Space",
	L"PageUp",
	L"PageDown",
	L"End",
	L"Home",
	L"←",
	L"↑",
	L"→",
	L"↓",
	L"PrintScreen",
	L"Insert",
	L"Delete",
	L"Left Windows",
	L"Right Windows",
	L"Num 0",
	L"Num 1",
	L"Num 2",
	L"Num 3",
	L"Num 4",
	L"Num 5",
	L"Num 6",
	L"Num 7",
	L"Num 8",
	L"Num 9",
	L"Num *",
	L"Num +",
	L"Num Enter",
	L"Num -",
	L"Num .",
	L"Num /",
	L"F1",
	L"F2",
	L"F3",
	L"F4",
	L"F5",
	L"F6",
	L"F7",
	L"F8",
	L"F9",
	L"F10",
	L"F11",
	L"F12",
	L"F13",
	L"F14",
	L"F15",
	L"F16",
	L"NumLock",
	L"ScrollLock",
	L":",
	L";",
	L",",
	L"-",
	L".",
	L"/",
	L"@",
	L"[",
	L"\\",
	L"]",
	L"^",
	L"\\",
};



inline DWORD toKey(char chara)
{
	return static_cast<DWORD>(chara);
}


std::wstring getKeyText(DWORD key)
{
	if (key == 0x00)
	{
		return L"なし";
	}

	if ((key >= 0x30 && key <= 0x39) || // 0-9
		(key >= 0x41 && key <= 0x5A)) // A-Z
	{
		std::wstring tmp;
		tmp = wchar_t(key);
		return tmp;
	}

	const int count = sizeof(bindKeyName) / sizeof(bindKeyName[0]);
	for (int i = 0; i < count; ++i)
	{
		if (key == bindKeyCode[i])
		{
			return bindKeyName[i];
		}
	}

	return (boost::wformat(L"(0x%1$02X)") % key).str();
}

std::wstring getModText(DWORD mod)
{
	std::wstring tmp;
	if (mod == MODIFY_CODE_NONE)
	{
		return L"";
	}

	if (mod & (DWORD_MAX ^ MODIFY_CODE_MASK))
	{
		return L"Unknown";
	}
	
	if (mod & MODIFY_CODE_SHIFT)
	{
		tmp += L"Shift";
	}

	if (mod & MODIFY_CODE_CTRL)
	{
		if (tmp != L"")
		{
			tmp += L" + ";
		}
		tmp += L"Ctrl";
	}

	if (mod & MODIFY_CODE_ALT)
	{
		if (tmp != L"")
		{
			tmp += L" + ";
		}
		tmp += L"Alt";
	}
	return tmp;
}


std::wstring getIDText(DWORD id)
{
	switch(id)
	{
	// 倍率
	case IDM_WINDOW_ZOOM_RANGE_BEGIN:	return L"ウィンドウ倍率1";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+1:	return L"ウィンドウ倍率2";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+2:	return L"ウィンドウ倍率3";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+3:	return L"ウィンドウ倍率4";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+4:	return L"ウィンドウ倍率5";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+5:	return L"ウィンドウ倍率6";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+6:	return L"ウィンドウ倍率7";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+7:	return L"ウィンドウ倍率8";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+8:	return L"ウィンドウ倍率9";
	case IDM_WINDOW_ZOOM_RANGE_BEGIN+9:	return L"ウィンドウ倍率10";

	// サイズ
	case IDM_WINDOW_SIZE_RANGE_BEGIN:	return L"ウィンドウサイズ1";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+1:	return L"ウィンドウサイズ2";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+2:	return L"ウィンドウサイズ3";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+3:	return L"ウィンドウサイズ4";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+4:	return L"ウィンドウサイズ5";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+5:	return L"ウィンドウサイズ6";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+6:	return L"ウィンドウサイズ7";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+7:	return L"ウィンドウサイズ8";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+8:	return L"ウィンドウサイズ9";
	case IDM_WINDOW_SIZE_RANGE_BEGIN+9:	return L"ウィンドウサイズ10";

	// アスペクト比
	case IDM_WINDOW_ASPECT_DEFAULT:			return L"アスペクト比:デフォルト";
	case IDM_WINDOW_ASPECT_NONE:			return L"アスペクト比:ウィンドウに合わせる";
	case IDM_WINDOW_ASPECT_RANGE_BEGIN:		return L"アスペクト比1";
	case IDM_WINDOW_ASPECT_RANGE_BEGIN+1:	return L"アスペクト比2";
	case IDM_WINDOW_ASPECT_RANGE_BEGIN+2:	return L"アスペクト比3";
	case IDM_WINDOW_ASPECT_RANGE_BEGIN+3:	return L"アスペクト比4";
	case IDM_WINDOW_ASPECT_RANGE_BEGIN+4:	return L"アスペクト比5";

	// 表示
	case IDM_WINDOW_TOPMOST:		return L"最前面表示";

	case IDM_WINDOW_FRAME:			return L"ウィンドウフレーム表示";
	case IDM_WINDOW_TITLE:			return L"タイトルバー表示";
	
	case IDM_WINDOW_STATUS:			return L"ステータスバー表示";
	case IDM_WINDOW_SEEK:			return L"シークバー表示(オフライン)";
	case IDM_WINDOW_BBS:			return L"BBSバー表示(オンライン)";

	case IDM_WINDOW_EDIT:			return L"BBSバーの書き込み欄表示";

	case IDM_WINDOW_BROWSER:		return L"内蔵BBSブラウザ表示";

	case IDM_WINDOW_BROWSER_TOP:	return L"内蔵BBSブラウザ表示位置：上";
	case IDM_WINDOW_BROWSER_BOTTOM:	return L"内蔵BBSブラウザ表示位置：下";
	case IDM_WINDOW_BROWSER_LEFT:	return L"内蔵BBSブラウザ表示位置：左";
	case IDM_WINDOW_BROWSER_RIGHT:	return L"内蔵BBSブラウザ表示位置：右";

	case IDM_WINDOW_BROWSER_TOGGLE_TOP:		return L"内蔵BBSブラウザ表示位置(トグル)：上";
	case IDM_WINDOW_BROWSER_TOGGLE_BOTTOM:	return L"内蔵BBSブラウザ表示位置(トグル)：下";
	case IDM_WINDOW_BROWSER_TOGGLE_LEFT:	return L"内蔵BBSブラウザ表示位置(トグル)：左";
	case IDM_WINDOW_BROWSER_TOGGLE_RIGHT:	return L"内蔵BBSブラウザ表示位置(トグル)：右";

	case IDM_TITLEBAR_SHOW_PLAYER_NAME:		return L"タイトルバーにプレイヤー名表示";
	case IDM_TITLEBAR_SHOW_WINDOW_SIZE:		return L"タイトルバーに画面サイズ表示";
	case IDM_TITLEBAR_SHOW_SOURCE_SIZE:		return L"タイトルバーにソースサイズ表示";
	case IDM_STATUSBAR_SHOW_FPS:			return L"ステータスバーにfps表示(オフライン)";
	case IDM_STATUSBAR_SHOW_WINDOW_SIZE:	return L"ステータスバーに画面サイズ表示";
	case IDM_STATUSBAR_TIME:				return L"ステータスバーの時間を可変表示(オンライン)";
	case IDM_STATUSBAR_PLAY:				return L"ステータスバーで再生情報を優先表示";
	case IDM_SEEKBAR_SHOW_FILENAME:			return L"シークバーにファイル名表示";
	case IDM_BBSBAR_SHOW_COUNTER:			return L"BBSバーにスレッド自動更新カウンター表示";

	// ウィンドウ
	case IDM_WINDOW_SCREEN_SNAP:	return L"スクリーンスナップ";
	case IDM_WINDOW_WINDOW_SNAP:	return L"ウィンドウスナップ";

	case IDM_WINDOW_FIXED_RATIO:	return L"サイズ変更時に比率固定";
	case IDM_WINDOW_PLAY_RATIO:		return L"再生開始時に自動的に比率適用";
	case IDM_WINDOW_HEIGHT_RATIO:	return L"比率適用するとき高さを基準にする";
	case IDM_WINDOW_INSCRIBE_RATIO:	return L"比率適用するとき元のサイズ内に収める";

	case IDM_START_WINDOW_ZOOM_100:	return L"起動時に倍率100%";
	case IDM_SLEEP_STOP:			return L"再生中モニター電源OFFとスタンバイ抑止";

	case IDM_FULLSCREEN_HIDE_BARS:			return L"全画面化時にバーを自動的に隠す";
	case IDM_FULLSCREEN_FLOATING:			return L"全画面化時にバーを自動的に隠すときフローティング表示";
	case IDM_FULLSCREEN_MOUSE:				return L"全画面化時に自動的にマウスカーソルを消す";
	case IDM_FULLSCREEN_TOPMOST:			return L"全画面化時に最前面表示";

	case IDM_MIXIMIZE_TO_FULLSCREEN:		return L"最大化ボタンで全画面化";
	case IDM_MIXIMIZE_SHOW_TASKBAR:			return L"最大化時にタスクバー領域を隠さない";

	// マウス
	case IDM_VOLUME_UP:				return L"音量を上げる";
	case IDM_VOLUME_DOWN:			return L"音量を下げる";
	case IDM_VOLUME_UP_LOW:			return L"音量を上げる(小)";
	case IDM_VOLUME_DOWN_LOW:		return L"音量を下げる(小)";
	case IDM_VOLUME_UP_HIGH:		return L"音量を上げる(大)";
	case IDM_VOLUME_DOWN_HIGH:		return L"音量を下げる(大)";

	case IDM_SEEK_FORWARD:			return L"進む方向にシーク";
	case IDM_SEEK_BACK:				return L"戻る方向にシーク";
	case IDM_SEEK_FORWARD_LOW:		return L"進む方向にシーク(小)";
	case IDM_SEEK_BACK_LOW:			return L"戻る方向にシーク(小)";
	case IDM_SEEK_FORWARD_HIGH:		return L"進む方向にシーク(大)";
	case IDM_SEEK_BACK_HIGH:		return L"戻る方向にシーク(大)";

	case IDM_THREAD_NEXT:			return L"次へスレッドスクロール";
	case IDM_THREAD_PREV:			return L"前へスレッドスクロール";
	case IDM_THREAD_NEXT_LOW:		return L"次へスレッドスクロール(小)";
	case IDM_THREAD_PREV_LOW:		return L"前へスレッドスクロール(小)";
	case IDM_THREAD_NEXT_HIGH:		return L"次へスレッドスクロール(大)";
	case IDM_THREAD_PREV_HIGH:		return L"前へスレッドスクロール(大)";

	case IDM_SCALE_UP:				return L"ウィンドウ拡大";
	case IDM_SCALE_DOWN:			return L"ウィンドウ縮小";
	case IDM_SCALE_UP_LOW:			return L"ウィンドウ拡大(小)";
	case IDM_SCALE_DOWN_LOW:		return L"ウィンドウ縮小(小)";
	case IDM_SCALE_UP_HIGH:			return L"ウィンドウ拡大(大)";
	case IDM_SCALE_DOWN_HIGH:		return L"ウィンドウ縮小(大)";

	case IDM_MUTE:					return L"ミュート";
	case IDM_MINIMIZE_MUTE:			return L"最小化ミュート";

	case IDM_PAUSE:					return L"一時停止";
	case IDM_MINIMIZE_PAUSE:		return L"最小化一時停止";

	case IDM_THREAD_RESET:			return L"スレッドスクロールリセット";
	case IDM_THREAD_RELOAD:			return L"スレッド自動更新";
	case IDM_THREAD_REFRESH:		return L"スレッド手動更新";
	case IDM_BBS_SAGE:				return L"sageで書き込む";

	case IDM_FULLSCREEN:			return L"全画面化";
	case IDM_MAXIMIZE:				return L"最大化";

	case IDM_BALANCE_CENTER:		return L"音量バランス中央";
	case IDM_BALANCE_LEFT:			return L"音量バランス左";
	case IDM_BALANCE_RIGHT:			return L"音量バランス右";

	/////////////////////////////////
	case IDM_UPDATE_CHANNEL_INFO:	return L"チャンネル情報更新";
	case IDM_CHANNEL_INFO_NAME:		return L"チャンネル情報表示：チャンネル名";
	case IDM_CHANNEL_INFO_TYPE:		return L"チャンネル情報表示：タイプ";
	case IDM_CHANNEL_INFO_GENRE:	return L"チャンネル情報表示：ジャンル/詳細";
	case IDM_CHANNEL_INFO_COMMENT:	return L"チャンネル情報表示：コメント";
	case IDM_CHANNEL_INFO_PLAYING:	return L"チャンネル情報表示：アーティスト/タイトル/アルバム";
	case IDM_CHANNEL_INFO_RELAY:	return L"チャンネル情報表示：リスナー/リレー";

	case IDM_BBS_BROWSER:			return L"BBSブラウザでコンタクトURLを開く";
	case IDM_WEB_BROWSER:			return L"コンタクトURLを開く";
	case IDM_CLIPBOARD_URL:			return L"コンタクトURLをクリップボードへコピー";

	case IDM_PEERCAST_BUMP:			return L"チャンネル再接続";
	case IDM_PEERCAST_STOP:			return L"チャンネル切断";

	case IDM_PLAYER_REBUILD:		return L"再生/再構築";
	case IDM_PLAYER_TERMINATE:		return L"停止";


	// フィルタグラフ

	// ファイル/URL
	case IDM_OPENFILE_DIALOG:		return L"ファイル/URLを開く...";
	case IDM_OPENFILE_CLIPBOARD:	return L"クリップボードからファイル/URLを開く";
	case IDM_CLIPBOARD_PATH:		return L"ファイル/URLをクリップボードへコピー";

	// オフライン再生
	case IDM_PREV_FILE:				return L"前のファイル";
	case IDM_NEXT_FILE:				return L"次のファイル";
	case IDM_PREV_FILE_LOOP:		return L"前のファイル(ループ)";
	case IDM_NEXT_FILE_LOOP:		return L"次のファイル(ループ)";

	case IDM_PLAY_MODE_NORMAL:		return L"通常再生";
	case IDM_PLAY_MODE_REPEAT:		return L"リピート再生";
	case IDM_PLAY_MODE_FOLDER:		return L"フォルダ再生";
	case IDM_PLAY_MODE_FOLDER_LOOP:	return L"フォルダループ再生";

	// スナップショット
	case IDM_SNAPSHOT_SAVE:			return L"スナップショット保存";
	case IDM_SNAPSHOT_FOLDER:		return L"スナップショット保存フォルダを開く...";

	// 実行優先度
	case IDM_PROCESS_PRIORITY_REALTIME:		return L"実行優先度：リアルタイム";
	case IDM_PROCESS_PRIORITY_HIGH:			return L"実行優先度：高";
	case IDM_PROCESS_PRIORITY_ABOVE_NORMAL:	return L"実行優先度：通常以上";
	case IDM_PROCESS_PRIORITY_NORMAL:		return L"実行優先度：通常";
	case IDM_PROCESS_PRIORITY_BELOW_NOMAL:	return L"実行優先度：通常以下";
	case IDM_PROCESS_PRIORITY_IDLE:			return L"実行優先度：低";

	// その他
	case IDM_LOG:					return L"ログ表示...";
	case IDM_VERSION:				return L"バージョン情報...";

	case IDM_OPTION:				return L"オプション...";

	case IDM_QUIT_STOP:				return L"チャンネル切断して終了";
	case IDM_QUIT:					return L"終了";

	default:
		break;
	}
	return L"Unknown";
}

///////////////////////////////////////////////////////////////////////////

ShortcutManager::ShortcutManager()
	: movement(24)
{
	// 倍率
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN,	toKey('1'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+1,	toKey('2'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+2,	toKey('3'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+3,	toKey('4'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+4,	toKey('5'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+5,	toKey('6'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+6,	toKey('7'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+7,	toKey('8'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+8,	toKey('9'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_WINDOW_ZOOM_RANGE_BEGIN+9,	toKey('0'), MODIFY_CODE_CTRL));

	// サイズ
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN,	toKey('1'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+1,	toKey('2'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+2,	toKey('3'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+3,	toKey('4'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+4,	toKey('5'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+5,	toKey('6'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+6,	toKey('7'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+7,	toKey('8'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+8,	toKey('9'), MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_WINDOW_SIZE_RANGE_BEGIN+9,	toKey('0'), MODIFY_CODE_SHIFT));

	// アスペクト比
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_DEFAULT,			toKey('1'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_NONE,				toKey('2'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_RANGE_BEGIN,		toKey('3'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_RANGE_BEGIN+1,	toKey('4'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_RANGE_BEGIN+2,	toKey('5'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_RANGE_BEGIN+3,	toKey('6'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_WINDOW_ASPECT_RANGE_BEGIN+4,	toKey('7'), MODIFY_CODE_ALT));

	// 表示
	data.push_back(Shortcut(IDM_WINDOW_TOPMOST,		toKey('T')));

	data.push_back(Shortcut(IDM_WINDOW_FRAME,		toKey('Z')));
	data.push_back(Shortcut(IDM_WINDOW_TITLE,		toKey('X')));
	data.push_back(Shortcut(IDM_WINDOW_STATUS,		toKey('B')));
	data.push_back(Shortcut(IDM_WINDOW_SEEK,		toKey('V')));
	data.push_back(Shortcut(IDM_WINDOW_BBS,			toKey('C')));

	data.push_back(Shortcut(IDM_WINDOW_EDIT,		VK_TAB));

	data.push_back(Shortcut(IDM_WINDOW_BROWSER,			VK_NUMPAD5));
	
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_TOP));
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_BOTTOM));
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_LEFT));
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_RIGHT));

	data.push_back(Shortcut(IDM_WINDOW_BROWSER_TOGGLE_TOP,		VK_NUMPAD8));
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_TOGGLE_BOTTOM,	VK_NUMPAD2));
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_TOGGLE_LEFT,		VK_NUMPAD4));
	data.push_back(Shortcut(IDM_WINDOW_BROWSER_TOGGLE_RIGHT,	VK_NUMPAD6));

	data.push_back(Shortcut(IDM_TITLEBAR_SHOW_PLAYER_NAME));
	data.push_back(Shortcut(IDM_TITLEBAR_SHOW_WINDOW_SIZE));
	data.push_back(Shortcut(IDM_TITLEBAR_SHOW_SOURCE_SIZE));
	data.push_back(Shortcut(IDM_STATUSBAR_SHOW_FPS));
	data.push_back(Shortcut(IDM_STATUSBAR_SHOW_WINDOW_SIZE));
	data.push_back(Shortcut(IDM_STATUSBAR_TIME));
	data.push_back(Shortcut(IDM_STATUSBAR_PLAY));
	data.push_back(Shortcut(IDM_SEEKBAR_SHOW_FILENAME));
	data.push_back(Shortcut(IDM_BBSBAR_SHOW_COUNTER));

	// ウィンドウ
	data.push_back(Shortcut(IDM_WINDOW_SCREEN_SNAP));
	data.push_back(Shortcut(IDM_WINDOW_WINDOW_SNAP));

	data.push_back(Shortcut(IDM_WINDOW_FIXED_RATIO));
	data.push_back(Shortcut(IDM_WINDOW_PLAY_RATIO));
	data.push_back(Shortcut(IDM_WINDOW_HEIGHT_RATIO));
	data.push_back(Shortcut(IDM_WINDOW_INSCRIBE_RATIO));

	data.push_back(Shortcut(IDM_START_WINDOW_ZOOM_100));
	data.push_back(Shortcut(IDM_SLEEP_STOP));

	data.push_back(Shortcut(IDM_FULLSCREEN_HIDE_BARS));
	data.push_back(Shortcut(IDM_FULLSCREEN_FLOATING));
	data.push_back(Shortcut(IDM_FULLSCREEN_MOUSE));
	data.push_back(Shortcut(IDM_FULLSCREEN_TOPMOST));

	data.push_back(Shortcut(IDM_MIXIMIZE_TO_FULLSCREEN));
	data.push_back(Shortcut(IDM_MIXIMIZE_SHOW_TASKBAR));


	// マウス
	data.push_back(Shortcut(IDM_VOLUME_UP,			VK_UP));
	data.push_back(Shortcut(IDM_VOLUME_DOWN,		VK_DOWN));
	data.push_back(Shortcut(IDM_VOLUME_UP_LOW,		VK_UP, MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_VOLUME_DOWN_LOW,	VK_DOWN, MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_VOLUME_UP_HIGH,		VK_UP, MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_VOLUME_DOWN_HIGH,	VK_DOWN, MODIFY_CODE_CTRL));

	data.push_back(Shortcut(IDM_SEEK_FORWARD,		VK_RIGHT));
	data.push_back(Shortcut(IDM_SEEK_BACK,			VK_LEFT));
	data.push_back(Shortcut(IDM_SEEK_FORWARD_LOW,	VK_RIGHT, MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_SEEK_BACK_LOW,		VK_LEFT, MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_SEEK_FORWARD_HIGH,	VK_RIGHT, MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_SEEK_BACK_HIGH,		VK_LEFT, MODIFY_CODE_CTRL));

	data.push_back(Shortcut(IDM_THREAD_NEXT));
	data.push_back(Shortcut(IDM_THREAD_PREV));
	data.push_back(Shortcut(IDM_THREAD_NEXT_LOW));
	data.push_back(Shortcut(IDM_THREAD_PREV_LOW));
	data.push_back(Shortcut(IDM_THREAD_NEXT_HIGH));
	data.push_back(Shortcut(IDM_THREAD_PREV_HIGH));

	data.push_back(Shortcut(IDM_SCALE_UP,			VK_ADD));
	data.push_back(Shortcut(IDM_SCALE_DOWN,			VK_SUBTRACT));
	data.push_back(Shortcut(IDM_SCALE_UP_LOW,		VK_ADD, MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_SCALE_DOWN_LOW,		VK_SUBTRACT, MODIFY_CODE_SHIFT));
	data.push_back(Shortcut(IDM_SCALE_UP_HIGH,		VK_ADD, MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_SCALE_DOWN_HIGH,	VK_SUBTRACT, MODIFY_CODE_CTRL));
	
	data.push_back(Shortcut(IDM_MUTE,				toKey('M')));
	data.push_back(Shortcut(IDM_MINIMIZE_MUTE));

	data.push_back(Shortcut(IDM_PAUSE,				VK_SPACE));
	data.push_back(Shortcut(IDM_MINIMIZE_PAUSE));

	data.push_back(Shortcut(IDM_THREAD_RESET));
	data.push_back(Shortcut(IDM_THREAD_RELOAD));
	data.push_back(Shortcut(IDM_THREAD_REFRESH));
	data.push_back(Shortcut(IDM_BBS_SAGE));

	data.push_back(Shortcut(IDM_FULLSCREEN));
	data.push_back(Shortcut(IDM_MAXIMIZE));

	data.push_back(Shortcut(IDM_BALANCE_CENTER));
	data.push_back(Shortcut(IDM_BALANCE_LEFT));
	data.push_back(Shortcut(IDM_BALANCE_RIGHT));

	/////////////////////////////////

	data.push_back(Shortcut(IDM_UPDATE_CHANNEL_INFO));
	data.push_back(Shortcut(IDM_CHANNEL_INFO_NAME));
	data.push_back(Shortcut(IDM_CHANNEL_INFO_TYPE));
	data.push_back(Shortcut(IDM_CHANNEL_INFO_GENRE));
	data.push_back(Shortcut(IDM_CHANNEL_INFO_COMMENT));
	data.push_back(Shortcut(IDM_CHANNEL_INFO_PLAYING));
	data.push_back(Shortcut(IDM_CHANNEL_INFO_RELAY));

	data.push_back(Shortcut(IDM_BBS_BROWSER));
	data.push_back(Shortcut(IDM_WEB_BROWSER));
	data.push_back(Shortcut(IDM_CLIPBOARD_URL));

	data.push_back(Shortcut(IDM_PEERCAST_BUMP,		toKey('B'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_PEERCAST_STOP,		toKey('Z'), MODIFY_CODE_ALT));

	data.push_back(Shortcut(IDM_PLAYER_REBUILD,		toKey('R'), MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_PLAYER_TERMINATE,	toKey('S'), MODIFY_CODE_CTRL));

	// フィルタグラフ

	// ファイル/URL
	data.push_back(Shortcut(IDM_OPENFILE_DIALOG));
	data.push_back(Shortcut(IDM_OPENFILE_CLIPBOARD,	toKey('V'),	MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_CLIPBOARD_PATH,		toKey('C'),	MODIFY_CODE_CTRL));

	// オフライン再生
	data.push_back(Shortcut(IDM_PREV_FILE,			VK_PRIOR));
	data.push_back(Shortcut(IDM_NEXT_FILE,			VK_NEXT));
	data.push_back(Shortcut(IDM_PREV_FILE_LOOP,		VK_PRIOR,	MODIFY_CODE_CTRL));
	data.push_back(Shortcut(IDM_NEXT_FILE_LOOP,		VK_NEXT,	MODIFY_CODE_CTRL));

	data.push_back(Shortcut(IDM_PLAY_MODE_NORMAL));
	data.push_back(Shortcut(IDM_PLAY_MODE_REPEAT));
	data.push_back(Shortcut(IDM_PLAY_MODE_FOLDER));
	data.push_back(Shortcut(IDM_PLAY_MODE_FOLDER_LOOP));

	// スナップショット
	data.push_back(Shortcut(IDM_SNAPSHOT_SAVE,		toKey('P')));
	data.push_back(Shortcut(IDM_SNAPSHOT_FOLDER,	toKey('O')));

	// 実行優先度
	data.push_back(Shortcut(IDM_PROCESS_PRIORITY_REALTIME));
	data.push_back(Shortcut(IDM_PROCESS_PRIORITY_HIGH));
	data.push_back(Shortcut(IDM_PROCESS_PRIORITY_ABOVE_NORMAL));
	data.push_back(Shortcut(IDM_PROCESS_PRIORITY_NORMAL));
	data.push_back(Shortcut(IDM_PROCESS_PRIORITY_BELOW_NOMAL));
	data.push_back(Shortcut(IDM_PROCESS_PRIORITY_IDLE));

	// その他
	data.push_back(Shortcut(IDM_LOG, toKey('L')));
	data.push_back(Shortcut(IDM_VERSION));

	data.push_back(Shortcut(IDM_OPTION));

	data.push_back(Shortcut(IDM_QUIT_STOP,	toKey('X'), MODIFY_CODE_ALT));
	data.push_back(Shortcut(IDM_QUIT));
}

ShortcutManager::~ShortcutManager()
{
}

std::vector<Shortcut>::iterator ShortcutManager::find(DWORD key, DWORD mod)
{
	return std::find_if(data.begin(), data.end(), [=](const Shortcut& x)->bool{
		return x.key == key && x.mod == mod;
	});
}

std::vector<Shortcut>::iterator ShortcutManager::find(const std::vector<int>& gesture)
{
	return std::find_if(data.begin(), data.end(), [=](const Shortcut& x)->bool{
		return x.gesture == gesture;
	});
}

void ShortcutManager::reset(DWORD key, DWORD mod)
{
	if (key == 0 && mod == 0) { return; }
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		if (it->key == key && it->mod == mod)
		{
			it->key = 0;
			it->mod = 0;
		}
	}
}

void ShortcutManager::reset(const std::vector<int>& gesture)
{
	if (gesture.empty()) { return; }
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		if (it->gesture == gesture)
		{
			it->gesture.clear();
		}
	}
}

void ShortcutManager::reset(DWORD key, DWORD mod, const std::vector<int>& gesture)
{
	if (key == 0 && mod == 0) { reset(gesture); return; }
	if (gesture.empty()) { reset(key, mod); return; }

	for (auto it = data.begin(); it != data.end(); ++it)
	{
		if (it->gesture == gesture)
		{
			it->gesture.clear();
		}

		if (it->key == key && it->mod == mod)
		{
			it->key = 0;
			it->mod = 0;
		}
	}
}

} // namespace bind