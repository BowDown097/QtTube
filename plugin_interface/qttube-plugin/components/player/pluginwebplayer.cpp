#include "pluginwebplayer.h"
#include <QBoxLayout>
#include <QFile>
#include <QWebChannel>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

namespace QtTube
{
    FullScreenWindow::FullScreenWindow(QWebEngineView* oldView, QWidget* parent)
        : QWidget(parent),
          m_notification(new FullScreenNotification(this)),
          m_oldGeometry(oldView->window()->geometry()),
          m_oldView(oldView),
          m_view(new QWebEngineView(this))
    {
        m_view->stackUnder(m_notification);

        QAction* exitAction = new QAction(this);
        exitAction->setShortcut(Qt::Key_Escape);
        connect(exitAction, &QAction::triggered, this, [this] { m_view->triggerPageAction(QWebEnginePage::ExitFullScreen); });
        addAction(exitAction);

        m_view->setPage(m_oldView->page());
        setGeometry(m_oldGeometry);
        showFullScreen();
        m_oldView->window()->hide();
    }

    FullScreenWindow::~FullScreenWindow()
    {
        m_oldView->setPage(m_view->page());
        m_oldView->window()->setGeometry(m_oldGeometry);
        m_oldView->window()->show();
        hide();
    }

    void FullScreenWindow::resizeEvent(QResizeEvent* event)
    {
        QRect viewGeometry(QPoint(0, 0), size());
        m_view->setGeometry(viewGeometry);

        QRect notificationGeometry(QPoint(0, 0), m_notification->sizeHint());
        notificationGeometry.moveCenter(viewGeometry.center());
        m_notification->setGeometry(notificationGeometry);

        QWidget::resizeEvent(event);
    }

    PluginWebPlayer::PluginWebPlayer(QWidget* parent)
        : PluginPlayer(parent),
          m_interface(new WebChannelInterface(this)),
          m_view(new QWebEngineView(this))
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(m_view);
        layout->setContentsMargins(0, 0, 0, 0);

        m_channel = new QWebChannel(m_view->page());
        m_channel->registerObject("interface", m_interface);
        m_view->page()->setWebChannel(m_channel);

        m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);

        connect(m_interface, &WebChannelInterface::copyToClipboardRequested, this, &PluginPlayer::copyToClipboardRequested);
        connect(m_interface, &WebChannelInterface::newState, this, &PluginPlayer::newState);
        connect(m_interface, &WebChannelInterface::progressChanged, this, &PluginPlayer::progressChanged);
        connect(m_interface, &WebChannelInterface::switchVideoRequested, this, &PluginPlayer::switchVideoRequested);
        connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &PluginWebPlayer::fullScreenRequested);
    }

    void PluginWebPlayer::fullScreenRequested(QWebEngineFullScreenRequest request)
    {
        request.accept();
        if (request.toggleOn())
            m_fullScreenWindow.reset(new FullScreenWindow(m_view));
        else
            m_fullScreenWindow.reset();
    }

    void PluginWebPlayer::loadScriptData(const QString& data, QWebEngineScript::InjectionPoint injectionPoint, quint32 worldId)
    {
        QWebEngineScript script;
        script.setInjectionPoint(injectionPoint);
        script.setSourceCode(data);
        script.setWorldId(worldId);
        m_view->page()->scripts().insert(script);
    }

    void PluginWebPlayer::loadScriptFile(const QString& path, QWebEngineScript::InjectionPoint injectionPoint, quint32 worldId)
    {
        if (QFile file(path); file.open(QFile::ReadOnly))
            loadScriptData(file.readAll(), injectionPoint, worldId);
        else
            qWarning() << "Failed to load script file:" << path;
    }
}
