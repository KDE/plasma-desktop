/*
 * Copyright 2020 David Redondo <kde@david-redondo.de>
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

#include "filteredmodel.h"

#include <QKeySequence>

#include "basemodel.h"

FilteredShortcutsModel::FilteredShortcutsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
}

bool FilteredShortcutsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_filter.isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    const bool displayMatches = index.data(Qt::DisplayRole).toString().contains(m_filter, Qt::CaseInsensitive);
    if (!source_parent.isValid() || displayMatches) {
        return displayMatches;
    }

    if (index.parent().data(Qt::DisplayRole).toString().contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }

    const auto &defaultShortcuts = index.data(BaseModel::DefaultShortcutsRole).value<QSet<QKeySequence>>();
    for (const auto &shortcut : defaultShortcuts) {
        if (shortcut.toString(QKeySequence::NativeText).contains(m_filter, Qt::CaseInsensitive)
            || shortcut.toString(QKeySequence::PortableText).contains(m_filter, Qt::CaseInsensitive)) {
            return true;
        }
    }

    const auto &shortcuts = index.data(BaseModel::CustomShortcutsRole).value<QSet<QKeySequence>>();
    for (const auto &shortcut : shortcuts) {
        if (shortcut.toString(QKeySequence::NativeText).contains(m_filter, Qt::CaseInsensitive)
            || shortcut.toString(QKeySequence::PortableText).contains(m_filter, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QString FilteredShortcutsModel::filter() const
{
    return m_filter;
}

void FilteredShortcutsModel::setFilter(const QString &filter)
{
    if (filter == m_filter) {
        return;
    }
    m_filter = filter;
    invalidateFilter();
    Q_EMIT filterChanged();
}
