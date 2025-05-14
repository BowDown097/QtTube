#pragma once
#include <QList>
#include <QString>

namespace QtTube
{
    struct PluginShelfBase
    {
        QString iconUrl;
        bool isDividerHidden{};
        QString subtitle;
        QString title;

        PluginShelfBase() = default;
        PluginShelfBase(const QString& iconUrl, bool isDividerHidden, const QString& subtitle, const QString& title)
            : iconUrl(iconUrl), isDividerHidden(isDividerHidden), subtitle(subtitle), title(title) {}
    };

    template<typename... ItemTypes>
    struct PluginShelf : PluginShelfBase
    {
        QList<std::variant<ItemTypes...>> contents;
        PluginShelf() = default;
        using PluginShelfBase::PluginShelfBase;
    };

    template<typename T>
    struct PluginShelf<T> : PluginShelfBase
    {
        QList<T> contents;
        PluginShelf() = default;
        using PluginShelfBase::PluginShelfBase;
    };
}
