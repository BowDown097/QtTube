#include "qttubeapplication.h"
#include "eastereggs.h"
#include "innertube.h"
#include "localcache.h"
#include "mainwindow.h"
#include "utils/uiutils.h"

void QtTubeApplication::doInitialSetup()
{
    m_creds.initialize();
    m_settings.initialize();

    UIUtils::g_defaultStyle = style()->objectName();
    UIUtils::setAppStyle(m_settings.appStyle, m_settings.darkTheme);

    LocalCache* cache = LocalCache::instance("client");
    cache->setMaxSeconds(86400);

    if (const QByteArray cver = cache->value("cver"); !cver.isNull())
    {
        InnerTube::instance()->createClient(InnertubeClient::ClientType::WEB, cver);
    }
    else
    {
        InnerTube::instance()->createClient(InnertubeClient::ClientType::WEB, "2.20250421.01.00", true);
        cache->insert("cver", InnerTube::instance()->context()->client.clientVersion.toLatin1());
    }

    if (const CredentialSet* activeLogin = m_creds.activeLogin())
    {
        m_creds.populateAuthStore(*activeLogin);
        if (InnerTube::instance()->hasAuthenticated())
            emit InnerTube::instance()->authStore()->authenticateSuccess();
    }
}

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (receiver->objectName() == "MainWindowWindow")
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
            EasterEggs::checkEasterEggs(static_cast<QKeyEvent*>(event));
            break;
        case QEvent::MouseMove:
            if (settings().autoHideTopBar)
                MainWindow::topbar()->handleMouseEvent(static_cast<QMouseEvent*>(event));
            break;
        default: break;
        }
    }

    return QApplication::notify(receiver, event);
}
