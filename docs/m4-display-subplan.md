# M4（表示/ウィンドウ）サブ分割計画

最終更新: 2026-06-10（初版）
親文書: `docs/implementation-plan.md` の「M4 表示/ウィンドウ」、`docs/architecture-decisions.md` の
ウィンドウサイズ/プリセット(Q10)・映像の上に浮かべる UI の範囲(Q5)・フルスクリーン(Q6)・スナップショット。

## 目的

M4 を**単独完結する小マイルストーン（M4.x）に分割**し、各セッションのコンテキスト肥大を防ぐ。
M3（BBS）と同じ運用——各 M4.x は「移植元の `file:function`」をピンポイントで持つので、実装セッションは
該当箇所だけ読めばよい。**PCRPlayer の表示系（DirectShow/MFC 依存）はコード写経しない**こと。窓サイズ計算・
プリセット定義・スナップショットの**挙動と既定値**だけを参照仕様として引き継ぐ。

M4 のスコープ（親文書より）:
- **ズーム% プリセット ＋ 絶対サイズプリセット**（ともにユーザー定義可。M4 ではハードコード既定、config 化は M5）。
- **フィット/アスペクト各モード**を mpv ネイティブオプションへ対応付け
  （inscribe=`keepaspect` / none=`no-keepaspect` / 充填=`panscan` / 明示比=`video-aspect-override` / 等倍=`video-unscaled`）。
- **バー除外の正確ピクセルモード**（映像のみを指定ピクセルに。OBS タイル配信用）。
- **全画面**（ウィンドウごと。Q6=A）。
- **スナップショット**（mpv `screenshot` コマンド）。

---

## 横断アーキテクチャ決定（全 M4.x で遵守。各セッションで再発見・再決定しない）

1. **`player::MpvBackend` のプロパティ/オプション API がすべての土台**。現状の `MpvBackend` は
   `attach`/`load`/`command` ＋ 4 シグナルのみ（`src/player/include/player/mpv_backend.h`）。M4.0 で
   **オプション設定（init 前）・プロパティ get/set・プロパティ監視（observe）** を足す。
   mpv API は **GUI スレッドからのみ**呼ぶ（既存方針: wakeup → `onWakeup` で排出）を厳守。

2. **ピクセルサイズは Qt、フレームの収め方は mpv**（Q10 確定）。
   - 「映像を N×M の正確ピクセルにする」= **`VideoHostWidget` を `setFixedSize(N,M)` し、ウィンドウを
     `adjustSize()`**。バー（タイトル帯/将来のシーク・ステータス）の高さは **Qt レイアウトが自動加算**する。
     → PCRPlayer の `InflateWindowRect`/`AbsoluteWindowSize`(`MainDlgSub.cpp:978`) を**手計算で移植しない**
     （Qt レイアウトに委譲＝堅牢化）。
   - 「フィット/アスペクト」はウィジェット内での映像の収め方なので **mpv オプション**に倒す（決定 1 の API 経由）。
     PCRPlayer は `getAspectRect`(`Util.cpp:653`) で自前レターボックス計算していたが**移植しない**。

3. **ネイティブ映像サイズは mpv から取得**。`dwidth`/`dheight`（アスペクト適用後の表示サイズ）を observe し、
   `videoSizeChanged(int w, int h)` を emit。ズーム% プリセットはこの値 × zoom で目標ピクセルを出す。
   移植元の意味論は `getNativeVideoSize`(`GraphManager.cpp:693`) ＝ 同じ「素の映像寸法を取る」役割。

4. **config は M5**。プリセット群（ズーム/サイズ/アスペクト）・スナップショット設定（フォルダ/形式/品質/
   テンプレート）・全画面時のバー表示有無は **M4 ではハードコード既定**にし、`// M5: config化` を残す。
   既定値は PCRPlayer の `sl::WindowConfig`(`SerializeDisplay.h:52-98`) を踏襲（下記）。

5. **映像の上に浮かべる UI はポップアップのみ**（Q5=A）。サイズ/フィット/全画面/スナップショットの操作系は
   **メニュー＋（M5 で）ショートカット**。映像オーバーレイの操作 UI は作らない。全画面は**クリーンな映像**が原則。

6. **テスト作法**: 既存 `tests/tst_*`（Qt Test）に倣う。**純計算ロジック（窓サイズ算術・プリセット選択・
   スナップショットファイル名生成）は UI 非依存の関数に切り出して単体テスト**する。mpv 実呼び出し・ウィンドウ
   ジオメトリ・全画面遷移は**手動 E2E**（M3 と同じ線引き：ネットワーク/ウィンドウ系は単体非対象）。

7. **入力集約の前提（Q12）**: `--wid` 子窓のキー横取り防止のため mpv 内蔵キーは無効
   （`input-default-bindings=no` / `input-vo-keyboard=no`）。M4.0 の `attach()` でオプション設定する
   （未設定なら本マイルストーンで入れる）。キー割当の本体（再マップ）は M5。

