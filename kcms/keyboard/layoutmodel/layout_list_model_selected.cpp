#include "layout_list_model_selected.h"

#include "input_sources.h"

#include <QDebug>

LayoutListModelSelected::LayoutListModelSelected(QObject* parent)
    : QAbstractListModel(parent)
{
}

int LayoutListModelSelected::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_enabledLayouts.size();
}

QVariant LayoutListModelSelected::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Roles::EnabledRole:
        return true;

    case Roles::SourceRole:
        return InputSources::Sources::UnknownSource;

    case Roles::IsConfigurableRole:
        return false;

    case Roles::PriorityRole:
        return index.row();

    case Qt::DisplayRole:
    case Roles::SaveNameRole:
    case Roles::NameRole:
    case Roles::DescriptionRole:
        return m_enabledLayouts[index.row()].saveName;

    case Roles::IsLatinModeEnabledRole:
        return m_enabledLayouts[index.row()].isLatinModeEnabled;

    case Roles::LatinModeLayoutRole:
        return m_enabledLayouts[index.row()].latinModeLayout;
    }

    return QVariant();
}

QList<LayoutListModelSelected::EnabledLayout> LayoutListModelSelected::enabledLayouts() const
{
    return m_enabledLayouts;
}

void LayoutListModelSelected::setEnabledLayouts(const QList<EnabledLayout>& enabledLayouts)
{
    beginResetModel();
    m_enabledLayouts = enabledLayouts;
    endResetModel();
}

void LayoutListModelSelected::add(const QString &saveName)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    EnabledLayout layout;
    layout.saveName = saveName;
    layout.isLatinModeEnabled = false;
    m_enabledLayouts << layout;
    endInsertRows();
}

void LayoutListModelSelected::remove(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    m_enabledLayouts.removeAt(row);
    endRemoveRows();
}

void LayoutListModelSelected::setOrder(const QVector<int> &order)
{
    //emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), VerticalSortHint);
    auto copy = m_enabledLayouts;
    for (int i = 0; i < order.size(); ++i) {
        m_enabledLayouts[i] = copy[order[i]];
    }
    //emit layoutChanged(QList<QPersistentModelIndex>(), VerticalSortHint);
    emit dataChanged(index(0), index(rowCount() - 1), {});
}
