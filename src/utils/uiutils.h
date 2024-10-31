#pragma once
#include <initializer_list>
#include <QCoreApplication>
#include <QWidget>

namespace InnertubeObjects
{
struct BackstagePost;
struct Channel;
struct Notification;
struct Reel;
struct Video;
}

class QLabel;
class QLayout;
class QListWidget;
class QListWidgetItem;
class QTabWidget;
class VideoRenderer;

namespace UIUtils
{
    extern QString g_defaultStyle;

    void addBackstagePostToList(QListWidget* list, const InnertubeObjects::BackstagePost& post);
    void addBoldLabelToList(QListWidget* list, const QString& text);
    void addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel);
    void addNotificationToList(QListWidget* list, const InnertubeObjects::Notification& notification);
    void addSeparatorToList(QListWidget* list);
    void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf);
    void addShelfTitleToList(QListWidget* list, const QString& title);
    void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel);
    void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video);
    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget);
    void addWrappedLabelToList(QListWidget* list, const QString& text);
    void clearLayout(QLayout* layout);
    void copyToClipboard(const QString& text);
    QIcon iconThemed(const QString& name, const QPalette& pal = QPalette());
    QPixmap pixmapRounded(const QPixmap& pixmap, double xRadius, double yRadius);
    QPixmap pixmapThemed(const QString& name, bool fromQIcon = false, const QSize& size = QSize(), const QPalette& pal = QPalette());
    bool preferDark(const QPalette& pal = QPalette());
    QString resolveThemedIconName(const QString& name, const QPalette& pal = QPalette());
    void setAppStyle(const QString& styleName, bool dark);
    void setMaximumLines(QWidget* widget, int lines);
    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
    void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest = false);

    void addRangeToList(QListWidget* list, std::ranges::range auto&& range)
    {
        for (auto it = std::ranges::begin(range); it != std::ranges::end(range); ++it)
        {
            using RangeValue = std::ranges::range_value_t<decltype(range)>;
            if constexpr (std::same_as<RangeValue, InnertubeObjects::BackstagePost>)
                addBackstagePostToList(list, *it);
            else if constexpr (std::same_as<RangeValue, InnertubeObjects::Channel>)
                addChannelRendererToList(list, *it);
            else if constexpr (std::same_as<RangeValue, InnertubeObjects::Notification>)
                addNotificationToList(list, *it);
            else if constexpr (std::same_as<RangeValue, InnertubeObjects::Reel> || std::same_as<RangeValue, InnertubeObjects::Video>)
                addVideoRendererToList(list, *it);
            QCoreApplication::processEvents();
        }
    }

    template<typename T>
    T findParent(QWidget* widget)
    {
        QWidget* parentWidget = widget->parentWidget();
        while (parentWidget)
        {
            if (T castedWidget = qobject_cast<T>(parentWidget))
                return castedWidget;
            parentWidget = parentWidget->parentWidget();
        }

        return nullptr;
    }
};
