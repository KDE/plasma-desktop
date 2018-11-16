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

#ifndef SIMPLEFAVORITESMODEL_H
#define SIMPLEFAVORITESMODEL_H

#include "abstractmodel.h"

#include <QPointer>

#include <KService>

class SimpleFavoritesModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QStringList favorites READ favorites WRITE setFavorites NOTIFY favoritesChanged)
    Q_PROPERTY(int maxFavorites READ maxFavorites WRITE setMaxFavorites NOTIFY maxFavoritesChanged)
    Q_PROPERTY(int dropPlaceholderIndex READ dropPlaceholderIndex WRITE setDropPlaceholderIndex NOTIFY dropPlaceholderIndexChanged)

    public:
        explicit SimpleFavoritesModel(QObject *parent = nullptr);
        ~SimpleFavoritesModel() override;

        QString description() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

        bool enabled() const;
        void setEnabled(bool enable);

        QStringList favorites() const;
        void setFavorites(const QStringList &favorites);

        int maxFavorites() const;
        void setMaxFavorites(int max);

        Q_INVOKABLE bool isFavorite(const QString &id) const;
        Q_INVOKABLE void addFavorite(const QString &id, int index = -1);
        Q_INVOKABLE void removeFavorite(const QString &id);

        Q_INVOKABLE void moveRow(int from, int to);

        int dropPlaceholderIndex() const;
        void setDropPlaceholderIndex(int index);

        AbstractModel* favoritesModel() override;

    public Q_SLOTS:
        void refresh() override;

    Q_SIGNALS:
        void enabledChanged() const;
        void favoritesChanged() const;
        void maxFavoritesChanged() const;
        void dropPlaceholderIndexChanged();

    private:
        AbstractEntry *favoriteFromId(const QString &id);

        bool m_enabled;

        QList<AbstractEntry *> m_entryList;
        QStringList m_favorites;
        int m_maxFavorites;

        int m_dropPlaceholderIndex;
};

#endif
