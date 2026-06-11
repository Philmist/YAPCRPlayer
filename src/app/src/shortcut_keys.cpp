#include "shortcut_keys.h"

#include <QDebug>
#include <QKeySequence>

namespace yapcr::app {

// ---- テンキー Num プレフィックス変換テーブル ----
// "Num<suffix>" ⇄ Qt::Key + Qt::KeypadModifier
struct NumEntry { const char* suffix; int key; };

static constexpr NumEntry kNumTable[] = {
    {"0",     Qt::Key_0},
    {"1",     Qt::Key_1},
    {"2",     Qt::Key_2},
    {"3",     Qt::Key_3},
    {"4",     Qt::Key_4},
    {"5",     Qt::Key_5},
    {"6",     Qt::Key_6},
    {"7",     Qt::Key_7},
    {"8",     Qt::Key_8},
    {"9",     Qt::Key_9},
    {"+",     Qt::Key_Plus},
    {"-",     Qt::Key_Minus},
    {"*",     Qt::Key_Asterisk},
    {"/",     Qt::Key_Slash},
    {".",     Qt::Key_Period},
    {"Enter", Qt::Key_Return},
    {"Del",   Qt::Key_Delete},
    {"Ins",   Qt::Key_Insert},
};

// ---- parseKeyChord ----

std::optional<KeyChord> parseKeyChord(const QString& s)
{
    if (s.isEmpty()) { return std::nullopt; }

    // テンキー: "Num" プレフィックス（大文字小文字不問）
    if (s.startsWith(QStringLiteral("Num"), Qt::CaseInsensitive)) {
        const QString suffix = s.mid(3);
        for (const auto& e : kNumTable) {
            if (suffix.compare(QLatin1String(e.suffix), Qt::CaseInsensitive) == 0) {
                return KeyChord{e.key, Qt::KeypadModifier};
            }
        }
        qWarning() << "[shortcut] 未知の Num テンキーキー:" << s;
        return std::nullopt;
    }

    // 通常キー: QKeySequence::fromString で解析
    const QKeySequence seq = QKeySequence::fromString(s, QKeySequence::PortableText);
    if (seq.isEmpty()) {
        qWarning() << "[shortcut] キー文字列を解析できません:" << s;
        return std::nullopt;
    }
    const QKeyCombination kc = seq[0];
    const int key             = kc.key();
    if (key == Qt::Key_unknown || key == 0) {
        qWarning() << "[shortcut] 解析したがキー不明:" << s;
        return std::nullopt;
    }
    return KeyChord{key, kc.keyboardModifiers()};
}

// ---- keyChordToString ----

QString keyChordToString(const KeyChord& c)
{
    if (c.key == 0) { return {}; }

    // テンキー逆引き
    if (c.mods & Qt::KeypadModifier) {
        for (const auto& e : kNumTable) {
            if (c.key == e.key) {
                return QStringLiteral("Num") + QLatin1String(e.suffix);
            }
        }
        // 未知テンキー → KeypadModifier を除いて文字列化（ベストエフォート）
    }

    // QKeySequence 経由で文字列化
    // KeypadModifier は QKeySequence では表現できないため除去してから変換する
    const Qt::KeyboardModifiers cleanMods = c.mods & ~Qt::KeypadModifier;
    const QKeySequence seq(QKeyCombination(cleanMods, static_cast<Qt::Key>(c.key)));
    return seq.toString(QKeySequence::PortableText);
}

// ---- keyChordFromEvent ----

KeyChord keyChordFromEvent(const QKeyEvent* ev)
{
    const int key = ev->key();
    // 修飾キー単体押下（Shift/Ctrl/Alt/Meta）はアクションに結びつけない
    switch (key) {
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
        case Qt::Key_AltGr:
        case Qt::Key_CapsLock:
        case Qt::Key_unknown:
        case 0:
            return KeyChord{};
        default:
            break;
    }
    return KeyChord{key, ev->modifiers()};
}

// ---- buildReverseMap ----

QHash<KeyChord, ActionId> buildReverseMap(const QMap<ActionId, QStringList>& keyMap)
{
    QHash<KeyChord, ActionId> rev;
    for (auto it = keyMap.constBegin(); it != keyMap.constEnd(); ++it) {
        const ActionId id = it.key();
        for (const QString& s : it.value()) {
            const auto chord = parseKeyChord(s);
            if (!chord) {
                qWarning() << "[shortcut] buildReverseMap: キー解析失敗（スキップ）:" << s;
                continue;
            }
            if (rev.contains(*chord)) {
                qWarning() << "[shortcut] buildReverseMap: キー衝突（後勝ち）:" << s;
            }
            rev.insert(*chord, id);
        }
    }
    return rev;
}

} // namespace yapcr::app
