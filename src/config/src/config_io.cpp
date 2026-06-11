#include "config/config.h"

// toml++ はヘッダオンリー。TOML_HEADER_ONLY=1 が既定のため include だけで使える。
#include <toml++/toml.hpp>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

namespace yapcr::config {

// ============================================================
//  デフォルトプリセット
// ============================================================

Config Config::makeDefault()
{
    Config c;

    // [display] — docs/m5-config-subplan.md「TOML スキーマ（確定）」§ に合わせる
    c.display.zoom_presets = {25, 50, 75, 100, 125, 150, 200, 300, 400, 500};
    c.display.size_presets = {
        {640, 360}, {960, 540}, {1280, 720}, {1920, 1080},
        {854, 480}, {1024, 576}, {1366, 768}, {1600, 900},
        {2560, 1440}, {3840, 2160}
    };
    c.display.aspect_presets = {
        {16, 9}, {4, 3}, {5, 4}, {235, 100}, {185, 100}
    };

    // [shortcuts] — PCRPlayer Shortcut.cpp:430-625 踏襲（M5.1/M5.2 で消費）
    // ActionId 文字列 → キー文字列配列（複数キー別名可）
    c.shortcuts = {
        {QStringLiteral("bump"),             {QStringLiteral("Alt+B")}},
        {QStringLiteral("stop"),             {QStringLiteral("Alt+Z")}},
        {QStringLiteral("rebuild"),          {QStringLiteral("Ctrl+R")}},
        {QStringLiteral("terminate"),        {QStringLiteral("Ctrl+S")}},
        {QStringLiteral("pause"),            {QStringLiteral("Space")}},
        {QStringLiteral("volume_up"),        {QStringLiteral("Up")}},
        {QStringLiteral("volume_down"),      {QStringLiteral("Down")}},
        {QStringLiteral("volume_up_low"),    {QStringLiteral("Shift+Up")}},
        {QStringLiteral("volume_down_low"),  {QStringLiteral("Shift+Down")}},
        {QStringLiteral("volume_up_high"),   {QStringLiteral("Ctrl+Up")}},
        {QStringLiteral("volume_down_high"), {QStringLiteral("Ctrl+Down")}},
        {QStringLiteral("mute"),             {QStringLiteral("M")}},
        {QStringLiteral("topmost"),          {QStringLiteral("T")}},
        {QStringLiteral("toggle_title"),     {QStringLiteral("X")}},
        {QStringLiteral("toggle_status"),    {QStringLiteral("B")}},
        {QStringLiteral("toggle_seek"),      {QStringLiteral("V")}},
        {QStringLiteral("toggle_bbs"),       {QStringLiteral("C")}},
        {QStringLiteral("toggle_frame"),     {QStringLiteral("Z")}},
        {QStringLiteral("zoom_preset_1"),    {QStringLiteral("Ctrl+1")}},
        {QStringLiteral("zoom_preset_2"),    {QStringLiteral("Ctrl+2")}},
        {QStringLiteral("zoom_preset_3"),    {QStringLiteral("Ctrl+3")}},
        {QStringLiteral("zoom_preset_4"),    {QStringLiteral("Ctrl+4")}},
        {QStringLiteral("zoom_preset_5"),    {QStringLiteral("Ctrl+5")}},
        {QStringLiteral("zoom_preset_6"),    {QStringLiteral("Ctrl+6")}},
        {QStringLiteral("zoom_preset_7"),    {QStringLiteral("Ctrl+7")}},
        {QStringLiteral("zoom_preset_8"),    {QStringLiteral("Ctrl+8")}},
        {QStringLiteral("zoom_preset_9"),    {QStringLiteral("Ctrl+9")}},
        {QStringLiteral("zoom_preset_10"),   {QStringLiteral("Ctrl+0")}},
        {QStringLiteral("size_preset_1"),    {QStringLiteral("Shift+1")}},
        {QStringLiteral("size_preset_2"),    {QStringLiteral("Shift+2")}},
        {QStringLiteral("size_preset_3"),    {QStringLiteral("Shift+3")}},
        {QStringLiteral("size_preset_4"),    {QStringLiteral("Shift+4")}},
        {QStringLiteral("size_preset_5"),    {QStringLiteral("Shift+5")}},
        {QStringLiteral("size_preset_6"),    {QStringLiteral("Shift+6")}},
        {QStringLiteral("size_preset_7"),    {QStringLiteral("Shift+7")}},
        {QStringLiteral("size_preset_8"),    {QStringLiteral("Shift+8")}},
        {QStringLiteral("size_preset_9"),    {QStringLiteral("Shift+9")}},
        {QStringLiteral("size_preset_10"),   {QStringLiteral("Shift+0")}},
        {QStringLiteral("aspect_default"),   {QStringLiteral("Alt+1")}},
        {QStringLiteral("aspect_none"),      {QStringLiteral("Alt+2")}},
        {QStringLiteral("aspect_preset_1"),  {QStringLiteral("Alt+3")}},
        {QStringLiteral("aspect_preset_2"),  {QStringLiteral("Alt+4")}},
        {QStringLiteral("aspect_preset_3"),  {QStringLiteral("Alt+5")}},
        {QStringLiteral("aspect_preset_4"),  {QStringLiteral("Alt+6")}},
        {QStringLiteral("aspect_preset_5"),  {QStringLiteral("Alt+7")}},
        {QStringLiteral("fullscreen"),       {QStringLiteral("F")}},
        {QStringLiteral("snapshot_save"),    {QStringLiteral("P")}},
        {QStringLiteral("snapshot_folder"),  {QStringLiteral("O")}},
        {QStringLiteral("quit"),             {}},
        {QStringLiteral("quit_stop"),        {QStringLiteral("Alt+X")}},
        {QStringLiteral("log"),              {QStringLiteral("L")}},
        {QStringLiteral("seek_forward"),     {QStringLiteral("Right")}},
        {QStringLiteral("seek_back"),        {QStringLiteral("Left")}},
        {QStringLiteral("seek_forward_low"), {QStringLiteral("Shift+Right")}},
        {QStringLiteral("seek_back_low"),    {QStringLiteral("Shift+Left")}},
        {QStringLiteral("seek_forward_high"), {QStringLiteral("Ctrl+Right")}},
        {QStringLiteral("seek_back_high"),   {QStringLiteral("Ctrl+Left")}},
    };

    return c;
}

// ============================================================
//  内部ユーティリティ
// ============================================================

namespace {

// std::string ⇄ QString の薄いラッパ
inline QString qs(const std::string& s) { return QString::fromStdString(s); }
inline std::string ss(const QString& q)  { return q.toStdString(); }

// テーブルから整数値を安全に取得（キーが無い・型違いは defaultVal を返す）
template<typename T>
T getOr(const toml::table& t, std::string_view key, T defaultVal)
{
    if (auto v = t[key].value<T>()) { return *v; }
    return defaultVal;
}

bool getBoolOr(const toml::table& t, std::string_view key, bool def)
{
    return getOr<bool>(t, key, def);
}

int getIntOr(const toml::table& t, std::string_view key, int def)
{
    if (auto v = t[key].value<int64_t>()) {
        return static_cast<int>(*v);
    }
    return def;
}

QString getStringOr(const toml::table& t, std::string_view key, const QString& def)
{
    if (auto v = t[key].value<std::string>()) { return qs(*v); }
    return def;
}

// ============================================================
//  TOML → Config（既知キーのみ取り出す＝未知キーは自然に破棄）
// ============================================================

Config parseTable(const toml::table& root)
{
    Config c = Config::makeDefault();   // デフォルトをベースに差分上書き

    // ---- [general] ----
    if (auto* g = root["general"].as_table()) {
        c.general.config_version = getIntOr(*g, "config_version", c.general.config_version);
        c.general.quit_stop      = getBoolOr(*g, "quit_stop",     c.general.quit_stop);
    }

    // ---- [restore] ----
    if (auto* r = root["restore"].as_table()) {
        c.restore.position = getBoolOr(*r, "position", c.restore.position);
        c.restore.size     = getBoolOr(*r, "size",     c.restore.size);
        c.restore.aspect   = getBoolOr(*r, "aspect",   c.restore.aspect);
        c.restore.volume   = getBoolOr(*r, "volume",   c.restore.volume);
        c.restore.mute     = getBoolOr(*r, "mute",     c.restore.mute);
    }

    // ---- [shortcuts] ----
    if (auto* s = root["shortcuts"].as_table()) {
        c.shortcuts.clear();
        for (auto& [k, v] : *s) {
            if (auto* arr = v.as_array()) {
                QStringList keys;
                for (auto& elem : *arr) {
                    if (auto sv = elem.value<std::string>()) {
                        keys << qs(*sv);
                    }
                }
                c.shortcuts.insert(qs(std::string(k)), keys);
            }
        }
    }

    // ---- [display] ----
    if (auto* d = root["display"].as_table()) {
        c.display.fit_mode       = getStringOr(*d, "fit_mode",       c.display.fit_mode);
        c.display.start_zoom_100 = getBoolOr  (*d, "start_zoom_100", c.display.start_zoom_100);

        if (auto* zp = d->at_path("zoom_presets").as_array()) {
            c.display.zoom_presets.clear();
            for (auto& e : *zp) {
                if (auto v = e.value<int64_t>()) { c.display.zoom_presets << static_cast<int>(*v); }
            }
        }
        if (auto* sp = d->at_path("size_presets").as_array()) {
            c.display.size_presets.clear();
            for (auto& e : *sp) {
                if (auto* pair = e.as_array(); pair && pair->size() == 2) {
                    auto w = (*pair)[0].value<int64_t>();
                    auto h = (*pair)[1].value<int64_t>();
                    if (w && h) {
                        c.display.size_presets << QPair<int,int>(static_cast<int>(*w),
                                                                  static_cast<int>(*h));
                    }
                }
            }
        }
        if (auto* ap = d->at_path("aspect_presets").as_array()) {
            c.display.aspect_presets.clear();
            for (auto& e : *ap) {
                if (auto* pair = e.as_array(); pair && pair->size() == 2) {
                    auto x = (*pair)[0].value<int64_t>();
                    auto y = (*pair)[1].value<int64_t>();
                    if (x && y) {
                        c.display.aspect_presets << QPair<int,int>(static_cast<int>(*x),
                                                                    static_cast<int>(*y));
                    }
                }
            }
        }
    }

    // ---- [snapshot] ----
    if (auto* s = root["snapshot"].as_table()) {
        c.snapshot.directory    = getStringOr(*s, "directory",    c.snapshot.directory);
        c.snapshot.format       = getStringOr(*s, "format",       c.snapshot.format);
        c.snapshot.jpeg_quality = getIntOr   (*s, "jpeg_quality", c.snapshot.jpeg_quality);
    }

    // ---- [bbs] ----
    if (auto* b = root["bbs"].as_table()) {
        c.bbs.name              = getStringOr(*b, "name",              c.bbs.name);
        c.bbs.post_submit_key   = getStringOr(*b, "post_submit_key",   c.bbs.post_submit_key);
        c.bbs.popup_delay_ms    = getIntOr   (*b, "popup_delay_ms",    c.bbs.popup_delay_ms);
        c.bbs.popup_position    = getStringOr(*b, "popup_position",    c.bbs.popup_position);
        c.bbs.res_order_reverse = getBoolOr  (*b, "res_order_reverse", c.bbs.res_order_reverse);
    }

    // ---- [playback] ----
    if (auto* p = root["playback"].as_table()) {
        c.playback.volume_step      = getIntOr (*p, "volume_step",      c.playback.volume_step);
        c.playback.volume_step_low  = getIntOr (*p, "volume_step_low",  c.playback.volume_step_low);
        c.playback.volume_step_high = getIntOr (*p, "volume_step_high", c.playback.volume_step_high);
        c.playback.minimize_mute    = getBoolOr(*p, "minimize_mute",    c.playback.minimize_mute);
    }

    // ---- [state] ----
    if (auto* s = root["state"].as_table()) {
        c.state.window_x = getIntOr (*s, "window_x", c.state.window_x);
        c.state.window_y = getIntOr (*s, "window_y", c.state.window_y);
        c.state.window_w = getIntOr (*s, "window_w", c.state.window_w);
        c.state.window_h = getIntOr (*s, "window_h", c.state.window_h);
        c.state.volume   = getIntOr (*s, "volume",   c.state.volume);
        c.state.mute     = getBoolOr(*s, "mute",     c.state.mute);
        c.state.sage     = getBoolOr(*s, "sage",     c.state.sage);
    }

    return c;
}

// ============================================================
//  Config → TOML（toml::table を構築してフォーマット出力）
// ============================================================

toml::table buildTable(const Config& c)
{
    using namespace toml::literals;

    toml::table root;

    // [general]
    root.insert("general", toml::table{{
        {"config_version", c.general.config_version},
        {"quit_stop",      c.general.quit_stop},
    }});

    // [restore]
    root.insert("restore", toml::table{{
        {"position", c.restore.position},
        {"size",     c.restore.size},
        {"aspect",   c.restore.aspect},
        {"volume",   c.restore.volume},
        {"mute",     c.restore.mute},
    }});

    // [shortcuts]
    {
        toml::table scTbl;
        for (auto it = c.shortcuts.constBegin(); it != c.shortcuts.constEnd(); ++it) {
            toml::array arr;
            for (const QString& k : it.value()) {
                arr.push_back(ss(k));
            }
            scTbl.insert(ss(it.key()), std::move(arr));
        }
        root.insert("shortcuts", std::move(scTbl));
    }

    // [display]
    {
        toml::array zp;
        for (int v : c.display.zoom_presets) { zp.push_back(static_cast<int64_t>(v)); }

        toml::array sp;
        for (const auto& [w, h] : c.display.size_presets) {
            toml::array pair;
            pair.push_back(static_cast<int64_t>(w));
            pair.push_back(static_cast<int64_t>(h));
            sp.push_back(std::move(pair));
        }

        toml::array ap;
        for (const auto& [x, y] : c.display.aspect_presets) {
            toml::array pair;
            pair.push_back(static_cast<int64_t>(x));
            pair.push_back(static_cast<int64_t>(y));
            ap.push_back(std::move(pair));
        }

        root.insert("display", toml::table{{
            {"fit_mode",       ss(c.display.fit_mode)},
            {"zoom_presets",   std::move(zp)},
            {"size_presets",   std::move(sp)},
            {"aspect_presets", std::move(ap)},
            {"start_zoom_100", c.display.start_zoom_100},
        }});
    }

    // [snapshot]
    root.insert("snapshot", toml::table{{
        {"directory",    ss(c.snapshot.directory)},
        {"format",       ss(c.snapshot.format)},
        {"jpeg_quality", static_cast<int64_t>(c.snapshot.jpeg_quality)},
    }});

    // [bbs]
    root.insert("bbs", toml::table{{
        {"name",              ss(c.bbs.name)},
        {"post_submit_key",   ss(c.bbs.post_submit_key)},
        {"popup_delay_ms",    static_cast<int64_t>(c.bbs.popup_delay_ms)},
        {"popup_position",    ss(c.bbs.popup_position)},
        {"res_order_reverse", c.bbs.res_order_reverse},
    }});

    // [playback]
    root.insert("playback", toml::table{{
        {"volume_step",       static_cast<int64_t>(c.playback.volume_step)},
        {"volume_step_low",   static_cast<int64_t>(c.playback.volume_step_low)},
        {"volume_step_high",  static_cast<int64_t>(c.playback.volume_step_high)},
        {"minimize_mute",     c.playback.minimize_mute},
    }});

    // [state]
    root.insert("state", toml::table{{
        {"window_x", static_cast<int64_t>(c.state.window_x)},
        {"window_y", static_cast<int64_t>(c.state.window_y)},
        {"window_w", static_cast<int64_t>(c.state.window_w)},
        {"window_h", static_cast<int64_t>(c.state.window_h)},
        {"volume",   static_cast<int64_t>(c.state.volume)},
        {"mute",     c.state.mute},
        {"sage",     c.state.sage},
    }});

    return root;
}

} // anonymous namespace

// ============================================================
//  公開 API 実装
// ============================================================

Config load(const QString& path)
{
    QFile f(path);
    if (!f.exists()) {
        // ファイルが無い場合は警告なしでデフォルトを返す（初回起動の通常ケース）
        qDebug() << "[config] ファイルが存在しないためデフォルト設定を使用:" << path;
        return Config::makeDefault();
    }

    // ファイルを std::string に読み込む
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[config] ファイルを開けません（デフォルト設定で起動）:" << path;
        return Config::makeDefault();
    }
    const QByteArray bytes = f.readAll();
    f.close();

