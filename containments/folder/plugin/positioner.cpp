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

#include "positioner.h"
#include "foldermodel.h"

#include <QDebug>

Positioner::Positioner(QObject *parent): QAbstractItemModel(parent)
, m_enabled(false)
, m_folderModel(0)
, m_perStripe(0)
, m_pendingPositionsUpdate(false)
, m_applyPositions(false)
, m_resetting(false)
, m_lastIndex(-1)
, m_ignoreNextTransaction(false)
{
}

Positioner::~Positioner()
{
}

bool Positioner::enabled() const
{
    return m_enabled;
}

void Positioner::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        beginResetModel();

        m_enabled = enabled;

        if (enabled) {
            initMaps();
        }

        endResetModel();

        emit enabledChanged();

        if (!m_pendingPositionsUpdate && m_folderModel->rowCount()) {
            m_pendingPositionsUpdate = true;
            QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);
        }
    }
}

FolderModel* Positioner::folderModel() const
{
    return m_folderModel;
}

void Positioner::setFolderModel(QObject *folderModel)
{
    if (m_folderModel != folderModel) {
        beginResetModel();

        if (m_folderModel) {
            disconnectSignals(m_folderModel);
        }

        m_folderModel = qobject_cast<FolderModel *>(folderModel);
        connect(m_folderModel, SIGNAL(urlChanged()), this, SLOT(reset()), Qt::UniqueConnection);

        if (m_folderModel) {
            connectSignals(m_folderModel);
        }

        endResetModel();

        emit folderModelChanged();
    }
}

int Positioner::perStripe() const
{
    return m_perStripe;
}

void Positioner::setPerStripe(int perStripe)
{
    if (m_perStripe != perStripe) {
        m_perStripe = perStripe;

        emit perStripeChanged();

        if (m_enabled && m_perStripe > 0) {
            applyPositions();

            if (!m_pendingPositionsUpdate) {
                m_pendingPositionsUpdate = true;
                QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);
            }
        }
    }
}

QStringList Positioner::positions() const
{
    return m_positions;
}

void Positioner::setPositions(QStringList positions)
{
    if (m_positions != positions) {
        m_positions = positions;

        emit positionsChanged();
    }
}

int Positioner::map(int row) const
{
    if (m_enabled) {
        if (m_proxyToSource.contains(row)) {
            return m_proxyToSource.value(row);
        } else {
            return -1;
        }
    }

    return row;
}

bool Positioner::isBlank(int row) const
{
    if (!m_enabled && m_folderModel) {
        return m_folderModel->isBlank(row);
    }

    if (m_proxyToSource.contains(row) && !m_folderModel->isBlank(m_proxyToSource.value(row))) {
        return false;
    }

    return true;
}

int Positioner::indexForUrl(const QUrl &url) const
{
    if (!m_folderModel) {
        return -1;
    }

    const QString &name = url.fileName();

    int sourceIndex = -1;

    // TODO Optimize.
    for (int i = 0; i < m_folderModel->rowCount(); ++i) {
        if (m_folderModel->data(m_folderModel->index(i, 0), FolderModel::FileNameRole).toString() == name) {
            sourceIndex = i;

            break;
        }
    }

    if (m_sourceToProxy.contains(sourceIndex)) {
        return m_sourceToProxy.value(sourceIndex);
    }

    return -1;
}

QHash< int, QByteArray > Positioner::roleNames() const
{
    if (m_folderModel) {
        return m_folderModel->roleNames();
    }

    return QAbstractItemModel::roleNames();
}

QModelIndex Positioner::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex Positioner::parent(const QModelIndex &index) const
{
    if (m_folderModel) {
        m_folderModel->parent(index);
    }

    return QModelIndex();
}

QVariant Positioner::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (m_folderModel) {
        if (m_enabled) {
            if (m_proxyToSource.contains(index.row())) {
                return m_folderModel->data(m_folderModel->index(m_proxyToSource.value(index.row()), 0), role);
            } else if (role == FolderModel::BlankRole) {
                return true;
            }
        } else {
            return m_folderModel->data(m_folderModel->index(index.row(), 0), role);
        }
    }

    return QVariant();
}

