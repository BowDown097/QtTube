#pragma once
#include "innertube/objects/innertubestring.h"
#include <QObject>

class HttpReply;

class InnertubeStringFormatter : public QObject
{
    Q_OBJECT
public:
    explicit InnertubeStringFormatter(QObject* parent = nullptr) : QObject(parent) {}

    const QString& data() const { return m_data; }
    void setData(const InnertubeObjects::InnertubeString& str, bool useLinkText);

    // no support for emojis!!
    static QString formatSimple(const InnertubeObjects::InnertubeString& str, bool useLinkText);
private:
    QString m_data;
    uint16_t m_pendingEmojis{};

    void insertEmoji(const QJsonValue& emoji);
    static void insertNavigationEndpoint(QString& data, const QJsonValue& navigationEndpoint, QString text, bool useLinkText);
    static void truncateUrlString(QString& url, bool prefix);
private slots:
    void replaceEmojiPlaceholder(const QString& placeholder, const HttpReply& reply);
signals:
    void finished();
    void readyRead(const QString& data);
};
