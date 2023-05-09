#ifndef UIUTILITIES_H
#define UIUTILITIES_H
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include <initializer_list>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTabWidget>

namespace UIUtilities
{
    static QString defaultStyle{};

    void addBoldLabelToList(QListWidget* list, const QString& text);
    void addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel);
    void addSeparatorToList(QListWidget* list);
    void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf);
    void addShelfTitleToList(QListWidget* list, const QString& title);
    void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel);
    void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video);
    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget);
    void addWrappedLabelToList(QListWidget* list, const QString& text);
    void clearLayout(QLayout* layout);
    void copyToClipboard(const QString& text);
    void elide(QLabel* label, int targetWidth);
    bool preferDark(const QPalette& pal = QPalette());
    void setAppStyle(const QString& styleName);
    void setMaximumLines(QWidget* widget, int lines);
    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
    void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest = false);

    template<typename T>
    T findParent(QWidget* widget);
};

#include "uiutilities.tpp"

#endif // UIUTILITIES_H
