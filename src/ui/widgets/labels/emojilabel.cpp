#include "emojilabel.h"
#include "http.h"
#include "utils/httputils.h"

EmojiLabel::EmojiLabel(const QStringList& shortcuts, const QStringList& searchTerms, const QString& image, QWidget* parent)
    : TubeLabel(parent), m_primaryShortcut(shortcuts[0]), m_searchTerms(shortcuts + searchTerms)
{
    setClickable(true);
    setFixedSize(24, 24);
    setScaledContents(true);
    setToolTip(m_primaryShortcut);

    HttpReply* reply = HttpUtils::cachedInstance().get(image);
    connect(reply, &HttpReply::finished, this, &EmojiLabel::setIcon);
}

void EmojiLabel::setIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    setPixmap(pixmap);
}
