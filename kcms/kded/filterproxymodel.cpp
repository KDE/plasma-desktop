/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