int Positioner::rowCount(const QModelIndex& parent) const
{
    if (m_folderModel) {
        if (m_enabled) {
            return lastIndex() + 1;
        } else {
            return m_folderModel->rowCount(parent);
        }
    }

    return 0;
}

int Positioner::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    if (m_folderModel) {
        return 1;
    }

    return 0;
}

void Positioner::reset()
{
    m_resetting = true;

    beginResetModel();

    initMaps();

    m_positions = QStringList();
    emit positionsChanged();

    endResetModel();
}

void Positioner::move(int from, int to) {
    if (!m_proxyToSource.contains(from) || from == to) {
        return;
    }

    if (to == -1) {
        to = firstFreeRow();

        if (to == -1) {
            to = lastIndex() + 1;
        }
    }

    if (!isBlank(to)) {
        return;
    }

    int oldCount = rowCount();

    int sourceRow = m_proxyToSource.value(from);
    m_proxyToSource.remove(from);
    m_proxyToSource.insert(to, sourceRow);
    m_lastIndex = -1;
    m_sourceToProxy.insert(sourceRow, to);

    const QModelIndex &fromIdx = index(from, 0);
    emit dataChanged(fromIdx, fromIdx);

    if (to < oldCount) {
        const QModelIndex &toIdx = index(to, 0);
        emit dataChanged(toIdx, toIdx);
    } else {
        beginInsertRows(QModelIndex(), oldCount, to);
        endInsertRows();
    }

    int newCount = rowCount();

    if (newCount < oldCount) {
        beginRemoveRows(QModelIndex(), newCount, oldCount - 1);
        endRemoveRows();
    }

    if (!m_pendingPositionsUpdate) {
        m_pendingPositionsUpdate = true;
        QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);
    }
}

void Positioner::updatePositions()
{
    if (m_perStripe < 1) {
        m_pendingPositionsUpdate = false;

        return;
    }

    QStringList positions;

    if (m_enabled && !m_proxyToSource.isEmpty()) {
        positions.append(QString::number((1 + ((rowCount() - 1) / m_perStripe))));
        positions.append(QString::number(m_perStripe));

        QHashIterator<int, int> it(m_proxyToSource);

        while (it.hasNext()) {
            it.next();

            const QString &name = m_folderModel->data(m_folderModel->index(it.value(), 0),
                FolderModel::UrlRole).toString();

            if (name.isEmpty()) {
                qDebug() << it.value() << "Source model doesn't know this index!";

                return;
            }

            positions.append(name);
            positions.append(QString::number(qMax(0, it.key() / m_perStripe)));
            positions.append(QString::number(qMax(0, it.key() % m_perStripe)));
        }
    }

    setPositions(positions);

    m_pendingPositionsUpdate = false;
}

void Positioner::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
    const QVector<int>& roles)
{
    if (m_enabled) {
        int start = topLeft.row();
        int end = bottomRight.row();

        for (int i = start; i <= end; ++i) {
            if (m_sourceToProxy.contains(i)) {
                const QModelIndex &idx = index(m_sourceToProxy.value(i), 0);

                emit dataChanged(idx, idx);
            }
        }
    } else {
        emit dataChanged(topLeft, bottomRight, roles);
    }
}

void Positioner::sourceModelAboutToBeReset()
{
    emit beginResetModel();
}

void Positioner::sourceModelReset()
{
    emit endResetModel();
}

void Positioner::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    if (m_enabled) {
        if (m_sourceToProxy.isEmpty()) {
            if (m_positions.isEmpty()) {
                emit beginInsertRows(parent, start, end);

                initMaps(end);
            } else {
                m_applyPositions = true;
            }

            return;
        }

        int free = -1;
        int rest = -1;

        for (int i = start; i <= end; ++i) {
            free = firstFreeRow();

            if (free != -1) {
                m_proxyToSource.insert(free, i);
                m_lastIndex = -1;
                m_sourceToProxy.insert(i, free);
                m_pendingChanges << createIndex(free, 0);
            } else {
                rest = i;
                break;
            }
        }

        if (rest != -1) {
            int firstNew = lastIndex() + 1;
            int remainder = (end - rest);

            beginInsertRows(parent, firstNew, firstNew + remainder);

            for (int i = 0; i <= remainder; ++i) {
                m_proxyToSource.insert(firstNew + i, rest +i);
                m_lastIndex = -1;
                m_sourceToProxy.insert(rest + i, firstNew + i);
            }
        } else {
            m_ignoreNextTransaction = true;
        }
    } else {
        emit beginInsertRows(parent, start, end);
    }
}

