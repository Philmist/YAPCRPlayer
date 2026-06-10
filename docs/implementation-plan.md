# YAPCRPlayer 実装計画

最終更新: 2026-06-08
前提: 設計決定は `docs/architecture-decisions.md` を正本とする（本書はその実装への落とし込み）。

スタック: **C++20 / Qt 6（Widgets, Network, Core5Compat）/ libmpv / toml++**。
プラットフォーム: Windows。ビルド: CMake + aqtinstall(Qt) + libmpv dev パッケージ + windeployqt。

---

## 1. モジュール構成（依存は一方向の DAG）

単一リポジトリ・単一 CMake プロジェクト。各モジュールは静的ライブラリ、最後に `app` が実行ファイル。
**UI 非依存のドメイン層**（player/peercast/bbs/config/net/common）と **UI 層**を分離し、
オーケストレーションは `app` に集約（旧 PCRPlayer のグローバル `gl_` は廃し、明示的な所有関係にする）。

```
common  ── 基盤（文字列/エンコーディング(Core5Compat ラッパ)/ログ/URLParser/小物）。依存なし
  ▲
config  ── TOML 読み書き＋設定モデル（プリセット群/ショートカット/BBS/表示/色補正）。依存: common
net     ── HTTP クライアント（QNetworkAccessManager ラッパ: TLS/Cookie/gzip/タイムアウト）。依存: common, Qt::Network
  ▲
peercast ── URL 解析(/pls/→host/port/id)、制御(bump/stop/viewxml)、/stream/ 解決、ChannelInfo。依存: net, common
bbs      ── 板抽象(2ch互換/したらば・jpnkn)、dat/subject/setting パース、書込(2段階Cookie)、エンコ変換。依存: net, common
player   ── libmpv ラッパ（生成/--wid アタッチ/loadfile/プロパティ/screenshot/コマンド）＋ watchdog
            （停止検知→reload、bump 要求はシグナルで上位へ通知）。依存: common, libmpv ※ peercast/bbs に依存しない
  ▲
ui      ── Qt: MainWindow / VideoHostWidget(--wid 保持) / 下部バー(reply/seek/status) /
            レス・ポップアップ(トップレベル半透明) / 簡易レス一覧ペイン / 各 QDialog(設定/版/ログ)。
            依存: player, peercast, bbs, config, common（表示とビューモデル）
  ▲
app     ── エントリポイント、CLI 解析(path/name/contact)、SessionController（旧 OpenFile 相当の配線:
            player+peercast+bbs+ui を結線。watchdog の停止検知→必要なら peercast.bump 等）。依存: 全部
```

主要クラスの責務:
- `app::SessionController` — 旧 `OpenFile` のオーケストレーションを担う中心。CLI/設定を受け、
  peercast で /stream/ 解決 → player.load、bbs.init(contact) → 表示更新、watchdog シグナル処理。
- `player::MpvBackend` — `mpv_create`/`mpv_set_option`(wid)/`mpv_command`/プロパティ監視。
  mpv イベントは `mpv_set_wakeup_callback` → Qt のイベントへポスト → `mpv_wait_event(0)` で排出（GUI スレッド集約）。
- `player::Watchdog` — `core-idle`/`demuxer-cache-time`/EOF・エラーを監視し、停止が閾値超で `reloadRequested`/
  `bumpRequested` を emit（閾値は PCRPlayer の経験値を翻訳。`docs/architecture-decisions.md` 参照）。
- `ui::VideoHostWidget` — `setAttribute(WA_NativeWindow)` で得た `winId()` を mpv の `wid` に渡す。
- `ui::ResPopup` — `Qt::Tool|FramelessWindowHint`＋`WA_TranslucentBackground`＋`WA_ShowWithoutActivating`
  （レス欄のフォーカスを奪わない）。`paintEvent` 自前描画、`wheelEvent` で過去レス遡行。

並行性: libmpv は内部スレッド。ネットワークは QNetworkAccessManager の非同期で **手動スレッドは原則不要**
（PCRPlayer の boost::thread 群を Qt イベントループ＋非同期 I/O に置換＝堅牢化）。

---

## 2. ビルド整備

