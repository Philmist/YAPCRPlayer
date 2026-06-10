#include "display_modes.h"

namespace yapcr::app {

QList<MpvProp> fitModeToMpvProps(FitMode mode, int aspectX, int aspectY)
{
    // 4 プロパティの既定値（Inscribe 相当）
    QString keepaspect        = QStringLiteral("yes");
    QString panscan           = QStringLiteral("0.0");
    QString videoUnscaled     = QStringLiteral("no");
    QString videoAspectOverride = QStringLiteral("-1");

    switch (mode) {
    case FitMode::Inscribe:
        // 既定値のまま
        break;

    case FitMode::Stretch:
        keepaspect = QStringLiteral("no");
        break;

    case FitMode::Fill:
        panscan = QStringLiteral("1.0");
        break;

    case FitMode::Unscaled:
        videoUnscaled = QStringLiteral("yes");
        break;

    case FitMode::AspectOverride:
        if (aspectX > 0 && aspectY > 0) {
            videoAspectOverride =
                QStringLiteral("%1:%2").arg(aspectX).arg(aspectY);
        }
        // 不正値は -1 のまま（内接相当にフォールバック）
        break;
    }

    return {
        { QStringLiteral("keepaspect"),             keepaspect        },
        { QStringLiteral("panscan"),                panscan           },
        { QStringLiteral("video-unscaled"),         videoUnscaled     },
        { QStringLiteral("video-aspect-override"),  videoAspectOverride },
    };
}

QList<AspectPreset> aspectPresets()
{
    // M5: config化
    return {
        { "16:9",    16,   9 },
        { "4:3",      4,   3 },
        { "5:4",      5,   4 },
        { "2.35:1", 235, 100 },
        { "1.85:1", 185, 100 },
    };
}

}  // namespace yapcr::app
