# Render API 移行設計決定記録

対象ブランチ: `feature/render-api`
決定日: 2026-06-14

---

## ⚠️ 訂正記録（2026-06-15・実装時の実機検証で判明）

本文書の当初の決定には**誤った技術的前提が3点**含まれていた。実装・デバッグで判明した
事実に基づき訂正する。当該箇所は各決定内に「**【訂正 2026-06-15】**」として併記した。
詳細な経緯と検証手段はメモリ `render-api-pitfalls.md` を参照。

1. **`vo=libmpv` は明示設定が必須**（決定 3・決定 4）。
   「`mpv_render_context_create()` が `vo=libmpv` を自動選択する」は**誤り**。
   この mpv ビルドの既定 vo は `gpu-next` で、render context を作っても vo は上書き
   されない。`vo=libmpv` を設定しないと、デコードは進む（`time-pos` 進行・`dwidth` 設定）
   のに映像が render context へ供給されず**黒画面**になり、update callback も初回1回しか
   来ない。`attach()` 内で `mpv_set_option_string(h, "vo", "libmpv")` を設定する。
   なお「`vo=gpu-next` を残すと VO が競合してエラーになる」という記述も誤りで、
   実際にはエラーにならず黙って gpu-next が使われてしまう（より厄介）。

2. **`attach()`（Phase 1）はコンストラクタ末尾で呼ぶ**（決定 3）。
   「Phase 1 は showEvent から呼ぶ」という前提は**誤り**。Qt6/Windows では
   `QOpenGLWidget::initializeGL()`（Phase 2）が `showEvent` より**先**に走るため、
   attach を showEvent に置くと `initRenderContext()` 時に `mpv_` が null で早期 return し、
   `renderContextReady` が永久に発火しない。Render API では attach に winId 依存が
   ないので、コンストラクタ末尾（全 QAction 構築後）で先行実行できる。

3. **update callback は `mpv_render_context_update()` で再アームが必要**（決定 5）。
   コールバックから直接 `frameReady` を emit するだけでは不十分。mpv の update 通知は
   ダーティフラグ方式で、GUI スレッドのスロットで `mpv_render_context_update()` を呼んで
   フラグをクリアし、`MPV_RENDER_UPDATE_FRAME` 立っている時のみ描画要求を出す。

---

## 背景

現在の実装は libmpv を **WID 埋め込み**（`mpv_set_option("wid", ...)`）で使用している。
この方式には以下の問題がある。

- mpv が専用スレッドで子 HWND を生成するため、Qt のマウスイベントが映像領域に届かない
- ウィンドウドラッグを `WM_PARENTNOTIFY` + `GetCursorPos` ポーリングで擬似実装せざるを得ない
- 「映像領域のみドラッグ検知」が難しい（`WM_PARENTNOTIFY` は全子 HWND から伝播する）
- Windows 専用コード（`#ifdef Q_OS_WIN`）がメインウィンドウに集中する

**libmpv の Render API** に切り替えることでこれらを根本解消する。

---

## 決定 1：Qt 描画ウィジェットの型

**決定: `QOpenGLWidget`**

### 検討した選択肢

| 選択肢 | 判断 |
|--------|------|
| `QOpenGLWidget` | 採用。`QWidget` のサブクラスなので既存の QSplitter レイアウトをそのまま使える |
| `QRhiWidget` | 却下。libmpv の公開 Render API は `MPV_RENDER_API_TYPE_OPENGL` と `MPV_RENDER_API_TYPE_SW` の2種類しかなく、Qt6 デフォルトの D3D11 バックエンドと直結できない |
| `QOpenGLWindow` + `createWindowContainer()` | 却下。QSplitter との相性が悪く、合成が壊れやすい |

### QRhiWidget を選べない理由の詳細

libmpv の Render API が提供するバックエンドは以下のみ（`render.h` 実測）：

```
MPV_RENDER_API_TYPE_OPENGL  "opengl"  GPU パス
MPV_RENDER_API_TYPE_SW      "sw"      CPU パス（ヘッダ自身が "slow" と警告）
```

D3D11 / D3D12 / Vulkan / Metal に対応する API は公開されていない。
`QRhiWidget` を Windows で使うと Qt は D3D11 バックエンドで動くため、
mpv の GPU レンダリング結果を受け取る手段がない。
ソフトウェアレンダリング（CPU 転送）か OpenGL 強制しか選択肢がなく、どちらも問題がある。

---

## 決定 2：`mpv_render_context*` の所有者

