#include <QtTest>
#include <QKeyEvent>

#include "action_id.h"
#include "shortcut_keys.h"
#include "action_registry.h"
#include "volume_state.h"   // M5.3: 純ロジック（ヘッダオンリー）
#include "media_source.h"   // M5.4: 純ロジック（ヘッダオンリー）

using namespace yapcr::app;

// ============================================================
//  tst_shortcuts — M5.1: ActionRegistry / キー正規化 単体テスト
//
//  テスト対象（横断決定8 の純ロジック）:
//    1. キー文字列 ⇄ KeyChord ラウンドトリップ
//    2. Num プレフィックス規約（テンキー）
//    3. 複数キー別名（1 ActionId に複数キー）
//    4. 衝突後勝ち（buildReverseMap）
//    5. QKeyEvent → KeyChord 正規化（keyChordFromEvent）
//    6. ActionRegistry on/trigger/dispatch
// ============================================================
class TestShortcuts : public QObject {
    Q_OBJECT

private slots:

    // ------------------------------------------------------------------
    // 1. キー文字列 ⇄ KeyChord ラウンドトリップ
    // ------------------------------------------------------------------
    void testParseRoundtrip_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("expectedKey");
        QTest::addColumn<Qt::KeyboardModifiers>("expectedMods");

        QTest::newRow("F")           << QStringLiteral("F")        << (int)Qt::Key_F        << Qt::KeyboardModifiers{Qt::NoModifier};
        QTest::newRow("Space")       << QStringLiteral("Space")     << (int)Qt::Key_Space    << Qt::KeyboardModifiers{Qt::NoModifier};
        QTest::newRow("Up")          << QStringLiteral("Up")        << (int)Qt::Key_Up       << Qt::KeyboardModifiers{Qt::NoModifier};
        QTest::newRow("Shift+Up")    << QStringLiteral("Shift+Up")  << (int)Qt::Key_Up       << Qt::KeyboardModifiers{Qt::ShiftModifier};
        QTest::newRow("Ctrl+R")      << QStringLiteral("Ctrl+R")    << (int)Qt::Key_R        << Qt::KeyboardModifiers{Qt::ControlModifier};
        QTest::newRow("Alt+B")       << QStringLiteral("Alt+B")     << (int)Qt::Key_B        << Qt::KeyboardModifiers{Qt::AltModifier};
        QTest::newRow("Ctrl+1")      << QStringLiteral("Ctrl+1")    << (int)Qt::Key_1        << Qt::KeyboardModifiers{Qt::ControlModifier};
        QTest::newRow("Shift+Right") << QStringLiteral("Shift+Right")<< (int)Qt::Key_Right   << Qt::KeyboardModifiers{Qt::ShiftModifier};
    }

    void testParseRoundtrip() {
        QFETCH(QString, input);
        QFETCH(int, expectedKey);
        QFETCH(Qt::KeyboardModifiers, expectedMods);

        const auto chord = parseKeyChord(input);
        QVERIFY(chord.has_value());
        QCOMPARE(chord->key,  expectedKey);
        QCOMPARE(chord->mods, expectedMods);

        // 逆変換（文字列復元）→ 再解析で同じ KeyChord が得られること
        const QString str  = keyChordToString(*chord);
        const auto   back  = parseKeyChord(str);
        QVERIFY(back.has_value());
        QCOMPARE(*back, *chord);
    }

    // ------------------------------------------------------------------
    // 2. Num プレフィックス規約（テンキー）
    // ------------------------------------------------------------------
    void testNumPrefix_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("expectedKey");

        QTest::newRow("Num5")     << QStringLiteral("Num5")     << (int)Qt::Key_5;
        QTest::newRow("Num+")     << QStringLiteral("Num+")     << (int)Qt::Key_Plus;
        QTest::newRow("Num-")     << QStringLiteral("Num-")     << (int)Qt::Key_Minus;
        QTest::newRow("NumEnter") << QStringLiteral("NumEnter") << (int)Qt::Key_Return;
    }

    void testNumPrefix() {
        QFETCH(QString, input);
        QFETCH(int, expectedKey);

        const auto chord = parseKeyChord(input);
        QVERIFY(chord.has_value());
        QCOMPARE(chord->key, expectedKey);
        // KeypadModifier が立っていること
        QVERIFY(chord->mods & Qt::KeypadModifier);

        // 逆変換で "Num" プレフィックスが復元されること
        const QString back = keyChordToString(*chord);
        QVERIFY(back.startsWith(QStringLiteral("Num"), Qt::CaseInsensitive));
    }

    void testUnknownNumPrefixReturnsNullopt() {
        // 未知の Num サフィックスは解析不能
        const auto chord = parseKeyChord(QStringLiteral("NumX"));
        QVERIFY(!chord.has_value());
    }

    void testInvalidKeyReturnsNullopt() {
        const auto chord = parseKeyChord(QStringLiteral("NotAKey!!!"));
        QVERIFY(!chord.has_value());
    }

    void testEmptyStringReturnsNullopt() {
        const auto chord = parseKeyChord(QStringLiteral(""));
        QVERIFY(!chord.has_value());
    }

    // ------------------------------------------------------------------
    // 3. 複数キー別名（1 ActionId に2キー → 両方が同じ id に解決）
    // ------------------------------------------------------------------
    void testMultipleKeysForSameAction() {
        QMap<ActionId, QStringList> keyMap;
        keyMap[ActionId::Mute] = {QStringLiteral("M"), QStringLiteral("Ctrl+M")};

        const auto rev = buildReverseMap(keyMap);

        const auto chordM     = parseKeyChord(QStringLiteral("M"));
        const auto chordCtrlM = parseKeyChord(QStringLiteral("Ctrl+M"));
        QVERIFY(chordM.has_value() && chordCtrlM.has_value());

        QCOMPARE(rev.value(*chordM),     ActionId::Mute);
        QCOMPARE(rev.value(*chordCtrlM), ActionId::Mute);
    }

    // ------------------------------------------------------------------
    // 4. 衝突後勝ち（同一 KeyChord を2アクションに割当 → 後者が勝つ）
    // ------------------------------------------------------------------
    void testConflictLastWins() {
        QMap<ActionId, QStringList> keyMap;
        // QMap は key 順にソートされるため挿入順序ではなく id 順で処理される。
        // テスト目的: 同じキーに対して後に挿入した id が残ること。
        keyMap[ActionId::Bump] = {QStringLiteral("Alt+B")};
        keyMap[ActionId::Stop] = {QStringLiteral("Alt+B")};  // 意図的衝突

        // buildReverseMap は map のイテレーション順（QMap = key 昇順）で登録する。
        // Bump < Stop（enum 値で Bump=0, Stop=1）なので Stop が後勝ち。
        const auto rev = buildReverseMap(keyMap);

        const auto chord = parseKeyChord(QStringLiteral("Alt+B"));
        QVERIFY(chord.has_value());
        // 衝突では後勝ち（Stop の enum 値 > Bump）
        QCOMPARE(rev.value(*chord), ActionId::Stop);
    }

    // ------------------------------------------------------------------
    // 5. QKeyEvent → KeyChord 正規化
    // ------------------------------------------------------------------
    void testKeyChordFromEvent_normal() {
        // Ctrl+R のイベントを手動構築
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_R, Qt::ControlModifier,
                      QStringLiteral("r"));
        const KeyChord c = keyChordFromEvent(&ev);
        QCOMPARE(c.key,  (int)Qt::Key_R);
        QCOMPARE(c.mods, Qt::KeyboardModifiers{Qt::ControlModifier});
    }

    void testKeyChordFromEvent_modifierOnly() {
        // 修飾キー単体は {0, NoModifier} を返すこと
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Shift, Qt::ShiftModifier,
                      QStringLiteral(""));
        const KeyChord c = keyChordFromEvent(&ev);
        QCOMPARE(c.key, 0);
    }

    void testKeyChordFromEvent_numpad() {
        // テンキー 5（KeypadModifier 付き）
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_5, Qt::KeypadModifier,
                      QStringLiteral("5"));
        const KeyChord c = keyChordFromEvent(&ev);
        QCOMPARE(c.key,  (int)Qt::Key_5);
        QVERIFY(c.mods & Qt::KeypadModifier);

        // parseKeyChord("Num5") と一致すること
        const auto numChord = parseKeyChord(QStringLiteral("Num5"));
        QVERIFY(numChord.has_value());
        QCOMPARE(c, *numChord);
    }

    // ------------------------------------------------------------------
    // 6. ActionRegistry: on / trigger / dispatch
    // ------------------------------------------------------------------
    void testRegistryTrigger() {
        ActionRegistry reg;
        reg.setKeyMap(defaultKeyMap());

        bool fired = false;
        reg.on(ActionId::FullScreen, [&fired]{ fired = true; });

        // trigger() で直接実行
        QVERIFY(reg.trigger(ActionId::FullScreen));
        QVERIFY(fired);

        // 未登録アクションは false
        QVERIFY(!reg.trigger(ActionId::Minimize));
    }

    void testRegistryDispatch() {
        ActionRegistry reg;
        reg.setKeyMap(defaultKeyMap());

        bool fired = false;
        reg.on(ActionId::FullScreen, [&fired]{ fired = true; });

        // F キーは FullScreen にマップされているはず
        const auto fChord = parseKeyChord(QStringLiteral("F"));
        QVERIFY(fChord.has_value());
        QVERIFY(reg.dispatch(*fChord));
        QVERIFY(fired);
    }

    void testRegistryDispatchUnregisteredHandler() {
        ActionRegistry reg;
        // キーマップ設定（Mute = M）
        QMap<ActionId, QStringList> km;
        km[ActionId::Mute] = {QStringLiteral("M")};
        reg.setKeyMap(km);
        // ハンドラは登録しない

        const auto chord = parseKeyChord(QStringLiteral("M"));
        QVERIFY(chord.has_value());
        // 逆引きヒットするがハンドラ未登録 → false（素通し）
        QVERIFY(!reg.dispatch(*chord));
    }

    void testRegistryDispatchUnknownKey() {
        ActionRegistry reg;
        reg.setKeyMap(defaultKeyMap());
        reg.on(ActionId::FullScreen, []{ });

        // キーマップにない KeyChord は false
        const KeyChord unknown{Qt::Key_F12, Qt::NoModifier};
        QVERIFY(!reg.dispatch(unknown));
    }

    void testRegistryKeysFor() {
        ActionRegistry reg;
        reg.setKeyMap(defaultKeyMap());

        // FullScreen のデフォルトキーは "F"
        const QStringList keys = reg.keysFor(ActionId::FullScreen);
        QVERIFY(keys.contains(QStringLiteral("F")));
    }

    // ------------------------------------------------------------------
    // 7. M5.2: mergeShortcuts / configIdToActionId
    // ------------------------------------------------------------------

    // configIdToActionId: 代表 configId が正しい ActionId に解決されること
    void testConfigIdToActionId() {
        const auto& lookup = configIdToActionId();
        QCOMPARE(lookup.value(QStringLiteral("mute"),        ActionId::Quit), ActionId::Mute);
        QCOMPARE(lookup.value(QStringLiteral("fullscreen"),  ActionId::Quit), ActionId::FullScreen);
        QCOMPARE(lookup.value(QStringLiteral("zoom_preset_1"), ActionId::Quit), ActionId::ZoomPreset1);
    }

    // 差分上書き: mute のキーを変更 → 変更されたアクションが上書き、他は既定のまま
    void testMergeShortcuts_override() {
        QMap<QString, QStringList> user;
        user[QStringLiteral("mute")] = {QStringLiteral("M"), QStringLiteral("Ctrl+M")};

        const auto merged = mergeShortcuts(user);

        // mute は上書きされている
        QCOMPARE(merged.value(ActionId::Mute),
                 (QStringList{QStringLiteral("M"), QStringLiteral("Ctrl+M")}));

        // fullscreen は既定 "F" のまま
        QVERIFY(merged.value(ActionId::FullScreen).contains(QStringLiteral("F")));
    }

    // アンバインド: 空配列を渡すとキー無しになる
    void testMergeShortcuts_unbind() {
        QMap<QString, QStringList> user;
        user[QStringLiteral("fullscreen")] = {};

        const auto merged = mergeShortcuts(user);
        QVERIFY(merged.value(ActionId::FullScreen).isEmpty());

        // 他は既定のまま（例: mute = "M"）
        QVERIFY(merged.value(ActionId::Mute).contains(QStringLiteral("M")));
    }

    // 未知 configId はスキップ（既知分は反映・クラッシュしない）
    void testMergeShortcuts_unknownConfigId() {
        QMap<QString, QStringList> user;
        user[QStringLiteral("no_such_action")] = {QStringLiteral("G")};
        user[QStringLiteral("mute")]           = {QStringLiteral("N")};

        const auto merged = mergeShortcuts(user);
        // mute は上書きされている
        QCOMPARE(merged.value(ActionId::Mute),
                 QStringList{QStringLiteral("N")});
        // 未知 configId のキーは誰にも割り当たっていない
        // （ActionId のどれかに "G" が割り当たらないことは exhaustive に確認困難だが、
        //   少なくとも FullScreen は既定 "F" のまま）
        QVERIFY(merged.value(ActionId::FullScreen).contains(QStringLiteral("F")));
    }

    // 空入力 = 恒等（mergeShortcuts({}) == defaultKeyMap()）
    void testMergeShortcuts_empty() {
        const auto merged   = mergeShortcuts({});
        const auto defaults = defaultKeyMap();
        QCOMPARE(merged, defaults);
    }

    // ------------------------------------------------------------------
    // 8. M5.3: 音量クランプ / ステップ ＋ MuteState 状態遷移
    // ------------------------------------------------------------------

    // applyVolumeStep: 通常域・クランプ上限・クランプ下限
    void testApplyVolumeStep() {
        // 通常域: 50 + 5 = 55
        QCOMPARE(applyVolumeStep(50,  5),   55);
        // 通常域: 50 - 5 = 45
        QCOMPARE(applyVolumeStep(50, -5),   45);
        // 上限クランプ: 98 + 5 = 100（切り捨てなし）
        QCOMPARE(applyVolumeStep(98,  5),  100);
        // 下限クランプ: 2 - 5 = 0
        QCOMPARE(applyVolumeStep(2,  -5),    0);
        // 既に上限: 100 + 1 = 100
        QCOMPARE(applyVolumeStep(100, 1),  100);
        // 既に下限: 0 - 1 = 0
        QCOMPARE(applyVolumeStep(0,  -1),    0);
    }

    // MuteState: toggleUser で effective が反転すること
    void testMuteState_toggleUser() {
        MuteState m;
        QVERIFY(!m.effective());   // 初期: ミュートなし
        m.toggleUser();
        QVERIFY(m.effective());    // ON
        QVERIFY(m.userMute());
        m.toggleUser();
        QVERIFY(!m.effective());   // OFF に戻る
    }

    // MuteState: 手動ミュートなし → 最小化で自動ミュート → 復帰で解除
    void testMuteState_autoMuteLifecycle() {
        MuteState m;
        // minimize_mute=true で最小化 → 自動ミュート
        m.onMinimize(/*minimizeMuteEnabled=*/true);
        QVERIFY(m.effective());
        QVERIFY(!m.userMute());    // ユーザーミュートではない
        // 復帰 → 解除
        m.onRestore();
        QVERIFY(!m.effective());
    }

    // MuteState: 手動ミュート中の最小化→復帰でユーザーミュートが保持されること
    void testMuteState_manualMuteSurvivesRestore() {
        MuteState m;
        m.toggleUser();            // ユーザーが手動ミュート
        QVERIFY(m.userMute());

        m.onMinimize(/*minimizeMuteEnabled=*/true);
        QVERIFY(m.effective());    // まだミュート（変わらず）

        m.onRestore();             // 復帰: autoMute だけ解除、userMute 保持
        QVERIFY(m.userMute());     // ユーザーミュートは残る
        QVERIFY(m.effective());    // effective もまだ true
    }

    // MuteState: minimize_mute=false では最小化しても自動ミュートしない
    void testMuteState_minimizeMuteDisabled() {
        MuteState m;
        m.onMinimize(/*minimizeMuteEnabled=*/false);
        QVERIFY(!m.effective());   // ミュートされない
    }

    // ------------------------------------------------------------------
    // 9. M5.4: classifyMediaSource 再生ソース種別判定
    // ------------------------------------------------------------------

    // PeerCast /pls/ URL
    void testClassifyMediaSource_pls() {
        // 正規の 32 文字 hex ID
        const QString pls = QStringLiteral("http://localhost:7144/pls/ABCDEF0123456789ABCDEF0123456789");
        QCOMPARE(classifyMediaSource(pls), MediaSourceKind::PlsUrl);
        // 大文字小文字を混在させても pls 判定
        QCOMPARE(classifyMediaSource(QStringLiteral("HTTP://HOST:7144/pls/abcdef0123456789ABCDEF0123456789")),
                 MediaSourceKind::PlsUrl);
    }

    // 一般 http(s) URL（pls でないもの）
    void testClassifyMediaSource_http() {
        QCOMPARE(classifyMediaSource(QStringLiteral("http://example.com/stream")),
                 MediaSourceKind::HttpUrl);
        QCOMPARE(classifyMediaSource(QStringLiteral("https://example.com/video.mp4")),
                 MediaSourceKind::HttpUrl);
        // /pls/ の ID が 32 文字未満なら HttpUrl にフォールバック
        QCOMPARE(classifyMediaSource(QStringLiteral("http://host:7144/pls/TOOSHORT")),
                 MediaSourceKind::HttpUrl);
    }

    // ローカルファイルパス
    void testClassifyMediaSource_localPath() {
        QCOMPARE(classifyMediaSource(QStringLiteral("C:\\video.mp4")),   MediaSourceKind::LocalPath);
        QCOMPARE(classifyMediaSource(QStringLiteral("C:/video.mp4")),    MediaSourceKind::LocalPath);
        QCOMPARE(classifyMediaSource(QStringLiteral("D:\\dir\\sub.mkv")), MediaSourceKind::LocalPath);
        QCOMPARE(classifyMediaSource(QStringLiteral("\\\\server\\share\\file.ts")), MediaSourceKind::LocalPath);
        QCOMPARE(classifyMediaSource(QStringLiteral("file:///C:/movie.mp4")), MediaSourceKind::LocalPath);
    }

    // 認識できない文字列は Invalid
    void testClassifyMediaSource_invalid() {
        QCOMPARE(classifyMediaSource(QStringLiteral("")),          MediaSourceKind::Invalid);
        QCOMPARE(classifyMediaSource(QStringLiteral("   ")),       MediaSourceKind::Invalid);
        QCOMPARE(classifyMediaSource(QStringLiteral("foobar")),    MediaSourceKind::Invalid);
        QCOMPARE(classifyMediaSource(QStringLiteral("movie.mp4")), MediaSourceKind::Invalid); // 相対パス
        QCOMPARE(classifyMediaSource(QStringLiteral("ftp://example.com/file.mp4")), MediaSourceKind::Invalid);
    }

    // 前後空白を trim して正しく判定する
    void testClassifyMediaSource_trim() {
        const QString pls = QStringLiteral("  http://localhost:7144/pls/ABCDEF0123456789ABCDEF0123456789  ");
        QCOMPARE(classifyMediaSource(pls), MediaSourceKind::PlsUrl);
        QCOMPARE(classifyMediaSource(QStringLiteral("\tC:\\video.mp4\n")), MediaSourceKind::LocalPath);
    }
};

QTEST_MAIN(TestShortcuts)
#include "tst_shortcuts.moc"
