# YAPCRPlayer 設計決定記録（ADR / 依存チェーン）

最終更新: 2026-06-11

## 0. 背景と目的

参照用 `./PCRPlayer`（DirectShow + MFC, VC++2015。FLV/H264 と ASF/WMV のみ対応）を
**作り直す**プロジェクト。狙いは、現代の PeerCast(PeerCastStation) が扱う
**MPEG2-TS / MKV(WebM)** コンテナと **HEVC / AV1 / Opus / Vorbis** コーデックを
再生可能にすること。FFmpeg を内蔵する **libmpv** ならこれらをネイティブに demux/decode できる。

評価基準はユーザー明示で「**実装が素直（straightforward）かつ堅牢（robust）**」。実装コストは度外視してよい。

PCRPlayer の資産は「コードそのまま流用」ではなく、**BBS/PeerCast の挙動・エッジケースを
継承する参照仕様書**として扱う（DirectShow/MFC 部は全捨て）。

---

## 1. 決定木（依存チェーン）

各ノードは「問い → 選択 → 理由 → 次への依存」。上から順に依存している。

### 根: 言語/フレームワークを何で決めるか
- **問い**: Rust と C++20、あるいは他のどれを採るか。何が決め手になるか。
- **発見**: コア UX「映像を見ながら**日本語で**掲示板へレスをする」が決め手。
  = **IME（日本語入力）をどう扱うか**が言語選定を一意に決めるレバー。
  （可変ウィンドウ・別ディスプレイ全画面は Rust/C++ で大差なく、選定に効かない）

### Q1. UI 構成モデル
- **選択: ハイブリッド（A）**。映像は専用サーフェス、レス欄/メニュー/設定はネイティブ UI。
- **理由**: IME の表示実装（変換ウィンドウ位置・候補・再変換）は複雑怪奇で自前実装は負債。
  **IME は OS に委譲する**と決定。
- **依存**: 「IME 自前実装しない」が次を縛る → イミディエイトモード GUI(egui/Dear ImGui)は
  IME ブリッジが部分的なので**除外**。これにより Rust の主目的（egui の手軽さ）が消える。

### Q2. ネイティブ UI ツールキット → 言語確定
- **選択: Qt 6 Widgets（C++20）（A）**。
- **理由**:
  - OS 品質の IME を持つ本物のウィジェット群。
  - **libmpv + Qt は実績の宝庫**（mpv 公式サンプル、多数の実在 mpv 系 GUI）。
  - レス欄・コメント一覧・バー・小さな設定画面に使える成熟ウィジェット。
  - **旧来の作法は簡便化済み**: CMake が一級市民（qmake 退役）、moc/uic/rcc は AUTOMOC 等で自動、
    `connect` は関数ポインタ/ラムダ（SIGNAL/SLOT 文字列マクロ不要）、配布は `windeployqt`、
    SDK 取得は `aqtinstall` でスクリプタブル。
  - ライセンス的にも問題なし（本プロジェクトは GPLv3＝下記参照。Qt は GPLv3 で利用可）。
- **言語確定**: **C++20 + Qt 6**。Rust は積極理由（egui）が Q1 で消滅したため不採用。
  ※「D3D11 ネイティブ合成がどうしても必要」化した時のみ libVLC 4.0/別案を再評価。

### 再生ライブラリ選定（並行確定）
- **選択: libmpv 本命**（libVLC / FFmpeg直叩き / Media Foundation と比較）。
- **理由**: ライブ低遅延/キャッシュ制御がプロパティで容易、stream_cb がシンプル、
  再生品質/AV同期が高い、API 一貫、submodule で mpv 採用済み。
  - libVLC の唯一の優位は VLC 4.0 の D3D11 出力コールバックだが、4.0 未リリースで 3.x は劣るため不採用。

