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

#include "colorsmodel.h"

FilterProxyModel::FilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

FilterProxyModel::~FilterProxyModel() = default;

QString FilterProxyModel::selectedScheme() const
{
    return m_selectedScheme;
}

void FilterProxyModel::setSelectedScheme(const QString &scheme)
{
    if (m_selectedScheme == scheme) {
        return;
    }

    m_selectedScheme = scheme;

    emit selectedSchemeChanged();
    emit selectedSchemeIndexChanged();
}

int FilterProxyModel::selectedSchemeIndex() const
{
    // We must search in the source model and then map the index to our proxy model.
    const auto results = sourceModel()->match(sourceModel()->index(0, 0), ColorsModel::SchemeNameRole, m_selectedScheme);

    if (results.count() == 1) {
        const QModelIndex result = mapFromSource(results.first());
        if (result.isValid()) {
            return result.row();
        }
    }

    return -1;
}

QString FilterProxyModel::query() const
{
    return m_query;
}

void FilterProxyModel::setQuery(const QString &query)
{
    if (m_query != query) {
        const int oldIndex = selectedSchemeIndex();

        m_query = query;
        invalidateFilter();

        emit queryChanged();

        if (selectedSchemeIndex() != oldIndex) {
            emit selectedSchemeIndexChanged();
        }
    }
}

KCMColors::SchemeFilter FilterProxyModel::filter() const
{
    return m_filter;
}

void FilterProxyModel::setFilter(KCMColors::SchemeFilter filter)
{
    if (m_filter != filter) {
        const int oldIndex = selectedSchemeIndex();

        m_filter = filter;
        invalidateFilter();

        emit filterChanged();

        if (selectedSchemeIndex() != oldIndex) {
            emit selectedSchemeIndexChanged();
        }
    }
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    if (!m_query.isEmpty()) {
        if (!idx.data(Qt::DisplayRole).toString().contains(m_query, Qt::CaseInsensitive)
                && !idx.data(ColorsModel::SchemeNameRole).toString().contains(m_query, Qt::CaseInsensitive)) {
            return false;
        }
    }

    if (m_filter != KCMColors::AllSchemes) {
        const QPalette palette = idx.data(ColorsModel::PaletteRole).value<QPalette>();

        const int windowBackgroundGray = qGray(palette.window().color().rgb());

        if (m_filter == KCMColors::DarkSchemes) {
            return windowBackgroundGray < 192;
        } else if (m_filter == KCMColors::LightSchemes) {
            return windowBackgroundGray >= 192;
        }
    }

    return true;
}
