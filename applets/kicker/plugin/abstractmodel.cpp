/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
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
, m_favoritesModel(nullptr)
, m_iconSize(32)
{
}

AbstractModel::~AbstractModel()
{
}

QHash<int, QByteArray> AbstractModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(Kicker::GroupRole, "group");
    roles.insert(Kicker::DescriptionRole, "description");
    roles.insert(Kicker::FavoriteIdRole, "favoriteId");
    roles.insert(Kicker::IsParentRole, "isParent");
    roles.insert(Kicker::IsSeparatorRole, "isSeparator");
    roles.insert(Kicker::HasChildrenRole, "hasChildren");
    roles.insert(Kicker::HasActionListRole, "hasActionList");
    roles.insert(Kicker::ActionListRole, "actionList");
    roles.insert(Kicker::UrlRole, "url");

    return roles;
}

int AbstractModel::count() const
{
    return rowCount();
}

int AbstractModel::separatorCount() const
{
    return 0;
}

int AbstractModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

int AbstractModel::iconSize() const
{
    return m_iconSize;
}

void AbstractModel::setIconSize(int iconSize) {
    if (m_iconSize != iconSize) {
        m_iconSize = iconSize;
        refresh();
    }
}

void AbstractModel::refresh()
{
}

QString AbstractModel::labelForRow(int row)
{
    return data(index(row, 0), Qt::DisplayRole).toString();
}

AbstractModel *AbstractModel::modelForRow(int row)
{
    Q_UNUSED(row)

    return nullptr;
}

int AbstractModel::rowForModel(AbstractModel *model)
{
    Q_UNUSED(model)

    return -1;
}

bool AbstractModel::hasActions() const
{
    return false;
}

QVariantList AbstractModel::actions() const
{
    return QVariantList();
}

AbstractModel* AbstractModel::favoritesModel()
{
    if (m_favoritesModel) {
        return m_favoritesModel;
    } else {
        AbstractModel *model = rootModel();

        if (model && model != this) {
            return model->favoritesModel();
        }
    }

    return nullptr;
}

void AbstractModel::setFavoritesModel(AbstractModel *model)
{
    if (m_favoritesModel != model) {
        m_favoritesModel = model;

        emit favoritesModelChanged();
    }
}

AbstractModel* AbstractModel::rootModel()
{
    if (!parent()) {
        return nullptr;
    }

    QObject *p = this;
    AbstractModel *rootModel = nullptr;

    while (p) {
        if (qobject_cast<AbstractModel *>(p)) {
            rootModel = qobject_cast<AbstractModel *>(p);
        } else {
            return rootModel;
        }

        p = p->parent();
    }

    return rootModel;
}

void AbstractModel::entryChanged(AbstractEntry *entry)
{
    Q_UNUSED(entry)
}
