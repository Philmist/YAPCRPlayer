# M3（BBS）サブ分割計画

最終更新: 2026-06-09
親文書: `docs/implementation-plan.md` の「M3 BBS」、`docs/architecture-decisions.md` の BBS 対応(Q8)。

## 目的

M3 を**単独完結する小マイルストーン（M3.x）に分割**し、各セッションのコンテキスト肥大を防ぐ。
各 M3.x は「移植元の `file:function`」をピンポイントで持つので、実装セッションは該当関数だけ読めばよい。
**PCRPlayer の BBS 系ソース（Shift_JIS, BaseBBS.cpp 964 行 / BBSManager.cpp 696 行 / BBSOperator.cpp 1147 行）を
毎回フル読みしない**こと——本書のポインタで足りる。

M3 のスコープ（親文書より）: したらば(jbbs.shitaraba.net) + jpnkn(bbs.jpnkn.com) の取得/パース、簡易レス一覧ペイン、
hover レス・ポップアップ、書き込み(2段階 Cookie)、HTTPS、文字コード処理（jpnkn=Shift_JIS / したらば=EUC-JP）。

---

## 横断アーキテクチャ決定（全 M3.x で遵守。各セッションで再発見・再決定しない）

1. **新モジュール `bbs`（静的ライブラリ, namespace `yapcr::bbs`）**。依存: `net`, `common`, `Qt::Core`,
   `Qt::Core5Compat`。`peercast`/`bbs` は相互非依存。配線は既存 `peercast` モジュールの CMake に倣う。

2. **bbs は QObject の非同期シグナル方式**で再設計する。`loadSetting/loadSubject/loadDat/post` は
   完了/エラーを **signal で emit**。PCRPlayer の `BaseBBS`/`BBSManager` は同期 + `boost::thread` +
   `recursive_mutex` だが **その並行構造は移植しない**（`mutex_`/`boost::shared_ptr`/`boost::optional` を写経しない）。
   ネットワークは `net::HttpClient`（QNAM 非同期）に委ね、手動スレッドは持たない。

3. **移植の線引き（重要）**:
   - **純パース**（`setting.txt`/`subject.txt`/`dat` の各行 → 構造体）は**直訳移植**でよく、**UI 非依存で単体テスト可**。
     → M3.2 / M3.3。
   - **fetch オーケストレーション**（setting→subject→dat の順序、`change`、差分取得、ポーリング、dedup）は
     **シグナル駆動で再設計**する。ポートではない。→ M3.4。

4. **正規表現は `boost::xpressive` → `QRegularExpression`**（`QString`=UTF-16 が原典 `std::wstring` と素直に対応）。
   全角文字を落とさない: `BBSRegex.h` はアンカーで全角 `０-９` / 長音 `ー` / 全角カンマ `，` / 全角 `＞` を受ける（M3.3 注記）。

5. **文字コード**: `Qt6 Core5Compat` の `QStringDecoder`/`QStringEncoder`（`QStringConverter::Encoding` ではなく
   名前指定 "Shift-JIS" / "EUC-JP"）。jpnkn=Shift_JIS、したらば=EUC-JP。**実板レスポンスで最終確認**（M3.8）。

6. **config 未存在**: `config` モジュールは M5。M3 では **User-Agent / timeout をハードコード既定**にし、
   M5 で config 配線する旨をコードに `// M5: config 化` で残す。proxy は当面なし（直結）。

7. **テストフィクスチャ（前提・M3.0 で用意）**: パーサ単体テストには**オフラインのサンプル**が要る。
   したらば・jpnkn 両方の `setting.txt` / `subject.txt` / `dat`（数レス）を `tests/fixtures/bbs/` に配置。
   出所は実板から一度キャプチャ、または最小手書き。**無いと M3.2/M3.3 のテストが書けない**ため M3.0 で確保する。

8. **テスト作法**: 既存 `tests/tst_peercast` / `tests/tst_watchdog`（Qt Test）に倣い `tests/tst_bbs` を追加。

---

## サブマイルストーン一覧

実装順は依存順（純ロジック → net → パース → オーケストレーション → 書込 → UI → 結合）。
各 M3.x は単独でビルド+テスト緑にして閉じる。**M3.2 と M3.3 は薄ければ実装時に併合可**。