### Q3. mpv 映像の埋め込み方式（C オミットを受けて再評価・撤回あり）
- **当初**: Render API（`QOpenGLWidget`+`mpv_render_context`）で映像＋コメントを一枚合成、を検討。
- **撤回 → 選択: `--wid` 子ウィンドウ（mpv が vo=gpu-next で直接提示）**。
- **理由**: 後述 C のオミットで「一枚合成」の必要が消滅。色補正/スケーラ/スナップショットも
  **mpv 標準機能**で足りる。よって最も素直・最堅牢・最小実装の `--wid` を採用。
  - 補足: `--wid`（ネイティブ子窓）上に重なるのが hover ポップアップ 1 種だけなら、
    それを**トップレベル半透明 Qt ウィンドウ**として浮かべれば足り、合成 API は不要。

### Q4. 「簡易的に掲示板を見たい」軽量ビューアのスコープ
- **選択: ポップアップ + 任意表示の簡易一覧ペイン（B）**。
- **内容**:
  - (1) **hover レス・ポップアップ**: 画面上の掲示板タイトルにカーソルを乗せると直近レスを
    カーソル位置に半透明オーバーレイ、ホイールで過去レスを遡る（旧 `CResDlg` 挙動）。
    データは**プレーンテキスト**（HTML 不要、レイアウト描画のみ）。
  - (2) **簡易レス一覧ペイン**: ウィンドウ脇/下のスクロール可能なテキスト一覧（任意表示）。
- **理由**: ユーザーには「レスできること」に加え「**簡易的にレスを見たい**」需要がある
  （映像に対しデフォルト1行のレス欄がレスの証左）。HTML 専ブラの代替を軽量に埋める。

### Q5. 映像の上に浮かべる UI の範囲
- **選択: ポップアップのみ（A）**。操作系はウィンドウ下部バー、設定は独立画面。
- **理由**: フルスクリーンは原則クリーンな映像。設定も映像オーバーレイ不要。
- **依存解決**: これにより Q3 を `--wid`（最小・堅牢）に倒せることが確定。

### Q6. フルスクリーン（別ディスプレイ視聴）のモデル
- **選択: ウィンドウごと全画面（A）＝ PCRPlayer UX に追随**。
- **理由**: 別画面に操作を分離してまでレスする需要はない（そこまでするなら Web ブラウザで書く）。

---

## 2. 確定事項サマリ

- **ライセンス（確定）**: **GPLv3**（参照元 PCRPlayer が GPLv3 のため継承）。よって GPL コンポーネントの
  利用に制約はなく、Qt は GPLv3 で、libmpv/FFmpeg も任意構成（GPL 有効ビルド含む）で利用可。
  「LGPL でクローズド配布」という以前の検討は本方針では不要（過去の選択肢として残すのみ）。
- **言語/UI**: C++20 + Qt 6（ハイブリッド：周辺 UI/レス欄はネイティブ Qt、映像は mpv `--wid`）。
- **再生**: libmpv（`vo=gpu-next`）。HW デコード/色補正/スケーラ/スナップショットは mpv 標準機能。
- **BBS 表示**: hover レス・ポップアップ（トップレベル半透明 Qt ウィジェット）＋任意の簡易テキスト一覧ペイン。
- **CLI 起動互換（厳守）**: `PCRPlayer.exe <path> <name> <contact>`（path=再生URL, name=チャンネル名,
  contact=掲示板URL）。3引数時のみ commandline 扱いで BBS 自動接続。
  CLI→OpenFile→peca/bbs/player のオーケストレーション層は再生エンジン非依存なので流用。
- **v1 スコープ（Q9 確定: A）**: **ライブ PeerCast 視聴＋BBS 実況に集中**。ローカル再生は mpv 標準で
  「開けば再生」する最小限のみ（フォルダ連続再生/リピート/プレイリスト/前後ファイル移動/録画日時の凝った
  表示は v1 では持たない＝他アプリの守備範囲）。ライブは pause/seek 不可、シークは位置表示のみ
  （ローカル時のみ操作可。PCRPlayer の online/offline 挙動を踏襲）。
