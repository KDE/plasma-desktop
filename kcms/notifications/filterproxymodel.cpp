/*
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

#include "sourcesmodel.h"

FilterProxyModel::FilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
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
        emit queryChanged();
        emit currentIndexChanged();
    }
}

int FilterProxyModel::currentIndex() const
{
    if (m_currentIndex.isValid()) {
        return m_currentIndex.row();
    }
    return -1;
}

void FilterProxyModel::setCurrentIndex(const QPersistentModelIndex &idx)
{
    const int oldIndex = currentIndex();
    m_currentIndex = idx;
    if (oldIndex != currentIndex()) {
        emit currentIndexChanged();
    }
}

QPersistentModelIndex FilterProxyModel::makePersistentModelIndex(int row) const
{
    return QPersistentModelIndex(index(row, 0));
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_query.isEmpty()) {
        return true;
    }

    const QModelIndex idx = source_parent.child(source_row, 0);

    const QString display = idx.data(Qt::DisplayRole).toString();
    if (display.contains(m_query, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}
