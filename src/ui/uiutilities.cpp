#include "uiutilities.h"
#include "http.h"
#include "settingsstore.h"
#include "widgets/labels/tubelabel.h"
#include "widgets/renderers/browsechannelrenderer.h"
#include "widgets/renderers/browsevideorenderer.h"
#include <QApplication>
#include <QClipboard>
#include <QStyleFactory>

#ifdef Q_OS_LINUX
#include <QThread>
#endif

#ifdef Q_OS_WIN
#include "osutilities.h"
#include "ui/forms/mainwindow.h"
#endif

namespace UIUtilities
{
    void addBoldLabelToList(QListWidget* list, const QString& text)
    {
        QLabel* label = new QLabel(text);
        label->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
        addWidgetToList(list, label);
    }

    void addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel)
    {
        BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
        renderer->setData(channel.channelId, channel.descriptionSnippet.text, channel.title.text, channel.subscribed,
                          channel.subscriberCountText.text, channel.videoCountText.text);
        addWidgetToList(list, renderer);

        HttpReply* reply = Http::instance().get(channel.thumbnails.last().url);
        QObject::connect(reply, &HttpReply::finished, renderer, &BrowseChannelRenderer::setThumbnail);
    }

    void addSeparatorToList(QListWidget* list)
    {
        QFrame* line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        addWidgetToList(list, line);
    }

    void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf)
    {
        addShelfTitleToList(list, InnertubeObjects::InnertubeString(shelf["title"]).text);
    }

    void addShelfTitleToList(QListWidget* list, const QString& title)
    {
        TubeLabel* shelfLabel = new TubeLabel(title);
        shelfLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));
        addWidgetToList(list, shelfLabel);
    }

    void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel)
    {
        BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
        renderer->setChannelData(reel.owner);
        renderer->setTargetElisionWidth(list->width() - 240);
        renderer->setVideoData("SHORTS", "", 0, reel.headline, reel.videoId, reel.viewCountText.text);
        addWidgetToList(list, renderer);

        HttpReply* reply = Http::instance().get(reel.thumbnails[0].url);
        QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
    }

    void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video)
    {
        BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
        renderer->setChannelData(video.owner);
        renderer->setTargetElisionWidth(list->width() - 240);
        renderer->setVideoData(video.lengthText.text, video.publishedTimeText.text, video.startTimeSeconds, video.title.text,
            video.videoId, SettingsStore::instance().condensedViews ? video.shortViewCountText.text : video.viewCountText.text);
        addWidgetToList(list, renderer);

        HttpReply* reply = Http::instance().get(video.thumbnail.mqdefault);
        QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
    }

    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget)
    {
        QListWidgetItem* item = new QListWidgetItem(list);
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

    // apparently a workaround for copying to clipboard sometimes not working on linux. see https://www.medo64.com/2019/12/copy-to-clipboard-in-qt/
    #ifdef Q_OS_LINUX
        QThread::msleep(1);
    #endif
    }

    void elide(QLabel* label, int targetWidth)
    {
        QFontMetrics fm(label->font());
        QString elidedText = fm.elidedText(label->text(), Qt::ElideRight, targetWidth);
        label->setFixedWidth(targetWidth);
        label->setText(elidedText);
    }

    bool preferDark(const QPalette& pal)
    {
        return pal == QPalette()
                ? qApp->palette().alternateBase().color().lightness() < 60
                : pal.alternateBase().color().lightness() < 60;
    }

    void setAppStyle(const QString& styleName)
    {
    #ifdef Q_OS_WIN
        OSUtilities::setWinDarkModeEnabled(MainWindow::windowId(), styleName == "Dark");
    #endif
        if (styleName == "Default")
        {
            if (qApp->style()->objectName() != defaultStyle)
                qApp->setStyle(QStyleFactory::create(defaultStyle));
        }
        else if (styleName == "Dark")
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
            qApp->setStyleSheet(R"(
                QLineEdit {
                    background: rgb(42,42,42);
                    border: 1px solid rgb(30,30,30);
                }
                QListView::item {
                    background: rgb(49,49,49);
                }
                QComboBox, QPushButton, QScrollBar::vertical, QSpinBox, QTabBar::tab {
                    background: rgb(42,42,42);
                }
                QPushButton::hover, QTabBar::tab::hover, QTabBar::tab::selected {
                    background: rgb(30,30,30);
                }
                QTabWidget::pane {
                    border-color: rgb(30,30,30);
                }
            )");
            MainWindow::topbar()->updatePalette(darkPalette);
        }
        else if (QStyle* style = QStyleFactory::create(styleName))
        {
            qApp->setStyle(style);
        }
    }

    // this will be used for the description and perhaps elsewhere
    void setMaximumLines(QWidget* widget, int lines)
    {
        QFontMetrics fm(qApp->font());
        widget->setMaximumHeight(fm.lineSpacing() * lines);
    }

    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
    {
        for (int i : indexes)
            widget->setTabEnabled(i, enabled);
    }

    void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest)
    {
        QJsonArray::const_iterator thumbsBegin = getBest
            ? std::ranges::max_element(thumbsArr, [](const QJsonValue& a, const QJsonValue& b) { return a["height"].toInt() < b["height"].toInt(); })
            : thumbsArr.begin();
        if (thumbsBegin == thumbsArr.end())
            return;

        const QJsonValue& thumbnail = *thumbsBegin;
        HttpReply* reply = Http::instance().get(QUrl(thumbnail["url"].toString()));
        QObject::connect(reply, &HttpReply::finished, reply, [label](const HttpReply& reply)
        {
            QPixmap pixmap;
            pixmap.loadFromData(reply.body());
            label->setPixmap(pixmap.scaled(label->width(), label->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        });
    }
}