void Positioner::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart,
    int sourceEnd, const QModelIndex& destinationParent, int destinationRow)
{
    qDebug();

    emit beginMoveRows(sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);
}

void Positioner::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (m_enabled) {
        int oldLast = lastIndex();

        for (int i = first; i <= last; ++i) {
            int proxyRow = m_sourceToProxy.take(i);
            m_proxyToSource.remove(proxyRow);

            QHash<int, int> newProxyToSource;
            QHashIterator<int, int> it(m_proxyToSource);

            while (it.hasNext()) {
                it.next();

                if (it.value() > i) {
                    newProxyToSource.insert(it.key(), it.value() - 1);
                } else {
                    newProxyToSource.insert(it.key(), it.value());
                }
            }

            m_proxyToSource = newProxyToSource;
            m_lastIndex = -1;

            QHash<int, int> newSourceToProxy;
            QHashIterator<int, int> it2(m_sourceToProxy);

            while (it2.hasNext()) {
                it2.next();

                if (it2.key() > i) {
                    newSourceToProxy.insert(it2.key() - 1, it2.value());
                } else {
                    newSourceToProxy.insert(it2.key(), it2.value());
                }
            }

            m_sourceToProxy = newSourceToProxy;

            m_pendingChanges << createIndex(proxyRow, 0);
        }

        int newLast = lastIndex();

        if (oldLast > newLast) {
            int diff = oldLast - newLast;
            beginRemoveRows(QModelIndex(), ((oldLast - diff) + 1), oldLast);
        } else {
            m_ignoreNextTransaction = true;
        }
    } else {
        emit beginRemoveRows(parent, first, last);
    }
}

void Positioner::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (!m_ignoreNextTransaction) {
        if (!m_applyPositions) {
            emit endInsertRows();
        } else {
            if (m_perStripe > 0) {
                applyPositions();
            }
        }
    } else {
        m_ignoreNextTransaction = false;
    }

    flushPendingChanges();

    if (!m_pendingPositionsUpdate && !m_applyPositions) {
        m_pendingPositionsUpdate = true;
        QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);
    }
}

void Positioner::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart,
    int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    qDebug();

    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

    emit endMoveRows();
}

void Positioner::sourceRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (!m_ignoreNextTransaction) {
        emit endRemoveRows();
    } else {
        m_ignoreNextTransaction = false;
    }

    flushPendingChanges();

    if (!m_pendingPositionsUpdate) {
        m_pendingPositionsUpdate = true;
        QMetaObject::invokeMethod(this, "updatePositions", Qt::QueuedConnection);
    }
}

void Positioner::initMaps(int size)
{
    m_proxyToSource.clear();
    m_sourceToProxy.clear();

    if (size == -1) {
        size = m_folderModel->rowCount() - 1;
    }

    if (!size) {
        return;
    }

    for (int i = 0; i <= size; ++i) {
        m_proxyToSource.insert(i, i);
        m_sourceToProxy.insert(i, i);
    }

    m_lastIndex = -1;
}

int Positioner::lastIndex() const
{
    if (!m_proxyToSource.isEmpty()) {
        if (m_lastIndex != -1) {
            return m_lastIndex;
        } else {
            QList<int> keys = m_proxyToSource.keys();
            qSort(keys);
            return keys.last();
        }
    }

    return 0;
}

int Positioner::firstFreeRow() const
{
    if (!m_proxyToSource.isEmpty()) {
        int last = lastIndex();

        for (int i = 0; i <= last; ++i) {
            if (!m_proxyToSource.contains(i)) {
                return i;
            }
        }
    }

    return -1;
}

