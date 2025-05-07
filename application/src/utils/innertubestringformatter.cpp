#include "innertubestringformatter.h"
#include "utils/httputils.h"
#include <QLabel>
#include <QUrlQuery>

constexpr QLatin1String EmojiPlaceholder("<img src='data:%1;base64,%2' width='20' height='20'>");
constexpr int MaxUrlLength = 37;

QString InnertubeStringFormatter::formatSimple(const InnertubeObjects::InnertubeString& str, bool useLinkText)
{
    QString out;

    for (const InnertubeObjects::InnertubeRun& run : str.runs)
    {
        if (run.navigationEndpoint.isObject())
            insertNavigationEndpoint(out, run.navigationEndpoint, run.text, useLinkText);
        else
            out += run.text.toHtmlEscaped().replace('\n', "<br>");
    }

    return out;
}

void InnertubeStringFormatter::insertEmoji(const QJsonValue& emoji)
{
    ++m_pendingEmojis;

    const QString url = emoji["image"]["thumbnails"][0]["url"].toString();
    const QString placeholder = EmojiPlaceholder.arg(
        emoji["shortcuts"][0].toString() + emoji["searchTerms"][0].toString() + url);

    if (!m_data.contains(placeholder))
    {
        HttpReply* emojiReply = HttpUtils::cachedInstance().get(url);
        connect(emojiReply, &HttpReply::finished, this,
                std::bind_front(&InnertubeStringFormatter::replaceEmojiPlaceholder, this, placeholder));
    }

    m_data += placeholder;
}

void InnertubeStringFormatter::insertNavigationEndpoint(
    QString& data, const QJsonValue& navigationEndpoint, QString text, bool useLinkText)
{
    QString href;

    if (const QJsonValue urlEndpoint = navigationEndpoint["urlEndpoint"]; urlEndpoint.isObject())
    {
        const QString urlStr = urlEndpoint["url"].toString();
        const QUrl url(urlStr);
        const QUrlQuery query(url);

        if (query.hasQueryItem("q"))
        {
            href = QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8());
            if (!useLinkText)
                text = href;
        }
        else if (urlStr.contains("youtube.com/channel"))
        {
            href = url.path();
            if (!useLinkText)
                text = urlStr;
        }
        else
        {
            href = urlStr;
            if (!useLinkText)
                text = href;
        }

        if (!useLinkText)
            truncateUrlString(text, false);
    }
    else if (const QJsonValue browseEndpoint = navigationEndpoint["browseEndpoint"]; browseEndpoint.isObject())
    {
        const QString browseId = browseEndpoint["browseId"].toString();
        if (browseId.startsWith("UC"))
        {
            text.replace(text.indexOf('/'), 1, "").replace("/xc2/xa0", "");
            if (text[0] != '@')
                text.prepend('@');
            href = "/channel/" + browseId;
        }
        else if (browseId.startsWith("FE"))
        {
            href = navigationEndpoint["commandMetadata"]["webCommandMetadata"]["url"].toString();
            if (!useLinkText)
            {
                text = href;
                truncateUrlString(text, true);
            }
        }
    }
    else
    {
        href = navigationEndpoint["commandMetadata"]["webCommandMetadata"]["url"].toString();
        if (const QJsonValue watchEndpoint = navigationEndpoint["watchEndpoint"]; watchEndpoint.isObject())
        {
            if (!watchEndpoint["continuePlayback"].isBool())
            {
                if (!useLinkText)
                {
                    text = "https://www.youtube.com" + href;
                }
            }
            else
            {
                href += "&continuePlayback=1";
            }
        }

        if (!useLinkText)
            truncateUrlString(text, false);
    }

    data += QStringLiteral("<a href=\"%1\">%2</a>").arg(href, text);
}

void InnertubeStringFormatter::replaceEmojiPlaceholder(const QString& placeholder, const HttpReply& reply)
{
    m_pendingEmojis -= m_data.count(placeholder);
    m_data.replace(placeholder, EmojiPlaceholder.arg(reply.header("content-type"), reply.body().toBase64()));
    emit readyRead(m_data);

    if (m_pendingEmojis == 0)
        emit finished();
}

void InnertubeStringFormatter::setData(const InnertubeObjects::InnertubeString& str, bool useLinkText)
{
    for (const InnertubeObjects::InnertubeRun& run : str.runs)
    {
        if (run.emoji.isObject())
            insertEmoji(run.emoji);
        else if (run.navigationEndpoint.isObject())
            insertNavigationEndpoint(m_data, run.navigationEndpoint, run.text, useLinkText);
        else
            m_data += run.text.toHtmlEscaped().replace('\n', "<br>");
        emit readyRead(m_data);
    }

    if (m_pendingEmojis == 0)
        emit finished();
}

void InnertubeStringFormatter::truncateUrlString(QString& url, bool prefix)
{
    if (prefix)
        url.prepend("https://www.youtube.com");
    if (url.length() > MaxUrlLength)
        url = url.left(MaxUrlLength) + "...";
}
