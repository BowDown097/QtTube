#pragma once
#include <QHashFunctions>
#include <unicode/casemap.h>
#include <unicode/stringoptions.h>
#include <unicode/unistr.h>

struct CaseInsensitiveEqual
{
    using is_transparent = void;

    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return QUtf8StringView(lhs).compare(rhs, Qt::CaseInsensitive) == 0;
    }

    bool operator()(const QString& lhs, const QString& rhs) const noexcept
    {
        return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
    }
};

class CaseInsensitiveHash
{
    // FNV-1a hash function
    class HashSink : public icu::ByteSink
    {
    public:
        void Append(const char* data, int32_t n) override
        {
            for (int32_t i = 0; i < n; ++i)
            {
                hash ^= static_cast<unsigned char>(data[i]);
                hash *= 1099511628211ULL;
            }
        }

        std::size_t GetHash() const { return hash; }
    private:
        std::size_t hash = 14695981039346656037ULL;
    };
public:
    using is_transparent = void;

    [[nodiscard]] std::size_t operator()(const QString& str) const
    {
        return std::hash<QString>()(str.toCaseFolded());
    }

    // with this we don't have to create a new string :D
    [[nodiscard]] std::size_t operator()(std::string_view str) const
    {
        HashSink sink;
        UErrorCode status;
        icu::CaseMap::utf8Fold(U_FOLD_CASE_DEFAULT, str, sink, nullptr, status);
        return sink.GetHash();
    }
};