/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "abstractmodel.h"
#include "actionlist.h"

AbstractModel::AbstractModel(QObject *parent) : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(Kicker::IsParentRole, "isParent");
    roles.insert(Kicker::HasChildrenRole, "hasChildren");
    roles.insert(Kicker::FavoriteIdRole, "favoriteId");
    roles.insert(Kicker::HasActionListRole, "hasActionList");
    roles.insert(Kicker::ActionListRole, "actionList");
    roles.insert(Kicker::UrlRole, "url");

    setRoleNames(roles);
}

AbstractModel::~AbstractModel()
{
}

int AbstractModel::count() const
{
    return rowCount();
}

AbstractModel *AbstractModel::modelForRow(int row)
{
    Q_UNUSED(row)

    return 0;
}

int AbstractModel::rowForFavoriteId(const QString &favoriteId)
{
    Q_UNUSED(favoriteId)

    return -1;
}
