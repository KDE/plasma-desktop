/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                   *
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

#include "forwardingmodel.h"

ForwardingModel::ForwardingModel(QObject *parent) : AbstractModel(parent)
{
}

ForwardingModel::~ForwardingModel()
{
}

QAbstractItemModel *ForwardingModel::sourceModel() const
{
    return m_sourceModel;
}

void ForwardingModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    disconnectSignals();

    beginResetModel();

    m_sourceModel = sourceModel;

    endResetModel();

    connectSignals();

    emit countChanged();
}

bool ForwardingModel::canFetchMore(const QModelIndex &parent) const
{
    if (!m_sourceModel) {
        return false;
    }

    return m_sourceModel->canFetchMore(parent);
}

void ForwardingModel::fetchMore(const QModelIndex &parent)
{
    if (m_sourceModel) {
        m_sourceModel->fetchMore(parent);
    }
}

QModelIndex ForwardingModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_sourceModel) {
        return QModelIndex();
    }

    return m_sourceModel->index(row, column, parent);
}

QModelIndex ForwardingModel::parent(const QModelIndex &index) const
{
    if (!m_sourceModel) {
        return QModelIndex();
    }

    return m_sourceModel->parent(index);
}

QVariant ForwardingModel::data(const QModelIndex &index, int role) const
{
    if (m_sourceModel) {
        m_sourceModel->data(index, role);
    }

    return QVariant();
}

int ForwardingModel::rowCount(const QModelIndex &parent) const
{
    if (!m_sourceModel) {
        return 0;
    }

    return m_sourceModel->rowCount(parent);
}

void ForwardingModel::connectSignals()
{
    if (!m_sourceModel) {
        return;
    }

    connect(m_sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            this, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(countChanged()), Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(countChanged()), Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(modelAboutToBeReset()),
            this, SIGNAL(modelAboutToBeReset()),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(modelReset()),
            this, SIGNAL(modelReset()),
            Qt::UniqueConnection);
    connect(m_sourceModel, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            this, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            Qt::UniqueConnection);
}

void ForwardingModel::disconnectSignals()
{
    if (!m_sourceModel) {
        return;
    }

    disconnect(m_sourceModel, 0, this, 0);
}