void Positioner::applyPositions()
{
    if (m_positions.size() < 5) {
        if (m_folderModel) {
            if (m_resetting) {
                m_resetting = false;
            } else {
                beginResetModel();
                m_proxyToSource.clear();
                m_sourceToProxy.clear();
                endResetModel();

                initMaps();
                beginInsertRows(QModelIndex(), 0, lastIndex());
                endInsertRows();
            }
        }

        return;
    }

    beginResetModel();
    m_proxyToSource.clear();
    m_sourceToProxy.clear();
    endResetModel();

    QStringList positions(m_positions);

    bool ok = false;

    int stripes = positions.takeFirst().toInt(&ok);
    if (!ok) { return; }
    int perStripe = positions.takeFirst().toInt(&ok);
    if (!ok) { return; }

    Q_UNUSED(stripes)
    Q_UNUSED(perStripe)

    QString name;
    int stripe = -1;
    int pos = -1;
    int sourceIndex = -1;
    int index = -1;

    QHash<QString, int> sourceIndices;

    for (int i = 0; i < m_folderModel->rowCount(); ++i) {
        sourceIndices.insert(m_folderModel->data(m_folderModel->index(i, 0),
            FolderModel::UrlRole).toString(), i);
    }

    int items = positions.count() / 3;

    QStringList leftOvers;

    for (int i = 0; i < items; ++i) {
        name = positions.takeFirst();
        stripe = positions.takeFirst().toInt(&ok);
        if (!ok) { return; }
        pos = positions.takeFirst().toInt(&ok);
        if (!ok) { return; }

        if (pos <= m_perStripe) {
            if (!sourceIndices.contains(name)) {
                continue;
            } else {
                sourceIndex = sourceIndices.value(name);
            }

            index = (stripe * m_perStripe) + pos;

            if (m_proxyToSource.contains(index)) {
                continue;
            }

            m_proxyToSource.insert(index, sourceIndex);
            m_lastIndex = -1;
            m_sourceToProxy.insert(sourceIndex, index);

            sourceIndices.remove(name);
        } else {
            leftOvers.append(name);
            leftOvers.append(QString::number(stripe));
            leftOvers.append(QString::number(pos));
        }
    }

    items = leftOvers.count() / 3;

    for (int i = 0; i < items; ++i) {
        name = leftOvers.takeFirst();
        stripe = leftOvers.takeFirst().toInt(&ok);
        if (!ok) { return; }
        pos = leftOvers.takeFirst().toInt(&ok);
        if (!ok) { return; }

        if (!sourceIndices.contains(name)) {
            continue;
        } else {
            sourceIndex = sourceIndices.take(name);
        }

        index = firstFreeRow();

        if (index == -1) {
            index = lastIndex() + 1;
        }

        m_proxyToSource.insert(index, sourceIndex);
        m_lastIndex = -1;
        m_sourceToProxy.insert(sourceIndex, index);
    }

    QHashIterator<QString, int> it(sourceIndices);

    while (it.hasNext()) {
        it.next();

        index = firstFreeRow();

        if (index == -1) {
            index = lastIndex() + 1;
        }

        m_proxyToSource.insert(index, it.value());
        m_lastIndex = -1;
        m_sourceToProxy.insert(it.value(), index);
    }

    m_applyPositions = false;

    beginInsertRows(QModelIndex(), 0, lastIndex());
    endInsertRows();
}

void Positioner::flushPendingChanges()
{
    if (m_pendingChanges.isEmpty()) {
        return;
    }

    int last = lastIndex();

    foreach (const QModelIndex &idx, m_pendingChanges) {
        if (idx.row() <= last) {
            emit dataChanged(idx, idx);
        }
    }

    m_pendingChanges.clear();
}

void Positioner::connectSignals(FolderModel* model)
{
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(sourceDataChanged(QModelIndex,QModelIndex,QVector<int>)),
            Qt::UniqueConnection);
    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            Qt::UniqueConnection);
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)),
            Qt::UniqueConnection);
    connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)),
            Qt::UniqueConnection);
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)),
            Qt::UniqueConnection);
}

void Positioner::disconnectSignals(FolderModel* model)
{
    disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(sourceDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    disconnect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
}
