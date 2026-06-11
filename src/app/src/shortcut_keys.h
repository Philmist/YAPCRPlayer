#pragma once

#include <optional>
#include <QHash>
#include <QKeyEvent>
#include <QMap>
#include <QString>

#include "action_id.h"

namespace yapcr::app {

// =============================================================
//  M5.1: キー正規化・変換ユーティリティ（純・Qt6::Gui/Core のみ）
//
//  横断決定3: キー表現は QKeySequence 互換文字列。
//  テンキーは独自 "Num" プレフィックス規約（"Num5"/"Num+"/"NumEnter" 等）を使い
//  Qt::KeypadModifier に変換する薄いラッパを提供する。
// =============================================================

// キーと修飾子の組（ディスパッチの最小単位）
struct KeyChord {
    int                   key{0};
    Qt::KeyboardModifiers mods{Qt::NoModifier};

    bool operator==(const KeyChord& o) const noexcept
    {
        return key == o.key && mods == o.mods;
    }
};

inline size_t qHash(const KeyChord& c, size_t seed = 0) noexcept
{
    return qHashMulti(seed, c.key, static_cast<int>(c.mods));
}

// --- 変換 API ---

// キー文字列 → KeyChord（解析不能は std::nullopt）。
//   "Num0".."Num9"/"Num+"/"Num-"/"Num*"/"Num/"/"Num."/"NumEnter"/"NumDel"/"NumIns"
//   → Qt::KeypadModifier 付き KeyChord。
//   それ以外は QKeySequence::fromString(s, PortableText) で解析。
std::optional<KeyChord> parseKeyChord(const QString& s);

// KeyChord → キー文字列（parseKeyChord の逆変換）。
//   Qt::KeypadModifier が立っていれば "Num" プレフィックス形式で返す。
QString keyChordToString(const KeyChord& c);

// QKeyEvent → KeyChord（ライブイベント正規化）。
//   ev->key() + ev->modifiers() をそのまま格納する。
//   修飾キー単体押下（Qt::Key_Shift 等）は {0, NoModifier} を返す。
KeyChord keyChordFromEvent(const QKeyEvent* ev);

// {ActionId → keys[]} から逆引き {KeyChord → ActionId} を構築。
//   同一 KeyChord の重複は後勝ち＋qWarning。解析不能キーはスキップ＋qWarning。
QHash<KeyChord, ActionId> buildReverseMap(const QMap<ActionId, QStringList>& keyMap);

} // namespace yapcr::app
