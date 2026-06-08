# ビルド手順

対象: Windows x64 / **MSVC**（VS 2022 または 2026）+ **Qt 6.11.1 msvc2022_64** + **CMake ≥ 3.21** + **Ninja**。
スタックの背景は `docs/architecture-decisions.md` / `docs/implementation-plan.md` を参照。

## 1. 前提ツール

| ツール | 確認済みバージョン | 入手 |
|--------|-------------------|------|
| Visual Studio (C++ ワークロード) | 2026 / v18（MSVC 14.50） | VS Installer。`vswhere` で自動検出される |
| Qt | 6.11.1 `msvc2022_64`（`C:/Qt/6.11.1/msvc2022_64`） | Qt オンラインインストーラ |
| CMake | 4.2.1 | — |
| Ninja | 1.11.1 | — |
| PowerShell | 7+ | libmpv 取得スクリプトに必須 |

> Qt のパスが異なる場合は `CMakePresets.json` の `CMAKE_PREFIX_PATH`、または構成時に
> `-DCMAKE_PREFIX_PATH=<Qt>/msvc2022_64` を指定する。

## 2. libmpv dev の取得（初回のみ）

libmpv のヘッダと import lib はリポジトリに含めない。スクリプトで取得する：

```powershell
pwsh -File scripts/fetch-mpv-dev.ps1
```

これは [zhongfly/mpv-winbuild] の最新リリースから GPL ビルドの `mpv-dev-x86_64-*.7z` を
`third_party/` に取得・展開し、`mpv.lib`（MSVC import lib）を生成する。生成物：

```
third_party/mpv-dev/include/mpv/client.h   ヘッダ
third_party/mpv-dev/libmpv-2.dll           ランタイム（配布時に同梱）
third_party/mpv-dev/mpv.lib                MSVC リンク用（CMake の FindMpv が参照）
```

補足:
- この dev 書庫には `mpv.def` が含まれないため、スクリプトは `dumpbin /exports` から
  DLL のエクスポート表を起こして `.def` を作り、`lib /def` で `mpv.lib` を生成する。
- 展開はこの環境に `7z.exe` が無いため Windows 同梱の `tar`(bsdtar、7z 対応)を使う。
  `7z.exe` があればそちらを優先。
- 冪等。再取得は `-Force`。`third_party/` は `.gitignore` 済み。

## 3. 構成・ビルド

MSVC + Ninja は `cl`/`ninja` に PATH が通った環境（= **x64 Native Tools / vcvars64**）で実行する。

```powershell
# 開発者シェルを使わない場合は vcvars64 を読み込んでから：
& "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

cmake --preset msvc-ninja
cmake --build --preset msvc-ninja
```

出力: `build/msvc-ninja/bin/YAPCRPlayer.exe`（`libmpv-2.dll` は後処理で隣にコピーされる）。

起動確認: ウィンドウタイトルに `Qt 6.11.1 / mpv api X.Y` と出れば、Qt と libmpv の
リンク・ランタイムロードが成功している（M0 の合格条件）。
build ツリーから直接起動する場合は Qt の bin に PATH を通す：

```powershell
$env:PATH = 'C:\Qt\6.11.1\msvc2022_64\bin;' + $env:PATH
build/msvc-ninja/bin/YAPCRPlayer.exe
```

## 4. 配布フォルダの作成

```powershell
cmake --build --preset msvc-ninja --target deploy
```

`windeployqt` が Qt の DLL/プラグインを集め、`libmpv-2.dll` と exe を `build/msvc-ninja/dist/` に
まとめる。`dist/` 単体で他環境へ持ち出して起動できる。

## 5. トラブルシュート

- **`libmpv dev が見つからない`（構成エラー）**: 手順2のスクリプトを未実行。先に実行する。
- **`cl が見つからない` / Ninja がコンパイラを検出しない**: vcvars64 を読み込んでいない。
- **日本語コメントで C4819 警告**: ソースは UTF-8。`/utf-8` を全ターゲットに適用済み（出ないはず）。
- **Qt が見つからない**: `CMAKE_PREFIX_PATH` を実際の Qt msvc キットへ。

[zhongfly/mpv-winbuild]: https://github.com/zhongfly/mpv-winbuild/releases
