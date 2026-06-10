#include "snapshot_filename.h"

#include <QDateTime>

namespace yapcr::app {

QString snapshotFilename(const QDateTime& dt, SnapshotFormat fmt)
{
    // 移植元 PCRPlayer/Snapshot.cpp createSnapshotFilename() の命名規則:
    //   "%04d%02d%02d_%02d%02d%02d_%03d" → "yyyyMMdd_HHmmss_zzz"
    // QDateTime::toString の書式: yyyy=年, MM=月, dd=日, HH=24h時, mm=分, ss=秒, zzz=3桁msec
    QString name = dt.toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));

    switch (fmt) {
        case SnapshotFormat::Jpg: name += QStringLiteral(".jpg"); break;
        case SnapshotFormat::Bmp: name += QStringLiteral(".bmp"); break;
        case SnapshotFormat::Png:
        default:                  name += QStringLiteral(".png"); break;
    }
    return name;
}

}  // namespace yapcr::app
