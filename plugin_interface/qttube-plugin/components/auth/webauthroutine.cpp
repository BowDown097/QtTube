#include "webauthroutine.h"
#include <QMessageBox>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace QtTubePlugin
{
    void WebAuthRoutine::cookieAdded(const QNetworkCookie& cookie)
    {
        const QByteArray name = cookie.name();
        const QString domain = cookie.domain();
        const QString path = cookie.path();

        auto match = [&](const std::pair<SearchCookie, QByteArray>& p) {
            return (p.first.name == name) &&
                   (p.first.domain.isEmpty() || p.first.domain == domain) &&
                   (p.first.path.isEmpty() || p.first.path == path);
        };

        if (auto it = std::ranges::find_if(m_searchCookies, match); it != m_searchCookies.end())
        {
            it->second = cookie.value();
            onNewCookie(cookie.name(), cookie.value());
        }

        if (nothingToSearch())
            emit success();
    }

    bool WebAuthRoutine::nothingToSearch() const
    {
        auto allFound = [](auto&& r) { return std::ranges::none_of(r, [](const auto& p) { return p.second.isEmpty(); }); };
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        return allFound(m_searchCookies) && allFound(m_searchHeaders.asKeyValueRange());
    #else
        return allFound(m_searchCookies);
    #endif
    }

    QHash<QByteArray, QByteArray> WebAuthRoutine::searchCookies() const
    {
        QHash<QByteArray, QByteArray> out;
        out.reserve(m_searchCookies.size());

        for (const auto& [searchCookie, value] : m_searchCookies)
            out.emplace(searchCookie.name, value);

        return out;
    }

    void WebAuthRoutine::setSearchCookies(const QList<SearchCookie>& cookies)
    {
        for (const SearchCookie& searchCookie : cookies)
            m_searchCookies.emplaceBack(searchCookie, QByteArray());
    }

    void WebAuthRoutine::start()
    {
        if (m_url.isEmpty() || nothingToSearch())
        {
            QMessageBox::critical(nullptr, "Cannot Authenticate", "Authentication routine is misconfigured. Either a URL has not been set or there is nothing to capture.");
            return;
        }

        QWidget* loginWindow = new QWidget;
        loginWindow->setFixedSize(loginWindow->size());
        loginWindow->setWindowTitle("Login Window");
        loginWindow->show();

        QWebEngineView* view = new QWebEngineView(loginWindow);
        view->setFixedSize(loginWindow->size());

        QWebEngineProfile* profile = new QWebEngineProfile(view);
        QWebEnginePage* page = new QWebEnginePage(profile, view);
        view->setPage(page);

        view->load(m_url);
        view->show();

    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        QList<QByteArray> headerKeys;
        headerKeys.reserve(m_searchHeaders.size());
        for (auto it = m_searchHeaders.begin(); it != m_searchHeaders.end(); ++it)
            headerKeys.append(it.key());

        m_interceptor = new WebAuthRequestInterceptor(headerKeys, this);
        profile->setUrlRequestInterceptor(m_interceptor);
        connect(m_interceptor, &WebAuthRequestInterceptor::foundHeader, this, &WebAuthRoutine::foundHeader);
    #endif

        connect(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded, this, &WebAuthRoutine::cookieAdded);
        connect(this, &AuthRoutine::success, loginWindow, &QObject::deleteLater);
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    void WebAuthRoutine::foundHeader(const QByteArray& name, const QByteArray& value)
    {
        if (auto it = m_searchHeaders.find(name); it != m_searchHeaders.end())
        {
            it.value() = value;
            onNewHeader(name, value);
        }

        if (nothingToSearch())
            emit success();
    }

    void WebAuthRoutine::setSearchHeaders(const QList<QByteArray>& headers)
    {
        for (const QByteArray& header : headers)
            m_searchHeaders.emplace(header, QByteArray());
    }

    void WebAuthRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
    {
        if (!m_searchHeaders.isEmpty())
        {
            QHash<QByteArray, QByteArray> headers = info.httpHeaders();
            for (auto it = m_searchHeaders.begin(); it != m_searchHeaders.end();)
            {
                if (auto it2 = headers.find(*it); it2 != headers.end())
                {
                    emit foundHeader(*it, it2.value());
                    it = m_searchHeaders.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
#endif
}
