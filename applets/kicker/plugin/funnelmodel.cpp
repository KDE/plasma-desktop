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

#include "funnelmodel.h"

FunnelModel::FunnelModel(QObject *parent) : AbstractModel(parent)
{
}

FunnelModel::~FunnelModel()
{
}

QVariant FunnelModel::data(const QModelIndex &index, int role) const
{
    if (!m_sourceModel) {
        return QVariant();
    }

    return m_sourceModel->data(index, role);
}

int FunnelModel::rowCount(const QModelIndex &parent) const
{
    if (!m_sourceModel) {
        return 0;
    }

    return m_sourceModel->rowCount(parent);
}

bool FunnelModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (!m_sourceModel) {
        return false;
    }

    return m_sourceModel->trigger(row, actionId, argument);
}

AbstractModel* FunnelModel::modelForRow(int row)
{
    if (!m_sourceModel) {
        return 0;
    }

    return m_sourceModel->modelForRow(row);
}

AbstractModel* FunnelModel::sourceModel() const
{
    return m_sourceModel;
}

void FunnelModel::setSourceModel(AbstractModel *model)
{
    if (m_sourceModel == model) {
        return;
    }

    if (!model) {
        reset();

        return;
    }

    connect(model, SIGNAL(destroyed(QObject*)), this, SLOT(reset()));
    connect(model, SIGNAL(modelReset()), this, SLOT(reset()));

    if (!m_sourceModel) {
        emit beginResetModel();

        m_sourceModel = model;

        emit endResetModel();

        emit countChanged();

        emit sourceModelChanged();

        return;
    }

    int oldCount = m_sourceModel->count();
    int newCount = model->count();

    disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

    m_sourceModel = model;

    connect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

    if (newCount > oldCount) {
        beginInsertRows(QModelIndex(), oldCount, newCount - 1);

        endInsertRows();
    } else if (newCount < oldCount) {
        if (newCount == 0) {
            beginResetModel();

            endResetModel();
        } else {
            beginRemoveRows(QModelIndex(), newCount, oldCount - 1);

            endRemoveRows();
        }
    }

    if (newCount > 0) {
        emit dataChanged(index(0, 0), index(qMin(oldCount, newCount) - 1, 0));
    }

    if (oldCount != newCount) {
        emit countChanged();
    }

    emit sourceModelChanged();
}

void FunnelModel::reset()
{
    emit beginResetModel();
    emit endResetModel();
    emit countChanged();
}

void FunnelModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    emit beginRemoveRows(parent, first, last);
}

void FunnelModel::sourceRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    emit endRemoveRows();

    emit countChanged();
}
