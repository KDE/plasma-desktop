#include "layout_list_filter_duplicates_proxy_model.h"

#include "input_sources.h"

#include <QDebug>

LayoutListFilterDuplicatesProxyModel::LayoutListFilterDuplicatesProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool LayoutListFilterDuplicatesProxyModel::filterAcceptsRow(int source_row, const QModelIndex&) const
{
    int source = sourceModel()->data(sourceModel()->index(source_row, 0), Roles::SourceRole).toInt();
    QString name = sourceModel()->data(sourceModel()->index(source_row, 0), Roles::SaveNameRole).toString();

    if (source != InputSources::Sources::UnknownSource)
        return true;

    if ((source_row > 0 && name == sourceModel()->data(sourceModel()->index(source_row - 1, 0), Roles::SaveNameRole).toString())
        || (source_row < sourceModel()->rowCount() - 1 && name == sourceModel()->data(sourceModel()->index(source_row + 1, 0), Roles::SaveNameRole).toString())) {
        return false;
    }
    return true;
}

QVariant LayoutListFilterDuplicatesProxyModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) {
        return QVariant();
    }

    if (role == Roles::EnabledRole) {
        if (data(idx, Roles::SourceRole).toInt() == InputSources::Sources::UnknownSource) {
            return true;
        }
        return getGhostItemRow(idx) >= 0;
    }
    if ((role == Roles::PriorityRole ||
         role == Roles::IsLatinModeEnabledRole ||
         role == Roles::LatinModeLayoutRole) && data(idx, Roles::SourceRole).toInt() != InputSources::Sources::UnknownSource) {
        int source_row = getGhostItemRow(idx);
        return sourceModel()->data(sourceModel()->index(source_row, 0), role);
    }
    return QSortFilterProxyModel::data(idx, role);
}

void LayoutListFilterDuplicatesProxyModel::add(int idx)
{
    QString name = data(index(idx, 0), Roles::SaveNameRole).toString();
    emit itemAdded(name);
}

int LayoutListFilterDuplicatesProxyModel::getGhostItemRow(const QModelIndex& idx) const
{
    int source_row = mapToSource(idx).row();
    if (data(idx, Roles::SourceRole).toInt() == InputSources::Sources::UnknownSource) {
        return source_row;
    }
    QString name = data(idx, Roles::SaveNameRole).toString();
    if (source_row > 0 && name == sourceModel()->data(sourceModel()->index(source_row - 1, 0), Roles::SaveNameRole).toString()) {
        return source_row - 1;
    }
    if (source_row < sourceModel()->rowCount() - 1 && name == sourceModel()->data(sourceModel()->index(source_row + 1, 0), Roles::SaveNameRole).toString()) {
        return source_row + 1;
    }
    return -1;
}

void LayoutListFilterDuplicatesProxyModel::updateFilter()
{
    invalidateFilter();
}

void LayoutListFilterDuplicatesProxyModel::updateEnabled()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { Roles::EnabledRole });
}
