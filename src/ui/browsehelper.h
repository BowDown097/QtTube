#ifndef BROWSEHELPER_H
#define BROWSEHELPER_H
#include "innertube.h"
#include <mutex>
#include <QListWidget>
#include <QMessageBox>
#include <QScrollBar>
#include <type_traits>

class BrowseHelper : public QObject
{
    Q_OBJECT
public:
    static BrowseHelper* instance();
    explicit BrowseHelper(QObject* parent = nullptr) : QObject(parent) {}

    void browseChannel(QListWidget* channelTab, int index, const InnertubeEndpoints::ChannelResponse& channelResp);
    void browseHistory(QListWidget* historyWidget, const QString& query = "");
    void browseHome(QListWidget* homeWidget);
    void browseNotificationMenu(QListWidget* menuWidget);
    void browseSubscriptions(QListWidget* subsWidget);
    void browseTrending(QListWidget* trendingWidget);
    void search(QListWidget* searchWidget, const QString& query, int dateF = -1, int typeF = -1, int durF = -1, int featF = -1, int sort = -1);

    template<typename T> requires std::derived_from<T, InnertubeEndpoints::BaseEndpoint>
    void tryContinuation(int value, QListWidget* widget, const QString& data = "", int threshold = 10)
    {
        if (value < widget->verticalScrollBar()->maximum() - threshold || continuationToken.isEmpty() || continuationOngoing
            || InnerTube::instance().context()->client.visitorData.isEmpty() || widget->count() == 0)
        {
            return;
        }

        continuationOngoing = true;

        try
        {
            T newData = InnerTube::instance().getBlocking<T>(data, continuationToken);
            if constexpr (std::is_same_v<T, InnertubeEndpoints::Search>)
            {
                setupChannelList(newData.response.channels, widget);
                setupVideoList(newData.response.videos, widget);
            }
            else if constexpr (std::is_same_v<T, InnertubeEndpoints::GetNotificationMenu>)
            {
                setupNotificationList(newData.response.notifications, widget);
            }
            else
            {
                setupVideoList(newData.response.videos, widget);
            }

            continuationToken = newData.continuationToken;
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get continuation browsing info", ie.message());
        }

        continuationOngoing = false;
    }
private slots:
    void browseFailed(const InnertubeException& ie, const QString& title);
private:
    static inline BrowseHelper* m_instance;
    static inline std::once_flag m_onceFlag;

    void setupChannelList(const QList<InnertubeObjects::Channel>& channels, QListWidget* widget);
    void setupNotificationList(const QList<InnertubeObjects::Notification>& notifications, QListWidget* widget);
    void setupVideoList(const QList<InnertubeObjects::Video>& videos, QListWidget* widget);

    bool continuationOngoing = false;
    QString continuationToken;
    const QMap<int, QString> featureMap = {
        { 0, "isLive" },
        { 1, "is4K" },
        { 2, "isHD" },
        { 3, "hasSubtitles" },
        { 4, "isCreativeCommons" },
        { 5, "is360Degree" },
        { 6, "isVR180" },
        { 7, "is3D" },
        { 8, "isHDR" },
        { 9, "hasLocation" },
        { 10, "isPurchased" },
    };
    const QVariantMap searchMsgFields = {
        { "sort", QVariantList{1, 0} },
        {
            "filter", QVariantList{2, 2, QVariantMap{
                { "uploadDate", QVariantList{1, 0} },
                { "type", QVariantList{2, 0} },
                { "duration", QVariantList{3, 0} },
                { "isHD", QVariantList{4, 0} },
                { "hasSubtitles", QVariantList{5, 0} },
                { "isCreativeCommons", QVariantList{6, 0} },
                { "is3D", QVariantList{7, 0} },
                { "isLive", QVariantList{8, 0} },
                { "isPurchased", QVariantList{9, 0} },
                { "is4K", QVariantList{14, 0} },
                { "is360Degree", QVariantList{15, 0} },
                { "hasLocation", QVariantList{23, 0} },
                { "isHDR", QVariantList{25, 0} },
                { "isVR180", QVariantList{26, 0} }
            }}
        }
    };
};

#endif // BROWSEHELPER_H
