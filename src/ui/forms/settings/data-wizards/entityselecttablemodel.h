#pragma once
#include <QAbstractTableModel>

struct Entity
{
    Qt::CheckState checked = Qt::Unchecked;
    QString id;
    QString name;

    Entity(const QString& id, const QString& name) : id(id), name(name) {}
};

class EntitySelectTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit EntitySelectTableModel(const QString& checkHeader, const QString& nameHeader, QObject* parent = nullptr)
        : QAbstractTableModel(parent), m_checkHeader(checkHeader), m_nameHeader(nameHeader) {}

    int columnCount(const QModelIndex& = QModelIndex()) const override { return 2; }
    int rowCount(const QModelIndex& = QModelIndex()) const override { return m_data.count(); }
    int checkedRowCount(const QModelIndex& = QModelIndex()) const
    { return std::ranges::count_if(m_data, &Entity::checked); }
    Qt::ItemFlags flags(const QModelIndex& = QModelIndex()) const override
    { return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled; }

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    void append(const Entity& entity);
    const Entity& entityAt(const QModelIndex& index) const { return m_data[index.row()]; }
private:
    QString m_checkHeader;
    QList<Entity> m_data;
    QString m_nameHeader;
signals:
    void checkedRowCountChnaged(int count);
};