- **スナップショット（搭載）**: mpv の `screenshot` コマンドで実装（`screenshot video` = OSD/字幕なしの
  素のフレームを元解像度で保存。`screenshot-format`/`-jpeg-quality`/`-directory`/`-template` で設定）。
  PCRPlayer の Snapshot/saveCurrentImage(EVRCustom 依存) は mpv screenshot に置換。保存＋保存フォルダを開くを提供。
- **ウィンドウサイズ/プリセット（Q10 確定）**: サイズは**映像基準**（レス欄等のバーは別扱い・各バーは隠せる）。
  **ズーム% プリセット＋絶対サイズプリセットの両方**を持ち、**ユーザー定義可能**。背景: PeerCast は映像サイズも
  タイル割りも任意なため固定の工場プリセットでは不足し、**RTA レース等で複数配信を1画面に敷き詰めて OBS の
  ウィンドウキャプチャで再配信する需要**に応えるには正確な固定サイズ指定が要る。よって**映像のみを正確な
  ピクセルサイズにする（バー除外）モード**を明示的に持つ。フィット/アスペクトは PCRPlayer の各モードの能力を
  踏襲しつつ **mpv ネイティブオプションへ対応付け**: inscribe=`keepaspect` / none=`no-keepaspect` /
  充填=`panscan` / 明示比=`video-aspect-override` / 等倍=`video-unscaled`。**ピクセルサイズは Qt、
  フレームの収め方は mpv** が制御。ウィンドウスナップは OS 任せ。
- **オミット**: 専ブラ風 HTML 表示（MSHTML/IE 依存一式）、クライアント側の常時テロップ表示
  （現代 PeerCast では配信者が映像に焼き込むのが通例）。
- **BBS 対応（Q8 確定: A）**: 対象は **したらば(jbbs.shitaraba.net) + BBS.JPNKN(bbs.jpnkn.com)**。
  どちらも 2ch 互換系（read.cgi/bbs.cgi/dat 方式）。**表示・書き込み両対応**。5ch は対象外（実況利用者がいない）。
  **HTTPS で送受信**。HTTP スタックは **Qt `QNetworkAccessManager`**（TLS/Cookie/gzip 内蔵）で
  WinHTTP+boost gzip+Mlang を置換。文字コードは jpnkn=Shift_JIS / したらば=EUC-JP を扱うため
  **Qt6 Core5Compat(QTextCodec)** 等の旧コーデック対応が必要（実装時に各板の実エンコーディングを最終確認）。
  書き込みは PCRPlayer 同様の **2 段階 Cookie フロー**（確認 Cookie 往復 + name/mail/message）を移植。
  **投稿の name/mail（M5 確定）**: PCRPlayer は `BBSDlg.cpp:888` で `post(L"", sage?L"sage":L"", text)` ＝
  **name は常に空（名無し）・mail は sage トグルのみ**（自由入力 mail 欄を持たない。PeerCast 実況文化では
  名無し投稿が通例で、sage か否かだけが重要。任意 mail が要るなら通常の Web ブラウザを使う）。よって YAPCRPlayer も
  **name は設定項目として残すが既定空、mail は廃止して `sage` ブール（トグル）に置換**する。書き込み確定キーは
  PCRPlayer の Shift/Ctrl/Alt+Enter（`EDIT_SHORTCUT_*`）を踏襲。
