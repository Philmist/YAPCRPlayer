# M5（設定/操作）サブ分割計画

最終更新: 2026-06-11（初版）
親文書: `docs/implementation-plan.md` の「M5 設定/操作」、`docs/architecture-decisions.md` の
設定永続化(Q11)・ショートカット(Q12)・小機能仕分け(Q13)。

## 目的

M5 を**単独完結する小マイルストーン（M5.x）に分割**し、各セッションのコンテキスト肥大を防ぐ。
M3/M4 と同じ運用——各 M5.x は「移植元の `file:function`」をピンポイントで持つので、実装セッションは
該当箇所だけ読めばよい。**PCRPlayer の設定/操作系（MFC ダイアログ・Win32 入力・バイナリ Serialize）は
コード写経しない**こと。**挙動・既定値・キー割当だけを参照仕様として引き継ぐ**。

M5 のスコープ（親文書より）:
- **TOML 永続化**（`%APPDATA%\YAPCRPlayer\` ／ポータブル配置）。
- **ショートカット**（PCRPlayer 既定 ＋ TOML 再マップ、Qt 集約／mpv 内蔵キー無効）。
- **最小化およびミュート**（別々に実行できること）。
- **ファイル/クリップボード URL を開く**（軽量）。

---

## 横断アーキテクチャ決定（全 M5.x で遵守。各セッションで再発見・再決定しない）

1. **中央集権 `ActionRegistry` がすべての土台**。旧 `gl_` グローバルは廃止済み（ADR）。
   `enum class ActionId`（v1 スコープ分）＋ `id → {表示名, デフォルトキー, ハンドラ}` のテーブルを `app` に置く。
   既存の散在 `QAction`（bump/stop/reload/fullscreen/snapshot/表示モード…）を**このレジストリ経由に統一**する。
   移植元の意味論は `utl::ShortcutManager`(`Shortcut.cpp:430`) ＝ `id↔(key,mod)` の単一テーブル。
   ゲームのキーバインド同様、中間レジストリで間接化することで TOML リマップ・メニュー生成・（将来の）GUI 編集を
   単一テーブルから派生させる。

2. **バインド多重度は「1アクションに複数キー可、1キーは1アクションまで」**。
   - TOML 表現は `keys = [...]` の**配列**（PCRPlayer の単一 `key` を配列に拡張）。未割当は空配列 `[]`。
   - 逆引き `(key,mod) → ActionId` のマップを引いてディスパッチ。表示用は `ActionId → keys[]`。
   - **衝突（同じ (key,mod) が複数アクション）はロード時に後勝ちで上書き＋警告ログ**。アプリは落とさない。
   - PCRPlayer は厳密 1:1（割当時 `reset(key,mod)` で他をクリア `Shortcut.cpp:645`）だったが、複数キー別名割当を
     許す方向に**意図的に緩める**（実況時の利便）。同時2動作の需要（OBS 等）はプレイヤーには無いと判断。

3. **キー表現は `QKeySequence` 互換文字列**（例 `"Ctrl+1"`, `"Alt+B"`, `"Space"`, `"Up"`, `"Shift+Up"`）。
   `QKeySequence::fromString`/`toString` で双方向変換。**テンキーは独自 `Num` プレフィックス規約**
   （`"Num5"`, `"Num+"`, `"Num-"` …）を1つ設け、パーサで `Qt::KeypadModifier` に変換する薄いラッパを噛ませる。
   数値 VK コード（PCRPlayer の `key=DWORD`）は TOML にする利点（手編集・共有）を捨てるため**採らない**。

4. **入力ディスパッチは中央 `keyPressEvent` 集約**（現状の延長）。`QShortcut`/`QAction::setShortcut` を多数並べない。
   - **最重要: レス入力欄にフォーカスがある／IME 変換中はプレイヤーショートカットを発火させない**
     （日本語入力中の `B`/`Space` を奪わない）。判定を中央1箇所に置く。
   - **Tab でレス欄とプレイヤーのフォーカスを往復**（PCRPlayer `IDM_WINDOW_EDIT` 挙動を踏襲）。レス欄フォーカス中は
     文字入力優先。Qt 流（毎回 Esc で戻す等）は実況時に操作過多なので採らない。
   - mpv 内蔵キーは M4.0 で無効化済み（`input-default-bindings=no`/`input-vo-keyboard=no`）。

5. **設定とランタイム状態を単一 `config.toml` 内でセクション分離**。
   - 共有したい設定/プリセット: `[general]` `[restore]` `[shortcuts]` `[display]` `[snapshot]` `[bbs]`。
   - 前回終了時の状態: `[state]`（`window_*`・`volume`・`mute`・`sage`）。共有時は手で消せばよい。
   - **保存は終了時一括のみ**（`closeEvent` で `[state]` を現在値に更新して書き出し）。即時保存はしない。
     PCRPlayer も `sl_.save`(`MainDlg.cpp:703`) は終了処理パスのみで、sage トグル(`MainDlgMenu.cpp:1688`)すら
     メモリ保持→終了時保存だったと確認済み。クラッシュ耐性は v1 では過剰。
   - **設定 GUI は DEFER**（ADR Q12「GUI 編集画面は後日」）。変更は TOML 手編集のみ。代わりに「設定フォルダを開く」
     「設定を再読み込み」の軽い導線だけ用意（TOML 直開きはしない＝フォルダを開く）。

6. **config 配置**: 起動時に**実行ファイル隣接 `config.toml` が存在すればポータブルモード**でそれを使い、無ければ
   `%APPDATA%\YAPCRPlayer\config.toml`（初回起動でディレクトリ作成）。**起動時1回ロードして `Config` を DI**
   （MainWindow 等へ参照渡し）。パース失敗時は**ログを出して全項目デフォルトで起動**（落とさない）。`config_version`
   を持ち、不明キーは保持せず破棄。toml++（ヘッダオンリー）を FetchContent or submodule で導入。

7. **終了時保存トグル群（PCRPlayer `EndConfig` 踏襲）**: `[restore]` に `position/size/aspect/volume/mute` の
   個別 bool。既定は `position=size=volume=mute=true`, `aspect=false`（移植元 `SerializeDetail.h:72-88`）。
   起動時に true のものだけ `[state]` から適用する。

8. **テスト作法**: 既存 `tests/tst_*`（Qt Test）に倣う。**純ロジックは UI 非依存関数に切り出して単体テスト**:
   TOML ラウンドトリップ（プリセット/ショートカット配列の入れ子）、キー文字列⇄`QKeySequence`＋Num 規約変換、
   衝突解決（後勝ち）、`(key,mod)→ActionId` 逆引き、クリップボード URL/パス判定。mpv 実呼び出し・ウィンドウ状態・
   ファイル I/O・全画面遷移は**手動 E2E**（M3/M4 と同じ線引き）。

---

## v1 アクション集合（ActionId）

KEEP（v1 実装）。**固定枠**＝ズーム10/サイズ10/アスペクト5 はスロット自体を ActionId 化（中身は config 可変）。
移植元のデフォルトキーは `Shortcut.cpp:430-625`。

| カテゴリ | ActionId | 既定キー |
|---|---|---|
| 再生制御 | Bump / Stop / Rebuild / Terminate / Pause | Alt+B / Alt+Z / Ctrl+R / Ctrl+S / Space |
| 音量 | VolumeUp/Down（通常・小・大の6種） / Mute | ↑↓ ＋Shift/Ctrl / M |
| 表示トグル | Topmost / ToggleTitle / ToggleStatus / ToggleSeek / ToggleBbs / ToggleFrame | T / X / B / V / C / Z |
| ズーム枠×10 | ZoomPreset1..10 | Ctrl+1..0 |
| サイズ枠×10 | SizePreset1..10 | Shift+1..0 |
| アスペクト | AspectDefault / AspectNone / AspectPreset1..5 | Alt+1..7 |
| 全画面/最大化 | FullScreen / Maximize | F（暫定直書き）/ — |
| BBS | ThreadReload / ThreadRefresh / ThreadReset / SagePost / スレッドスクロール次・前 | — |
| シーク（ローカル限定） | SeekForward/Back（通常・小・大の6種） | →← ＋Shift/Ctrl ※online 時 no-op |
| スナップショット | SnapshotSave / SnapshotFolder | P / O |
| ファイル/URL | OpenFileDialog / OpenFromClipboard / CopyPathToClipboard / CopyContactToClipboard / OpenContactInBrowser | — / Ctrl+V / Ctrl+C / — / — |
| 最小化 | Minimize / MinimizeMute（設定トグル） | — |
| チャンネル情報 | UpdateChannelInfo / 情報表示トグル群 | — |
| その他 | OpenConfigFolder / ReloadConfig / Log / Version / Quit / QuitStop | — / — / L / — / — / Alt+X |

**DEFER**: マウスジェスチャ（`gesture` フィールドごと持たない）、最小化時一時停止（`IDM_MINIMIZE_PAUSE`）、
音量バランス（`IDM_BALANCE_*`、mpv 直接プロパティ無く `af=pan` 要）。
**DROP**: プロセス優先度6種、オフライン前後ファイル/再生モード、専ブラ起動・専ブラ表示位置、お気に入り/YP 系。

---

## TOML スキーマ（確定）

```toml
[general]
config_version = 1
quit_stop = false          # 終了時にチャンネル切断（PCRPlayer NetworkConfig.stop）

