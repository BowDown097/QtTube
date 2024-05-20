#pragma once
#include "tubelabel.h"

class HttpReply;

class EmojiLabel : public TubeLabel
{
public:
    EmojiLabel(const QStringList& shortcuts, const QStringList& searchTerms, const QString& image, QWidget* parent = nullptr);
    EmojiLabel(const QString& shortcut, const QString& image, QWidget* parent = nullptr)
        : EmojiLabel(QStringList { shortcut }, QStringList { shortcut }, image, parent) {}

    QString primaryShortcut() const { return m_primaryShortcut; }
    QStringList searchTerms() const { return m_searchTerms; }
private:
    QString m_primaryShortcut;
    QStringList m_searchTerms;
private slots:
    void setIcon(const HttpReply& reply);
};
