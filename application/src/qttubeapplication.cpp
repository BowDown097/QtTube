#include "qttubeapplication.h"
#include "eastereggs.h"
#include "mainwindow.h"
#include "ui/views/viewcontroller.h"
#include "utils/uiutils.h"
#include <QDesktopServices>
#include <QMessageBox>

void QtTubeApplication::doInitialSetup()
{
    m_plugins.reloadPlugins();
    m_settings.initialize();

    UIUtils::g_defaultStyle = style()->objectName();
    UIUtils::setAppStyle(m_settings.appStyle, m_settings.darkTheme);
}

void QtTubeApplication::handleUrlOrID(const QString& in)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTubePlugin::ResolveUrlReply* reply = plugin->interface->resolveUrlOrID(in);
        connect(reply, &QtTubePlugin::ResolveUrlReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
            QMessageBox::critical(nullptr, "Error in resolution", ex.message());
        });
        connect(reply, &QtTubePlugin::ResolveUrlReply::finished, this, [this](const QtTubePlugin::ResolveUrlData& data) {
            switch (data.target)
            {
            case QtTubePlugin::ResolveUrlTarget::Channel:
                ViewController::loadChannel(data.data);
                break;
            case QtTubePlugin::ResolveUrlTarget::Search:
                emit MainWindow::topbar()->searchBox->searchRequested(data.data, SearchBox::SearchType::ByQuery);
                break;
            case QtTubePlugin::ResolveUrlTarget::Video:
                ViewController::loadVideo(data.data, data.videoProgress, nullptr, data.continuePlayback);
                break;
            case QtTubePlugin::ResolveUrlTarget::PlainUrl:
                QDesktopServices::openUrl(data.data);
                break;
            case QtTubePlugin::ResolveUrlTarget::NotResolved:
                QMessageBox::warning(nullptr, "Nothing found!", "Could not find anything from your input.");
                break;
            }
        });
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