[restore]                  # PCRPlayer EndConfig 踏襲（SerializeDetail.h:72-88）
position = true
size     = true
aspect   = false
volume   = true
mute     = true

[shortcuts]                # ActionId = [keys]（横断決定2の配列形式）
zoom_preset_1 = ["Ctrl+1"]
mute          = ["M"]
# ... 既定は Shortcut.cpp:430-625 を踏襲

[display]
fit_mode       = "inscribe"              # inscribe/none/panscan/aspect/unscaled
zoom_presets   = [25,50,75,100,125,150,200, ...]   # 10枠
size_presets   = [[640,360],[960,540], ...]        # 10枠 w×h
aspect_presets = [[16,9],[4,3],[5,4],[235,100],[185,100]]  # 5枠
start_zoom_100 = false

[snapshot]
directory    = ""          # 空=既定 Pictures/YAPCRPlayer
format       = "png"
jpeg_quality = 90

[bbs]
name            = ""             # 既定 名無し。通常触らない（PeerCast 文化）
post_submit_key = "shift+enter"  # PCRPlayer EDIT_SHORTCUT（Shift/Ctrl/Alt+Enter）
popup_delay_ms  = 50
popup_position  = "center"       # center/left/right/both
res_order_reverse = false

[playback]
volume_step      = 5
volume_step_low  = 1
volume_step_high = 10
minimize_mute    = false   # ON: 最小化で自動ミュート、復帰で自動解除（KEEP）

