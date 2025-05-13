#include "entityselecttablemodel.h"

void EntitySelectTableModel::append(const Entity& entity)
{
    beginInsertRows({}, m_data.count(), m_data.count());
    m_data.append(entity);
    endInsertRows();
}

QVariant EntitySelectTableModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::CheckStateRole && index.column() == 0)
            return m_data[index.row()].checked;
        else if (role == Qt::DisplayRole && index.column() == 1)
            return m_data[index.row()].name;
    }

    return QVariant();
}

QVariant EntitySelectTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
        case 0: return m_checkHeader;
        case 1: return m_nameHeader;
        default: return QVariant();
    }
}

bool EntitySelectTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::CheckStateRole)
        return false;

    m_data[index.row()].checked = value.value<Qt::CheckState>();
    emit dataChanged(index, index, {role});
    return true;
}
