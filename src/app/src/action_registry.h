#pragma once

#include <functional>
#include <QHash>
#include <QMap>
#include <QStringList>

#include "action_id.h"
#include "shortcut_keys.h"

namespace yapcr::app {

// =============================================================
//  M5.1: 中央アクションディスパッチャ
//
//  横断決定1: id → ハンドラの単一テーブル。
//  横断決定4: dispatch() を MainWindow::keyPressEvent から呼ぶ。
//  ハンドラ未登録のアクション（M5.3〜対象）は dispatch() が false を返すため、
//  キーは QMainWindow 既定へ素通しになる（誤消費しない）。
// =============================================================
class ActionRegistry {
public:
    ActionRegistry() = default;

    // キーマップを設定して逆引きテーブルを再構築する。
    // M5.1: defaultKeyMap() を渡す。M5.2: TOML 差分マージ後のマップを渡す。
    void setKeyMap(const QMap<ActionId, QStringList>& keyMap);

    // アクションにハンドラを登録する（上書き可）。
    void on(ActionId id, std::function<void()> fn);

    // アクションを直接トリガーする。
    // ハンドラ登録済みなら実行して true。未登録は false。
    bool trigger(ActionId id);

    // KeyChord からアクションを逆引きしてトリガーする。
    // 逆引きヒット＋ハンドラ登録済みで true。どちらか欠けていれば false。
    bool dispatch(const KeyChord& c);

    // アクションに割り当てられたキー文字列リストを返す（メニューのヒント表示用）。
    QStringList keysFor(ActionId id) const;

private:
    QHash<ActionId, std::function<void()>> handlers_;
    QHash<KeyChord, ActionId>              reverse_;
    QMap<ActionId, QStringList>            keyMap_;  // keysFor 用に保持
};

} // namespace yapcr::app
