#include "webplayer.h"
#include <QBoxLayout>
#include <QFile>
#include <QWebChannel>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

// wanted to put this in a resource file but it seemingly doesn't work in this context. sad!
constexpr QLatin1StringView UtilitiesScriptData(R"(
function addStyle(css) {
    let style = document.createElement("style");
    style.type = "text/css";
    style.appendChild(document.createTextNode(css));
    document.documentElement.appendChild(style);
}

function h264ify() {
    // return a custom MIME type checker that can defer to the original function
    function makeModifiedTypeChecker(origChecker) {
        // Check if a video type is allowed
        return function (type) {
            if (!type)
                return '';

            const disallowedTypes = ["webm", "vp8", "vp9", "vp09", "av01"];
            if (disallowedTypes.some(disallowedType => type.includes(disallowedType)))
                return '';

            // Otherwise, ask the browser
            return origChecker(type);
        };
    }

    // Override video element canPlayType() function
    const videoElem = document.createElement('video');
    const origCanPlayType = videoElem.canPlayType.bind(videoElem);
    videoElem.canPlayType = makeModifiedTypeChecker(origCanPlayType);

    // Override media source extension isTypeSupported() function
    const mse = window.MediaSource;
    // Check for MSE support before use
    if (!mse)
        return;
    const origIsTypeSupported = mse.isTypeSupported.bind(mse);
    mse.isTypeSupported = makeModifiedTypeChecker(origIsTypeSupported);
}

function waitForElement(selector) {
    return new Promise(resolve => {
        const query = document.querySelector(selector);
        if (query)
            return resolve(query);

        const observer = new MutationObserver(() => {
            const query = document.querySelector(selector);
            if (query) {
                resolve(query);
                observer.disconnect();
            }
        });

        observer.observe(document, {
            childList: true,
            subtree: true
        });
    });
})");

namespace QtTubePlugin
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

    WebPlayer::WebPlayer(QWidget* parent)
        : Player(parent),
          m_interface(new WebChannelInterface(this)),
          m_view(new QWebEngineView(this))
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(m_view);
        layout->setContentsMargins(0, 0, 0, 0);

        m_channel = new QWebChannel(m_view->page());
        m_channel->registerObject("interface", m_interface);
        m_view->page()->setWebChannel(m_channel);

        loadScriptFile(":/qtwebchannel/qwebchannel.js", QWebEngineScript::DocumentCreation);
        loadScriptData(UtilitiesScriptData, QWebEngineScript::DocumentCreation);

        m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);

        connect(m_interface, &WebChannelInterface::newState, this, &Player::newState);
        connect(m_interface, &WebChannelInterface::progressChanged, this, &Player::progressChanged);
        connect(m_interface, &WebChannelInterface::switchVideoRequested, this, &Player::switchVideoRequested);
        connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &WebPlayer::fullScreenRequested);
    }

    void WebPlayer::fullScreenRequested(QWebEngineFullScreenRequest request)
    {
        request.accept();
        if (request.toggleOn())
            m_fullScreenWindow.reset(new FullScreenWindow(m_view));
        else
            m_fullScreenWindow.reset();
    }

    void WebPlayer::loadScriptData(const QString& data, QWebEngineScript::InjectionPoint injectionPoint, quint32 worldId)
    {
        QWebEngineScript script;
        script.setInjectionPoint(injectionPoint);
        script.setSourceCode(data);
        script.setWorldId(worldId);
        m_view->page()->scripts().insert(script);
    }

    void WebPlayer::loadScriptFile(const QString& path, QWebEngineScript::InjectionPoint injectionPoint, quint32 worldId)
    {
        if (QFile file(path); file.open(QFile::ReadOnly))
            loadScriptData(file.readAll(), injectionPoint, worldId);
        else
            qWarning() << "Failed to load script file:" << path;
    }

    void WebPlayer::loadStyleData(const QString& data)
    {
        QWebEngineScript script;
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setSourceCode(QStringLiteral("addStyle(`%1`);").arg(data));
        script.setWorldId(QWebEngineScript::MainWorld);
        m_view->page()->scripts().insert(script);
    }

    void WebPlayer::loadStyleFile(const QString& path)
    {
        if (QFile file(path); file.open(QFile::ReadOnly))
            loadStyleData(file.readAll());
        else
            qWarning() << "Failed to load style file:" << path;
    }
}
