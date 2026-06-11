# 対応ソースの提供 (Written Offer for Corresponding Source)

YAPCRPlayer は **GPL-3.0-or-later** で配布される。GPLv3 §6 に基づき、本ファイルは
YAPCRPlayer のバイナリ配布物に含まれる、または動的リンクされるすべての GPL/LGPL
コンポーネントの **完全な対応ソース (Complete Corresponding Source)** の入手方法を
記載した書面オファーである。配布物（`dist/`）には本ファイルを必ず同梱すること。

> **このオファーは、対応するバイナリを受領した者すべてに対し、配布日から少なくとも
> 3 年間有効とする（GPLv3 §6(b)）。** 下記の各 upstream が将来消失する場合に備え、
> 配布者は §6(b) の義務を満たすため `scripts/mirror-mpv-source.ps1` で対応ソースを
> あらかじめ取得・保管しておくこと。

---

## 1. YAPCRPlayer 本体

- ライセンス: GPL-3.0-or-later（`LICENSE`）
- 対応ソース: 本リポジトリそのもの。
  - 公開リポジトリ URL: **（公開時に記入）** — 例: `https://github.com/<owner>/YAPCRPlayer`
  - 連絡先: philmist <knagi3@gmail.com>
- ビルドに必要な情報・手順は `docs/build.md` を参照。

---

## 2. libmpv (`libmpv-2.dll`) — mpv ＋ FFmpeg ＋ 静的依存

配布する `libmpv-2.dll` は自前ビルドではなく、[zhongfly/mpv-winbuild] が公開する
プリビルド（GPL ビルド）である。その「対応ソース」は、ビルドに使われたすべての
upstream ソースと、それらを取得・ビルドするスクリプト一式（GPLv3 §1 の定義）から成る。

### 配布バイナリの同定情報（ピン留め）

| 項目 | 値 |
|---|---|
| リリース tag | `2026-06-08-6444c05059` |
| 取得アセット | `mpv-dev-x86_64-20260608-git-6444c05059.7z` |
| アセット SHA-256 | `0441491A7E275CC00570EDDE5B353A166866B5D80B0C986C60FBDAE6D8669B7E` |
| mpv コミット | `6444c050592991d94cf36ecdb013dac193c24ff5` |
| FFmpeg コミット | `6028720d70d0f50512c66df43f7c9e05d6797463` |
| コンパイラ | clang（LTO） |
| ビルド系統（CMake/clang） | [zhongfly/mpv-winbuild] @ tag `2026-06-08-6444c05059` |
| ビルドログ（CI run） | https://github.com/zhongfly/mpv-winbuild/actions/runs/27136135306 |

> この同定情報は `scripts/fetch-mpv-dev.ps1` のピン値と一致しなければならない。
> libmpv を更新する際は、本表・`THIRD-PARTY-NOTICES.md`・fetch スクリプトのピンと
> `licenses/` のライセンス全文を同時に更新すること。

### 対応ソースの入手先

1. **ビルドスクリプト（"scripts to control build"）**:
   [zhongfly/mpv-winbuild] をリリース tag `2026-06-08-6444c05059` の状態で取得
   （`build.sh` と CMake パッケージ定義が、各依存の取得元コミットを規定する）。
2. **mpv 本体**: `https://github.com/mpv-player/mpv` のコミット
   `6444c050592991d94cf36ecdb013dac193c24ff5`。
3. **FFmpeg**: `https://github.com/FFmpeg/FFmpeg` のコミット
   `6028720d70d0f50512c66df43f7c9e05d6797463`。
4. **その他の静的依存（x264/x265/libass/dav1d/libplacebo 等）**: 上記 (1) の
   ビルド定義が規定する各 upstream の固定コミット。

配布者は `scripts/mirror-mpv-source.ps1` を実行して上記 (1)〜(3) を
`third_party/mpv-source-mirror/`（git 管理外）へクローン保存し、§6(b) の
3 年間提供義務を自力で履行できる状態にしておくこと。

[zhongfly/mpv-winbuild]: https://github.com/zhongfly/mpv-winbuild

---

## 3. Qt 6

- 利用ライセンス: GPLv3
- 使用バージョン: **Qt 6.11.1 `msvc2022_64`**（`docs/build.md` 参照）
- 対応ソース: The Qt Company / KDE が公開する公式ソースアーカイブ。
  - `https://download.qt.io/archive/qt/6.11/6.11.1/single/`
  - もしくは `https://code.qt.io/`（各モジュールの `v6.11.1` タグ）

---

## 4. toml++ (MIT)

- ソース: `https://github.com/marzer/tomlplusplus`（submodule `third_party/tomlplusplus`、
  本リポジトリにピン留め済み）。MIT のため §6 のソース提供義務の対象ではないが、
  ライセンス全文は配布物に同梱する。