---

## サブマイルストーン一覧

実装順は依存順（基盤 → フィット → サイズ/プリセット → 全画面 → スナップショット）。
各 M4.x は単独でビルド+テスト緑にして閉じる。**M4.3/M4.4 は薄ければ実装時に併合可**。

### M4.0 — `MpvBackend` プロパティ/オプション拡張 ＋ 映像サイズ通知 ＋ 窓サイズ純計算
- **成果物**:
  - `player::MpvBackend` 拡張（`src/player/`）:
    - `setOption(name, value)`: `mpv_set_option_string`（**init 前**専用。`attach()` 内で使う）。
    - `setProperty(name, value)` / `setPropertyFlag(name, bool)` / `setPropertyDouble(name, double)`:
      `mpv_set_property_string` ほか（init 後の実行時変更）。
    - `getPropertyDouble(name)` / `getPropertyString(name)`: 同期 get（`mpv_get_property`）。
    - `dwidth`/`dheight` を `mpv_observe_property` し、変化時 `videoSizeChanged(int w, int h)` を emit
      （`onWakeup` のイベント排出ループに `MPV_EVENT_PROPERTY_CHANGE` 分岐を追加。既存の core-idle/
      cache-time と同じ作法）。
    - `attach()` に `input-default-bindings=no` / `input-vo-keyboard=no` を `setOption` で投入（横断決定 7）。
  - **窓サイズ純計算ヘルパ**（UI 非依存・テスト対象。`src/app/src/window_geometry.{h,cpp}` か小ヘッダ）:
    - `videoTargetForZoom(nativeW, nativeH, zoomPercent) -> QSize`: `round(native * zoom/100)`。
      移植元 `WindowZoom`(`MainDlgSub.cpp:925-949`) の算術部のみ（DirectShow 依存は捨てる）。
    - `applyAspectOverride(nativeW, nativeH, ax, ay) -> QSize`: 明示比指定時の表示寸法
      （移植元 `getAspectRect`(`Util.cpp:653`) の算術を QSize で再実装）。
  - **mpv 内蔵キー無効化**（横断決定 7）。
- **テスト**: `videoTargetForZoom`（25/50/100/150/200% × 端数丸め）、`applyAspectOverride`（16:9/4:3/等）。
- **完了基準**: プロパティ get/set/observe が動き、`videoSizeChanged` が発火する（手動）。純計算が単体で緑。

### M4.1 — フィット/アスペクトモード（mpv オプション対応付け）
- **成果物**:
  - **フィットモード enum** ＋ 適用関数（`MpvBackend` のプロパティ API 経由）:
    | モード | mpv 設定 |
    |---|---|
    | inscribe（内接・既定） | `keepaspect=yes` |
    | none（引き伸ばし） | `keepaspect=no` |
    | 充填（はみ出し充填） | `panscan=1.0`（または段階） |
    | 等倍（ドットバイドット） | `video-unscaled=yes` |
    | 明示比 | `video-aspect-override=<ax>:<ay>` |
  - **アスペクト比プリセット**（ハードコード既定）: 16:9 / 4:3 / 5:4 / 2.35:1 / 1.85:1
    （移植元 `WindowConfig::aspect`(`SerializeDisplay.h:89-93`)）。`video-aspect-override` に流す。
  - **メニュー配線**: `MainWindow` に「表示」メニューを新設し、フィットモード（ラジオ）・アスペクト
    （プリセット＋既定/なし）を追加。移植元の操作 ID は `OnWindowAspectRange/Default/None`(`MainDlgMenu.cpp:673-691`)。
- **テスト**: モード→mpv オプション対応表の単体（純関数 `fitModeToMpvProps(mode) -> {name,value}...`）。
- **完了基準**: 各フィットモードで `--wid` 内の映像の収まりが切り替わる（手動）。対応表が単体で緑。

### M4.2 — ズーム% / 絶対サイズ / バー除外正確ピクセル プリセット
- **成果物**:
  - **プリセットデータモデル**（ハードコード既定。横断決定 4）:
    - ズーム%: 25/50/75/100/125/150/200（`WindowConfig::zoom` `SerializeDisplay.h:55-61`）。
    - 絶対サイズ: 16:9 系（640×360, 800×450, 960×540, 1280×720 …）＋ 4:3 系（320×240 …）
      （`WindowConfig::size` `SerializeDisplay.h:64-86`。M4 は代表値の縮約セットで可）。
    - 各プリセットは `enable` フラグ相当を持つ（M5 で config 編集）。
  - **適用ロジック**（横断決定 2＝Qt レイアウト委譲）:
    - ズーム%: `videoTargetForZoom(native, zoom)`（M4.0）→ `videoWidget_->setFixedSize(...)` →
      `adjustSize()`。バー高さは自動加算される。
    - 絶対サイズ（**バー除外の正確ピクセル**＝OBS タイル用）: `videoWidget_->setFixedSize(W,H)` →
      `adjustSize()`。**映像領域がちょうど W×H** になり、ウィンドウ全体はバー分だけ大きい。
    - サイズ固定の解除（自由リサイズに戻す）アクションも用意（`setMinimumSize`/`setMaximumSize` 復帰）。
  - **メニュー配線**: 「表示」メニューにズーム%・絶対サイズのサブメニュー。移植元の操作は
    `OnWindowZoom100/OnWindowZoomRange`(`MainDlgMenu.cpp:562-651`)・`OnWindowSizeRange`（同系）と
    `AbsoluteWindowSize`(`MainDlgSub.cpp:978`)。
  - **注記**: 正確ピクセルを保証するため `keepaspect` 下でも**ウィジェット自体を映像アスペクトに合わせる**
    （= ズーム/絶対サイズ適用時はレターボックスが出ない）。フィットモード（M4.1）は自由リサイズ時の収め方。
