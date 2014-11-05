/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H

#include "abstractmodel.h"

#include <QPointer>

#include <KService>

class FavoritesModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList favorites READ favorites WRITE setFavorites NOTIFY favoritesChanged)

    public:
        explicit FavoritesModel(QObject *parent = 0);
        ~FavoritesModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

        QStringList favorites() const;
        void setFavorites(const QStringList &favorites);

        Q_INVOKABLE bool isFavorite(const QString &favoriteId) const;
        Q_INVOKABLE void addFavorite(const QString &favoriteId);
        Q_INVOKABLE void removeFavorite(const QString &favoriteId);

        Q_INVOKABLE void moveRow(int from, int to);

        void setSourceModel(AbstractModel *model);

    public Q_SLOTS:
        virtual void refresh();

    Q_SIGNALS:
        void favoritesChanged() const;

    private:
        KService::Ptr defaultAppByName(const QString &name);

        QStringList m_favorites;
        QHash<QString, KService::Ptr> m_serviceCache;
        QPointer<AbstractModel> m_sourceModel;
};

#endif
