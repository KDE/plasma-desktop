/*
 * Copyright (c) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

#include "colors.h"

class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectedScheme READ selectedScheme WRITE setSelectedScheme NOTIFY selectedSchemeChanged)
    Q_PROPERTY(int selectedSchemeIndex READ selectedSchemeIndex NOTIFY selectedSchemeIndexChanged)

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(KCMColors::SchemeFilter filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    FilterProxyModel(QObject *parent = nullptr);
    ~FilterProxyModel() override;

    QString selectedScheme() const;
    void setSelectedScheme(const QString &scheme);

    int selectedSchemeIndex() const;

    QString query() const;
    void setQuery(const QString &query);

    KCMColors::SchemeFilter filter() const;
    void setFilter(KCMColors::SchemeFilter filter);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void queryChanged();
    void filterChanged();

    void selectedSchemeChanged();
    void selectedSchemeIndexChanged();

private:
    void emitSelectedSchemeIndexChange();

    QString m_selectedScheme;

    QString m_query;
    KCMColors::SchemeFilter m_filter = KCMColors::AllSchemes;

};
