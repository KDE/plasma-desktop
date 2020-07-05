/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "layout_list_current_proxy_model.h"
#include "input_sources.h"
#include <QDebug>

LayoutListCurrentProxyModel::LayoutListCurrentProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(InputSources::self(), &InputSources::currentSourceChanged, this, &LayoutListCurrentProxyModel::currentInputSourceChanged);
}

bool LayoutListCurrentProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    int source = sourceModel()->data(
                sourceModel()->index(source_row, 0),
                Roles::SourceRole).toInt();
    return source == InputSources::self()->currentSource();
}

void LayoutListCurrentProxyModel::currentInputSourceChanged()
{
    invalidateFilter();
}