### M3.0 — `bbs` モジュール土台 + URL 解析 + 文字コード + フィクスチャ
- **成果物**:
  - `src/bbs/`（CMake, `yapcr::bbs`）を新設し、ルート CMake / `tests` に配線。
  - `Board`/`Thread` データモデル（移植元 `BaseBBS.h:37-71` の `BBSInfo`、ただし `std::wstring`→`QString`,
    `Mlang::CODE`→エンコ enum）。`ResInfo`/`ThreadInfo` も同ヘッダ `74-114` を移植（フィールドのみ、振る舞いは後続）。
  - **URL 種別判定 + パース**: 起動 contact URL → したらば(`/test/read.cgi/<board>/<num>/`)か jpnkn かを判定し
    `Board{scheme,host,base,board,number}` / `Thread{key}` を埋める。移植元の正規表現は `BaseBBS.h:155`
    （`.*://(([^/]+).*)/test/read\.cgi/([^/]+)/([^/]+)`）と `ShitarabaBBS` 系の URL 生成
    `BaseBBS.cpp:893-933`（board/thread/setting/subject/dat の各 URL 組み立て）。
  - **エンコ往復ヘルパ**（`common` or `bbs` 内）: バイト列⇄`QString`（Shift_JIS/EUC-JP）。
  - `tests/fixtures/bbs/` に両板の `setting.txt`/`subject.txt`/`dat` サンプルを配置（横断決定 7）。
- **テスト**: URL 解析（したらば/jpnkn 各種）、エンコ往復（日本語+全角記号）。
- **完了基準**: `bbs` がビルドされ `tst_bbs` が緑。net 依存なし（純ロジックのみ）。

### M3.1 — `net` 拡張（POST / カスタムヘッダ / Cookie / 条件付き GET）
- **成果物**: `net::HttpClient`（`src/net/include/net/http_client.h`、現状 GET のみ）を拡張:
  - POST（`Content-Type: application/x-www-form-urlencoded`, 任意ヘッダ, body=バイト列）。
  - リクエストヘッダ設定（Referer / Cookie / Accept-Encoding）。
  - レスポンスの **status code / Set-Cookie / Last-Modified** 参照。
  - **条件付き GET**（`If-Modified-Since` → 304 を「更新なし」として返す）。gzip は QNAM 自動。
  - **同一 QNAM 再利用で cookie jar を保持**（M3.5 の確認 Cookie 半自動化のため）。
- 移植元の意味論: `BaseBBS.cpp` の `download()@470`（modified/304/gzip 判定）, `partial()@519`（Range/差分）,
  `post()@337`（POST ヘッダ + Cookie + status 取得）。**実装は QNAM ベースで再設計**（boost は移植しない）。
- **テスト**: ヘッダ/Cookie 組み立ての単体、可能なら `QHttpServer` かローカルスタブで 200/304/Set-Cookie。
- **完了基準**: bbs が必要とする「GET（条件付き含む）/POST + ヘッダ + Cookie + status」が出揃う。

### M3.2 — `setting.txt` / `subject.txt` パース（板情報・スレッド一覧）
- **成果物**:
  - `subject.txt` → `ThreadInfo` 一覧（key/title/count）。移植元 `BaseBBS.cpp` `parser(ThreadInfo&)@610` と
    `ShitarabaBBS::parser(ThreadInfo&)@845`、取得側 `loadSubject@195` / `getSubject@307`。
  - `setting.txt` → noname/title/stop。移植元 `parser(name,value)@592`, `loadSetting@168`,
    `setting()@704/910`。
- **テスト**: M3.0 のフィクスチャで両板の subject/setting をパースし件数・title・noname・stop を検証。
- **完了基準**: パーサが純関数として緑。

### M3.3 — `dat` パース + 抽出（アンカー / ID / URL / 被参照）
- **成果物**:
  - dat 行 → `ResInfo`（number/name/mail/datetime/message/title/id）。移植元 `parser(ResInfo&)@655` と
    `ShitarabaBBS::parser(ResInfo&)@865`、`loadDat@255` / `getDat@314`、`datetimeid()@444`（日時→ID 補完）。
  - **正規表現抽出**（`BBSRegex.h` 一式を `QRegularExpression` へ）: アンカー `>>n` / 範囲 `n-m` / 群 `n,m`
    （`gt/number/hyphen/comma/pair/groupe`, 全角対応 `BBSRegex.h:40-83`）、ID 抽出（`head/serial` `:85-98`）、
    URL 抽出（`scheme/body` `:28-38`）、被参照 `ref`。`BBSReplace`/`URIConvert`(`BBSRegex.h:107-238`) が
    message 中のアンカー/ID/URL を切り出す本体。`link`/`ref`/`split` を `ResInfo` に充填。
  - **差分取得**: `partial()@519`（既読位置から続き dat を取り追記）。
- **注記**: 全角 `０-９ ー ， ＞` を必ず受ける（横断決定 4）。`ShitarabaBBS` は dat 形式が 2ch と異なる点に注意。
- **テスト**: フィクスチャ dat → ResInfo 群、アンカー/ID/URL 抽出のケース表。
- **完了基準**: 1レス〜複数レス・全角アンカー・ID 集計の単体が緑。

