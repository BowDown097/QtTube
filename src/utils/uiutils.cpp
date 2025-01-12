#include "uiutils.h"
#include "http.h"
#include "innertube/objects/ad/adslot.h"
#include "innertube/objects/backstage/backstagepost.h"
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/notification/notification.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/backstage/backstagepostrenderer.h"
#include "ui/widgets/renderers/backstage/postrenderer.h"
#include "ui/widgets/renderers/browsechannelrenderer.h"
#include "ui/widgets/renderers/browsenotificationrenderer.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "ui/widgets/renderers/video/gridvideorenderer.h"
#include <QClipboard>
#include <QFile>
#include <QJsonArray>
#include <QLayout>
#include <QPainter>
#include <QStyleFactory>

constexpr QLatin1String DarkStylesheet(R"(
    QLineEdit {
        background: rgb(42,42,42);
        border: 1px solid rgb(30,30,30);
    }
    QListView::item {
        background: rgb(49,49,49);
    }
    QComboBox, QMessageBox, QPushButton, QScrollBar::vertical, QSpinBox, QTabBar::tab {
        background: rgb(42,42,42);
    }
    QPushButton::hover, QTabBar::tab::hover, QTabBar::tab::selected {
        background: rgb(30,30,30);
    }
    QTabWidget::pane {
        border-color: rgb(30,30,30);
    }
    QToolButton {
        background: transparent;
        border: 1px solid rgb(30,30,30);
    }
)");

namespace UIUtils
{
    QString g_defaultStyle;

    void addBackstagePostToList(QListWidget* list, const InnertubeObjects::BackstagePost& post)
    {
        if (qtTubeApp->settings().channelIsFiltered(post.authorEndpoint["browseEndpoint"]["browseId"].toString()))
            return;

        BackstagePostRenderer* renderer = new BackstagePostRenderer;
        renderer->setData(post);

        QListWidgetItem* item = addWidgetToList(list, renderer);
        QObject::connect(renderer, &BackstagePostRenderer::dynamicSizeChange,
                         std::bind_front(&QListWidgetItem::setSizeHint, item));
    }

    void addBoldLabelToList(QListWidget* list, const QString& text)
    {
        QLabel* label = new QLabel(text);
        label->setFont(QFont(label->font().toString(), -1, QFont::Bold));
        addWidgetToList(list, label);
    }

    void addChannelToList(QListWidget* list, const InnertubeObjects::Channel& channel)
    {
        if (qtTubeApp->settings().channelIsFiltered(channel.channelId))
            return;

        BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
        renderer->setData(channel);
        addWidgetToList(list, renderer);

        if (const InnertubeObjects::GenericThumbnail* recAvatar = channel.thumbnail.recommendedQuality(QSize(80, 80)))
        {
            HttpReply* reply = Http::instance().get("https:" + recAvatar->url);
            QObject::connect(reply, &HttpReply::finished, renderer, &BrowseChannelRenderer::setThumbnail);
        }
    }

    void addNotificationToList(QListWidget* list, const InnertubeObjects::Notification& notification)
    {
        BrowseNotificationRenderer* renderer = new BrowseNotificationRenderer;
        renderer->setData(notification);
        addWidgetToList(list, renderer);

        if (const InnertubeObjects::GenericThumbnail* recAvatar = notification.channelIcon.recommendedQuality(QSize(48, 48)))
        {
            HttpReply* iconReply = Http::instance().get(recAvatar->url);
            QObject::connect(iconReply, &HttpReply::finished, renderer, &BrowseNotificationRenderer::setChannelIcon);
        }

        // notification.videoThumbnail returns images with black bars, so we're going to use mqdefault instead
        HttpReply* thumbReply = Http::instance().get("https://i.ytimg.com/vi/" + notification.videoId + "/mqdefault.jpg");
        QObject::connect(thumbReply, &HttpReply::finished, renderer, &BrowseNotificationRenderer::setThumbnail);
    }

