#pragma once
#include <QList>
#include <QString>

namespace QtTubePlugin
{
    struct ShelfBase
    {
        QString iconUrl;
        bool isDividerHidden{};
        QString subtitle;
        QString title;

        ShelfBase() = default;
        ShelfBase(const QString& iconUrl, bool isDividerHidden, const QString& subtitle, const QString& title)
            : iconUrl(iconUrl), isDividerHidden(isDividerHidden), subtitle(subtitle), title(title) {}
    };

    template<typename... ItemTypes>
    struct Shelf : ShelfBase
    {
        QList<std::variant<ItemTypes...>> contents;
        Shelf() = default;
        using ShelfBase::ShelfBase;
    };

    template<typename T>
    struct Shelf<T> : ShelfBase
    {
        QList<T> contents;
        Shelf() = default;
        using ShelfBase::ShelfBase;
    };
}
