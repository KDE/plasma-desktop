/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
