#pragma once

#include "bbs/models.h"

#include <QByteArray>
#include <QString>

namespace yapcr::bbs {

// QString → Charset で指定したエンコーディングのバイト列に変換する。
// POST body 生成など、バイト列が必要な場面で使用。
// 移植元の urlencode(std::wstring, Mlang::CODE) の入力変換部に相当。
QByteArray encodeTo(const QString& s, Charset c);

// バイト列 → Charset で指定したエンコーディングから QString へデコードする。
// ok が非 null の場合、デコードエラーの有無（true=正常、false=エラーあり）を設定する。
// 無効バイトがあっても黙置換した結果を返し、エラーは ok=false で通知する。
QString decodeFrom(const QByteArray& bytes, Charset c, bool* ok = nullptr);

// Charset に対応するコードページが実行環境で利用可能かを返す。
// Windows では CP51932(EUC-JP) が IsValidCodePage=false になる環境があり、
// その場合はしたらば(EUC-JP)の取得/デコードが動作しない。
// false が返る場合は M3.8 の実機確認で対処が必要。
bool isCharsetSupported(Charset c);

}  // namespace yapcr::bbs
