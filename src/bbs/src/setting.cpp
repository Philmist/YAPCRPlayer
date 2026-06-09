#include "bbs/setting.h"

#include <QHash>
#include <QStringList>

namespace yapcr::bbs {

SettingInfo parseSetting(const QString& text)
{
    SettingInfo info;
    QHash<QString, QString> map;

    // 行分割（\r\n / \n 両対応）
    const QStringList lines = text.split(u'\n');
    for (const QString& rawLine : lines) {
        // 行末 \r を除去
        const QString line = rawLine.endsWith(u'\r')
            ? rawLine.left(rawLine.size() - 1)
            : rawLine;

        // 最初の '=' で name/value に分割（non-greedy name, greedy value を再現）。
        // 移植元 parser(name,value)@592: (s1 = -+_) >> L'=' >> (s2 = +_) を regex_match。
        // QRegularExpression の anchor 罠を避けるため indexOf で等価実装。
        const int eq = line.indexOf(u'=');
        if (eq <= 0) { continue; }  // '=' 無し or 空 name はスキップ

        const QString name  = line.left(eq);
        const QString value = line.mid(eq + 1);
        if (value.isEmpty()) { continue; }  // 移植元の +_ (1字以上) に相当

        map.insert(name, value);
    }

    // BBS_TITLE
    {
        auto it = map.find(QStringLiteral("BBS_TITLE"));
        if (it != map.end()) {
            info.title = it.value();
        }
    }

    // BBS_NONAME_NAME
    {
        auto it = map.find(QStringLiteral("BBS_NONAME_NAME"));
        if (it != map.end()) {
            info.noname = it.value();
        }
    }

    // BBS_THREAD_STOP（数値化失敗 or 0 以下なら既定 1000 維持）
    {
        auto it = map.find(QStringLiteral("BBS_THREAD_STOP"));
        if (it != map.end()) {
            bool ok = false;
            const int n = it.value().toInt(&ok);
            if (ok && n > 0) {
                info.stop = n;
            }
        }
    }

    return info;
}

}  // namespace yapcr::bbs
