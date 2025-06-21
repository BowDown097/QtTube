#include "emojilabel.h"

EmojiLabel::EmojiLabel(const QtTube::Emoji& data, QWidget* parent)
    : TubeLabel(parent), m_data(data), m_searchTerms(data.shortcodes + data.emoticons)
{
    setClickable(true);
    setFixedSize(24, 24);
    setImage(data.url, TubeLabel::Cached);
    setScaledContents(true);
    setToolTip(data.shortcodes.front());
}
