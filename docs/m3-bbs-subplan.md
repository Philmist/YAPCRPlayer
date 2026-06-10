# M3（BBS）サブ分割計画

最終更新: 2026-06-09（M3.9 追加）
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
  - `src/bbs/include/bbs/post.h` + `src/bbs/src/post.cpp`（純関数群）:
    - `postUrlEncode(s, charset)`: 安全集合 `[0-9a-zA-Z*-._]`（移植元と一致）。
    - `buildPostBody(board, key, type, name, mail, message, epochSec)`: jpnkn/したらば 切替。
    - `buildCookieHeader(name, mail, charset, extra)`: NAME/MAIL + Set-Cookie 合成。
    - `classifyWriteResponse(html)`: `check()@387` を 3値（Ok/NeedCookie/Error）に拡張。
      全角→半角 `halfWidthFold()` 済み HTML を 2ch_X/title/本文で判定。
    - `nextPostAction(result, attempt, hasNewCookie)`: 再送状態機械（高々 1 回）。
  - `BbsSession` 拡張: `post()` / `postSucceeded()` / `postFailed(reason)` を追加。
    `postClient_`（4本目）で再送も同一インスタンスを再利用。
  - write URL は現状（`board_url.cpp`）のまま正しい（変更不要）。
- **設計注記**:
  - 2段階 Cookie 確認は **2ch 派生掲示板（ex0ch 系）特有**。したらば/bbs.jpnkn.com には存在しないが、
    2ch 互換掲示板に書けるよう処理は実装。
  - jpnkn 拡張エラー: `ＥＲＲＯＲ：…利用認証が必要…`（Cloudflare Turnstile）/ `COOKIE got burnt` を
    本文検索で検出（移植元 check() では漏れる）。トークン再入力フローは M3.6/M3.8 に委譲。
  - submit 値 = u"書き込む"（移植元の化けバイト `L"��������"` は Shift-JIS `8f91 82ab 8d9e 82de`）。
- **テスト**: 25 件追加（計 117 PASS / 0 FAIL）。
  `halfWidthFold` / `postUrlEncode`（Shift-JIS/EUC-JP）/ `buildPostBody`（jpnkn/したらば）/
  `buildCookieHeader` / `classifyWriteResponse`（7分岐）/ `nextPostAction`（5ケース）/
  `BbsSession::post` key 未設定同期パス（2ケース）。
- **完了基準**: 書込リクエスト組み立てと確認再送ロジックが単体で緑（実投稿は M3.8 手動）。 **✓ 完了**

### M3.6 — UI: 下部レス入力欄 + 簡易レス一覧ペイン（任意表示）
- **成果物**:
  - `ui` 層（現状 `src/app`）に**着脱可能な簡易レス一覧ペイン**（スクロール可能テキスト）。`BbsSession` の
    ResInfo 列を表示、定期更新で追記。
  - 下部バーに**1行レス入力欄 + 送信**（→ M3.5 の post）。設計 Q4(B)/Q5(A) に従い操作系は下部バー。
- **テスト**: 手動中心（UI）。表示更新ロジックに薄い単体があれば可。
- **完了基準**: ペイン表示/非表示、新着追記、送信ボタンが動く。

### M3.7 — UI: hover レス・ポップアップ（トップレベル半透明）
- **成果物（本命）**:
  - `app::BoardTitleBar`（`src/app/src/board_title_bar.{h,cpp}`）:
    映像下部の掲示板タイトル帯。`setInfo(title, count)` で `[ <title> ]( <count> )` 形式の帯を更新。
    `enterEvent`/`mouseMoveEvent` で `hovered(QPoint)` を emit し、`leaveEvent` で `left()` を emit する。
  - `app::ResPopup::showRecent(all, anchorGlobal)`:
    直近 kWindow=10 件をスタック描画する Recent モードを追加。ホイール上回しで過去レスへ遡行、
    下回しで最新に戻る。帯直上のカーソル位置に展開し、ポップアップ自身を離れると自動 hide。
  - `SessionController` への追加:
    `bbsRecent(n)`: 末尾 n 件を `DatStore::byRange` で取得。
    `bbsThreadTitle()`: `threadTitle()`（空なら `boardTitle()`）を返す。
    シグナル `bbsThreadInfoChanged(title, count)`: `onBbsDatLoaded` 末尾で emit。
  - `MainWindow` 配線: `centralWidget` を `QVBoxLayout`（映像+タイトル帯）コンテナ化。
    `bbsThreadInfoChanged` → `boardTitleBar_->setInfo`; `hovered` → `showRecent(bbsRecent(40))`;
    `left` → カーソルがポップアップ外なら `hidePopup()`。
- **成果物（副次機能）**: `app::ResListPane` の右ドック内アンカーポップアップ（`showAt`）:
  レスヘッダ行 hover → byRef（被参照レス群）、`>>N` hover → byRange（指定レス）。
  `WA_TranslucentBackground`+`WA_ShowWithoutActivating` で右ドック内に浮かぶ。引き続き有効。
- **完了基準**: 映像下のタイトル帯に hover で直近レスが映像上に半透明スタック表示、ホイールで遡行、
  レス欄のフォーカスを奪わない。ビルド成功・既存ユニットテスト 4/4 PASS。 **✓ 完了**

