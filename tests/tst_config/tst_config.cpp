#include <QtTest>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "config/config.h"

using namespace yapcr::config;

// ============================================================
//  tst_config — M5.0: config モジュール単体テスト
//
//  テスト対象（横断決定8 の純ロジック）:
//    1. TOML ラウンドトリップ（プリセット配列・shortcuts・ネスト配列）
//    2. 破損入力 → デフォルト（例外を投げない）
//    3. 未知キー → 無視（既知のみ反映）
//    4. 配置解決純関数 resolveConfigPath()
//    5. ファイル不在 → デフォルト（例外を投げない）
// ============================================================
class TestConfig : public QObject {
    Q_OBJECT

private slots:

    // ------------------------------------------------------------------
    // 1. ラウンドトリップ: makeDefault() → save → load → 全フィールド一致
    // ------------------------------------------------------------------
    void testRoundtrip() {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        const QString path = tmpDir.path() + QStringLiteral("/config.toml");

        const Config orig = Config::makeDefault();
        QVERIFY(save(orig, path));

        const Config back = load(path);

        // [general]
        QCOMPARE(back.general.config_version, orig.general.config_version);
        QCOMPARE(back.general.quit_stop,      orig.general.quit_stop);

        // [restore]
        QCOMPARE(back.restore.position, orig.restore.position);
        QCOMPARE(back.restore.size,     orig.restore.size);
        QCOMPARE(back.restore.aspect,   orig.restore.aspect);
        QCOMPARE(back.restore.volume,   orig.restore.volume);
        QCOMPARE(back.restore.mute,     orig.restore.mute);

        // [display]
        QCOMPARE(back.display.fit_mode,        orig.display.fit_mode);
        QCOMPARE(back.display.start_zoom_100,  orig.display.start_zoom_100);
        QCOMPARE(back.display.zoom_presets,    orig.display.zoom_presets);
        QCOMPARE(back.display.size_presets,    orig.display.size_presets);
        QCOMPARE(back.display.aspect_presets,  orig.display.aspect_presets);

        // [snapshot]
        QCOMPARE(back.snapshot.directory,    orig.snapshot.directory);
        QCOMPARE(back.snapshot.format,       orig.snapshot.format);
        QCOMPARE(back.snapshot.jpeg_quality, orig.snapshot.jpeg_quality);

        // [bbs]
        QCOMPARE(back.bbs.name,              orig.bbs.name);
        QCOMPARE(back.bbs.post_submit_key,   orig.bbs.post_submit_key);
        QCOMPARE(back.bbs.popup_delay_ms,    orig.bbs.popup_delay_ms);
        QCOMPARE(back.bbs.popup_position,    orig.bbs.popup_position);
        QCOMPARE(back.bbs.res_order_reverse, orig.bbs.res_order_reverse);

        // [playback]
        QCOMPARE(back.playback.volume_step,      orig.playback.volume_step);
        QCOMPARE(back.playback.volume_step_low,  orig.playback.volume_step_low);
        QCOMPARE(back.playback.volume_step_high, orig.playback.volume_step_high);
        QCOMPARE(back.playback.minimize_mute,    orig.playback.minimize_mute);

        // [state]
        QCOMPARE(back.state.window_x, orig.state.window_x);
        QCOMPARE(back.state.window_y, orig.state.window_y);
        QCOMPARE(back.state.window_w, orig.state.window_w);
        QCOMPARE(back.state.window_h, orig.state.window_h);
        QCOMPARE(back.state.volume,   orig.state.volume);
        QCOMPARE(back.state.mute,     orig.state.mute);
        QCOMPARE(back.state.sage,     orig.state.sage);

        // [shortcuts] — キー集合が一致すること（並び順は問わない）
        QCOMPARE(back.shortcuts.keys(), orig.shortcuts.keys());
        for (const QString& id : orig.shortcuts.keys()) {
            QCOMPARE(back.shortcuts.value(id), orig.shortcuts.value(id));
        }
    }

    // ------------------------------------------------------------------
    // 2. 破損 TOML → デフォルト（例外を投げない）
    // ------------------------------------------------------------------
    void testCorruptedToml() {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        const QString path = tmpDir.path() + QStringLiteral("/corrupt.toml");

        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("[general\nthis is not valid toml @@@@\n");  // 壊れた TOML
        f.close();

        // 例外を投げないこと・デフォルトが返ること
        Config c;
        QVERIFY_THROWS_NO_EXCEPTION(c = load(path));

        const Config def = Config::makeDefault();
        QCOMPARE(c.general.config_version, def.general.config_version);
        QCOMPARE(c.general.quit_stop,      def.general.quit_stop);
        QCOMPARE(c.display.zoom_presets,   def.display.zoom_presets);
    }

