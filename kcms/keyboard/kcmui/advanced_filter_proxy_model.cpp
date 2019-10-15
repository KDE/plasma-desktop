#include "advanced_filter_proxy_model.h"

AdvancedFilterProxyModel::AdvancedFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool AdvancedFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QString groupName = sourceModel()->data(sourceModel()->index(source_row, 0, source_parent),
                                            Roles::SectionNameRole).toString();
    return !QStringList({"grp", "lv3", "lv5"}).contains(groupName);
}
