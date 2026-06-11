#include "action_id.h"

#include <QDebug>

namespace yapcr::app {

// =============================================================
//  アクション定義テーブル（ハードコード既定）
//
//  デフォルトキーは config_io.cpp の Config::makeDefault() shortcuts と整合。
//  移植元: PCRPlayer/Shortcut.cpp:430-625（ShortcutManager の id↔key テーブル）。
//  M5.2 で [shortcuts] TOML 差分マージへ切り替わる。
// =============================================================
const QVector<ActionDef>& actionTable()
{
    // clang-format off
    static const QVector<ActionDef> table = {
        // id                           configId                   defaultKeys
        // ---- 再生制御 ----
        { ActionId::Bump,              "bump",              {QStringLiteral("Alt+B")}},
        { ActionId::Stop,              "stop",              {QStringLiteral("Alt+Z")}},
        { ActionId::Rebuild,           "rebuild",           {QStringLiteral("Ctrl+R")}},
        { ActionId::Terminate,         "terminate",         {QStringLiteral("Ctrl+S")}},
        { ActionId::Pause,             "pause",             {QStringLiteral("Space")}},
        // ---- 音量 ----
        { ActionId::VolumeUp,          "volume_up",         {QStringLiteral("Up")}},
        { ActionId::VolumeDown,        "volume_down",       {QStringLiteral("Down")}},
        { ActionId::VolumeUpLow,       "volume_up_low",     {QStringLiteral("Shift+Up")}},
        { ActionId::VolumeDownLow,     "volume_down_low",   {QStringLiteral("Shift+Down")}},
        { ActionId::VolumeUpHigh,      "volume_up_high",    {QStringLiteral("Ctrl+Up")}},
        { ActionId::VolumeDownHigh,    "volume_down_high",  {QStringLiteral("Ctrl+Down")}},
        { ActionId::Mute,              "mute",              {QStringLiteral("M")}},
        // ---- 表示トグル ----
        { ActionId::Topmost,           "topmost",           {QStringLiteral("T")}},
        { ActionId::ToggleTitle,       "toggle_title",      {QStringLiteral("X")}},
        { ActionId::ToggleStatus,      "toggle_status",     {QStringLiteral("B")}},
        // ToggleSeek/ToggleFrame: このアプリは mpv --wid 直接描画のため独自シークバー/枠 UI を持たない。
        // デフォルトキー割当なし（対応 UI 要素なし）。
        { ActionId::ToggleSeek,        "toggle_seek",       {}},
        { ActionId::ToggleBbs,         "toggle_bbs",        {QStringLiteral("C")}},
        { ActionId::ToggleResList,     "toggle_res_list",   {}},
        { ActionId::ToggleFrame,       "toggle_frame",      {}},
        // ---- ズームプリセット × 10 ----
        { ActionId::ZoomPreset1,       "zoom_preset_1",     {QStringLiteral("Ctrl+1")}},
        { ActionId::ZoomPreset2,       "zoom_preset_2",     {QStringLiteral("Ctrl+2")}},
        { ActionId::ZoomPreset3,       "zoom_preset_3",     {QStringLiteral("Ctrl+3")}},
        { ActionId::ZoomPreset4,       "zoom_preset_4",     {QStringLiteral("Ctrl+4")}},
        { ActionId::ZoomPreset5,       "zoom_preset_5",     {QStringLiteral("Ctrl+5")}},
        { ActionId::ZoomPreset6,       "zoom_preset_6",     {QStringLiteral("Ctrl+6")}},
        { ActionId::ZoomPreset7,       "zoom_preset_7",     {QStringLiteral("Ctrl+7")}},
        { ActionId::ZoomPreset8,       "zoom_preset_8",     {QStringLiteral("Ctrl+8")}},
        { ActionId::ZoomPreset9,       "zoom_preset_9",     {QStringLiteral("Ctrl+9")}},
        { ActionId::ZoomPreset10,      "zoom_preset_10",    {QStringLiteral("Ctrl+0")}},
        // ---- サイズプリセット × 10 ----
        { ActionId::SizePreset1,       "size_preset_1",     {QStringLiteral("Shift+1")}},
        { ActionId::SizePreset2,       "size_preset_2",     {QStringLiteral("Shift+2")}},
        { ActionId::SizePreset3,       "size_preset_3",     {QStringLiteral("Shift+3")}},
        { ActionId::SizePreset4,       "size_preset_4",     {QStringLiteral("Shift+4")}},
        { ActionId::SizePreset5,       "size_preset_5",     {QStringLiteral("Shift+5")}},
        { ActionId::SizePreset6,       "size_preset_6",     {QStringLiteral("Shift+6")}},
        { ActionId::SizePreset7,       "size_preset_7",     {QStringLiteral("Shift+7")}},
        { ActionId::SizePreset8,       "size_preset_8",     {QStringLiteral("Shift+8")}},
        { ActionId::SizePreset9,       "size_preset_9",     {QStringLiteral("Shift+9")}},
        { ActionId::SizePreset10,      "size_preset_10",    {QStringLiteral("Shift+0")}},
        // ---- アスペクト ----
        { ActionId::AspectDefault,     "aspect_default",    {QStringLiteral("Alt+1")}},
        { ActionId::AspectNone,        "aspect_none",       {QStringLiteral("Alt+2")}},
        { ActionId::AspectPreset1,     "aspect_preset_1",   {QStringLiteral("Alt+3")}},
        { ActionId::AspectPreset2,     "aspect_preset_2",   {QStringLiteral("Alt+4")}},
        { ActionId::AspectPreset3,     "aspect_preset_3",   {QStringLiteral("Alt+5")}},
        { ActionId::AspectPreset4,     "aspect_preset_4",   {QStringLiteral("Alt+6")}},
        { ActionId::AspectPreset5,     "aspect_preset_5",   {QStringLiteral("Alt+7")}},
        // ---- 全画面/最大化 ----
        { ActionId::FullScreen,        "fullscreen",        {QStringLiteral("F")}},
        { ActionId::Maximize,          "maximize",          {}},
        // ---- BBS ----
        { ActionId::ThreadReload,      "thread_reload",     {}},
        { ActionId::ThreadRefresh,     "thread_refresh",    {}},
        { ActionId::ThreadReset,       "thread_reset",      {}},
        { ActionId::SagePost,          "sage_post",         {}},
        { ActionId::ThreadScrollNext,  "thread_scroll_next",{}},
        { ActionId::ThreadScrollPrev,  "thread_scroll_prev",{}},
        // ---- シーク ----
        { ActionId::SeekForward,       "seek_forward",      {QStringLiteral("Right")}},
        { ActionId::SeekBack,          "seek_back",         {QStringLiteral("Left")}},
        { ActionId::SeekForwardLow,    "seek_forward_low",  {QStringLiteral("Shift+Right")}},
        { ActionId::SeekBackLow,       "seek_back_low",     {QStringLiteral("Shift+Left")}},
        { ActionId::SeekForwardHigh,   "seek_forward_high", {QStringLiteral("Ctrl+Right")}},
        { ActionId::SeekBackHigh,      "seek_back_high",    {QStringLiteral("Ctrl+Left")}},
        // ---- スナップショット ----
        { ActionId::SnapshotSave,      "snapshot_save",     {QStringLiteral("P")}},
        { ActionId::SnapshotFolder,    "snapshot_folder",   {QStringLiteral("O")}},
        // ---- ファイル/URL ----
        { ActionId::OpenFileDialog,         "open_file_dialog",          {}},
        { ActionId::OpenFromClipboard,      "open_from_clipboard",       {QStringLiteral("Ctrl+V")}},
        { ActionId::CopyPathToClipboard,    "copy_path_to_clipboard",    {QStringLiteral("Ctrl+C")}},
        { ActionId::CopyContactToClipboard, "copy_contact_to_clipboard", {}},
        { ActionId::OpenContactInBrowser,   "open_contact_in_browser",   {}},
        // ---- 最小化 ----
        { ActionId::Minimize,          "minimize",          {}},
        { ActionId::MinimizeMute,      "minimize_mute",     {}},
        // ---- その他 ----
        { ActionId::OpenConfigFolder,  "open_config_folder",{}},
        { ActionId::ReloadConfig,      "reload_config",     {}},
        { ActionId::Log,               "log",               {QStringLiteral("L")}},
        { ActionId::Version,           "version",           {}},
        { ActionId::Quit,              "quit",              {}},
        { ActionId::QuitStop,          "quit_stop",         {QStringLiteral("Alt+X")}},
    };
    // clang-format on
    return table;
}

QMap<ActionId, QStringList> defaultKeyMap()
{
    QMap<ActionId, QStringList> map;
    for (const ActionDef& def : actionTable()) {
        map.insert(def.id, def.defaultKeys);
    }
    return map;
}

// =============================================================
//  M5.2: configId → ActionId 逆引きマップ（一度だけ構築）
// =============================================================
const QHash<QString, ActionId>& configIdToActionId()
{
    static const QHash<QString, ActionId> map = []() {
        QHash<QString, ActionId> h;
        for (const ActionDef& def : actionTable()) {
            h.insert(QString::fromLatin1(def.configId), def.id);
        }
        return h;
    }();
    return map;
}

// =============================================================
//  M5.2: defaultKeyMap() + userShortcuts の差分マージ
// =============================================================
QMap<ActionId, QStringList> mergeShortcuts(const QMap<QString, QStringList>& userShortcuts)
{
    QMap<ActionId, QStringList> result = defaultKeyMap();  // デフォルトをコピー

    const auto& lookup = configIdToActionId();
    for (auto it = userShortcuts.constBegin(); it != userShortcuts.constEnd(); ++it) {
        const auto found = lookup.constFind(it.key());
        if (found == lookup.constEnd()) {
            qWarning() << "[shortcuts] 未知の configId をスキップします:" << it.key();
            continue;
        }
        // 空配列もそのまま上書き（アンバインドとして扱う）
        result.insert(found.value(), it.value());
    }

    return result;
}

} // namespace yapcr::app
