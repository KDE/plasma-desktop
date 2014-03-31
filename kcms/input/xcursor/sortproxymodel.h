/*
 * Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 or at your option version 3 as published 
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SORTPROXYMODEL_H
#define SORTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <thememodel.h>

/**
 * SortProxyModel is a sorting proxy model intended to be used in combination
 * with the ItemDelegate class.
 *
 * First it compares the Qt::DisplayRoles, and if they match it compares
 * the CursorTheme::DisplayDetailRoles.
 *
 * The model assumes both display roles are QStrings.
 */
class SortProxyModel : public QSortFilterProxyModel
{
    public:
        SortProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
        ~SortProxyModel() {}
        inline const CursorTheme *theme(const QModelIndex &index) const;
        inline QModelIndex findIndex(const QString &name) const;
        inline QModelIndex defaultIndex() const;
        inline void removeTheme(const QModelIndex &index);

    private:
        int compare(const QModelIndex &left, const QModelIndex &right, int role) const;

    protected:
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};


const CursorTheme *SortProxyModel::theme(const QModelIndex &index) const
{
    CursorThemeModel *model = static_cast<CursorThemeModel*>(sourceModel());
    return model->theme(mapToSource(index));
}

QModelIndex SortProxyModel::findIndex(const QString &name) const
{
    CursorThemeModel *model = static_cast<CursorThemeModel*>(sourceModel());
    return mapFromSource(model->findIndex(name));
}

QModelIndex SortProxyModel::defaultIndex() const
{
    CursorThemeModel *model = static_cast<CursorThemeModel*>(sourceModel());
    return mapFromSource(model->defaultIndex());
}

void SortProxyModel::removeTheme(const QModelIndex &index)
{
    CursorThemeModel *model = static_cast<CursorThemeModel*>(sourceModel());
    model->removeTheme(mapToSource(index));
}

#endif // SORTPROXYMODEL_H
