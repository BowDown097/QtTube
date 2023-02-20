#include "uiutilities.h"
#include "http.h"
#include "settingsstore.h"
#include "widgets/browsechannelrenderer.h"
#include "widgets/browsevideorenderer.h"
#include "widgets/tubelabel.h"
#include <QApplication>
#include <QClipboard>

#ifdef Q_OS_LINUX
#include <QThread>
#endif

void UIUtilities::addBoldLabelToList(QListWidget* list, const QString& text)
{
    QLabel* label = new QLabel(text);
    label->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(label->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, label);
}

void UIUtilities::addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel)
{
    BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
    renderer->setData(channel.channelId, channel.descriptionSnippet.text, channel.title.text, channel.subscribed,
                      channel.subscriberCountText.text, channel.videoCountText.text);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(renderer->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, renderer);

    HttpReply* reply = Http::instance().get(channel.thumbnails.last().url);
    QObject::connect(reply, &HttpReply::finished, renderer, &BrowseChannelRenderer::setThumbnail);
}

void UIUtilities::addSeparatorToList(QListWidget* list)
{
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(line->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, line);
}

void UIUtilities::addShelfTitleToList(QListWidget* list, const QJsonValue& shelf)
{
    addShelfTitleToList(list, InnertubeObjects::InnertubeString(shelf["title"]).text);
}

void UIUtilities::addShelfTitleToList(QListWidget* list, const QString& title)
{
    TubeLabel* shelfLabel = new TubeLabel(title);
    shelfLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(shelfLabel->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, shelfLabel);
}

void UIUtilities::addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel)
{
    BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
    renderer->setChannelData(reel.owner);
    renderer->setTargetElisionWidth(list->width() - 240);
    renderer->setVideoData("SHORTS", "", 0, reel.headline, reel.videoId, reel.viewCountText.text);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(renderer->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, renderer);

    HttpReply* reply = Http::instance().get(reel.thumbnails[0].url);
    QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
}

void UIUtilities::addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video)
{
    BrowseVideoRenderer* renderer = new BrowseVideoRenderer;
    renderer->setChannelData(video.owner);
    renderer->setTargetElisionWidth(list->width() - 240);
    renderer->setVideoData(video.lengthText.text, video.publishedTimeText.text, video.startTimeSeconds, video.title.text,
        video.videoId, SettingsStore::instance().condensedViews ? video.shortViewCountText.text : video.viewCountText.text);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(renderer->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, renderer);

    HttpReply* reply = Http::instance().get(video.thumbnail.mqdefault);
    QObject::connect(reply, &HttpReply::finished, renderer, &BrowseVideoRenderer::setThumbnail);
}

void UIUtilities::addWrappedLabelToList(QListWidget* list, const QString& text)
{
    QLabel* label = new QLabel(text);
    label->setWordWrap(true);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(label->sizeHint());
    list->addItem(item);
    list->setItemWidget(item, label);
}

void UIUtilities::clearLayout(QLayout* layout)
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

void UIUtilities::copyToClipboard(const QString& text)
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

void UIUtilities::elide(QLabel* label, int targetWidth)
{
    QFontMetrics fm(label->font());
    QString elidedText = fm.elidedText(label->text(), Qt::ElideRight, targetWidth);
    label->setFixedWidth(targetWidth);
    label->setText(elidedText);
}

// this will be used for the description and perhaps elsewhere
void UIUtilities::setMaximumLines(QWidget* widget, int lines)
{
    QFontMetrics fm(qApp->font());
    widget->setMaximumHeight(fm.lineSpacing() * lines);
}

void UIUtilities::setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
{
    for (int i : indexes)
        widget->setTabEnabled(i, enabled);
}
