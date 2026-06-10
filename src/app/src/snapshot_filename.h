#pragma once

#include <QString>

class QDateTime;

namespace yapcr::app {

// ---- M4.4: スナップショットファイル名生成（UI 非依存・単体テスト対象） ---------
//
// このファイルの関数は Qt ウィジェット・mpv に依存しない純粋な関数であり、
// 単体テスト（tests/tst_display）でカバーする。
//
// 移植元: PCRPlayer/Snapshot.cpp createSnapshotFilename()
//   命名規則: yyyyMMdd_HHmmss_zzz + 拡張子（例: 20260610_153012_004.png）

enum class SnapshotFormat {
    Png,  // .png（既定）
    Jpg,  // .jpg
    Bmp,  // .bmp
};

// 日時ベースのスナップショットファイル名を返す（ディレクトリ部は含まない）。
//   形式: "yyyyMMdd_HHmmss_zzz" + 拡張子
//   例  : 20260610_153012_004.png
//   zzz  = 3 桁ゼロ埋めミリ秒、HH = 24 時間表記
//
// 時刻を引数で受け取る設計にすることで、固定時刻を渡して決定的にテストできる。
// 実呼び出し側は QDateTime::currentDateTime() を渡す。
QString snapshotFilename(const QDateTime& dt, SnapshotFormat fmt = SnapshotFormat::Png);

}  // namespace yapcr::app
