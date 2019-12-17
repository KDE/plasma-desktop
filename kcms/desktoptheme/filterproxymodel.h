/*
 * Copyright (c) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

#pragma once

#include <QSortFilterProxyModel>

#include "kcm.h"

class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    enum ThemeFilter {
        AllThemes,
        LightThemes,
        DarkThemes,
        ThemesFollowingColors
    };
    Q_ENUM(ThemeFilter)

    Q_PROPERTY(QString selectedTheme READ selectedTheme WRITE setSelectedTheme NOTIFY selectedThemeChanged)
    Q_PROPERTY(int selectedThemeIndex READ selectedThemeIndex NOTIFY selectedThemeIndexChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(ThemeFilter filter READ filter WRITE setFilter NOTIFY filterChanged)

    FilterProxyModel(QObject *parent = nullptr);
    ~FilterProxyModel() override;

    QString selectedTheme() const;
    void setSelectedTheme(const QString &pluginName);

    int selectedThemeIndex() const;

    QString query() const;
    void setQuery(const QString &query);

    ThemeFilter filter() const;
    void setFilter(ThemeFilter filter);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

Q_SIGNALS:
    void filterChanged();
    void queryChanged();

    void selectedThemeChanged();
    void selectedThemeIndexChanged();

private:
    QString m_selectedTheme;
    QString m_query;
    ThemeFilter m_filter = AllThemes;
};
