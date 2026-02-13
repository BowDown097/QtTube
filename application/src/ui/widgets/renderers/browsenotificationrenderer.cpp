#include "browsenotificationrenderer.h"
#include "mainwindow.h"
#include "plugins/pluginbrowser.h"
#include "qttube-plugin/objects/notification.h"
#include "qttubeapplication.h"
#include "ui/forms/plugins/plugindownloaddialog.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/topbar/topbar.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QPushButton>

namespace
{
const QString updateBodyTemplate = QStringLiteral("An update is available for **%1**!");
}

BrowseNotificationRenderer::BrowseNotificationRenderer(QWidget* parent)
    : QWidget(parent),
      m_bodyLabel(new TubeLabel(this)),
      m_buttonsLayout(new QHBoxLayout),
      m_channelIconLabel(new TubeLabel(this)),
      m_innerLayout(new QVBoxLayout),
      m_primaryLayout(new QHBoxLayout(this)),
      m_sentTimeLabel(new TubeLabel(this)),
      m_thumbLabel(new TubeLabel(this))
{
    m_bodyLabel->setWordWrap(true);

    m_innerLayout->addStretch();
    m_innerLayout->addWidget(m_bodyLabel);
    m_innerLayout->addLayout(m_buttonsLayout);
    m_innerLayout->addWidget(m_sentTimeLabel);
    m_innerLayout->addStretch();

    m_channelIconLabel->setFixedSize(48, 48);
    m_channelIconLabel->setScaledContents(true);
    m_primaryLayout->addWidget(m_channelIconLabel);

    m_primaryLayout->addLayout(m_innerLayout, 1);

    m_thumbLabel->setFixedSize(128, 72);
    m_thumbLabel->setScaledContents(true);
    m_primaryLayout->addWidget(m_thumbLabel);

    m_sentTimeLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
}

void BrowseNotificationRenderer::setData(const QtTubePlugin::Notification& notification)
{
    UIUtils::clearLayout(m_buttonsLayout);
    m_bodyLabel->setTextFormat(Qt::PlainText);
    m_bodyLabel->setText(notification.body);
    m_channelIconLabel->setImage(notification.channelAvatarUrl, TubeLabel::LazyLoaded);
    m_sentTimeLabel->setText(notification.sentTimeText);
    m_thumbLabel->setImage(notification.thumbnailUrl, TubeLabel::LazyLoaded);
}

void BrowseNotificationRenderer::setData(const QString& pluginName, const ReleaseData& data)
{
    UIUtils::clearLayout(m_buttonsLayout);
    m_bodyLabel->setTextFormat(Qt::MarkdownText);
    m_bodyLabel->setText(updateBodyTemplate.arg(pluginName));
    m_channelIconLabel->setPixmap(QPixmap(":/qttube.svg"), TubeLabel::KeepAspectRatio);
    m_sentTimeLabel->setText(UIUtils::relativeTimeString(QDateTime::currentDateTimeUtc(), data.asset->updatedAt));

    QPushButton* installButton = new QPushButton("Install");
    m_buttonsLayout->addWidget(installButton);
    m_buttonsLayout->addStretch();

    connect(installButton, &QPushButton::clicked, this, [=, this] {
        UIUtils::clearLayout(m_buttonsLayout);

        PluginDownloadDialog* dialog = new PluginDownloadDialog(pluginName, data);
        dialog->show();

        connect(dialog, &PluginDownloadDialog::success, dialog, [=] {
            qtTubeApp->plugins().updatablePlugins().erase(pluginName);
            UIUtils::getMainWindow()->topbar()->updateNotificationCount(qtTubeApp->plugins().updatablePlugins().size());
        });
    });
}
