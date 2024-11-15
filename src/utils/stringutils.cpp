#include "stringutils.h"
#include "innertube/objects/innertubestring.h"
#include <QLocale>
#include <QUrlQuery>

#ifdef QTTUBE_HAS_ICU
#include <unicode/errorcode.h>
#include <unicode/numberformatter.h>
#endif

namespace StringUtils
{
    // Qt provides no way to do this (surprisingly), so we have to do it manually with ICU.
    #ifdef QTTUBE_HAS_ICU
    QString condensedNumericString(qint64 num, int precision)
    {
        using namespace icu::number;
        static const LocalizedNumberFormatter formatter = NumberFormatter::withLocale(icu::Locale::getDefault())
            .notation(Notation::compactShort()).precision(Precision::maxFraction(1));

        icu::ErrorCode errorCode;
        FormattedNumber formatted = formatter.formatInt(num, errorCode);
        if (errorCode.isFailure())
        {
            qDebug().noquote().nospace() << "Error condensing " << num << ": " << errorCode.errorName();
            return QString();
        }

        icu::UnicodeString formattedStr = formatted.toTempString(errorCode);
        if (errorCode.isFailure())
        {
            qDebug().noquote().nospace() << "Error condensing " << num << ": " << errorCode.errorName();
            return QString();
        }

        return QString(reinterpret_cast<const QChar*>(formattedStr.getBuffer()), formattedStr.length());
    }
    #endif

    QString extractDigits(const QString& str, bool useLocale, bool reserve)
    {
        QString out;
        if (reserve)
            out.reserve(str.size());

        for (QChar c : str)
            if (c.isDigit())
                out.append(c);

        out.squeeze();
        return useLocale ? QLocale::system().toString(out.toLongLong()) : out;
    }

    QString extractPath(const QString& str)
    {
        QString out;
        quint8 quoteCount{};

        for (QChar c : str)
        {
            if (c == '"' || c == '\'')
                ++quoteCount;
            else if (c.isSpace() && quoteCount != 1)
                break;
            else
                out += c;
        }

        return out;
    }

    QString innertubeStringToRichText(const InnertubeObjects::InnertubeString& istr, bool useLinkText)
    {
        QString out;

        for (const InnertubeObjects::InnertubeRun& run : istr.runs)
        {
            if (run.navigationEndpoint.isNull() || run.navigationEndpoint.isUndefined())
            {
                out += run.text;
                continue;
            }

            QString href, text = run.text;
            if (run.navigationEndpoint["urlEndpoint"].isObject())
            {
                QUrl url(run.navigationEndpoint["urlEndpoint"]["url"].toString());
                QUrlQuery query(url);

                if (query.hasQueryItem("q"))
                {
                    href = QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8());
                    if (!useLinkText)
                        text = href;
                }
                else if (QString urlStr = url.toString(); urlStr.contains("youtube.com/channel"))
                {
                    href = url.path();
                    if (!useLinkText)
                        text = urlStr;
                }
                else
                {
                    href = url.toString();
                    if (!useLinkText)
                        text = href;
                }

                if (!useLinkText)
                    truncateUrlString(text, false);
            }
            else if (run.navigationEndpoint["browseEndpoint"].isObject())
            {
                QString browseId = run.navigationEndpoint["browseEndpoint"]["browseId"].toString();
                QString code = browseId.left(2);

                if (code == "UC")
                {
                    text.replace(text.indexOf('/'), 1, "").replace("/xc2/xa0", "");
                    if (text[0] != '@')
                        text.prepend('@');
                    href = "/channel/" + browseId;
                }
                else if (code != "FE")
                {
                    href = run.navigationEndpoint["commandMetadata"]["webCommandMetadata"]["url"].toString();
                    if (!useLinkText)
                    {
                        text = href;
                        truncateUrlString(text, true);
                    }
                }
            }
            else
            {
                href = run.navigationEndpoint["commandMetadata"]["webCommandMetadata"]["url"].toString();
                if (run.navigationEndpoint["watchEndpoint"].isObject())
                {
                    if (!run.navigationEndpoint["watchEndpoint"]["continuePlayback"].isBool())
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

            out += QStringLiteral("<a href=\"%1\">%2</a>").arg(href, text);
        }

        return out.replace("\n", "<br>");
    }

    void truncateUrlString(QString& url, bool prefix)
    {
        if (prefix)
            url.prepend("https://www.youtube.com");
        if (url.length() > 37)
            url = url.left(37) + "...";
    }
}
