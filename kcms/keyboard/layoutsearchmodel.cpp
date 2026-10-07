/*
    SPDX-FileCopyrightText: 2025 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutsearchmodel.h"

#include <KFuzzyMatcher>

#include "layoutmodel.h"

using namespace Qt::Literals::StringLiterals;

LayoutSearchModel::LayoutSearchModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(LayoutModel::Roles::DescriptionRole);
}

QString LayoutSearchModel::searchString() const
{
    return m_searchString;
}

void LayoutSearchModel::setSearchString(const QString &searchString)
{
    if (searchString == m_searchString) {
        return;
    }

    beginFilterChange();
    m_searchString = searchString;
    endFilterChange();
    sort(0, Qt::DescendingOrder);

    Q_EMIT searchStringChanged();
}

QString LayoutSearchModel::getFullName(const QModelIndex &idx) const
{
    const auto shortName = idx.data(LayoutModel::Roles::ShortNameRole).toString();
    const auto description = idx.data(LayoutModel::Roles::DescriptionRole).toString();
    const auto variantName = idx.data(LayoutModel::Roles::VariantNameRole).toString();

    return shortName + " - "_L1 + description + " - "_L1 + variantName;
}

bool LayoutSearchModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!index.data(LayoutModel::Roles::VariantNameRole).toString().trimmed().isEmpty()) {
        return false;
    }

    if (m_searchString.isEmpty()) {
        return true;
    }

    if (getFullName(index).contains(m_searchString, Qt::CaseInsensitive)) {
        return true;
    }

    const auto shortName = index.data(LayoutModel::ShortNameRole).toString();

    for (auto i = 0; i < sourceModel()->rowCount(); i++) {
        const auto sourceIndex = sourceModel()->index(i, 0, sourceParent);
        const auto sourceShortName = sourceIndex.data(LayoutModel::ShortNameRole).toString();
        if (sourceShortName == shortName && getFullName(sourceIndex).contains(m_searchString, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool LayoutSearchModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto leftScore = KFuzzyMatcher::match(m_searchString, getFullName(left)).score;
    auto rightScore = KFuzzyMatcher::match(m_searchString, getFullName(right)).score;

    if (leftScore == rightScore) {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    return leftScore < rightScore;
}

#include "moc_layoutsearchmodel.cpp"
