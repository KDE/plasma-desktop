/*
 * Copyright (C) 2020 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filterproxymodel.h"

#include "modulesmodel.h"

FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

FilterProxyModel::~FilterProxyModel() = default;

QString FilterProxyModel::query() const
{
    return m_query;
}

void FilterProxyModel::setQuery(const QString &query)
{
    if (m_query != query) {
        m_query = query;
        invalidateFilter();
        Q_EMIT queryChanged();
    }
}

KDEDConfig::ModuleStatus FilterProxyModel::statusFilter() const
{
    return m_statusFilter;
}

void FilterProxyModel::setStatusFilter(KDEDConfig::ModuleStatus statusFilter)
{
    if (m_statusFilter != statusFilter) {
        m_statusFilter = statusFilter;
        invalidateFilter();
        Q_EMIT statusFilterChanged();
    }
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    if (!m_query.isEmpty()) {
        if (!idx.data(Qt::DisplayRole).toString().contains(m_query, Qt::CaseInsensitive)
            && !idx.data(ModulesModel::ModuleNameRole).toString().contains(m_query, Qt::CaseInsensitive)) {
            return false;
        }
    }

    if (m_statusFilter != KDEDConfig::UnknownStatus) {
        const auto status = static_cast<KDEDConfig::ModuleStatus>(idx.data(ModulesModel::StatusRole).toInt());
        if (m_statusFilter != status) {
            return false;
        }
    }

    return true;
}
