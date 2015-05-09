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

#include <QString>
#include "sortproxymodel.h"
#include "cursortheme.h"


int SortProxyModel::compare(const QModelIndex &left, const QModelIndex &right, int role) const
{
    const QAbstractItemModel *model = sourceModel();

    QString first  = model->data(left,  role).toString();
    QString second = model->data(right, role).toString();

    if (filterCaseSensitivity() == Qt::CaseSensitive)
    {
        first  = first.toLower();
        second = second.toLower();
    }

    return QString::localeAwareCompare(first, second);
}


bool SortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const int result = compare(left, right, Qt::DisplayRole);

    if (result != 0)
        return (result < 0);
    else
        return compare(left, right, CursorTheme::DisplayDetailRole) < 0;
}