**決定: `MpvBackend` が所有する（設計 B）**

### 根拠

- 現在 `MpvBackend` は mpv に関する全操作を集約している（`mpv_handle*` 所有）
- `mpv_render_context` も mpv コアに紐付くオブジェクトであり、同クラスに置くのが自然
- `VideoHostWidget` を「描画の引き金を引くだけ」の薄いアダプタに保てる
- `MainWindow` からの使い方が変わらない

### 追加する API

```cpp
// MpvBackend に追加
bool initRenderContext(void* (*getProcAddr)(void*, const char*), void* procAddrCtx);
void renderFrame(int fboId, int w, int h);

signals:
    void frameReady();  // 新フレームが描画可能になったとき（内部スレッドから委譲）
```

---

## 決定 3：初期化の2フェーズ化

**決定: Phase 1 と Phase 2 に分離し、`load()` は Phase 1 完了後すぐ呼んでよい**

### フロー

> **【訂正 2026-06-15】** 下記フローの「Phase 1 は showEvent から呼ぶ」は誤り。
> Qt6/Windows では `initializeGL()`（Phase 2）が `showEvent` より**先**に走るため、
> attach は**コンストラクタ末尾**で行う（正しいフローは後述）。また各種オプションの
> 「vo は削除」も誤りで、正しくは `vo=libmpv` を**明示設定**する。

```
Phase 1（★訂正: MainWindow コンストラクタ末尾で呼ぶ。winId 非依存のため show 不要）:
  MpvBackend::attach()
    ├─ mpv_create()
    ├─ vo=libmpv を明示設定（★訂正: 削除ではなく必須）／wid は設定しない
    ├─ その他オプション（idle/cache/terminal/reconnect 等）
    ├─ mpv_set_wakeup_callback()
    └─ mpv_initialize()

  ↓ attach() 完了後すぐ load() 呼び出し可（映像デコードはバッファリング開始）

Phase 2（VideoHostWidget::initializeGL() から呼ぶ。show 後の初回ペイントで Qt が自動起動）:
  MpvBackend::initRenderContext(getProcAddr, glCtx)
    ├─ mpv_render_context_create()
    ├─ mpv_render_context_set_update_callback()
    └─ 成功後 renderContextReady シグナルを emit
       → MainWindow が遅延していた loadfile（pending media）の再生を開始する

  ↓ 以降 update callback → onRenderUpdate() → frameReady → update() → paintGL() で映像が映る
```

### Phase 1 をコンストラクタで行う理由（★訂正の核心）

当初は WID 埋め込みの名残で「Phase 1 は showEvent から」と想定していたが、
**`QOpenGLWidget::initializeGL()` は `showEvent()` より先に呼ばれる**（Qt6/Windows 実測）。
attach を showEvent に置くと、Phase 2 が走る時点で `mpv_` がまだ null となり、
`initRenderContext()` が早期 return して `renderContextReady` が永久に発火しない
（＝黒画面・再生位置も出ない）。Render API の `attach()` は winId に依存しないため、
全 QAction 構築後のコンストラクタ末尾で先行実行すれば、Phase 2 がいつ走っても
`mpv_` が確定済みになる。

### `load()` を Phase 2 完了前に呼ぶ理由

このアプリはライブストリーム再生が主目的であり、起動を遅らせるより早くバッファリングを始める方が望ましい。
mpv は VO 未確定でも内部デコードを続けるため、`initRenderContext()` 完了の瞬間から映像が出る。
ただし**最初の `loadfile` は `renderContextReady`（Phase 2 完了）以降に送る**こと。
render context 確立前に `loadfile` を送ると、mpv が独自ウィンドウを開く VO に
フォールバックする（`render.h` 記載の挙動）。

---

## 決定 4：`attach()` のオプション変更

**決定: `wid` を削除し `attach()` 引数から `quintptr wid` も除去。`vo` は `libmpv` を明示設定**

> **【訂正 2026-06-15】** 当初は「`vo` も削除（render_context_create が vo=libmpv を
> 自動選択する）」としていたが**誤り**。`vo=libmpv` の**明示設定が必須**。

### 変更前後

```cpp
// 変更前
bool MpvBackend::attach(quintptr wid) {
    mpv_set_option(h, "wid", MPV_FORMAT_INT64, &widInt);
    mpv_set_option_string(h, "vo", "gpu-next");
    // ...
}

// 変更後（★訂正反映）
bool MpvBackend::attach() {
    // wid → 削除（Render API は子 HWND を作らない）
    mpv_set_option_string(h, "vo", "libmpv");  // ★必須: 削除でも自動でもなく明示設定
    // その他のオプション（idle/cache/terminal/reconnect 等）はそのまま維持
}
```

