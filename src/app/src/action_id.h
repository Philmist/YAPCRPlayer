#pragma once

#include <QHash>
#include <QMap>
#include <QStringList>
#include <QVector>

namespace yapcr::app {

// =============================================================
//  M5.1: アクション識別子（v1 集合）
//
//  docs/m5-config-subplan.md「v1 アクション集合」と 1:1 対応。
//  TOML [shortcuts] の configId（snake_case 文字列）と対応付けられる。
// =============================================================
enum class ActionId {
    // ---- 再生制御 ----
    Bump, Stop, Rebuild, Terminate, Pause,
    // ---- 音量 ----
    VolumeUp,     VolumeDown,
    VolumeUpLow,  VolumeDownLow,
    VolumeUpHigh, VolumeDownHigh,
    Mute,
    // ---- 表示トグル ----
    Topmost, ToggleTitle, ToggleStatus, ToggleSeek, ToggleBbs, ToggleResList, ToggleFrame,
    // ---- ズームプリセット × 10 ----
    ZoomPreset1, ZoomPreset2, ZoomPreset3, ZoomPreset4, ZoomPreset5,
    ZoomPreset6, ZoomPreset7, ZoomPreset8, ZoomPreset9, ZoomPreset10,
    // ---- サイズプリセット × 10 ----
    SizePreset1, SizePreset2, SizePreset3, SizePreset4, SizePreset5,
    SizePreset6, SizePreset7, SizePreset8, SizePreset9, SizePreset10,
    // ---- アスペクト ----
    AspectDefault, AspectNone,
    AspectPreset1, AspectPreset2, AspectPreset3, AspectPreset4, AspectPreset5,
    // ---- 全画面/最大化 ----
    FullScreen, Maximize,
    // ---- BBS ----
    ThreadReload, ThreadRefresh, ThreadReset, SagePost,
    ThreadScrollNext, ThreadScrollPrev,
    // ---- シーク（ローカル限定）----
    SeekForward,     SeekBack,
    SeekForwardLow,  SeekBackLow,
    SeekForwardHigh, SeekBackHigh,
    // ---- スナップショット ----
    SnapshotSave, SnapshotFolder,
    // ---- ファイル/URL ----
    OpenFileDialog, OpenFromClipboard,
    CopyPathToClipboard, CopyContactToClipboard, OpenContactInBrowser,
    // ---- 最小化 ----
    Minimize, MinimizeMute,
    // ---- その他 ----
    OpenConfigFolder, ReloadConfig, Log, Version, Quit, QuitStop,
};

// ActionId を QHash のキーとして使えるようにする
inline size_t qHash(ActionId id, size_t seed = 0) noexcept
{
    return ::qHash(static_cast<int>(id), seed);
}

// アクション定義エントリ
struct ActionDef {
    ActionId    id;
    const char* configId;    // TOML [shortcuts] キー（snake_case）
    QStringList defaultKeys; // デフォルトキー文字列（複数可。空 = 未割当）
};

// 全アクション定義テーブル（唯一の真実源。アプリ生存期間を通じて有効）
const QVector<ActionDef>& actionTable();

// テーブルから {ActionId → keys[]} マップを構築して返す。
// M5.1 ではハードコード既定として使用（M5.2 で TOML 差分マージへ差し替え）。
QMap<ActionId, QStringList> defaultKeyMap();

// configId（snake_case）→ ActionId の逆引きマップ（M5.2）。
// actionTable() から1回だけ構築され、アプリ生存期間を通じて有効。
const QHash<QString, ActionId>& configIdToActionId();

// defaultKeyMap() をベースに userShortcuts（configId→keys[]）で差分上書きしたマップを返す（M5.2）。
// 未知 configId は qWarning を出してスキップ（落とさない）。
// 空 keys[] は「アンバインド」として上書きする。
QMap<ActionId, QStringList> mergeShortcuts(const QMap<QString, QStringList>& userShortcuts);

} // namespace yapcr::app
