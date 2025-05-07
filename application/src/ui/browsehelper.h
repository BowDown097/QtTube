#pragma once
#include "innertube.h"
#include "utils/uiutils.h"
#include "ui/widgets/continuablelistwidget.h"
#include <mutex>
#include <QMessageBox>
#include <QScrollBar>

class BrowseHelper : public QObject
{
    Q_OBJECT
public:
    static BrowseHelper* instance();
    explicit BrowseHelper(QObject* parent = nullptr) : QObject(parent) {}

    void browseChannel(ContinuableListWidget* widget, int index, const InnertubeEndpoints::ChannelResponse& resp);
    void browseHistory(ContinuableListWidget* widget, const QString& query = "");
    void browseHome(ContinuableListWidget* widget);
    void browseNotificationMenu(ContinuableListWidget* widget);
    void browseSubscriptions(ContinuableListWidget* widget);
    void browseTrending(ContinuableListWidget* widget);
    void continueChannel(ContinuableListWidget* widget, const QJsonValue& contents);
    void search(ContinuableListWidget* widget, const QString& query,
                int dateF = -1, int typeF = -1, int durF = -1, int featF = -1, int sort = -1);

    template<EndpointWithData E>
    void continuation(ContinuableListWidget* widget, const QString& data = "", int threshold = 10)
    {
        if (widget->continuationToken.isEmpty())
            return;

        widget->setPopulatingFlag(true);

        try
        {
            E newData = browseRequest<E>(widget->continuationToken, data);
            if constexpr (std::same_as<E, InnertubeEndpoints::Search>)
                setupSearch(widget, newData.response);
            else if constexpr (std::same_as<E, InnertubeEndpoints::GetNotificationMenu>)
                UIUtils::addRangeToList(widget, newData.response.notifications);
            else if constexpr (std::same_as<E, InnertubeEndpoints::BrowseChannel>)
                continueChannel(widget, newData.response.contents);
            else if constexpr (std::same_as<E, InnertubeEndpoints::BrowseHome>)
                setupHome(widget, newData.response);
            else
                UIUtils::addRangeToList(widget, newData.response.videos);

            // continuationToken is added by ChannelBrowser::continuation() for channels
            if constexpr (!std::same_as<E, InnertubeEndpoints::BrowseChannel>)
                widget->continuationToken = newData.continuationToken;
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(nullptr, "Failed to get continuation browsing info", ie.message());
        }

        widget->setPopulatingFlag(false);
    }
private slots:
    void browseFailed(const QString& title, ContinuableListWidget* widget, const InnertubeException& ie);
private:
    static inline BrowseHelper* m_instance;
    static inline std::once_flag m_onceFlag;

    template<EndpointWithData E>
    E browseRequest(const QString& continuationToken, const QString& data = "")
    {
        if constexpr (innertube_is_any_v<E, InnertubeEndpoints::BrowseHome, InnertubeEndpoints::BrowseSubscriptions>)
            return InnerTube::instance()->getBlocking<E>(continuationToken);
        else
            return InnerTube::instance()->getBlocking<E>(data, continuationToken);
    }

    void removeTrailingSeparator(QListWidget* list);
    void setupHome(QListWidget* widget, const InnertubeEndpoints::HomeResponse& response);
    void setupSearch(QListWidget* widget, const InnertubeEndpoints::SearchResponse& response);
    void setupTrending(QListWidget* widget, const InnertubeEndpoints::TrendingResponse& response);

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
