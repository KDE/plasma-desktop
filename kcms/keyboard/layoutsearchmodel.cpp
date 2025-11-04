/*
    SPDX-FileCopyrightText: 2025 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutsearchmodel.h"

#include <algorithm>

#include <QDebug>
#include <QStringLiteral>

#include <KFuzzyMatcher>

#include "layoutmodel.h"

using namespace Qt::Literals::StringLiterals;

LayoutSearchModel::LayoutSearchModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QSortFilterProxyModel::sourceModelChanged, [this]() {
        const auto originalRoles = sourceModel()->roleNames().keys();
        m_searchScoreRole = *std::max_element(originalRoles.begin(), originalRoles.end()) + 1;
    });
}

void LayoutSearchModel::setSearchString(QStringView searchString)
{
    beginResetModel();
    m_searchString = searchString.toString();
    endResetModel();
    Q_EMIT searchStringChanged();
}

QString LayoutSearchModel::getFullName(const QModelIndex &idx) const
{
    const auto shortName = sourceModel()->data(idx, LayoutModel::Roles::ShortNameRole).toString();
    const auto description = sourceModel()->data(idx, LayoutModel::Roles::DescripionRole).toString();
    const auto variantName = sourceModel()->data(idx, LayoutModel::Roles::VariantNameRole).toString();

    return shortName + " - "_L1 + description + " - "_L1 + variantName;
}

QVariant LayoutSearchModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if (role == m_searchScoreRole)
        return QVariant(KFuzzyMatcher::match(m_searchString, getFullName(idx)).score);

    return sourceModel()->data(idx, role);
}

bool LayoutSearchModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return true;
    const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto res = KFuzzyMatcher::match(m_searchString, getFullName(idx));
    return res.matched;
}

QHash<int, QByteArray> LayoutSearchModel::roleNames() const
{
    QHash<int, QByteArray> newRoles(sourceModel()->roleNames());
    newRoles.insert(m_searchScoreRole, "searchScore"_ba);
    return newRoles;
}

#include "moc_layoutsearchmodel.cpp"