    void addPostToList(QListWidget* list, const InnertubeObjects::Post& post)
    {
        if (qtTubeApp->settings().channelIsFiltered(post.authorEndpoint["browseEndpoint"]["browseId"].toString()))
            return;

        PostRenderer* renderer = new PostRenderer;
        renderer->setFixedSize(350, 196);
        renderer->setData(post);

        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(renderer->size());
        list->addItem(item);
        list->setItemWidget(item, renderer);
    }

    void addSeparatorToList(QListWidget* list)
    {
        QFrame* line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        QSize hint = line->sizeHint();
        if (list->flow() == QListWidget::LeftToRight)
            hint.setWidth(QWIDGETSIZE_MAX);

        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(hint);
        list->addItem(item);
        list->setItemWidget(item, line);
    }

    void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf)
    {
        addShelfTitleToList(list, InnertubeObjects::InnertubeString(shelf["title"]).text);
    }

    void addShelfTitleToList(QListWidget* list, const QString& title)
    {
        if (title.isEmpty())
            return;

        TubeLabel* shelfLabel = new TubeLabel(title);
        shelfLabel->setFont(QFont(shelfLabel->font().toString(), shelfLabel->font().pointSize() + 2));


        QSize hint = shelfLabel->sizeHint();
        if (list->flow() == QListWidget::LeftToRight)
            hint.setWidth(QWIDGETSIZE_MAX);

        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(hint);
        list->addItem(item);
        list->setItemWidget(item, shelfLabel);
    }

    VideoRenderer* constructVideoRenderer(QListWidget* list)
    {
        VideoRenderer* renderer;
        if (list->flow() == QListWidget::LeftToRight)
            renderer = new GridVideoRenderer(list);
        else
            renderer = new BrowseVideoRenderer(list);

        return renderer;
    }

    void addVideoToList(QListWidget* list, const InnertubeObjects::AdSlot& adSlot,
                        bool useThumbnailFromData)
    {
        if (qtTubeApp->settings().videoIsFiltered(adSlot))
            return;

        std::visit([&adSlot, list, useThumbnailFromData](auto&& v) {
            VideoRenderer* renderer = constructVideoRenderer(list);
            renderer->setData(v, useThumbnailFromData);
            addWidgetToList(list, renderer);
        }, adSlot.fulfillmentContent.fulfilledLayout.renderingContent);
    }

    void addVideoToList(QListWidget* list, const InnertubeObjects::LockupViewModel& lockup,
                        bool useThumbnailFromData)
    {
        if (qtTubeApp->settings().videoIsFiltered(lockup))
            return;

        VideoRenderer* renderer = constructVideoRenderer(list);
        renderer->setData(lockup, useThumbnailFromData);
        addWidgetToList(list, renderer);
    }

    void addVideoToList(QListWidget* list, const InnertubeObjects::Reel& reel,
                        bool useThumbnailFromData)
    {
        if (qtTubeApp->settings().videoIsFiltered(reel))
            return;

        VideoRenderer* renderer = constructVideoRenderer(list);
        renderer->setData(reel, list->flow() == QListWidget::LeftToRight, useThumbnailFromData);
        addWidgetToList(list, renderer);
    }

    void addVideoToList(QListWidget* list, const InnertubeObjects::ShortsLockupViewModel& shortsLockup,
                        bool useThumbnailFromData)
    {
        if (qtTubeApp->settings().videoIsFiltered(shortsLockup))
            return;

        VideoRenderer* renderer = constructVideoRenderer(list);
        renderer->setData(shortsLockup, list->flow() == QListWidget::LeftToRight, useThumbnailFromData);
        addWidgetToList(list, renderer);
    }

    void addVideoToList(QListWidget* list, const InnertubeObjects::Video& video,
                        bool useThumbnailFromData)
    {
        if (qtTubeApp->settings().videoIsFiltered(video))
            return;

        VideoRenderer* renderer = constructVideoRenderer(list);
        renderer->setData(video, useThumbnailFromData);
        addWidgetToList(list, renderer);
    }

    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget)
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(widget->sizeHint());
        list->addItem(item);
        list->setItemWidget(item, widget);
        return item;
    }

    void addWrappedLabelToList(QListWidget* list, const QString& text)
    {
        QLabel* label = new QLabel(text);
        label->setWordWrap(true);
        addWidgetToList(list, label);
    }

    void clearLayout(QLayout* layout)
    {
        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
            if (QLayout* childLayout = item->layout())
                clearLayout(childLayout);
            delete item;
        }
    }

    void copyToClipboard(const QString& text)
    {
        QClipboard* clipboard = qApp->clipboard();
        clipboard->setText(text, QClipboard::Clipboard);

        if (clipboard->supportsSelection())
            clipboard->setText(text, QClipboard::Selection);
    }

    QIcon iconThemed(const QString& name, const QPalette& pal)
    {
        return QIcon(resolveThemedIconName(name, pal));
    }

    QPixmap pixmapRounded(const QPixmap& pixmap, double xRadius, double yRadius)
    {
        QPixmap rounded(pixmap.size());
        rounded.fill(Qt::transparent);

        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(pixmap));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(pixmap.rect(), xRadius, yRadius);
        painter.end();

        return rounded;
    }

    QPixmap pixmapThemed(const QString& name, const QPalette& pal)
    {
        return QPixmap(resolveThemedIconName(name, pal));
    }

    bool preferDark(const QPalette& pal)
    {
        return pal == QPalette()
                ? qApp->palette().alternateBase().color().lightness() < 60
                : pal.alternateBase().color().lightness() < 60;
    }

    QString resolveThemedIconName(const QString& name, const QPalette& pal)
    {
        const QString baseFile = ":/" + name + ".svg";
        const QString lightFile = ":/" + name + "-light.svg";
        return QFile::exists(lightFile) && preferDark(pal) ? lightFile : baseFile;
    }

    void setAppStyle(const QString& styleName, bool dark)
    {
        if (styleName == "Default" && qApp->style()->objectName() != g_defaultStyle)
            qApp->setStyle(QStyleFactory::create(g_defaultStyle));
        else if (QStyle* style = QStyleFactory::create(styleName))
            qApp->setStyle(style);

        if (dark)
        {
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(49,49,49));
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, QColor(34,34,34));
            darkPalette.setColor(QPalette::AlternateBase, QColor(42,42,42));
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, QColor(48,48,48));
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Link, QColor(47,163,198));
            darkPalette.setColor(QPalette::Highlight, QColor(145,205,92));
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            darkPalette.setColor(QPalette::PlaceholderText, Qt::darkGray);
            qApp->setPalette(darkPalette);
            qApp->setStyleSheet(DarkStylesheet);
            MainWindow::topbar()->updatePalette(darkPalette);
        }
        else if (qApp->styleSheet() == DarkStylesheet)
        {
            qApp->setPalette(qApp->style()->standardPalette());
            qApp->setStyleSheet(QString());
            MainWindow::topbar()->updatePalette(qApp->palette());
        }

    #ifdef Q_OS_WIN // for some reason, wrong palette is applied to topbar on windows
        MainWindow::topbar()->updatePalette(qApp->palette());
    #endif
    }

    // this will be used for the description and perhaps elsewhere
    void setMaximumLines(QWidget* widget, int lines)
    {
        QFontMetrics fm(widget->font());
        widget->setMaximumHeight(fm.lineSpacing() * lines);
    }

    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
    {
        for (int i : indexes)
            widget->setTabEnabled(i, enabled);
    }

    void setThumbnail(QLabel* label, const QJsonValue& thumbnails)
    {
        label->setMinimumSize(1, 1);
        label->setScaledContents(true);

        InnertubeObjects::ResponsiveImage image(thumbnails);
        const InnertubeObjects::GenericThumbnail* best = image.bestQuality();
        if (!best)
            return;

        HttpReply* reply = Http::instance().get(QUrl(best->url));
        QObject::connect(reply, &HttpReply::finished, reply, [label](const HttpReply& reply)
        {
            QPixmap pixmap;
            pixmap.loadFromData(reply.body());
            label->setPixmap(pixmap);
        });
    }
}