### M3.4 — `BbsSession` オーケストレーション（シグナル駆動・dedup・extract）
- **成果物**: `BBSManager` 相当を **`yapcr::bbs::BbsSession`（QObject）として再設計**:
  - `init(contactUrl)` → したらば/jpnkn を判定して内部 BBS 実装を選択（`init@20`）。
  - fetch 順序 setting→subject→dat をシグナルで連鎖（`loadSetting@77`/`loadSubject@94`/`loadDat@115`）、
    `change(key)@62`、ポーリングは上位（SessionController）がタイマ駆動。
  - **dedup / ID 集計 / アンカー map**（`prepareDat@170`, `cleanDat@304`, `getDat@351/368`）。
  - **抽出 API**（hover ポップアップ用）: `getExtract` 群（`@439-554`：全件/テキスト/range/id/ref/vec）。
  - `pos`（既読位置）管理（`getPos@555`）、各種 getter（`getURL@576` ほか）。
- **mutex/optional は移植しない**（横断決定 2）。GUI スレッド集約・シグナルで完結。
- **テスト**: スタブ fetch（フィクスチャを流す fake `HttpClient`）で dedup・ID 集計・extract を検証。
- **完了基準**: contact → setting/subject/dat → ResInfo 列 + extract が UI なしで動く。

### M3.5 — 書き込み（2段階 Cookie POST）
- **成果物**:
  - POST body / Cookie / write URL 生成: `query()@733` + `ShitarabaBBS::query()@942`, `cookie()@752`,
    `write()@726` + `ShitarabaBBS::write()@935`。
  - `post()@337` を非同期に再設計。`check()@387`（応答の error/cookie/angel 判定で確認ページ/エラーを識別）。
  - **2段階 Cookie**: 1回目 POST → 確認ページ（Cookie 要求）を `check()` で検知 → 受領 Cookie を載せて再 POST。
    **同一 QNAM の cookie jar で Cookie は半自動保持**されうるが、「確認検知→再送」トリガは明示実装する（M3.1 の Cookie 制御に依存）。
- **テスト**: query 文字列（submit/FROM/mail/MESSAGE/bbs/key/time, エンコ別）、Cookie ヘッダ、`check()` の
  正常/cookie確認/error 判定。
- **完了基準**: 書込リクエスト組み立てと確認再送ロジックが単体で緑（実投稿は M3.8 手動）。

### M3.6 — UI: 下部レス入力欄 + 簡易レス一覧ペイン（任意表示）
- **成果物**:
  - `ui` 層（現状 `src/app`）に**着脱可能な簡易レス一覧ペイン**（スクロール可能テキスト）。`BbsSession` の
    ResInfo 列を表示、定期更新で追記。
  - 下部バーに**1行レス入力欄 + 送信**（→ M3.5 の post）。設計 Q4(B)/Q5(A) に従い操作系は下部バー。
- **テスト**: 手動中心（UI）。表示更新ロジックに薄い単体があれば可。
- **完了基準**: ペイン表示/非表示、新着追記、送信ボタンが動く。

### M3.7 — UI: hover レス・ポップアップ（トップレベル半透明）
- **成果物**: `ui::ResPopup`（`Qt::Tool|FramelessWindowHint` + `WA_TranslucentBackground` +
  `WA_ShowWithoutActivating`、`paintEvent` 自前描画、`wheelEvent` で過去レス遡行）。移植元の挙動は
  PCRPlayer `ResDlg`/`DisplayResDlg` 系（参照のみ、コード移植不要）。アンカー hover で被参照レス表示（M3.4 `getExtract(ref)`）。
- **完了基準**: タイトル hover で直近レスをカーソル位置に半透明表示、ホイールで遡行、レス欄のフォーカスを奪わない。

### M3.8 — SessionController 結合 + HTTPS / 文字コード実機確認
- **成果物**:
  - `app::SessionController`（`session_controller.h:38` の「M3: contact を bbs.init()」）に `BbsSession` を結線。
    `commandline && contact` のとき自動接続（設計の CLI 互換）。dat ポーリングタイマ、watchdog/peercast と共存、
    エラーはステータスバーへ。
  - **実機確認**: ローカル/実板でしたらば(EUC-JP)・jpnkn(Shift_JIS) の取得/表示/書込、HTTPS 送受信、
    文字コード往復を最終確認（親文書「留意・リスク」: したらば=EUC-JP は要検証）。
- **完了基準**: 3引数起動で映像 + BBS 実況が end-to-end で成立（M3 完了 = 差別化価値の立ち上がり）。

---

## 進捗

- [x] M3.0  モジュール土台 / URL 解析 / 文字コード / フィクスチャ
- [x] M3.1  net 拡張（POST/ヘッダ/Cookie/条件付きGET）
- [x] M3.2  setting/subject パース
- [x] M3.3  dat パース + 抽出
- [x] M3.4  BbsSession オーケストレーション
- [ ] M3.5  書き込み（2段階 Cookie）
- [ ] M3.6  UI レス入力欄 + 一覧ペイン
- [ ] M3.7  UI hover ポップアップ
- [ ] M3.8  結合 + 実機確認
