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

#include "favoritesmodel.h"
#include "actionlist.h"
#include "appsmodel.h"

#include <KRun>

FavoritesModel::FavoritesModel(QObject *parent) : AbstractModel(parent)
, m_sourceModel(0)
{
}

FavoritesModel::~FavoritesModel()
{
}

QVariant FavoritesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_favorites.count()) {
        return QVariant();
    }

    const QString &favoritesId = m_favorites.at(index.row());

    if (m_sourceModel) {
        int rowInSource = m_sourceModel->rowForFavoriteId(favoritesId);

        QModelIndex sourceIndex = m_sourceModel->index(rowInSource);

        return m_sourceModel->data(sourceIndex, role);
    } else if (m_serviceCache.contains(favoritesId)) {
        KService::Ptr service = m_serviceCache[favoritesId];

        if (role == Qt::DisplayRole) {
            return AppsModel::nameFromService(service,
                (AppsModel::NameFormat)qobject_cast<AppsModel *>(parent())->appNameFormat());
        } else if (role == Qt::DecorationRole) {
            return service->icon();
        } else if (role == Kicker::FavoriteIdRole) {
            return QVariant("app:" + service->storageId());
        } else if (role == Kicker::UrlRole) {
            return QUrl::fromLocalFile(service->entryPath());
        }
    }

    return QVariant();
}

int FavoritesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_favorites.count();
}

bool FavoritesModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= m_favorites.count()) {
        return false;
    }

    const QString &favoritesId = m_favorites.at(row);

    if (m_sourceModel) {
        int rowInSource = m_sourceModel->rowForFavoriteId(favoritesId);

        return m_sourceModel->trigger(rowInSource, actionId, argument);
    } else if (m_serviceCache.contains(favoritesId)) {
        KService::Ptr service = m_serviceCache[favoritesId];

        bool ran = KRun::run(*service, QList<QUrl>(), 0);

        if (ran) {
            emit appLaunched(service->storageId());
        }

        return ran;
    }

    return false;
}

QStringList FavoritesModel::favorites() const
{
    return m_favorites;
}

void FavoritesModel::setFavorites(const QStringList& favorites)
{
    QStringList _favorites(favorites);
    _favorites.removeDuplicates();

    if (m_favorites != _favorites) {
        if (!m_sourceModel) {
            m_serviceCache.clear();

            QMutableStringListIterator i(_favorites);
            KService::Ptr service;

            while (i.hasNext()) {
                i.next();

                service = KService::serviceByStorageId(i.value());

                if (service) {
                    i.setValue(service->storageId());
                    m_serviceCache[i.value()] = service;
                } else {
                    i.remove();
                }
            }

            if (m_favorites == _favorites) {
                return;
            }
        }

        bool emitCountChanged = (m_favorites.count() != _favorites.count());

        beginResetModel();

        m_favorites = _favorites;

        endResetModel();

        if (emitCountChanged) {
            emit countChanged();
        }

        emit favoritesChanged();
    }
}

bool FavoritesModel::isFavorite(const QString &favoriteId) const
{
    return m_favorites.contains(favoriteId);
}

void FavoritesModel::addFavorite(const QString &favoriteId)
{
    if (favoriteId.isEmpty()) {
        return;
    }

    QString _favoriteId = favoriteId;

    if (!m_sourceModel) {
        KService::Ptr service = KService::serviceByStorageId(favoriteId);

        if (service) {
            _favoriteId = service->storageId();
            m_serviceCache[_favoriteId] = service;
        } else {
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_favorites.count(), m_favorites.count());

    m_favorites << _favoriteId;

    endInsertRows();

    emit countChanged();

    emit favoritesChanged();
}

void FavoritesModel::removeFavorite(const QString &favoriteId)
{
    int index = m_favorites.indexOf(favoriteId);

    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);

        m_favorites.removeAt(index);

        if (!m_sourceModel) {
            m_serviceCache.remove(favoriteId);
        }

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
        m_favorites.move(from, to);

        endMoveRows();

        emit favoritesChanged();
    }
}

void FavoritesModel::setSourceModel(AbstractModel *model)
{
    beginResetModel();

    m_sourceModel = model;

    if (!m_sourceModel) {
        m_serviceCache.clear();
    }

    endResetModel();
}

void FavoritesModel::refresh()
{
     setFavorites(m_favorites);
}

