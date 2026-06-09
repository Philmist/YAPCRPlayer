#include "bbs/charset.h"

// Shift-JIS / EUC-JP の変換を Qt6 Core5Compat の QTextCodec で実装する。
// QStringConverter (QtCore) はこの Qt ビルドでは組み込みエンコーディングのみ対応しており
// SJIS/EUC-JP には非対応なため、Core5Compat の QTextCodec を使用する。
// QTextCodec は Qt7 で廃止される予定だが Qt6 の過渡期には最も確実な選択。

#include <QTextCodec>

namespace yapcr::bbs {

namespace {

// Charset → QTextCodec に渡すコーデック名を返す。
const char* codecName(Charset c)
{
    switch (c) {
    case Charset::ShiftJis: return "Shift-JIS";
    case Charset::EucJp:    return "EUC-JP";
    }
    return "Shift-JIS";  // fallback（未到達）
}

}  // namespace

bool isCharsetSupported(Charset c)
{
    return QTextCodec::codecForName(codecName(c)) != nullptr;
}

QByteArray encodeTo(const QString& s, Charset c)
{
    QTextCodec* codec = QTextCodec::codecForName(codecName(c));
    if (!codec) { return {}; }
    return codec->fromUnicode(s);
}

QString decodeFrom(const QByteArray& bytes, Charset c, bool* ok)
{
    QTextCodec* codec = QTextCodec::codecForName(codecName(c));
    if (!codec) {
        if (ok) { *ok = false; }
        return {};
    }
    QTextCodec::ConverterState state;
    QString result = codec->toUnicode(bytes.constData(), bytes.size(), &state);
    if (ok) {
        *ok = (state.invalidChars == 0);
    }
    return result;
}

}  // namespace yapcr::bbs