- **テスト**: プリセット選択ロジック（index→QSize、enable フィルタ）の単体。実ジオメトリは手動。
- **完了基準**: ズーム% でウィンドウが映像基準に拡縮、絶対サイズで**映像領域が指定ピクセル**になる（手動・
  OBS ウィンドウキャプチャでピクセル一致を確認）。

### M4.3 — 全画面（ウィンドウごと）
- **成果物**:
  - 全画面トグル（`MainWindow`）: `showFullScreen()`/`showNormal()`。Q6=A（ウィンドウごと全画面）。
  - 全画面時は**クリーンな映像**（Q5）: メニューバー・ステータスバー・タイトル帯・各ドックを隠し、復帰時に
    元の表示状態へ戻す（事前状態を退避）。`--wid` 子窓は親に追従するので mpv 側操作は不要。
  - メニュー＋（既定キー）`F`/`Esc` でトグル（キー本体の集約は M5 だが、最小トグルは M4 で配線可）。
  - ダブルクリックでトグル（`VideoHostWidget` は無フォーカス・親がキー受領のため、イベントは `MainWindow`
    側で扱う）。
- **テスト**: 退避/復帰の状態管理に純ロジックがあれば薄く単体。遷移自体は手動。
- **完了基準**: 全画面で映像のみ、復帰でバー/メニューが元通り（手動）。

### M4.4 — スナップショット（mpv `screenshot`）
- **成果物**:
  - `MpvBackend` 経由で `screenshot video`（OSD/字幕なしの素フレーム・元解像度）を発行。
    関連プロパティをハードコード既定で設定（横断決定 4）: `screenshot-format`（png 既定）/
    `screenshot-jpeg-quality` / `screenshot-directory` / `screenshot-template`。
  - **保存先 ＋ 保存フォルダを開く**を提供（移植元 `OnSnapshotSave`/`OnSnapshotFolder`(`MainDlgMenu.cpp:517-559`)）。
  - **ファイル名生成**を純関数で（テスト対象）: 移植元 `createSnapshotFilename`(`Snapshot.h:33` /
    `Snapshot.cpp`)＝日時ベース命名を `QString` で再実装（`screenshot-template` に寄せるなら mpv 任せでも可・
    どちらにするか実装時に判断）。
  - メニュー＋（既定キー）でトリガ。
  - **注記**: PCRPlayer の GDI+ `Snapshot::save`/EVRCustom 依存は**全捨て**（決定どおり mpv `screenshot` に置換）。
- **テスト**: ファイル名生成（日時→命名規則、拡張子）の単体。実保存は手動。
- **完了基準**: スナップショットが指定フォルダに保存され、フォルダを開ける（手動）。

### M4.5 —（任意・軽量）結合 + 手動 E2E
- 実況実機（PeerCast 映像）で M4.0〜M4.4 を通し確認: ズーム/絶対サイズ/フィット切替/全画面/スナップショット。
- バー除外の正確ピクセルが OBS のウィンドウキャプチャで**ピクセル一致**することを確認（Q10 の主目的）。
- 薄ければ M4.2/M4.4 完了時に各自確認して本項は省略可。

---

## 進捗

- [ ] M4.0  MpvBackend プロパティ/オプション拡張 + 映像サイズ通知 + 窓サイズ純計算
- [ ] M4.1  フィット/アスペクトモード（mpv オプション対応付け）
- [ ] M4.2  ズーム% / 絶対サイズ / バー除外正確ピクセル プリセット
- [ ] M4.3  全画面（ウィンドウごと）
- [ ] M4.4  スナップショット（mpv screenshot）
- [ ] M4.5  （任意）結合 + 手動 E2E

> 各サブM完了時にここを更新する（M3 サブプランと同じ運用）。

関連: `docs/architecture-decisions.md`(Q5/Q6/Q10/スナップショット), `docs/implementation-plan.md`(M4),
`docs/m3-bbs-subplan.md`（分割運用の先例）。
