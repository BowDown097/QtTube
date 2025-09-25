#pragma once
#include "ui/forms/plugins/basepluginentry.h"

struct RepositoryItem
{
    QString defaultBranch;
    QString fullName;
    QString ownerLogin;
    QString url;
};

using PluginEntryMetadataPtr = std::shared_ptr<PluginEntryMetadata>;
using RepositoryItemPtr = std::shared_ptr<RepositoryItem>;

class PluginMetadataCache
{
public:
    const std::unordered_map<QString, PluginEntryMetadataPtr>& data() const { return m_data; }
    bool isEmpty() const { return m_data.empty(); }
    qsizetype size() const { return m_data.size(); }

    auto emplace(const QString& k, PluginEntryMetadata&& v)
    {
        return m_data.emplace(k, std::make_shared<PluginEntryMetadata>(std::move(v)));
    }

    PluginEntryMetadataPtr find(const QString& fullName) const
    {
        if (auto it = m_data.find(fullName); it != m_data.end())
            return it->second;
        else
            return nullptr;
    }
private:
    std::unordered_map<QString, PluginEntryMetadataPtr> m_data;
};

class PluginRepoCache
{
public:
    const QList<RepositoryItemPtr>& data() const { return m_data; }
    bool isEmpty() const { return m_data.isEmpty(); }
    void reserve(qsizetype size) { m_data.reserve(size); }
    qsizetype size() const { return m_data.size(); }

    void append(RepositoryItem&& item)
    {
        m_data.append(std::make_shared<RepositoryItem>(std::move(item)));
    }
private:
    QList<RepositoryItemPtr> m_data;
};

struct PluginBrowserCache
{
    static PluginMetadataCache& metadata()
    {
        static PluginMetadataCache _metadata;
        return _metadata;
    }

    static PluginRepoCache& repos()
    {
        static PluginRepoCache _repos;
        return _repos;
    }
};
