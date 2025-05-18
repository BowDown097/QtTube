#include "emojilabel.h"

EmojiLabel::EmojiLabel(const QStringList& shortcuts, const QStringList& searchTerms, const QString& image, QWidget* parent)
    : TubeLabel(parent), m_primaryShortcut(shortcuts[0]), m_searchTerms(shortcuts + searchTerms)
{
    setClickable(true);
    setFixedSize(24, 24);
    setImage(image, TubeLabel::Cached);
    setScaledContents(true);
    setToolTip(m_primaryShortcut);
}
