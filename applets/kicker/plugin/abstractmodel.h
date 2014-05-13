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

#ifndef ABSTRACTMODEL_H
#define ABSTRACTMODEL_H

#include <QAbstractListModel>

class AbstractModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    public:
        explicit AbstractModel(QObject *parent = 0);
        ~AbstractModel();

        int count() const;

        Q_INVOKABLE virtual bool trigger(int row, const QString &actionId, const QVariant &argument) = 0;

        Q_INVOKABLE virtual AbstractModel *modelForRow(int row);

        virtual int rowForFavoriteId(const QString &favoriteId);

    Q_SIGNALS:
        void countChanged() const;
        void appLaunched(const QString& storageId) const;
};

#endif