- **ショートカット（Q12 確定）**: コア固定の妥当なデフォルト＋**TOML で再マップ可**（GUI 編集画面は後日）。
  入力は **Qt が一手に集約**し、mpv 内蔵キー処理は無効化（`input-default-bindings=no`/`input-vo-keyboard=no`）して
  `--wid` 子窓によるキー横取りを防ぐ。**デフォルトのキー割当は PCRPlayer を踏襲**
  （実装時に `Shortcut.cpp` / `OperationShortcutDlg` の既定値を抽出して移植）。
  **M5 確定（詳細は `docs/m5-config-subplan.md`）**: 中央集権 `ActionRegistry`（`enum class ActionId` ＋
  `id↔keys` テーブル）で間接化し、TOML リマップ・メニュー生成を単一テーブルから派生。キー表現は `QKeySequence`
  互換文字列（テンキーは `Num` プレフィックス規約）。**1アクションに複数キー可・1キーは1アクションまで・衝突は
  後勝ち＋警告**（PCRPlayer の厳密 1:1 から意図的に緩和）。ディスパッチは中央 `keyPressEvent` 集約で、**レス欄
  フォーカス／IME 変換中はプレイヤーショートカット無効・Tab でフォーカス往復**（`IDM_WINDOW_EDIT` 踏襲）。
- **設定永続化（Q11 確定）**: **TOML** ファイルを `%APPDATA%\YAPCRPlayer\` に保存（任意でポータブル配置＝
  実行ファイル隣も可）。入れ子/配列構造（プリセット群・ショートカット等）に素直で手編集・共有可
  （タイル配信のサイズプリセット使い回しにも有用）。実装は toml++ 等。将来 JSON へ変える積極的理由が
  出たら再検討。PCRPlayer のバイナリ Serialize は移植しない。
  **M5 確定（詳細は `docs/m5-config-subplan.md`）**: 単一 `config.toml` 内で**設定/プリセット（`[general]`
  `[restore]` `[shortcuts]` `[display]` `[snapshot]` `[bbs]` `[playback]`）とランタイム状態（`[state]`）を
  セクション分離**。配置は実行ファイル隣接優先 → `%APPDATA%\YAPCRPlayer\`。起動時1回ロードして DI、**保存は終了時
  一括のみ**（即時保存しない。PCRPlayer も `sl_.save`(`MainDlg.cpp:703`) は終了処理のみで、sage トグルすらメモリ
  保持→終了時保存だった）。パース失敗時はデフォルト起動＋ログ、不明キー破棄。**終了時保存トグル群**
  （`position/size/aspect/volume/mute`、既定 `aspect` のみ off）は PCRPlayer `EndConfig` を踏襲。**設定 GUI は
  DEFER**（TOML 手編集のみ。代わりに「設定フォルダを開く」「再読み込み」導線を用意）。
- **小機能仕分け（Q13 確定）**: KEEP=チャンネル情報表示・最小化時ミュート・ファイル/クリップボード URL を開く(軽量)。
  DEFER=マウスジェスチャ・最小化時一時停止(ローカルのみ)・音量バランス(`af=pan` 要・実況用途で重要度低)。
  DROP=プロセス優先度・お気に入り/チャンネル一覧/YP 閲覧・HTML 依存の BBS 抽出。**チャンネル探索/YP/お気に入りは
  YP ブラウザの責務**で YAPCRPlayer は持たない（PCRPlayer は PeCaRecorder という YP ブラウザの一部として
  配布されていた経緯）。
  - **最小化とミュートの独立（M5 確定）**: `Minimize`(最小化のみ)・`Mute`(mpv `mute` プロパティ。**音量0とは区別**)・
    `MinimizeMute`(設定トグル: 最小化で自動ミュート、復帰で自動解除)は互いに独立。自動ミュートで入った時だけ
    復帰解除する（手動ミュート中の最小化→復帰で勝手に解除しない）。
  - **ファイル/クリップボードを開く（M5 確定）**: 手動オープン（ダイアログ/クリップボード）は**パスのみ**を受け、
    BBS 自動接続はしない（pls URL なら channel info の取得・表示はする）。PCRPlayer 踏襲（`MainDlgSub.cpp:114`
    `OpenFile` は `bbs.execute` を `if(commandline)` でガードし、手動オープンでは BBS を繋がない）。
    クリップボード判定は pls/http/ローカルパスの3種。`OpenContactInBrowser` は `QDesktopServices` で軽量に。
    専ブラ起動(`IDM_BBS_BROWSER`)は DROP。
  - **【M6 拡張・意図的乖離】pls URL オープン時の BBS 自動接続**: pls URL を開いた際に PeerCast viewxml の
    `name`/`url`(コンタクト) を自動取得し、チャンネル名と掲示板を自動設定して BBS 自動接続する。`ChannelInfo` は
    既に `name`/`url`/`comment` をパース済みなので技術的に可能。**PCRPlayer は CLI 3引数起動時のみ自動接続**して
    いたため、これは明確な UX 改善であり PCRPlayer 踏襲の枠を超える拡張。実装は M6。
- **PeerCast 制御（プレイヤー側から実行・確定）**: 起動 URL `http://<host>:<port>/pls/<32桁ID>?tip=...` の
  **先頭 `host:port` がローカル PeerCast エンドポイント**。legacy admin HTTP で叩く（PeerCastStation も互換維持）。
  PCRPlayer `PeerCast.cpp` を参照仕様に移植:
  - bump(再接続): `GET http://host:port/admin?cmd=bump&id=<ID>`
  - stop(切断): `GET http://host:port/admin?cmd=stop&id=<ID>`
  - チャンネル情報: `GET http://host:port/admin?cmd=viewxml` → `peercast.channels_relayed.channel` を XML パース
    （name/bitrate/type/genre/desc/uptime/comment/age/bcflags、relay{listeners/relays/hosts/status/firewalled}、track{...}）
  - **stream URL 解決**: 起動 URL は `/pls/` プレイリスト。GET して本体 `scheme://host/stream/<32hex>(.ext)?` を
    正規表現抽出し、その `/stream/` URL を mpv に渡す（`connect()`/`analyse()` を移植）。
  - **リレー配慮**: stop/bump 前に viewxml で `relays==0` を確認するオプション（自分から配信を受けている人がいる時は抑止）。
  - bump はレート制限のため最終実行時刻を保持（連続 bump 抑止）。
