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

#include "favoritesmodel.h"
#include "appentry.h"
#include "contactentry.h"
#include "fileentry.h"
#include "systementry.h"
#include "actionlist.h"

#include <KLocalizedString>

FavoritesModel::FavoritesModel(QObject *parent) : AbstractModel(parent)
{
}

FavoritesModel::~FavoritesModel()
{
    qDeleteAll(m_entryList);
}

QString FavoritesModel::description() const
{
    return i18n("Favorites");
}

QVariant FavoritesModel::data(const QModelIndex& index, int role) const
{

    if (!index.isValid() || index.row() >= m_entryList.count()) {
        return QVariant();
    }

    const AbstractEntry *entry = m_entryList.at(index.row());

    if (role == Qt::DisplayRole) {
        return entry->name();
    } else if (role == Qt::DecorationRole) {
        return entry->icon();
    } else if (role == Kicker::FavoriteIdRole) {
        return entry->id();
    } else if (role == Kicker::UrlRole) {
        return entry->url();
    } else if (role == Kicker::HasActionListRole) {
        return entry->hasActions();
    } else if (role == Kicker::ActionListRole) {
        return entry->actions();
    }

    return QVariant();
}

int FavoritesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_entryList.count();
}

bool FavoritesModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= m_entryList.count()) {
        return false;
    }

    return m_entryList.at(row)->run(actionId, argument);
}

QStringList FavoritesModel::favorites() const
{
    return m_favorites;
}

void FavoritesModel::setFavorites(const QStringList& favorites)
{
    QStringList _favorites(favorites);
    _favorites.removeDuplicates();

    if (_favorites != m_favorites) {
        m_favorites = _favorites;
        refresh();
    }
}

bool FavoritesModel::isFavorite(const QString &id) const
{
    return m_favorites.contains(id);
}

void FavoritesModel::addFavorite(const QString &id)
{
    if (id.isEmpty()) {
        return;
    }

    AbstractEntry *entry = favoriteFromId(id);

    if (!entry || !entry->isValid()) {
        delete entry;
        return;
    }

    beginInsertRows(QModelIndex(), m_entryList.count(), m_entryList.count());

    m_entryList << entry;
    m_favorites << entry->id();

    endInsertRows();

    emit countChanged();
    emit favoritesChanged();
}

void FavoritesModel::removeFavorite(const QString &id)
{
    int index = m_favorites.indexOf(id);

    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);

        delete m_entryList[index];
        m_entryList.removeAt(index);
        m_favorites.removeAt(index);

        endRemoveRows();

        emit countChanged();
        emit favoritesChanged();
    }
}

void FavoritesModel::moveRow(int from, int to)
{
    if (from >= m_favorites.count() || to >= m_favorites.count()) {
        return;
    }

    if (from == to) {
        return;
    }

    int modelTo = to + (to > from ? 1 : 0);

    bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), modelTo);

    if (ok) {
        m_entryList.move(from, to);
        m_favorites.move(from, to);

        endMoveRows();

        emit favoritesChanged();
    }
}

AbstractModel *FavoritesModel::favoritesModel()
{
    return this;
}

void FavoritesModel::refresh()
{
    beginResetModel();

    int oldCount = m_entryList.count();

    qDeleteAll(m_entryList);
    m_entryList.clear();

    QStringList newFavorites;

    foreach(const QString &id, m_favorites) {
        AbstractEntry *entry = favoriteFromId(id);

        if (entry && entry->isValid()) {
            m_entryList << entry;
            newFavorites << entry->id();
        } else if (entry) {
            delete entry;
        }
    }

    m_favorites = newFavorites;

    endResetModel();

    if (oldCount != m_entryList.count()) {
        emit countChanged();
    }

    emit favoritesChanged();
}

AbstractEntry *FavoritesModel::favoriteFromId(const QString &id)
{
    const QUrl url(id);
    const QString &s = url.scheme();

    if ((s.isEmpty() && id.contains(QStringLiteral(".desktop"))) || s == QStringLiteral("preferred")) {
        return new AppEntry(this, id);
    } else if (s == QStringLiteral("ktp")) {
        return new ContactEntry(this, id);
    } else {
        AbstractEntry *entry = new SystemEntry(this, id);

        if (!entry->isValid()) {
            delete entry;
            return new FileEntry(this, url);
        }

        return entry;
    }

    return nullptr;
}
