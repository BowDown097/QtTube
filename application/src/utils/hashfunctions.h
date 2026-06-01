#pragma once
#include <QHashFunctions>

struct CaseInsensitiveEqual
{
    bool operator()(const QString& lhs, const QString& rhs) const
    {
        return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
    }
};

struct CaseInsensitiveHash
{
    [[nodiscard]] std::size_t operator()(const QString& str) const
    {
        return std::hash<QString>()(str.toCaseFolded());
    }
};