### vo に関する正しい挙動（★訂正）

- `mpv_render_context_create()` は**vo を上書きしない**。vo が未設定なら mpv 既定の
  `gpu-next` がそのまま使われる。
- `vo=libmpv` を設定しないと、デコードは進む（`time-pos` 進行・`dwidth`/`dheight` 設定）
  のに映像フレームが render context へ供給されず**黒画面**になる。さらに update callback も
  `set_update_callback()` 時の初回1回しか発火しない（新フレーム通知が来ない）。
- 当初記述の「`vo=gpu-next` を残すと VO が競合してエラーになる」も**誤り**。実際には
  エラーにならず黙って gpu-next が選ばれる（エラーで気付けない分むしろ厄介）。
- 確認コマンド: 実行中に `current-vo` プロパティを取得すると、正しく設定できていれば
  `"libmpv"` を返す（誤設定時は `"gpu-next"`）。

---

## 決定 5：描画更新通知の方式

**決定: `MpvBackend::frameReady` シグナル（設計 X）**

### 根拠

- `mpv_render_context_set_update_callback()` のコールバックは mpv 内部スレッドから呼ばれる
- コールバック内で mpv API・`mpv_render_*` 関数を呼ぶことは禁止されている
- 既存の `wakeupCallback` → `onWakeup()` パターンと対称にできる
- `MpvBackend` はすでに `QObject` に依存しているためシグナル追加コストはゼロに近い

### 実装パターン

> **【訂正 2026-06-15】** コールバックから直接 `frameReady` を emit するだけでは不十分。
> mpv の update 通知はダーティフラグ方式で、`mpv_render_context_update()` を呼んで
> フラグをクリアしないと**初回1回しかコールバックが来ない**。下記のとおり GUI スレッドの
> スロットを1段挟んで update() を呼び、`MPV_RENDER_UPDATE_FRAME` の時だけ emit する。

```cpp
// MpvBackend 内（static コールバック）— 内部スレッドからは委譲のみ
static void renderUpdateCallback(void* ctx) {
    QMetaObject::invokeMethod(static_cast<MpvBackend*>(ctx),
                              "onRenderUpdate", Qt::QueuedConnection);
}

// GUI スレッドのスロット（mpv_render_* を呼んでよいスレッド）
void MpvBackend::onRenderUpdate() {
    if (!renderCtx_) return;
    // ★ ダーティフラグをクリアして次回コールバックを再アームする（必須）
    const uint64_t flags = mpv_render_context_update(renderCtx_);
    if (flags & MPV_RENDER_UPDATE_FRAME) {
        emit frameReady();
    }
}

// VideoHostWidget 側で接続
connect(mpv_, &MpvBackend::frameReady,
        this, QOverload<>::of(&VideoHostWidget::update),
        Qt::QueuedConnection);
```

`mpv_render_context_update()` は ADVANCED_CONTROL 未使用時は仕様上「任意」だが、
本実装では再アームと無駄な再描画抑制（FRAME フラグ判定）のため常に呼ぶ。

---

## 決定 6：ウィンドウドラッグの置き換え

**決定: `VideoHostWidget::windowDragRequested(QPoint globalDelta)` シグナル → `MainWindow` が移動判断（選択肢 P）**

### 根拠

- 最大化・全画面中はドラッグ無効などのポリシーは既に `MainWindow` 側にある
- `VideoHostWidget` は「ドラッグの意図を通知する」だけにとどめる
- `QOpenGLWidget` では通常の Qt マウスイベント（`mousePressEvent` / `mouseMoveEvent` / `mouseReleaseEvent`）が映像領域で届くため、ポーリング不要

### 実装パターン

```cpp
// VideoHostWidget に追加
signals:
    void windowDragRequested(QPoint globalDelta);

// MainWindow 側で接続
connect(videoWidget_, &VideoHostWidget::windowDragRequested,
        this, [this](QPoint delta) {
            if (!isMaximized() && !isFullScreen())
                move(pos() + delta);
        });
```

---

## 決定 7：ビルド設定変更

**決定: `Qt6::OpenGLWidgets` を2箇所に追加**

