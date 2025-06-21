#pragma once
#include "qttube-plugin/objects/emoji.h"
#include "tubelabel.h"

class EmojiLabel : public TubeLabel
{
    Q_OBJECT
public:
    explicit EmojiLabel(const QtTube::Emoji& data, QWidget* parent = nullptr);
    const QtTube::Emoji& data() const { return m_data; }
    const QStringList& searchTerms() const { return m_searchTerms; }
private:
    const QtTube::Emoji& m_data;
    QStringList m_searchTerms;
};