### M3.8 — SessionController 結合 + HTTPS / 文字コード実機確認
- **成果物**:
  - `app::SessionController` に `BbsSession` を完全結線。
    `commandline && contact` のとき `start()` 末尾で `bbsRefresh()` を自動呼び出し（CLI 互換）。
  - **dat ポーリングタイマ（独立管理）**: `bbsPollTimer_`（`kBbsPollIntervalMs = 15 秒`）。
    `bbsRefresh()` のたびにリセット起動。再 `start()` 時は `stopBbsPolling()` で停止（二重起動防止）。
    peercast の `setupPeerCast/teardownPeerCast` とは独立（contact が `/pls/` 以外でも動く）。
  - **書込メール既定 = `sage`、名前 = 空**。sage on/off 切替・名前入力は M5 (config) で実装。
  - **実機確認**: したらば(EUC-JP)・jpnkn(Shift_JIS) の取得/表示/書込、HTTPS 送受信、文字コード往復確認。
- **仕様メモ（M5 へ持ち越し）**: メール既定=sage / 名前=空 / sage切替はメニューから（M5 config）。
- **完了基準**: 3引数起動で映像 + BBS 実況が end-to-end で成立。  **✓ 完了**

### M3.9 — スレッド自動追従（板URL→最速スレ解決 + 満了時の自動次スレ移動）
- **背景**: PCRPlayer の `BBSOperator::getFastest()`(BBSOperator.cpp:515) 相当機能。「板URLから最新スレ取得」と
  「満了スレから次スレへ自動移動」は**同一プリミティブを 2 箇所から呼ぶ構造**であり、M3 の実況価値に直結する。
- **成果物**:
  - **`ThreadInfo.speed` の充填（前段・硬い前提）**: `calcSpeed(count,key)`(BaseBBS.cpp:10 を直訳移植)を
    subject パース直後に適用。`speed = days>0 ? count/days : 0.0`（`days=(now−key)/86400`）。
    ※ M3.2/M3.4 が「速度算出は後続の責務」として持ち越した負債を本マイルストーンで回収する。
  - **`BbsSession::selectFastest(ThreadInfo& out) const`** プリミティブ:
    - `subject_` 一覧から「`count < stop` かつ `key > 現スレ key` かつ `speed` 最大」のスレを選ぶ。
      （board-only 起動時は現スレ key=0 なので「より新しいスレ全体から最速」を選ぶ）
    - `count >= stop` フィルタは PCRPlayer から**簡略化せず忠実に移植**（満了済みスレを除外）。
    - 該当無しなら false。
  - **呼び出し 2 箇所**:
    1. **cold-start（板URL→最速スレ解決）**: `bbsRefresh()` 内で `loc_.thread.key` が空のとき、
       `loadSubject` 完了後に `selectFastest`→`change(key)`→`loadDat` を連鎖。
    2. **poll-time（満了時の自動次スレ移動）**: M3.8 のポーリングで `isStop()` 検知時に
       `loadSubject`→`selectFastest`→`change(key)`→ペインクリア＆`loadDat`。
  - **stop ゲート（重要）**: `selectFastest` と `isStop()` はいずれも `stop`(setting.txt 由来) に依存する。
    `stop` の既定値 0 だと `isStop()`(= `count >= 0`) が常時 true になり毎ポーリングで誤発火する。
    → **自動切替は `stop > 0`（setting 実ロード済み）をゲート条件とする**。
    移植元 PCRPlayer の `if(getStop(stop))` ＋ `count >= stop` 判定と対応。
  - **制御フラグ**: M3 では move/board/thread 相当を既定 ON のハードコード。config 化は `// M5:` で残す。
  - **UI 無改修**: `ResListPane`/`ResInputBar` はスレ切替後もペインクリア＋新スレ追記で再利用できる。
- **テスト**: `calcSpeed`（既知 count/key→レス/日）の単体、`selectFastest`（満了除外・現key超え・speed最大・
  該当無し の各ケース）の単体。手動 E2E: 板URL起動で実況スレに着地、満了スレで次スレへ自動移動、
  `stop==0` 時に誤発火しないこと。
- **非スコープ**: お気に入り/履歴、複数板巡回、`block`（更新ブロック）、config 化（M5）、Range 差分取得。
- **完了基準**: 板URL起動で最速スレに自動着地、満了スレから次スレへポーリング中に自動移動。
  M3 全体完了＝M3 の差別化価値が end-to-end で成立。

---

## 進捗

- [x] M3.0  モジュール土台 / URL 解析 / 文字コード / フィクスチャ
- [x] M3.1  net 拡張（POST/ヘッダ/Cookie/条件付きGET）
- [x] M3.2  setting/subject パース
- [x] M3.3  dat パース + 抽出
- [x] M3.4  BbsSession オーケストレーション
- [x] M3.5  書き込み（2段階 Cookie）
- [x] M3.6  UI レス入力欄 + 一覧ペイン
- [x] M3.7  UI hover ポップアップ
- [x] M3.8  結合 + 実機確認
- [x] M3.9  スレッド自動追従（板URL→最速スレ + 満了時 自動次スレ移動）
