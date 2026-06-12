# YAPCRPlayer

**YAPCRPlayer** は [PeerCast](https://www.peercast.org/) 実況配信向けの Windows デスクトッププレイヤーです。
[libmpv](https://mpv.io/) で映像再生し、2ch/したらば系の掲示板をインライン表示・書き込みできます。

本ソフトウェアは kasahara 氏 / PeerCast Station Project による
[**PCRPlayer**](http://pecatv.s25.xrea.com/)（Copyright © 2009–2011, GPL）を
libmpv / Qt6 ベースで再実装した派生版です（詳細は[ライセンス](#ライセンス)節）。

> **⚠ 開発中（M6 仕上げフェーズ）。** 機能は随時追加中です。

---

## 機能概要

- PeerCast `/pls/` URL の自動解決・再生（`/stream/` 変換）
- libmpv による映像/音声再生（ローカルファイルも対応）
- bump / stop / 再読込（PeerCast チャンネル制御）
- 停止検知 Watchdog（映像供給切れを検知して自動再接続）
- 2ch/したらば 掲示板 dat 取得・レス一覧表示・書き込み
- 最速スレ自動追従（M3.9）
- 音量・ミュート・最小化時ミュート連動
- ズームプリセット・絶対サイズ・フィットモード・アスペクト比制御
- スナップショット保存
- TOML 設定ファイル（`config.toml`）によるショートカットカスタマイズ
- 起動時ウィンドウ状態・音量・ミュート・sage 復元

---

## 動作要件

| 項目 | 要件 |
|------|------|
| OS | Windows 10/11 x64 |
| ランタイム | Visual C++ 再頒布可能パッケージ（VS 2022 / v143 以降） |
| Qt | Qt 6.x（配布パッケージに同梱予定） |

---

## ビルド方法

詳細は [`docs/build.md`](docs/build.md) を参照してください。概要:

1. **前提ツール**: Visual Studio 2022/2026（C++ ワークロード）、Qt 6.11.1 msvc2022_64、CMake ≥ 3.21、Ninja
2. **libmpv dev の取得**（初回のみ）:
   ```powershell
   pwsh -File scripts/fetch-mpv-dev.ps1
   ```
3. **ビルド**（x64 Native Tools / vcvars64 環境で）:
   ```powershell
   cmake --preset msvc-ninja
   cmake --build --preset msvc-ninja
   ```
4. **配布パッケージ作成**:
   ```powershell
   cmake --build --preset msvc-ninja --target deploy
   ```
   `build/msvc-ninja/dist/` に `windeployqt` 済みの配布物が生成されます。

---

## 起動方法

```
YAPCRPlayer.exe [<url_or_path> [<channel_name> [<bbs_url>]]]
```

- 引数なし: ウィンドウを開く
- `<url_or_path>`: PeerCast pls URL / ローカルファイルパス
- `<channel_name>`: チャンネル名（オプション）
- `<bbs_url>`: 掲示板 URL（指定時は起動と同時に BBS 自動接続）

例:
```
YAPCRPlayer.exe http://localhost:7144/pls/ABC123 "チャンネル名" http://bbs.example.com/test/l50
```

---

## ライセンス

**YAPCRPlayer 本体**: GNU General Public License v3 以降（GPL-3.0-or-later）
→ [`LICENSE`](LICENSE)

**派生元（原著作物）**: 本ソフトウェアは **PCRPlayer** を移植・再実装した派生著作物です。
原 PCRPlayer の著作権表示を以下のとおり保持します（GPL の条件に基づく）。

> Copyright © 2009–2011 PeerCast Station Project, kasahara
> 配布元（ソース・バイナリを zip で配布）: http://pecatv.s25.xrea.com/
> サイト: http://pecastation.web.fc2.com/
> ライセンス: GNU General Public License

YAPCRPlayer のコードは PCRPlayer の挙動を参考に libmpv / Qt6 上で新規実装したものですが、
GPL の系譜に従い同一ライセンス（GPL-3.0-or-later）で配布します。

**サードパーティコンポーネント**（libmpv / FFmpeg / Qt / toml++）の帰属情報とライセンス:
→ [`licenses/THIRD-PARTY-NOTICES.md`](licenses/THIRD-PARTY-NOTICES.md)

**対応ソース（GPLv3 §6 書面オファー）**:
→ [`CORRESPONDING-SOURCE.md`](CORRESPONDING-SOURCE.md)

連絡先: philmist &lt;knagi3@gmail.com&gt;
