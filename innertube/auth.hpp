#ifndef AUTH_HPP
#define AUTH_HPP
#include "objects/InnertubeContext.h"
#include <chrono>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QWidget>

class InnertubeAuthStore : public QObject
{
    Q_OBJECT
public:
    bool populated = false;
    QString apisid, hsid, sapisid, sid, ssid, visitorInfo;
    explicit InnertubeAuthStore(QObject* parent = nullptr) : QObject(parent) {}

    void authenticate(InnertubeContext& context)
    {
        QWidget authWindow = QWidget();
        authWindow.setFixedSize(authWindow.size());
        authWindow.setWindowTitle("YouTube Login");
        authWindow.show();

        QWebEngineView view(&authWindow);
        view.setPage(new QWebEnginePage);
        view.setFixedSize(authWindow.size());
        view.load(QUrl("https://accounts.google.com/ServiceLogin/signinchooser?service=youtube&uilel=3&passive=true&continue=https%3A%2F%2Fwww.youtube.com%2Fsignin%3Faction_handle_signin%3Dtrue%26app%3Ddesktop%26hl%3Den%26next%3Dhttps%253A%252F%252Fwww.youtube.com%252F&hl=en&ec=65620&flowName=GlifWebSignIn&flowEntry=ServiceLogin"));
        view.show();

        QObject::connect(QWebEngineProfile::defaultProfile()->cookieStore(), &QWebEngineCookieStore::cookieAdded, this, &InnertubeAuthStore::cookieAdded);
        QEventLoop loop;
        QObject::connect(this, &InnertubeAuthStore::gotSids, &loop, &QEventLoop::quit);
        loop.exec();

        delete view.page();
        QByteArray visitorData("\x0a" + uleb128(visitorInfo.length()) + visitorInfo.toLatin1() + "\x28" + uleb128(ts()));
        context.client.visitorData = visitorData.toBase64().toPercentEncoding();
    }

    QString generateSAPISIDHash() const
    {
        const QString fmt = QStringLiteral("%1 %2 https://www.youtube.com").arg(ts()).arg(sapisid);
        const QString hash(QCryptographicHash::hash(fmt.toUtf8(), QCryptographicHash::Sha1).toHex());
        return QStringLiteral("SAPISIDHASH %1_%2").arg(ts()).arg(hash);
    }

    QString getNecessaryLoginCookies() const
    {
        return QStringLiteral("SID=%1; HSID=%2; SSID=%3; SAPISID=%4; APISID=%5").arg(sid, hsid, ssid, sapisid, apisid);
    }
private:
    int ts() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    QByteArray uleb128(uint64_t val)
    {
        uint8_t buf[128];
        size_t i = 0;
        do
        {
            if (i < 255)
            {
                uint8_t b = val & 0x7F;
                val >>= 7;
                if (val != 0)
                    b |= 0x80;
                buf[i++] = b;
            }
            else
            {
                return 0;
            }
        } while (val != 0);

        return QByteArray(reinterpret_cast<char*>(buf), i);
    }
signals:
    void gotSids();
private slots:
    void cookieAdded(const QNetworkCookie& cookie)
    {
        if (cookie.name() == "APISID")
            apisid = cookie.value();
        if (cookie.name() == "HSID")
            hsid = cookie.value();
        if (cookie.name() == "SAPISID")
            sapisid = cookie.value();
        if (cookie.name() == "SID")
            sid = cookie.value();
        if (cookie.name() == "SSID")
            ssid = cookie.value();
        if (cookie.name() == "VISITOR_INFO1_LIVE")
            visitorInfo = cookie.value();

        if (!apisid.isEmpty() && !hsid.isEmpty() && !sapisid.isEmpty() && !sid.isEmpty() && !ssid.isEmpty() && !visitorInfo.isEmpty())
        {
            populated = true;
            emit gotSids();
        }
    }
};

#endif // AUTH_HPP
