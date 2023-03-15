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

class UIUtilities
{
public:
    static void addBoldLabelToList(QListWidget* list, const QString& text);
    static void addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel);
    static void addSeparatorToList(QListWidget* list);
    static void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf);
    static void addShelfTitleToList(QListWidget* list, const QString& title);
    static void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel);
    static void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video);
    static void addWrappedLabelToList(QListWidget* list, const QString& text);
    static void clearLayout(QLayout* layout);
    static void copyToClipboard(const QString& text);
    static void elide(QLabel* label, int targetWidth);
    static void setMaximumLines(QWidget* widget, int lines);
    static void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
    static void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest = false);

    template<typename T>
    static T findParent(QWidget* widget);
};

#include "uiutilities.tpp"

#endif // UIUTILITIES_H