    // ------------------------------------------------------------------
    // 3. 未知キーを含む TOML → 既知のみ反映・未知は無視（落ちない）
    // ------------------------------------------------------------------
    void testUnknownKeysIgnored() {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        const QString path = tmpDir.path() + QStringLiteral("/unknown.toml");

        // 既知キー（quit_stop=true）＋ 未知セクション＋ 未知キー
        const char* toml = R"toml(
[general]
config_version = 1
quit_stop = true
unknown_future_key = "hello"

[unknown_section]
foo = 42
bar = [1, 2, 3]
)toml";

        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write(toml);
        f.close();

        Config c;
        QVERIFY_THROWS_NO_EXCEPTION(c = load(path));

        // 既知キーは反映される
        QCOMPARE(c.general.quit_stop, true);
        // デフォルト値は維持される
        const Config def = Config::makeDefault();
        QCOMPARE(c.display.zoom_presets,   def.display.zoom_presets);
        QCOMPARE(c.restore.position,       def.restore.position);
    }

    // ------------------------------------------------------------------
    // 4. 配置解決純関数 resolveConfigPath()
    // ------------------------------------------------------------------
    void testResolveConfigPath_portable() {
        const QString exeDir     = QStringLiteral("C:/app");
        const QString appDataDir = QStringLiteral("C:/Users/user/AppData/Roaming/YAPCRPlayer");
        // ポータブル: exeDir/config.toml を返す
        const QString result = resolveConfigPath(exeDir, appDataDir, /*portableExists=*/true);
        QCOMPARE(result, QStringLiteral("C:/app/config.toml"));
    }

    void testResolveConfigPath_appdata() {
        const QString exeDir     = QStringLiteral("C:/app");
        const QString appDataDir = QStringLiteral("C:/Users/user/AppData/Roaming/YAPCRPlayer");
        // APPDATA: appDataDir/config.toml を返す
        const QString result = resolveConfigPath(exeDir, appDataDir, /*portableExists=*/false);
        QCOMPARE(result, QStringLiteral("C:/Users/user/AppData/Roaming/YAPCRPlayer/config.toml"));
    }

    // ------------------------------------------------------------------
    // 5. ファイル不在 → デフォルト（例外を投げない）
    // ------------------------------------------------------------------
    void testFileNotFound() {
        const QString path = QStringLiteral("/non/existent/path/config.toml");

        Config c;
        QVERIFY_THROWS_NO_EXCEPTION(c = load(path));

        const Config def = Config::makeDefault();
        QCOMPARE(c.general.config_version, def.general.config_version);
        QCOMPARE(c.display.zoom_presets,   def.display.zoom_presets);
    }

    // ------------------------------------------------------------------
    // 6. shortcuts のラウンドトリップ（QMap<QString, QStringList> の配列形式）
    // ------------------------------------------------------------------
    void testShortcutsRoundtrip() {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        const QString path = tmpDir.path() + QStringLiteral("/config.toml");

        Config orig = Config::makeDefault();
        // 複数キー別名の確認
        orig.shortcuts.insert(QStringLiteral("mute"),
                               {QStringLiteral("M"), QStringLiteral("Ctrl+M")});
        // 空配列の確認
        orig.shortcuts.insert(QStringLiteral("quit"), QStringList{});

        QVERIFY(save(orig, path));
        const Config back = load(path);

        QCOMPARE(back.shortcuts.value(QStringLiteral("mute")),
                 QStringList({QStringLiteral("M"), QStringLiteral("Ctrl+M")}));
        QCOMPARE(back.shortcuts.value(QStringLiteral("quit")), QStringList{});
    }

    // ------------------------------------------------------------------
    // 7. ネスト配列（size_presets / aspect_presets）のラウンドトリップ
    // ------------------------------------------------------------------
    void testNestedArrayRoundtrip() {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        const QString path = tmpDir.path() + QStringLiteral("/config.toml");

        Config orig = Config::makeDefault();
        orig.display.size_presets   = {{640, 360}, {1920, 1080}};
        orig.display.aspect_presets = {{16, 9}, {4, 3}};

        QVERIFY(save(orig, path));
        const Config back = load(path);

        QCOMPARE(back.display.size_presets,   orig.display.size_presets);
        QCOMPARE(back.display.aspect_presets, orig.display.aspect_presets);
    }
};

QTEST_MAIN(TestConfig)
#include "tst_config.moc"
