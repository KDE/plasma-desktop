/*
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (c) 2019 David Redondo <kde@david-redondo.de>
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

#include "themesmodel.h"

FilterProxyModel::FilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

FilterProxyModel::~FilterProxyModel() = default;

QString FilterProxyModel::selectedTheme() const
{
    return m_selectedTheme;
}

void FilterProxyModel::setSelectedTheme(const QString &pluginName)
{
    if (m_selectedTheme == pluginName) {
        return;
    }

    const bool firstTime = m_selectedTheme.isNull();
    m_selectedTheme = pluginName;

    if (!firstTime) {
        emit selectedThemeChanged();
    }
    emit selectedThemeIndexChanged();
}

int FilterProxyModel::selectedThemeIndex() const
{
    // We must search in the source model and then map the index to our proxy model.
    const auto results = sourceModel()->match(sourceModel()->index(0, 0), ThemesModel::PluginNameRole, m_selectedTheme);

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
        const int oldIndex = selectedThemeIndex();

        m_query = query;
        invalidateFilter();

        emit queryChanged();

        if (selectedThemeIndex() != oldIndex) {
            emit selectedThemeIndexChanged();
        }
    }
}

FilterProxyModel::ThemeFilter FilterProxyModel::filter() const
{
    return m_filter;
}

void FilterProxyModel::setFilter(ThemeFilter filter)
{
    if (m_filter != filter) {
        const int oldIndex = selectedThemeIndex();

        m_filter = filter;
        invalidateFilter();

        emit filterChanged();

        if (selectedThemeIndex() != oldIndex) {
            emit selectedThemeIndexChanged();
        }
    }
}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!m_query.isEmpty()) {
        if (!idx.data(Qt::DisplayRole).toString().contains(m_query, Qt::CaseInsensitive)
                && !idx.data(ThemesModel::PluginNameRole).toString().contains(m_query, Qt::CaseInsensitive)) {
            return false;
        }
    }

    const auto type =  idx.data(ThemesModel::ColorTypeRole).value<ThemesModel::ColorType>();
    switch (m_filter) {
    case AllThemes:
        return true;
    case LightThemes:
        return type == ThemesModel::LightTheme;
    case DarkThemes:
        return type == ThemesModel::DarkTheme;
    case ThemesFollowingColors:
        return type == ThemesModel::FollowsColorTheme;
    }

    return true;
}