```cmake
# ルート CMakeLists.txt
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Network Core5Compat OpenGLWidgets)

# src/app/CMakeLists.txt
target_link_libraries(YAPCRPlayer PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets   # 追加
    yapcr::player
    ...
)
```

`OpenGL32.lib` は `Qt6::OpenGLWidgets` が推移的に引っ張るため個別指定不要。

---

## 移行で削除されるコード

| ファイル | 削除対象 |
|----------|----------|
| `main_window.cpp` | `nativeEvent()` の `WM_PARENTNOTIFY` ブロック全体 |
| `main_window.cpp` | `beginVideoDrag()` |
| `main_window.cpp` | `onDragMoveTick()` |
| `main_window.h` | `dragMoveTimer_` / `dragStartCursor_` / `dragStartWindow_` の3メンバ |
| `video_host_widget.cpp` | `setAttribute(Qt::WA_NativeWindow)` 行 |

Windows 専用 `#ifdef Q_OS_WIN` ブロックが消えることでコードがクロスプラットフォームに近づく。

---

## 移行で得られる副次的メリット

- 映像上への Qt ウィジェット合成が可能になる（WID 埋め込みでは mpv の HWND が常に最前面のため不可能）
- マウスイベントが映像領域で通常の Qt イベントとして届く（右クリック・ダブルクリック含む）
- `QOpenGLWidget::grabFramebuffer()` でフレームバッファを `QImage` として取得できる（スナップショットの代替手段）
- Wayland 対応への障壁解消（WID 埋め込みは Wayland 非対応）

---

## 実装順序

1. `MpvBackend` — `attach()` 引数削除・`wid` 除去・**`vo=libmpv` 明示設定**（★訂正: 除去ではない）・`initRenderContext()` / `renderFrame()` / `frameReady` シグナル・`onRenderUpdate()` スロット（`mpv_render_context_update()` で再アーム）追加
2. `VideoHostWidget` — `QOpenGLWidget` へ全書き換え（`initializeGL` / `paintGL` / `resizeGL` / マウスイベント）。`initializeGL` 末尾で `renderContextReady` を emit
3. `MainWindow` — **`attachMpv()` をコンストラクタ末尾で呼ぶ**（★訂正: showEvent ではない）・`renderContextReady` 受信で pending loadfile 開始・`nativeEvent` 削除・ドラッグポーリング削除・`windowDragRequested` 接続
4. `CMakeLists.txt` — `OpenGLWidgets` 追加
5. ビルド確認 → 動作確認（検証手段は `render-api-pitfalls.md` 参照）

### FBO 実装の注意点（`paintGL()` → `MpvBackend::renderFrame()`）

> **【補足 2026-06-15】** 決定 2 のとおり render context は `MpvBackend` の所有物なので、
> `renderCtx_` は private。`paintGL()` 側は FBO の id とサイズだけ渡し、
> `mpv_opengl_fbo` の組み立て・null ガード・`mpv_render_context_render()` は
> `MpvBackend::renderFrame()` 内で行う（下記は実装の実体）。

```cpp
// VideoHostWidget 側: FBO id と物理ピクセルサイズを渡すだけ
void VideoHostWidget::paintGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const qreal dpr = devicePixelRatio();
    mpv_->renderFrame(static_cast<int>(defaultFramebufferObject()), // Qt 管理 FBO（0 ではない）
                      static_cast<int>(width()  * dpr),             // 物理ピクセル（論理ではない）
                      static_cast<int>(height() * dpr));
}

// MpvBackend 側: render context は private なのでここで完結させる
void MpvBackend::renderFrame(int fboId, int w, int h) {
    if (!renderCtx_) return;  // Phase 2 完了前は no-op（黒画面のまま）

    mpv_opengl_fbo fbo{};
    fbo.fbo = fboId;
    fbo.w   = w;
    fbo.h   = h;
    fbo.internal_format = 0;  // GL_RGBA8 相当

    int flipY = 1;  // QOpenGLWidget は Y 反転が必要
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y,     &flipY},
        {MPV_RENDER_PARAM_INVALID,    nullptr}
    };
    mpv_render_context_render(renderCtx_, params);
}
```

`fbo.fbo = 0` や `dpr` の掛け忘れ、`flipY = 0` はいずれも映像が出ない原因になる。

なお、上記がすべて正しくても **`vo=libmpv` 未設定なら黒画面**になる（決定 4 訂正参照）。
切り分けの優先順位は「① `current-vo` が `libmpv` か → ② attach がコンストラクタで先行
しているか → ③ update callback が再アームされているか」。
