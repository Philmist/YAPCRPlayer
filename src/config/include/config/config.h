#pragma once

#include <QString>
#include <QMap>
#include <QStringList>
#include <QVector>
#include <QPair>

// config モジュール — TOML 永続化スキーマ（M5.0）
//
// ・スキーマ確定版は docs/m5-config-subplan.md「TOML スキーマ（確定）」§ を参照。
// ・shortcuts は M5.2 まで生のキー文字列 QMap として保持し、
//   QKeySequence 変換は ActionRegistry (M5.1/M5.2) 側で行う。
// ・このヘッダは Qt の Core 型を使うが toml++ には依存しない（実装詳細は config_io.cpp）。

namespace yapcr::config {

// ---------- サブ構造体 ----------

struct GeneralConfig {
    int  config_version{1};
    bool quit_stop{false};   // 終了時にチャンネル切断（PCRPlayer NetworkConfig.stop）
};

struct RestoreConfig {
    // PCRPlayer EndConfig 踏襲（SerializeDetail.h:72-88）
    bool position{true};
    bool size    {true};
    bool aspect  {false};
    bool volume  {true};
    bool mute    {true};
};

struct DisplayConfig {
    QString fit_mode{QStringLiteral("inscribe")};  // inscribe/stretch/fill/unscaled
    QVector<int>                zoom_presets;       // 10 枠（%）
    QVector<QPair<int,int>>     size_presets;       // 10 枠（w×h）
    QVector<QPair<int,int>>     aspect_presets;     // 5 枠（x:y）
    bool start_zoom_100{false};
};

struct SnapshotConfig {
    QString directory;            // 空=既定 Pictures/YAPCRPlayer
    QString format{QStringLiteral("png")};
    int     jpeg_quality{90};
};

struct BbsConfig {
    QString name;                                        // 既定 名無し
    QString post_submit_key{QStringLiteral("shift+enter")}; // PCRPlayer EDIT_SHORTCUT
    int     popup_delay_ms{50};
    QString popup_position{QStringLiteral("center")};   // center/left/right/both
    bool    res_order_reverse{false};
};

struct PlaybackConfig {
    int  volume_step     {5};
    int  volume_step_low {1};
    int  volume_step_high{10};
    bool minimize_mute   {false};  // ON: 最小化で自動ミュート、復帰で自動解除
};

struct StateConfig {
    int  window_x{0};
    int  window_y{0};
    int  window_w{960};
    int  window_h{540};
    int  volume  {100};
    bool mute    {false};
    bool sage    {false};  // mail 欄代替（PCRPlayer dialog.bbs.sage 相当）
};

// ---------- トップレベル ----------

struct Config {
    GeneralConfig                  general;
    RestoreConfig                  restore;
    QMap<QString, QStringList>     shortcuts;   // ActionId文字列 → キー文字列配列（M5.2 で消費）
    DisplayConfig                  display;
    SnapshotConfig                 snapshot;
    BbsConfig                      bbs;
    PlaybackConfig                 playback;
    StateConfig                    state;

    // 既定プリセットを持つデフォルト Config を生成する。
    // 「Config{} を作ってもプリセットが空のまま」という事故を防ぐためのファクトリ。
    static Config makeDefault();
};

// ---------- I/O API ----------

// path の TOML ファイルを読み込んで Config を返す。
// ファイルが存在しない・パース失敗の場合は qWarning を出してデフォルト Config を返す（例外を投げない）。
// config_version は読み取るが未知バージョンでも落とさない。未知キーは無視（破棄）する。
Config load(const QString& path);

// Config を path に TOML として書き出す。
// 書き出し失敗時は qWarning を出して false を返す（例外を投げない）。
bool   save(const Config& cfg, const QString& path);

// ---------- 配置解決 ----------
//
// 純関数（横断決定8: テスト対象）。I/O から分離してある。
//
// portableExists が true なら exeDir/config.toml のパスを返す（ポータブルモード）。
// false なら appDataDir/config.toml のパスを返す（APPDATA モード）。
QString resolveConfigPath(const QString& exeDir,
                          const QString& appDataDir,
                          bool           portableExists);

// アプリ実行時に使う配置解決ラッパ。
// QCoreApplication::applicationDirPath() / QStandardPaths::AppDataLocation を参照し、
// APPDATA 側を使う場合は親ディレクトリを mkpath で自動作成する。
QString defaultConfigPath();

} // namespace yapcr::config
