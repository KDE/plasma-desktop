/*
 * SPDX-FileCopyrightText: (C) 2019 Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "layout_list_filter_disabled_proxy_model.h"

LayoutListFilterDisabledProxyModel::LayoutListFilterDisabledProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterRole(Roles::EnabledRole);
}

bool LayoutListFilterDisabledProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    return sourceModel()->data(sourceModel()->index(source_row, 0), Roles::EnabledRole).toBool();
}
