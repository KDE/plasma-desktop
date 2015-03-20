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

#ifndef FUNNELMODEL_H
#define FUNNELMODEL_H

#include "abstractmodel.h"

#include <QPointer>

class FunnelModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(AbstractModel* sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged);

    public:
        explicit FunnelModel(QObject *parent = 0);
        ~FunnelModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

        Q_INVOKABLE AbstractModel *modelForRow(int row);

        AbstractModel *sourceModel() const;
        void setSourceModel(AbstractModel *model);

    public Q_SLOTS:
        void reset();

    Q_SIGNALS:
        void sourceModelChanged() const;

    private Q_SLOTS:
        void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
        void sourceRowsRemoved(const QModelIndex &parent, int first, int last);

    private:
        QPointer<AbstractModel> m_sourceModel;
};

#endif
