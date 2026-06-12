#include "about_dialog.h"

#include "common/version.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace yapcr::app {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("バージョン情報 — %1")
                       .arg(QString::fromLatin1(common::appName())));
    setMinimumWidth(520);
    setMinimumHeight(340);

    // ---- タイトル行 ----
    auto* title = new QLabel(
        QStringLiteral("<b>%1</b>  %2")
            .arg(QString::fromLatin1(common::appName()),
                 QString::fromLatin1(common::appVersion())),
        this);
    title->setTextFormat(Qt::RichText);

    // ---- 本文（スクロール可能なリッチテキスト）----
    auto* body = new QTextBrowser(this);
    body->setOpenExternalLinks(true);
    body->setHtml(QStringLiteral(
        "<p>YAPCRPlayer は <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">"
        "GNU General Public License v3 以降</a> の条件で配布されるフリーソフトウェアです。"
        "あなたはこのソフトウェアを配布・改変する自由を持ちます（GPL の条件のもとで）。"
        "このソフトウェアには一切の保証がありません。</p>"

        "<hr/>"

        "<p><b>派生元（原著作物）:</b><br/>"
        "本ソフトウェアは <a href=\"http://pecatv.s25.xrea.com/\">PCRPlayer</a> "
        "を移植・再実装した派生著作物です。<br/>"
        "Copyright &copy; 2009&ndash;2011 PeerCast Station Project, kasahara &mdash; "
        "GNU General Public License</p>"

        "<hr/>"

        "<p><b>対応ソースの入手先（CORRESPONDING-SOURCE.md 参照）:</b><br/>"
        "YAPCRPlayer 本体: <i>公開 URL は CORRESPONDING-SOURCE.md を確認してください</i><br/>"
        "libmpv / FFmpeg: <a href=\"https://github.com/zhongfly/mpv-winbuild\">"
        "zhongfly/mpv-winbuild</a></p>"

        "<hr/>"

        "<p><b>サードパーティコンポーネント:</b></p>"
        "<ul>"
        "<li><b>mpv</b> — "
        "<a href=\"https://github.com/mpv-player/mpv\">github.com/mpv-player/mpv</a> "
        "— GPL-2.0-or-later / LGPL-2.1-or-later</li>"
        "<li><b>FFmpeg</b> — "
        "<a href=\"https://ffmpeg.org\">ffmpeg.org</a> "
        "— GPL-2.0-or-later / LGPL-2.1-or-later</li>"
        "<li><b>Qt6</b> — "
        "<a href=\"https://www.qt.io\">qt.io</a> "
        "— GPLv3 (Qt Widgets / Gui / Core / Network / Core5Compat)</li>"
        "<li><b>toml++</b> — "
        "<a href=\"https://github.com/marzer/tomlplusplus\">marzer/tomlplusplus</a> "
        "— MIT License</li>"
        "</ul>"

        "<p>詳細は配布物内の <code>licenses/THIRD-PARTY-NOTICES.md</code> を参照してください。</p>"
    ));

    // ---- ボタン ----
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    // ---- レイアウト ----
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(body, 1);
    layout->addWidget(buttons);
}

}  // namespace yapcr::app
