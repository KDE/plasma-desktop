/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "advanced_filter_proxy_model.h"
#include "advanced_model.h"

AdvancedFilterProxyModel::AdvancedFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool AdvancedFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QString groupName = sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),
                                            AdvancedModel::Roles::SectionNameRole).toString();

    // This options are already available in the layout tab with a better UI.
    return !QStringList({"grp", "lv3", "lv5"}).contains(groupName);
}
