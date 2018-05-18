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

FunnelModel::FunnelModel(QObject *parent) : ForwardingModel(parent)
{
}

FunnelModel::~FunnelModel()
{
}

void FunnelModel::setSourceModel(QAbstractItemModel *model)
{
    if (model && m_sourceModel == model) {
        return;
    }

    if (!model) {
        reset();

        return;
    }

    connect(model, SIGNAL(destroyed(QObject*)), this, SLOT(reset()));

    if (!m_sourceModel) {
        beginResetModel();

        m_sourceModel = model;

        connectSignals();

        endResetModel();

        emit countChanged();

        emit sourceModelChanged();
        emit descriptionChanged();

        return;
    }

    int oldCount = m_sourceModel->rowCount();
    int newCount = model->rowCount();

    auto setNewModel = [this, model]() {
        disconnectSignals();
        m_sourceModel = model;
        connectSignals();
    };

    if (newCount > oldCount) {
        beginInsertRows(QModelIndex(), oldCount, newCount - 1);
        setNewModel();
        endInsertRows();
    } else if (newCount < oldCount) {
        if (newCount == 0) {
            beginResetModel();
            setNewModel();
            endResetModel();
        } else {
            beginRemoveRows(QModelIndex(), newCount, oldCount - 1);
            setNewModel();
            endRemoveRows();
        }
    } else {
        setNewModel();
    }

    if (newCount > 0) {
        emit dataChanged(index(0, 0), index(qMin(oldCount, newCount) - 1, 0));
    }

    if (oldCount != newCount) {
        emit countChanged();
    }

    emit sourceModelChanged();
    emit descriptionChanged();
}
