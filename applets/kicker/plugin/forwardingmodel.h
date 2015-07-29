/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
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

#ifndef FORWARDINGMODEL_H
#define FORWARDINGMODEL_H

#include "abstractmodel.h"

#include <QPointer>

class ForwardingModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged);

    public:
        explicit ForwardingModel(QObject *parent = 0);
        ~ForwardingModel();

        QString description() const;

        QAbstractItemModel *sourceModel() const;
        virtual void setSourceModel(QAbstractItemModel *sourceModel);

        bool canFetchMore(const QModelIndex &parent) const;
        void fetchMore(const QModelIndex &parent);

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

        Q_INVOKABLE QString labelForRow(int row);

        Q_INVOKABLE AbstractModel *modelForRow(int row);

        AbstractModel* favoritesModel();

        int separatorCount() const;

    public Q_SLOTS:
        void reset();

    Q_SIGNALS:
        void sourceModelChanged() const;

    protected:
        QModelIndex indexToSourceIndex(const QModelIndex &index) const;

        void connectSignals();
        void disconnectSignals();

        QPointer<QAbstractItemModel> m_sourceModel;
};

#endif