- **資産活用**: DirectShow/MFC は全捨て。BBS 取得・パース(BaseBBS/BBSManager)、
  PeerCast 制御(PeerCast/PeerCastManager)は PCRPlayer を参照仕様として C++ 移植。
- **ストリーム供給と障害復帰（Q7 確定: A）**: 直接 URL を mpv に渡し、**mpv 標準キャッシュ
  ＋ lavf 自動再接続**に復帰を委ね、アプリ側は **薄い watchdog**（長時間 core-idle/EOF/エラー →
  `loadfile` 再読込、必要なら PeerCast bump）だけ持つ。PCRPlayer の AsyncBufferingLoader/
  MemSourceFilter 相当は**作らない**（DirectShow の脆弱性への対症療法だったため不要）。
  mpv キャッシュは `demuxer-max-bytes`(前方,既定~150MiB)/`demuxer-max-back-bytes`(後方,既定~50MiB)
  で**有界**＝配信受信で青天井に膨らまない。実況用に**控えめな有界値**（前方数十MiB/後方数MiB）を既定にする。
  PCRPlayer の経験的再構築閾値は mpv プロパティ監視（`demuxer-cache-time` が伸びない＝供給停止 等）に翻訳。
  PeerCast の bump/relay/reconnect は制御プレーンで供給方式と直交、いずれにせよ移植。
  必要が判明した箇所のみ後から `pcr://`(`mpv_stream_cb_add_ro`) を部分導入（C）。

---

## 3. 未決事項（次にグリルで詰める）

（主要な設計決定はすべて解決済み。残るは実装計画レベルの検討：プロジェクト/モジュール構成、
ビルド整備(CMake+aqtinstall+libmpv リンク+windeployqt)、開発フェーズ/マイルストーン、テスト戦略。）
