# サードパーティ ライセンス告知 (THIRD-PARTY-NOTICES)

YAPCRPlayer 本体は **GNU General Public License v3 以降 (GPL-3.0-or-later)** で
配布される（リポジトリ直下の `LICENSE` に全文）。本ファイルは、YAPCRPlayer の
バイナリ配布物に同梱・動的リンクされるサードパーティ コンポーネントと、その
ライセンスおよび「対応するソース (Corresponding Source)」の入手方法を告知する。

GPLv3 §4/§5 に基づき、本ファイルと `licenses/` 配下のライセンス全文、および
`CORRESPONDING-SOURCE.md`（書面オファー）は配布物（`dist/`）に必ず同梱すること。

---

## 1. libmpv (`libmpv-2.dll`) — mpv / FFmpeg ＋ 静的依存

YAPCRPlayer は再生エンジンとして libmpv を **動的リンク**で利用し、
`libmpv-2.dll` を配布物に同梱する。この DLL は自前ビルドではなく、
[zhongfly/mpv-winbuild] が公開するプリビルド（GPL ビルド）を取得して使用している。
取得・ビルドの正確な同定情報と対応ソースの入手方法は **`CORRESPONDING-SOURCE.md`**
を参照。

ライセンス構成：

| コンポーネント | ライセンス | 全文 |
|---|---|---|
| mpv | GPLv2+ （`-Dgpl=false` 無効ビルドのため GPL only ファイルを含む） | `licenses/mpv/LICENSE.GPL`, `licenses/mpv/Copyright` |
| FFmpeg | `--enable-gpl` 構成（x264/x265 等の GPLv2 ライブラリをリンク）のため **GPLv2+** | `licenses/ffmpeg/COPYING.GPLv2`, `licenses/ffmpeg/LICENSE.md` |

- mpv 自体は既定で GPLv2+（`licenses/mpv/Copyright` 参照。`-Dgpl=false` 時のみ
  LGPLv2.1+ になりうるが、本プロジェクトが使うビルドは GPL 構成）。
- FFmpeg は大半が LGPLv2.1+ だが、本ビルドは `--enable-gpl` で x264 (GPLv2)、
  x265 (GPLv2) 等を取り込むため、結合後の FFmpeg は **GPLv2+**（`LICENSE.md` の
  "External libraries" 節参照）。
- 上記はいずれも「GPLv2 **以降**」であり、YAPCRPlayer 全体の GPLv3 配布と矛盾しない
  （v2+ は v3 での結合・再配布を許す）。

### 静的に取り込まれる主な依存ライブラリ

`libmpv-2.dll` には FFmpeg 経由・mpv 直結で多数のライブラリが静的にリンクされる。
**確定的な一覧とバージョンは、ビルド系統 [zhongfly/mpv-winbuild] の該当リリース
コミットにおけるビルド定義（`build.sh` と CMake パッケージ定義）が正典**であり、
`CORRESPONDING-SOURCE.md` のオファーが提供する完全な対応ソースにすべて含まれる。
代表的なものを参考として示す（ライセンスは各 upstream に従う）：

- GPLv2: x264, x265, libdvdcss 系 等（FFmpeg を GPL たらしめるもの）
- LGPL/MIT/BSD/ISC 系: libass, dav1d, libplacebo, freetype, fribidi, harfbuzz,
  fontconfig, libxml2, expat, zlib, brotli, lcms2, libpng, libjpeg-turbo,
  libwebp, uchardet, libarchive, zimg, rubberband, mujs/luajit ほか

これらの個別ライセンス全文は、対応ソース（書面オファー）に同梱される各ライブラリの
ソースツリー内に含まれる。

[zhongfly/mpv-winbuild]: https://github.com/zhongfly/mpv-winbuild

---

## 2. Qt 6 (Core / Gui / Widgets / Network / Core5Compat)

YAPCRPlayer は Qt 6 を **動的リンク**で利用し、`windeployqt` が収集する Qt の
DLL/プラグインを配布物に同梱する。

- 利用ライセンス: **GPLv3**（Qt のオープンソース提供のうち GPLv3 オプションを選択）。
  GPLv3 は YAPCRPlayer 本体の GPLv3 と同一であり結合に制約はない。
- Qt の対応ソースは The Qt Company / KDE が公開する公式ソースから入手できる。
  使用バージョンと入手先は `CORRESPONDING-SOURCE.md` を参照。

---

## 3. toml++ (tomlplusplus)

設定 (TOML) の読み書きに使用。ヘッダオンリーで YAPCRPlayer に静的に取り込まれる。

- ライセンス: **MIT**（GPLv3 互換）
- 全文: submodule `third_party/tomlplusplus/LICENSE`
  （配布物には `licenses/tomlplusplus-LICENSE` として同梱する）

---

## 4. PeerCastStation（リンクなし・別プロセス）

YAPCRPlayer は PeerCastStation と **HTTP（別プロセス、arm's-length）**で通信するのみで、
コードのリンク・取り込みは行わない。したがって PeerCastStation（GPLv3）は YAPCRPlayer
との結合著作物を構成せず、本配布物に同梱もしない（GPLv2 §2 / GPLv3 の「単なる集積」に
あたらず、そもそも同梱しないため告知義務の対象外）。参照用 submodule
`peercaststation/` はプロトコル参照目的でありビルドにも配布にも関与しない。