- **CMake ≥ 3.21**。`qt_standard_project_setup()`、`qt_add_executable`、AUTOMOC/AUTOUIC/AUTORCC。
- 依存:
  - Qt6: Core, Gui, Widgets, Network, **Core5Compat**(Shift_JIS/EUC-JP 用 QTextCodec)。CI は **aqtinstall**。
  - **libmpv**: dev パッケージ（`client.h` ほか + import lib）。`FindMpv`/pkg-config か `MPV_DIR` 指定。
    配布は LGPL ビルドの `libmpv-2.dll` 同梱。
  - **toml++**（ヘッダオンリー、FetchContent or submodule）。
  - テスト: Qt Test もしくは GoogleTest/Catch2。
- 配布: `windeployqt` で Qt DLL/プラグイン収集 ＋ `libmpv-2.dll` 同梱。
- **submodule**: mpv / peercaststation は取得済み・正常（path・初期化・HEAD 整合を確認済み）。修正不要。

---

## 3. 開発フェーズ / マイルストーン

- **M0 スケルトン**: CMake+Qt+libmpv リンク、空ウィンドウ、windeployqt、.gitignore 整備。
- **M1 再生 + CLI 互換**: `<path> <name> <contact>` 解析。`--wid` で映像表示。peercast で `/pls/`→`/stream/`
  解決して mpv に load。ウィンドウ可変・タイトル/名前表示。ローカルファイルも「開けば再生」。
- **M2 PeerCast 制御 + watchdog**: bump/stop/viewxml、チャンネル情報表示、停止検知→reload/bump、リレー配慮。
- **M3 BBS**: したらば+jpnkn の取得/パース、簡易レス一覧ペイン、hover レス・ポップアップ、
  書き込み(2段階 Cookie)、HTTPS、文字コード処理。
- **M4 表示/ウィンドウ**: ズーム%＋絶対サイズプリセット(ユーザー定義)、フィット各モード(mpv オプション)、
  バー除外の正確ピクセル(OBS タイル用)、全画面、スナップショット(`screenshot`)。
- **M5 設定/操作**: TOML 永続化、ショートカット(PCRPlayer 既定＋TOML 再マップ、Qt 集約/mpv 内蔵キー無効)、
  最小化およびミュート(別々に実行できること)、ファイル/クリップボード URL を開く。
  **M5.0〜M5.5 に分割済み**（`docs/m5-config-subplan.md`。config モジュール新設→ActionRegistry→TOML リマップ→
  音量/ミュート/最小化→ファイル/URL→終了時保存）。
- **M6 仕上げ**: 安定化、パッケージング、DEFER 項目(ジェスチャ等)の見直し。

差別化価値（ライブ視聴＋実況）が M3 完了時点で立ち上がる順序。

---

## 4. テスト戦略

- **単体テスト**（純ロジックを UI から切り離した恩恵）:
  - bbs: dat/subject/setting パース、エンコーディング往復、書込リクエスト(2段階 Cookie)組み立て。
  - peercast: URL 解析、コマンド URL 生成、viewxml パース、/stream/ 解決正規表現。
  - config: TOML ラウンドトリップ（プリセット/ショートカットの入れ子・配列）。
- **watchdog**: プロパティ値のスタブで擬似的な供給停止を与え reload/bump 発火を検証。
- **結合/手動**: ローカル PeerCastStation 実機で end-to-end（各コンテナ: FLV/TS/MKV、各コーデック: H264/HEVC/AV1/Opus/Vorbis）。

---

## 5. 留意・リスク

- libmpv の Windows 配布物は LGPL 構成を選ぶ（クローズド配布可・LAV/GPL 回避）。
- Core5Compat の Shift_JIS/EUC-JP は実際の板レスポンスで最終確認（したらば=EUC-JP 想定だが要検証）。
- `--wid` 埋め込み時のリサイズ追従・DPI・全画面遷移は Qt のネイティブウィンドウ属性で要調整。
- PeerCastStation は legacy `/admin?cmd=` 互換を維持しているが、将来非互換化に備え制御を `peercast` に隔離
  （必要時 JSON-RPC `/api/1` へ差し替え可能な内部 API にする）。
