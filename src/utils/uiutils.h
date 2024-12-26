#pragma once
#include <initializer_list>
#include <QCoreApplication>
#include <QWidget>
#include <variant>

namespace InnertubeObjects
{
struct BackstagePost;
struct Channel;
struct InnertubeString;
struct LockupViewModel;
struct Notification;
struct Reel;
struct ShortsLockupViewModel;
struct Video;
}

class QLabel;
class QLayout;
class QListWidget;
class QListWidgetItem;
class QTabWidget;

namespace UIUtils
{
    namespace detail
    {
        template<typename...>
        struct is_variant : std::false_type {};

        template<typename... Types>
        struct is_variant<std::variant<Types...>> : std::true_type {};

        template<typename T>
        inline constexpr bool is_variant_v = is_variant<T>::value;

        template<class T, class... U>
        inline constexpr bool is_any_v = std::disjunction_v<std::is_same<T, U>...>;
    }

    extern QString g_defaultStyle;

    void addBackstagePostToList(QListWidget* list, const InnertubeObjects::BackstagePost& post);
    void addBoldLabelToList(QListWidget* list, const QString& text);
    void addChannelToList(QListWidget* list, const InnertubeObjects::Channel& channel);
    void addNotificationToList(QListWidget* list, const InnertubeObjects::Notification& notification);
    void addSeparatorToList(QListWidget* list);
    void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf);
    void addShelfTitleToList(QListWidget* list, const QString& title);
    void addVideoToList(QListWidget* list, const InnertubeObjects::LockupViewModel& lockup,
                        bool useThumbnailFromData = true);
    void addVideoToList(QListWidget* list, const InnertubeObjects::Reel& reel,
                        bool useThumbnailFromData = true);
    void addVideoToList(QListWidget* list, const InnertubeObjects::ShortsLockupViewModel& shortsLockup,
                        bool useThumbnailFromData = true);
    void addVideoToList(QListWidget* list, const InnertubeObjects::Video& video,
                        bool useThumbnailFromData = true);
    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget);
    void addWrappedLabelToList(QListWidget* list, const QString& text);
    void clearLayout(QLayout* layout);
    void copyToClipboard(const QString& text);
    QIcon iconThemed(const QString& name, const QPalette& pal = {});
    QPixmap pixmapRounded(const QPixmap& pixmap, double xRadius, double yRadius);
    QPixmap pixmapThemed(const QString& name, bool fromQIcon = false,
                         const QSize& size = {}, const QPalette& pal = {});
    bool preferDark(const QPalette& pal = {});
    QString resolveThemedIconName(const QString& name, const QPalette& pal = {});
    void setAppStyle(const QString& styleName, bool dark);
    void setMaximumLines(QWidget* widget, int lines);
    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
    void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest = false);

    template<typename T>
    void addItemToList(QListWidget* list, const T& item)
    {
        namespace ITO = InnertubeObjects;
        if constexpr (std::same_as<T, ITO::BackstagePost>)
            addBackstagePostToList(list, item);
        else if constexpr (std::same_as<T, ITO::Channel>)
            addChannelToList(list, item);
        else if constexpr (std::same_as<T, ITO::Notification>)
            addNotificationToList(list, item);
        else if constexpr (std::same_as<T, ITO::InnertubeString>)
            addShelfTitleToList(list, item.text);
        else if constexpr (detail::is_any_v<T, ITO::LockupViewModel, ITO::Reel, ITO::ShortsLockupViewModel, ITO::Video>)
            addVideoToList(list, item);
    }

    void addRangeToList(QListWidget* list, std::ranges::range auto&& range)
    {
        for (auto it = std::ranges::begin(range); it != std::ranges::end(range); ++it)
        {
            if constexpr (detail::is_variant_v<std::ranges::range_value_t<decltype(range)>>)
                std::visit([list](auto&& item) { addItemToList(list, item); }, *it);
            else
                addItemToList(list, *it);
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
