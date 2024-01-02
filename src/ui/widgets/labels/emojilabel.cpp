#include "emojilabel.h"
#include "http.h"

EmojiLabel::EmojiLabel(const QStringList& shortcuts, const QStringList& searchTerms, const QString& image, QWidget* parent)
    : m_primaryShortcut(shortcuts[0]), m_searchTerms(shortcuts + searchTerms), TubeLabel(parent)
{
    setClickable(true, false);
    setToolTip(m_primaryShortcut);

    HttpReply* reply = Http::instance().get(image);
    connect(reply, &HttpReply::finished, this, &EmojiLabel::setIcon);
}

void EmojiLabel::setIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    setPixmap(pixmap.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
