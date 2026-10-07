// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "variantsmodel.h"

#include <KFuzzyMatcher>

#include "layoutmodel.h"

using namespace Qt::Literals::StringLiterals;

VariantsModel::VariantsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(LayoutModel::DescriptionRole);
}

QString VariantsModel::searchString() const
{
    return m_searchString;
}

void VariantsModel::setSearchString(const QString &searchString)
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

QString VariantsModel::shortName() const
{
    return m_shortName;
}

void VariantsModel::setShortName(const QString &shortName)
{
    if (shortName == m_shortName) {
        return;
    }

    beginFilterChange();
    m_shortName = shortName;
    endFilterChange();

    Q_EMIT searchStringChanged();
}

QString VariantsModel::getFullName(const QModelIndex &idx) const
{
    const auto shortName = idx.data(LayoutModel::Roles::ShortNameRole).toString();
    const auto description = idx.data(LayoutModel::Roles::DescriptionRole).toString();
    const auto variantName = idx.data(LayoutModel::Roles::VariantNameRole).toString();

    return shortName + " - "_L1 + description + " - "_L1 + variantName;
}

bool VariantsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_shortName.isEmpty()) {
        return false;
    }

    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (index.data(LayoutModel::ShortNameRole).toString() != m_shortName) {
        return false;
    }

    if (!m_searchString.isEmpty()) {
        return getFullName(index).contains(m_searchString, Qt::CaseInsensitive);
    }

    return true;
}

bool VariantsModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto leftScore = KFuzzyMatcher::match(m_searchString, getFullName(left)).score;
    auto rightScore = KFuzzyMatcher::match(m_searchString, getFullName(right)).score;

    if (leftScore == rightScore) {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    return leftScore < rightScore;
}

#include "moc_variantsmodel.cpp"
