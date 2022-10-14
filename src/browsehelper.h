#ifndef BROWSEHELPER_H
#define BROWSEHELPER_H
#include "innertube/endpoints/base/baseendpoint.h"
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/video/video.h"
#include <QListWidget>
#include <type_traits>

class BrowseHelper
{
    bool continuationOngoing = false;
    QString continuationToken{};
public:
    static BrowseHelper& instance() { static BrowseHelper bh; return bh; }
    void browseHistory(QListWidget* historyWidget);
    void browseHome(QListWidget* homeWidget);
    void browseSubscriptions(QListWidget* subsWidget);
    void search(QListWidget* searchWidget, const QString& query);

    template<typename T> requires std::derived_from<T, InnertubeEndpoints::BaseEndpoint>
    void tryContinuation(int value, QListWidget* widget, const QString& data = "");
private:
    void setupChannelList(const QList<InnertubeObjects::Channel>& channels, QListWidget* widget);
    void setupVideoList(const QList<InnertubeObjects::Video>& videos, QListWidget* widget);
};

#include "browsehelper.tpp"

#endif // BROWSEHELPER_H
