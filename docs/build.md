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

これは [zhongfly/mpv-winbuild] の**ピン留めしたリリース**から GPL ビルドの `mpv-dev-x86_64-*.7z` を
`third_party/` に取得（SHA-256 検証）・展開し、`mpv.lib`（MSVC import lib）を生成する。生成物：

```
third_party/mpv-dev/include/mpv/client.h   ヘッダ
third_party/mpv-dev/libmpv-2.dll           ランタイム（配布時に同梱）
third_party/mpv-dev/mpv.lib                MSVC リンク用（CMake の FindMpv が参照）
third_party/mpv-dev/PROVENANCE.txt         取得バイナリの同定情報（GPLv3 §6 用）
```

> **GPLv3 コンプライアンス**: 取得する DLL はスクリプト内の `$PinnedTag` / `$ExpectedSha256` に
> ピン留めされ、`CORRESPONDING-SOURCE.md`（書面オファー）・`THIRD-PARTY-NOTICES.md`・
> `licenses/`・`scripts/mirror-mpv-source.ps1` と一致する。`-Latest` で更新した場合はこれらも
> 必ず追従させること。配布物への同梱は `deploy` ターゲットが自動で行う（下記 §4）。

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

あわせて GPLv3 コンプライアンスのため、`LICENSE`・`CORRESPONDING-SOURCE.md`・`licenses/`
（`THIRD-PARTY-NOTICES.md` と各ライセンス全文）を `dist/` へ同梱する。第三者へ配布する際は、
`scripts/mirror-mpv-source.ps1` で取得した対応ソース一式（GPLv3 §6(b)・3 年間保管）も
頒布できる状態にしておくこと。

## 5. Release 配布物の作成（zip）

配布用の Release バイナリは Debug とは別ツリー（`build/msvc-ninja-release/`）でビルドする。
一括スクリプトで構成・ビルド・`deploy`・zip 化・SHA-256 出力まで行う：

```powershell
pwsh -File scripts/package-release.ps1
```

- 出力: `build/release-artifacts/YAPCRPlayer-v<ver>-win-x64.zip` と同名 `.sha256`。
- バージョンは `CMakeLists.txt` の `project(... VERSION)` から自動取得（`-Version` で上書き可）。
- `.pdb` / `.ilk` / `.exp` は配布物から除外される。
- 既存の `dist/` を再圧縮するだけなら `-SkipBuild`、上書きは `-Force`。
- **このスクリプトも vcvars64 環境（Native Tools シェル）で実行すること**（`cl` 検出に失敗すると停止）。

Release では CMake の `InstallRequiredSystemLibraries` で特定した再配布可能な VC++ ランタイム
（`vcruntime140.dll` / `msvcp140.dll` 等）を app-local 同梱し、Qt DLL はリリース版（`d` サフィックス無し）になる。
配布物は vc_redist の別途インストール不要で、単体で他環境へ持ち出して起動できる。

## 6. トラブルシュート

- **`libmpv dev が見つからない`（構成エラー）**: 手順2のスクリプトを未実行。先に実行する。
- **`cl が見つからない` / Ninja がコンパイラを検出しない**: vcvars64 を読み込んでいない。
- **日本語コメントで C4819 警告**: ソースは UTF-8。`/utf-8` を全ターゲットに適用済み（出ないはず）。
- **Qt が見つからない**: `CMAKE_PREFIX_PATH` を実際の Qt msvc キットへ。

[zhongfly/mpv-winbuild]: https://github.com/zhongfly/mpv-winbuild/releases
