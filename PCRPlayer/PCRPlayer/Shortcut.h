#pragma once

#include "Gesture.h"

#include <boost/format.hpp>

namespace utl {


enum MODIFYCODE {
	MODIFY_CODE_NONE	= 0x00000000,
	MODIFY_CODE_SHIFT	= 0x00000001,
	MODIFY_CODE_CTRL	= 0x00000002,
	MODIFY_CODE_ALT		= 0x00000004,
	MODIFY_CODE_MASK	= MODIFY_CODE_SHIFT | MODIFY_CODE_CTRL | MODIFY_CODE_ALT,
};

inline DWORD toKey(char chara);

std::wstring getKeyText(DWORD key);
std::wstring getModText(DWORD mod);
std::wstring getIDText(DWORD id);

struct Shortcut {
public:
	Shortcut(
		DWORD id = 0,
		DWORD key = 0,
		DWORD mod = 0,
		const std::vector<int>& gesture = std::vector<int>())
		: id(id)
		, key(key)
		, mod(mod)
		, gesture(gesture)
	{}

	DWORD id; // 呼び出すコマンドID
	DWORD key;	// キーコード
	DWORD mod; // 修飾キー
	std::vector<int> gesture; // マウスジェスチャ
};

class ShortcutManager {
public:
	int movement;
	std::vector<Shortcut> data;

public:
	ShortcutManager();
	virtual ~ShortcutManager();

	std::vector<Shortcut>::iterator find(DWORD key, DWORD mod);
	std::vector<Shortcut>::iterator find(const std::vector<int>& gesture);
	void reset(DWORD key, DWORD mod);
	void reset(const std::vector<int>& gesture);
	void reset(DWORD key, DWORD mod, const std::vector<int>& gesture);
};

} // namespace bind