[state]                     # 前回終了時の状態（[restore] トグルで適用判定）
window_x = 0
window_y = 0
window_w = 960
window_h = 540
volume   = 100
mute     = false
sage     = false           # mail 欄の代替（sage トグル・PCRPlayer dialog.bbs.sage 相当）
```

**BBS name/mail の扱い（PCRPlayer 調査済み）**: `BBSDlg.cpp:888` は `post(L"", sage?L"sage":L"", text)` で、
**name は常に空（名無し）・mail は sage トグルのみ**。自由入力 mail は廃止し `sage` ブールに置換。name は設定項目
として残すが既定空。

---

## サブマイルストーン一覧

実装順は依存順（config 基盤 → アクション → リマップ → 操作 → 保存/導線）。各 M5.x は単独でビルド+テスト緑で閉じる。

### M5.0 — config モジュール新設（基盤）
- **成果物**:
  - 新規 `config` 静的ライブラリ（`src/config/`、依存: common）。toml++ 導入（FetchContent or submodule）。
  - `config::Config` モデル（上記スキーマに対応する構造体群）＋ `load(path)`/`save(path)`。
  - **配置解決**（横断決定6）: 実行ファイル隣接優先 → `%APPDATA%\YAPCRPlayer\`。初回ディレクトリ作成。
  - **破損耐性**: パース失敗→デフォルト起動＋ログ。`config_version` 読み、不明キー破棄。
  - `main.cpp` で QApplication 構築直後にロードし `Config` を MainWindow へ DI。
  - **PCRPlayer のバイナリ Serialize は移植しない**（`Serialize.cpp` は参照仕様のみ）。
- **テスト**: TOML ラウンドトリップ（プリセット配列・ショートカット配列・ネスト）、破損入力→デフォルト、
  配置解決ロジック（純関数に切り出し）。
- **完了基準**: 既定 config を書き出し→読み戻して一致。隣接/APPDATA の切替が効く（手動）。単体緑。

### M5.1 — ActionRegistry ＋ 既存 QAction 統合
- **成果物**:
  - `enum class ActionId`（上記表の v1 集合）。`app` に `ActionRegistry`:
    `id → {表示名, デフォルトキー(複数), ハンドラ}` と逆引き `(key,mod)→ActionId`。
  - **中央 `keyPressEvent` ディスパッチ**（横断決定4）: 正規化キー → 逆引き → ハンドラ実行。
    **レス欄フォーカス／IME 変換中は素通し**。**Tab でフォーカス往復**（`IDM_WINDOW_EDIT` 踏襲）。
  - 既存の散在 `QAction`（bump/stop/reload/fullscreen/snapshot/表示モード群）を**レジストリ経由に移行**。
    現状の `MainWindow::keyPressEvent`（F/Esc/S 直書き `main_window.cpp:504`）を統合。
  - この段階の割当は**ハードコード既定**（TOML 読み込みは M5.2）。移植元 `Shortcut.cpp:430-625`。
- **テスト**: `(key,mod)→ActionId` 逆引き、複数キー別名、衝突後勝ち、Num 規約⇄`QKeySequence` 変換（純関数）。
- **完了基準**: 既存機能（全画面/スナップショット/bump 等）がレジストリ経由で動く（手動）。逆引き単体緑。

### M5.2 — ショートカット TOML リマップ ＋ 再読み込み導線
- **成果物**:
  - `[shortcuts]` を読み、デフォルト表に**差分上書き**してレジストリのキー表を再構築。衝突は後勝ち＋警告ログ。
  - キー文字列パーサ/シリアライザ（Num プレフィックス規約・横断決定3）を `config` または `common` に。
  - **ReloadConfig アクション**: TOML 再読み込み →**設定/プリセット/ショートカットのみ再適用**。`[state]`
    （ウィンドウ位置等）は触らない（編集中にウィンドウが飛ぶのを防ぐ）。
  - **OpenConfigFolder アクション**: `QDesktopServices` で config フォルダを開く（TOML 直開きはしない）。
- **テスト**: 差分上書き（デフォルト＋ユーザー差分→最終表）、衝突解決、ラウンドトリップに `[shortcuts]` を含める。
- **完了基準**: TOML でキーを変え、ReloadConfig で再起動なしに反映（手動）。フォルダが開く（手動）。

### M5.3 — 音量 / ミュート / 最小化
- **成果物**:
  - `MpvBackend` 経由で `volume`(0-100)/`mute` プロパティ操作（M4.0 のプロパティ API を使用）。
  - **音量6種**: 通常/小/大の上下を `[playback].volume_step{,_low,_high}` で（既定 5/1/10）。↑↓ ＋Shift/Ctrl。
  - **Mute**（M）: `mute` プロパティトグル。**音量0とは区別**（PCRPlayer の操作・表示と同じ）。
  - **Minimize / MinimizeMute の独立**（横断決定・ADR）:
    - `Minimize`: 最小化のみ（ミュートしない）。
    - `MinimizeMute`（`[playback].minimize_mute`）: ON のとき**最小化で自動ミュート・復帰で自動解除**。
    - **自動ミュートで入った時だけ復帰解除**（手動ミュート中の最小化→復帰で勝手に解除しない）。
  - 起動時、`[restore].volume`/`mute` が true なら `[state]` を mpv に適用。
  - **バランスは DEFER**（`af=pan` 要・実況用途で重要度低）。移植元 `IDM_BALANCE_*` は実装しない。
- **テスト**: 自動/手動ミュート状態遷移ロジック（純ステートマシンに切り出し）、ステップ計算。
- **完了基準**: M でミュート、↑↓ で音量、minimize_mute ON/OFF で挙動が変わる（手動）。
- **移植元**: `IDM_MUTE`/`IDM_MINIMIZE_MUTE`/音量系ハンドラ（`MainDlgMenu.cpp`、`Shortcut.cpp:521-553`）。

### M5.4 — ファイル/URL を開く ＋ クリップボード連携
- **成果物**:
  - `OpenFileDialog`/`OpenFromClipboard`: **パスのみ**を受け、`openMedia(path, "", "", commandline=false)`。
    現行セッションを停止→破棄→新規ロード。**BBS は自動接続しない**（pls URL なら channel info の取得・**表示**はする）。
    PCRPlayer 踏襲（`MainDlgSub.cpp:114` の `OpenFile`：`bbs.execute` は `if(commandline)` ガード）。
  - **クリップボード判定**: pls URL / http(s) / ローカルパス の3種を受理。既存 `peercast_url`/`board_url`
    パーサ＋軽い正規表現。妥当でなければステータスバーに通知して no-op。
  - `CopyPathToClipboard`（現 path）/`CopyContactToClipboard`（現 contact）/`OpenContactInBrowser`
    （`QDesktopServices::openUrl`）。専ブラ起動(`IDM_BBS_BROWSER`)は DROP。
- **テスト**: クリップボード文字列の種別判定（pls/http/path/不正）の純関数。セッション差し替えは手動。
- **完了基準**: ダイアログ/クリップボードからローカル/URL を開ける。コピー・ブラウザ起動が効く（手動）。
- **移植元**: `OnOpenfileClipboard`(`MainDlgMenu.cpp:1356`)・`OnClipboardPath`(`:1377`)・`OnClipboardUrl`(`:1397`)。

### M5.5 — 終了時一括保存 ＋ 起動時復元
- **成果物**:
  - `closeEvent` で `[state]`（window_*/volume/mute/sage）を現在値に更新し `config.toml` を書き出し（横断決定5）。
  - 起動時、`[restore]` トグルが true の項目だけ `[state]` から適用（位置/サイズ/アスペクト/音量/ミュート）。
  - `Quit`/`QuitStop`（Alt+X）: `QuitStop` または `[general].quit_stop` のとき**終了前にチャンネル切断**
    （`peercast` の stop。移植元 `MainDlg.cpp:711` の `network.stop` 分岐）。
- **テスト**: `[restore]` 適用の選択ロジック（どの値を state から取るか）の純関数。実保存/復元は手動。
- **完了基準**: 終了→再起動で前回の位置/サイズ/音量/ミュート/sage が復元（restore トグルに従う）（手動）。

### 文書更新（M5 着手前後で実施）
- `docs/architecture-decisions.md`: BBS の name/mail を **name=名無し既定・mail→sage トグル**に確定。
  **OpenFromClipboard の M6 拡張**（pls URL → viewxml の `name`/`url` 自動取得 → BBS 自動接続。PCRPlayer からの
  意図的乖離）を追記。config スキーマの確定を反映。
  → **M6.0 で実施済み**（2026-06-11）: `architecture-decisions.md` 更新完了。
- `docs/implementation-plan.md`: M5 のサブ分割（本書）への参照を追加。

---

## M6 へ送る項目（本マイルストーンでは実装しない・文書として残す）

- **pls URL オープン時の BBS 自動接続**（OpenFromClipboard 拡張）: pls URL を開いた際、PeerCast viewxml の
  `name`/`url`(コンタクト) を取得し、チャンネル名と掲示板を自動設定して BBS 自動接続する。`ChannelInfo` は既に
  `name`/`url`/`comment` をパース済み（`channel_info.h:16,21,23`）なので技術的に可能。**PCRPlayer は CLI 3引数
  起動時のみ自動接続していた**ため、これは明確な UX 改善＝意図的乖離。
- **設定 GUI**（ショートカット/プリセット編集画面）: ADR Q12 で後日。需要が出てから。
- **音量バランス**（`af=pan`）、**マウスジェスチャ**、**最小化時一時停止**: DEFER。