    // toml++ でパース。例外オン（デフォルト）の場合 parse() は toml::table を返す。
    // パース失敗時は toml::parse_error を throw する。
    try {
        const toml::table result = toml::parse(bytes.toStdString());
        return parseTable(result);
    } catch (const toml::parse_error& e) {
        qWarning() << "[config] TOML パース失敗（デフォルト設定で起動）:" << path
                   << "—" << qs(std::string(e.what()));
        return Config::makeDefault();
    } catch (const std::exception& e) {
        qWarning() << "[config] 予期しないエラー（デフォルト設定で起動）:" << path
                   << "—" << e.what();
        return Config::makeDefault();
    }
}

bool save(const Config& cfg, const QString& path)
{
    // 書き出し先のディレクトリが存在しなければ作成する
    const QFileInfo fi(path);
    if (!fi.dir().exists()) {
        if (!QDir().mkpath(fi.dir().absolutePath())) {
            qWarning() << "[config] 書き出し先ディレクトリの作成に失敗:" << fi.dir().absolutePath();
            return false;
        }
    }

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "[config] ファイルを書き出し用に開けません:" << path;
        return false;
    }

    try {
        const toml::table tbl = buildTable(cfg);
        std::ostringstream oss;
        oss << toml::toml_formatter{tbl};
        const QByteArray bytes = QByteArray::fromStdString(oss.str());
        if (f.write(bytes) == -1) {
            qWarning() << "[config] ファイル書き出し失敗:" << path;
            f.close();
            return false;
        }
    } catch (const std::exception& e) {
        qWarning() << "[config] TOML シリアライズエラー:" << e.what();
        f.close();
        return false;
    }

    f.close();
    return true;
}

// ============================================================
//  配置解決
// ============================================================

QString resolveConfigPath(const QString& exeDir,
                          const QString& appDataDir,
                          bool           portableExists)
{
    if (portableExists) {
        return exeDir + QStringLiteral("/config.toml");
    }
    return appDataDir + QStringLiteral("/config.toml");
}

QString defaultConfigPath()
{
    const QString exeDir     = QCoreApplication::applicationDirPath();
    const QString appDataDir = QStandardPaths::writableLocation(
                                   QStandardPaths::AppDataLocation);

    const bool portableExists = QFile::exists(exeDir + QStringLiteral("/config.toml"));
    const QString path        = resolveConfigPath(exeDir, appDataDir, portableExists);

    // APPDATA 側を使う場合はディレクトリを自動作成する
    if (!portableExists) {
        const QDir dir(appDataDir);
        if (!dir.exists()) {
            if (!QDir().mkpath(appDataDir)) {
                qWarning() << "[config] APPDATA ディレクトリの作成に失敗:" << appDataDir;
            }
        }
    }

    qDebug() << "[config] 設定ファイルパス:" << path
             << (portableExists ? "(ポータブル)" : "(APPDATA)");
    return path;
}

} // namespace yapcr::config
