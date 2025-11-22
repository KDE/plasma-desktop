/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "positioner.h"
#include "foldermodel.h"

#include <cstdlib>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

namespace PositionsConstants
{
// Stripes and items per stripe
constexpr qsizetype headerSize = 2;

// filename, stripe, pos
constexpr qsizetype bucketSize = 3;

// Offsets in bucket
constexpr qsizetype filenameOffset = 0;
constexpr qsizetype stripeOffset = 1;
constexpr qsizetype posOffset = 2;
}

Positioner::Positioner(QObject *parent)
    : QAbstractItemModel(parent)
    , m_enabled(false)
    , m_folderModel(nullptr)
    , m_perStripe(0)
    , m_ignoreNextTransaction(false)
    , m_deferApplyPositions(false)
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
        m_enabled = enabled;

        beginResetModel();

        if (enabled && m_folderModel) {
            initMaps();
        }

        endResetModel();

        Q_EMIT enabledChanged();

        setPerStripe(m_perStripe);
    }
}

FolderModel *Positioner::folderModel() const
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

        if (m_folderModel) {
            connectSignals(m_folderModel);
            if (m_enabled) {
                initMaps();
            }
        }

        endResetModel();

        Q_EMIT folderModelChanged();

        setPerStripe(m_perStripe);
    }
}

int Positioner::perStripe() const
{
    return m_perStripe;
}

void Positioner::setPerStripe(int perStripe)
{
    // Make sure we have screen in use before perStripe update
    // to make sure we update correct positions
    if (perStripe > 0 && screenInUse()) {
        const bool perStripeUpdated = m_perStripe != perStripe;
        const bool resolutionUpdated = updateResolution();
        const int prevPerStripe = m_perStripe;
        if (perStripeUpdated) {
            m_perStripe = perStripe;
            Q_EMIT perStripeChanged();
        }
        if (m_enabled && (perStripeUpdated || resolutionUpdated) && !m_folderModel->unsortedModeOnDrop()) {
            const bool existingResolution = configurationHasResolution(m_resolution);
            if (existingResolution) {
                loadAndApplyPositionsConfig();
            } else if (m_applet && m_positions.isEmpty()) {
                // We don't have any positions yet, restore them from last used resolution
                const QString lastResolution = m_applet->config().group(QStringLiteral("General")).readEntry(QStringLiteral("lastResolution"));
                if (configurationHasResolution(lastResolution)) {
                    loadAndApplyPositionsConfig(lastResolution);
                }
            }
            // If no longer deferring positions, update them
            if (!m_deferApplyPositions) {
                // If it's a new resolution, preserve icon positions by using previous "perStripe" (if available)
                updatePositionsList(existingResolution ? 0 : prevPerStripe);
                if (!existingResolution) {
                    convertFolderModelData();
                    savePositionsConfig();
                }
            }
        }
    }
}

QStringList Positioner::positions() const
{
    return m_positions;
}

int Positioner::map(int row) const
{
    if (m_enabled && m_folderModel) {
        return m_proxyToSource.value(row, -1);
    }

    return row;
}

int Positioner::nearestItem(int currentIndex, Qt::ArrowType direction)
{
    if (!m_enabled || currentIndex >= rowCount()) {
        return -1;
    }

    if (currentIndex < 0) {
        return firstRow();
    }

    int hDirection = 0;
    int vDirection = 0;

    switch (direction) {
    case Qt::LeftArrow:
        hDirection = -1;
        break;
    case Qt::RightArrow:
        hDirection = 1;
        break;
    case Qt::UpArrow:
        vDirection = -1;
        break;
    case Qt::DownArrow:
        vDirection = 1;
        break;
    default:
        return -1;
    }

    QList<int> rows(m_proxyToSource.keys());
    std::sort(rows.begin(), rows.end());

    int nearestItem = -1;
    const QPoint currentPos(currentIndex % m_perStripe, currentIndex / m_perStripe);
    int lastDistance = -1;
    int distance = 0;

    for (const int row : std::as_const(rows)) {
        const QPoint pos(row % m_perStripe, row / m_perStripe);

        if (row == currentIndex) {
            continue;
        }

        if (hDirection == 0) {
            if (vDirection * pos.y() > vDirection * currentPos.y()) {
                distance = (pos - currentPos).manhattanLength();

                if (nearestItem == -1 || distance < lastDistance || (distance == lastDistance && pos.x() == currentPos.x())) {
                    nearestItem = row;
                    lastDistance = distance;
                }
            }
        } else if (vDirection == 0) {
            if (hDirection * pos.x() > hDirection * currentPos.x()) {
                distance = (pos - currentPos).manhattanLength();

                if (nearestItem == -1 || distance < lastDistance || (distance == lastDistance && pos.y() == currentPos.y())) {
                    nearestItem = row;
                    lastDistance = distance;
                }
            }
        }
    }

    return nearestItem;
}

bool Positioner::isBlank(int row) const
{
    if (!m_enabled && m_folderModel) {
        return m_folderModel->isBlank(row);
    }

    if (m_proxyToSource.contains(row) && m_folderModel && !m_folderModel->isBlank(m_proxyToSource.value(row))) {
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

    return m_sourceToProxy.value(sourceIndex, -1);
}

void Positioner::setRangeSelected(int anchor, int to)
{
    if (!m_folderModel) {
        return;
    }

    if (m_enabled) {
        QVariantList indices;

        for (int i = qMin(anchor, to); i <= qMax(anchor, to); ++i) {
            if (m_proxyToSource.contains(i)) {
                indices.append(m_proxyToSource.value(i));
            }
        }

        if (!indices.isEmpty()) {
            m_folderModel->updateSelection(indices, false);
        }
    } else {
        m_folderModel->setRangeSelected(anchor, to);
    }
}

QHash<int, QByteArray> Positioner::roleNames() const
{
    return FolderModel::staticRoleNames();
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

int Positioner::rowCount(const QModelIndex &parent) const
{
    if (m_folderModel) {
        if (m_enabled) {
            if (parent.isValid()) {
                return 0;
            } else {
                return lastRow() + 1;
            }
        } else {
            return m_folderModel->rowCount(parent);
        }
    }

    return 0;
}

int Positioner::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (m_folderModel) {
        return 1;
    }

    return 0;
}

void Positioner::reset()
{
    beginResetModel();

    initMaps();

    endResetModel();

    m_positions = QStringList();
    updatePositionsList();
    savePositionsConfig();
}

int Positioner::move(const QVariantList &moves, bool save)
{
    struct RowMove {
        int from;
        int to;
        int sourceRow;
    };

    // Don't allow moves while listing.
    if (m_folderModel->status() == FolderModel::Listing) {
        m_deferMovePositions.append(moves);
        return -1;
    }

    QList<int> fromIndices;
    QList<int> toIndices;
    QVariantList sourceRows;
    QList<RowMove> actualMoves;

    for (int i = 0; i < moves.count(); ++i) {
        const int isFrom = (i % 2 == 0);
        int v = moves[i].toInt();

        if (isFrom) {
            if (m_proxyToSource.contains(v)) {
                sourceRows.append(m_proxyToSource.value(v));
            } else {
                sourceRows.append(-1);
            }
            fromIndices.append(v);
        } else {
            // if the to position is taken by something different than ourself, take a new to position
            while (toIndices.contains(v) && moves[i - 1].toInt() != v) {
                ++v;
            }
            toIndices.append(v);
        }
    }

    const int oldCount = rowCount();
    int newEnd = oldCount - 1;
    int maxTo = -1;
    QSet<int> toBeRemoved;

    // NOTE: this is the same code repeated twice: first it "tries" the move to see what the final count would look like to know if there would be rows
    // insertions or removals. then do it for real enclosed in beginRemoveRows/endRemoveRows or beginInsertRows/endRemoveRows
    for (int i = 0; i < fromIndices.count(); ++i) {
        const int from = fromIndices[i];
        int to = toIndices[i];
        const int sourceRow = sourceRows[i].toInt();

        if (sourceRow == -1 || from == to) {
            continue;
        }

        if (to == -1) {
            to = firstFreeRow();

            if (to == -1) {
                to = lastRow() + 1;
            }
        }

        if (!fromIndices.contains(to) && !isBlank(to)) {
            /* find the next blank space
             * we won't be happy if we're moving two icons to the same place
             */
            while ((!isBlank(to) && from != to) || toIndices.contains(to)) {
                to++;
            }
        }

        toIndices[i] = to;

        if (toBeRemoved.contains(to)) {
            toBeRemoved.remove(to);
        }

        if (from == newEnd) {
            for (int i = oldCount - 1; i >= 0 && (isBlank(i) || toBeRemoved.contains(i)); --i) {
                newEnd = i;
            }
            toBeRemoved.insert(newEnd);
        }
        maxTo = std::max(maxTo, to);
        newEnd = std::max(newEnd, maxTo);
        actualMoves.append({from, to, sourceRow});
    }

    if (newEnd < oldCount - 1) {
        beginRemoveRows(QModelIndex(), newEnd + 1, oldCount - 1);
    } else if (newEnd > oldCount - 1 && !m_beginInsertRowsCalled) {
        beginInsertRows(QModelIndex(), oldCount, newEnd);
    }

    for (const RowMove &move : std::as_const(actualMoves)) {
        if (!toIndices.contains(move.from)) {
            m_proxyToSource.remove(move.from);
        }

        updateMaps(move.to, move.sourceRow);

        const QModelIndex &fromIdx = index(move.from, 0);
        Q_EMIT dataChanged(fromIdx, fromIdx);

        if (move.to < oldCount) {
            const QModelIndex &toIdx = index(move.to, 0);
            Q_EMIT dataChanged(toIdx, toIdx);
        }
    }

    if (newEnd < oldCount - 1) {
        endRemoveRows();
    } else if (newEnd > oldCount - 1) {
        endInsertRows();
        m_beginInsertRowsCalled = false;
    }

    m_folderModel->updateSelection(sourceRows, true);

    updatePositionsList();
    if (save) {
        savePositionsConfig();
    }

    return toIndices.constFirst();
}

void Positioner::updatePositionsList(int perStripeForPositions)
{
    QStringList positions;

    if (m_enabled && screenInUse() && m_perStripe > 0) {
        if (perStripeForPositions <= 0)
            perStripeForPositions = m_perStripe;

        positions.reserve(PositionsConstants::headerSize + m_proxyToSource.size() * PositionsConstants::bucketSize);

        positions.append(QString::number(1 + ((rowCount() - 1) / m_perStripe)));
        positions.append(QString::number(m_perStripe));

        QHashIterator<int, int> it(m_proxyToSource);

        while (it.hasNext()) {
            it.next();

            const QString &name = m_folderModel->data(m_folderModel->index(it.value(), 0), FolderModel::UrlRole).toString();

            if (name.isEmpty()) {
                return;
            }

            positions.append(name);
            positions.append(QString::number(qMax(0, it.key() / perStripeForPositions)));
            positions.append(QString::number(qMax(0, it.key() % perStripeForPositions)));
        }
        m_positions = std::move(positions);
    }
}

void Positioner::sourceStatusChanged()
{
    if (m_deferApplyPositions && m_folderModel->status() != FolderModel::Listing) {
        convertFolderModelData();
        // If no longer deferring positions, update them
        if (!m_deferApplyPositions) {
            updatePositionsList();
        }
    }

    // When deferring moves, skip saving if screen is not in use
    // If screen is in use, this is likely because user created new file
    // so we want to move that file to the spot of creation
    if (m_deferMovePositions.count() > 0 && m_folderModel->status() != FolderModel::Listing) {
        move(m_deferMovePositions, screenInUse());
        m_deferMovePositions.clear();
        // Load the configuration to make sure any of the moved items that are in the configuration
        // are in their correct place after deferred movements
        loadAndApplyPositionsConfig();
    }
}

void Positioner::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    if (m_enabled) {
        int start = topLeft.row();
        int end = bottomRight.row();

        for (int i = start; i <= end; ++i) {
            if (m_sourceToProxy.contains(i)) {
                const QModelIndex &idx = index(m_sourceToProxy.value(i), 0);
                Q_EMIT dataChanged(idx, idx);
            }
        }
    } else {
        Q_EMIT dataChanged(topLeft, bottomRight, roles);
    }
}

void Positioner::sourceModelAboutToBeReset()
{
    beginResetModel();
}

void Positioner::sourceModelReset()
{
    if (m_enabled) {
        initMaps();
    }

    endResetModel();
}

void Positioner::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    if (m_enabled) {
        // Don't insert yet if we're waiting for listing to complete to apply
        // initial positions;
        if (m_deferApplyPositions) {
            return;
        } else if (m_proxyToSource.isEmpty()) {
            beginInsertRows(parent, start, end);
            m_beginInsertRowsCalled = true;

            initMaps(end + 1);

            return;
        }

        // When new rows are inserted, they might go in the beginning or in the middle.
        // In this case we must update first the existing proxy->source and source->proxy
        // mapping, otherwise the proxy items will point to the wrong source item.
        int count = end - start + 1;
        m_sourceToProxy.clear();
        for (auto it = m_proxyToSource.begin(); it != m_proxyToSource.end(); ++it) {
            int sourceIdx = *it;
            if (sourceIdx >= start) {
                *it += count;
            }
            m_sourceToProxy[*it] = it.key();
        }

        int free = -1;
        int rest = -1;

        for (int i = start; i <= end; ++i) {
            free = firstFreeRow();

            if (free != -1) {
                updateMaps(free, i);
                m_pendingChanges << createIndex(free, 0);
            } else {
                rest = i;
                break;
            }
        }

        if (rest != -1) {
            int firstNew = lastRow() + 1;
            int remainder = (end - rest);

            beginInsertRows(parent, firstNew, firstNew + remainder);
            m_beginInsertRowsCalled = true;

            for (int i = 0; i <= remainder; ++i) {
                updateMaps(firstNew + i, rest + i);
            }
        } else {
            m_ignoreNextTransaction = true;
        }
    } else {
        beginInsertRows(parent, start, end);
        m_beginInsertRowsCalled = true;
    }
}

void Positioner::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent,
                                          int sourceStart,
                                          int sourceEnd,
                                          const QModelIndex &destinationParent,
                                          int destinationRow)
{
    beginMoveRows(sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);
}

void Positioner::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (m_enabled) {
        int oldLast = lastRow();

        for (int i = first; i <= last; ++i) {
            int proxyRow = m_sourceToProxy.take(i);
            // Don't touch m_proxyToSource yet, as beginRemoveRows was not emitted yet
            m_pendingChanges << createIndex(proxyRow, 0);
        }

        QHash<int, int> newProxyToSource;
        QHash<int, int> newSourceToProxy;
        QHashIterator<int, int> it(m_sourceToProxy);
        int delta = std::abs(first - last) + 1;

        while (it.hasNext()) {
            it.next();

            if (it.key() > last) {
                newProxyToSource.insert(it.value(), it.key() - delta);
                newSourceToProxy.insert(it.key() - delta, it.value());
            } else {
                newProxyToSource.insert(it.value(), it.key());
                newSourceToProxy.insert(it.key(), it.value());
            }
        }

        int newLast = 0;
        // Duplicate lastRow instead of assigning m_proxyToSource now as rowCount() can't be changed before beginRemoveRows
        if (!newProxyToSource.isEmpty()) {
            QList<int> keys(newProxyToSource.keys());
            std::sort(keys.begin(), keys.end());
            newLast = keys.last();
        }

        if (oldLast > newLast) {
            int diff = oldLast - newLast;
            beginRemoveRows(QModelIndex(), ((oldLast - diff) + 1), oldLast);
        } else {
            m_ignoreNextTransaction = true;
        }

        m_proxyToSource = newProxyToSource;
        m_sourceToProxy = newSourceToProxy;
    } else {
        beginRemoveRows(parent, first, last);
    }
}

void Positioner::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)

    Q_EMIT layoutAboutToBeChanged(QList<QPersistentModelIndex>(), hint);
}

void Positioner::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (!m_ignoreNextTransaction) {
        if (m_beginInsertRowsCalled) {
            endInsertRows();
            m_beginInsertRowsCalled = false;
        }
    } else {
        m_ignoreNextTransaction = false;
    }

    flushPendingChanges();

    // Don't generate new positions data if we're waiting for listing to
    // complete to apply initial positions.
    if (!m_deferApplyPositions && screenInUse()) {
        // Load current config
        loadAndApplyPositionsConfig();
        // Update positions to append the missing items
        updatePositionsList();
    }
}

void Positioner::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

    endMoveRows();
}

void Positioner::sourceRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (!m_ignoreNextTransaction) {
        Q_EMIT endRemoveRows();
    } else {
        m_ignoreNextTransaction = false;
    }

    flushPendingChanges();

    if (screenInUse()) {
        // Load current config
        loadAndApplyPositionsConfig();
        // Update positions to remove the missing items
        // We do not wait for defer here since we want this to happen instantly
        // due to this being user action
        updatePositionsList();
    }
}

void Positioner::sourceLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)

    if (m_enabled) {
        initMaps();
    }

    Q_EMIT layoutChanged(QList<QPersistentModelIndex>(), hint);
}

void Positioner::initMaps(int size)
{
    m_proxyToSource.clear();
    m_sourceToProxy.clear();

    if (size == -1) {
        size = m_folderModel->rowCount();
    }

    if (!size) {
        return;
    }

    for (int i = 0; i < size; ++i) {
        updateMaps(i, i);
    }
}

void Positioner::updateMaps(int proxyIndex, int sourceIndex)
{
    m_proxyToSource.insert(proxyIndex, sourceIndex);
    m_sourceToProxy.insert(sourceIndex, proxyIndex);
}

int Positioner::firstRow() const
{
    if (!m_proxyToSource.isEmpty()) {
        QList<int> keys(m_proxyToSource.keys());
        std::sort(keys.begin(), keys.end());

        return keys.first();
    }

    return -1;
}

int Positioner::lastRow() const
{
    if (!m_proxyToSource.isEmpty()) {
        QList<int> keys(m_proxyToSource.keys());
        std::sort(keys.begin(), keys.end());
        return keys.last();
    }

    return 0;
}

int Positioner::firstFreeRow() const
{
    if (!m_proxyToSource.isEmpty()) {
        int last = lastRow();

        for (int i = 0; i <= last; ++i) {
            if (!m_proxyToSource.contains(i)) {
                return i;
            }
        }
    }

    return -1;
}

void Positioner::convertFolderModelData()
{
    // If no screen or no positions, we have nothing to convert
    if (!screenInUse() || positionsEmpty()) {
        return;
    }
    // We were called while the source model is listing. Defer applying positions
    // until listing completes.
    if (m_folderModel->status() == FolderModel::Listing) {
        m_deferApplyPositions = true;

        return;
    }

    // Do not allow saving during this operation
    beginResetModel();

    m_proxyToSource.clear();
    m_sourceToProxy.clear();

    const int folderModelRowCount = m_folderModel->rowCount();

    QHash<QString, int> sourceIndices;
    sourceIndices.reserve(folderModelRowCount);

    for (int i = 0; i < folderModelRowCount; ++i) {
        sourceIndices.insert(m_folderModel->data(m_folderModel->index(i, 0), FolderModel::UrlRole).toString(), i);
    }

    // Restore positions for items that still fit.
    iteratePositions([&](qsizetype offset) {
        bool ok = false;

        const int pos = m_positions.at(offset + PositionsConstants::posOffset).toInt(&ok);
        if (!ok) {
            return true;
        }

        if (pos <= m_perStripe) {
            const QString name = m_positions.at(offset);
            const int stripe = m_positions.at(offset + PositionsConstants::stripeOffset).toInt(&ok);
            if (!ok) {
                return true;
            }

            if (!sourceIndices.contains(name)) {
                return true;
            }

            const int sourceIndex = sourceIndices.value(name);

            const int index = (stripe * m_perStripe) + pos;

            if (m_proxyToSource.contains(index)) {
                return true;
            }

            updateMaps(index, sourceIndex);
            sourceIndices.remove(name);
        }

        return true;
    });

    // Find new positions for items that didn't fit.
    iteratePositions([&](qsizetype offset) {
        bool ok = false;

        const int pos = m_positions.at(offset + PositionsConstants::posOffset).toInt(&ok);
        if (!ok) {
            return true;
        }

        if (pos > m_perStripe) {
            const QString name = m_positions.at(offset);

            if (!sourceIndices.contains(name)) {
                return true;
            }

            const int sourceIndex = sourceIndices.take(name);

            int index = firstFreeRow();

            if (index == -1) {
                index = lastRow() + 1;
            }

            updateMaps(index, sourceIndex);
        }

        return true;
    });

    QHashIterator<QString, int> it(sourceIndices);

    // Find positions for new source items we don't have records for.
    while (it.hasNext()) {
        it.next();

        int index = firstFreeRow();

        if (index == -1) {
            index = lastRow() + 1;
        }

        updateMaps(index, it.value());
    }

    endResetModel();

    m_deferApplyPositions = false;
}

void Positioner::flushPendingChanges()
{
    if (m_pendingChanges.isEmpty()) {
        return;
    }

    int last = lastRow();

    for (const QModelIndex &index : std::as_const(m_pendingChanges)) {
        if (index.row() <= last) {
            Q_EMIT dataChanged(index, index);
        }
    }

    m_pendingChanges.clear();
}

Plasma::Applet *Positioner::applet() const
{
    return m_applet;
}

void Positioner::setApplet(Plasma::Applet *applet)
{
    if (m_applet != applet) {
        Q_ASSERT(!m_applet);
        m_applet = applet;

        Q_EMIT appletChanged();
    }
}

bool Positioner::screenInUse() const
{
    if (!m_folderModel) {
        return false;
    }
    return m_folderModel->screenUsed();
}

void Positioner::loadAndApplyPositionsConfig(const QString &resolution)
{
    const QString &resolutionToLoad = resolution.isEmpty() ? m_resolution : resolution;
    if (m_applet && m_enabled && screenInUse() && !resolutionToLoad.isEmpty()) {
        // The old configuration has commas with escape characters, so clean up those from the config
        auto confdata = loadConfigData();
        const QJsonDocument doc = QJsonDocument::fromJson(confdata.toUtf8());
        m_positions = doc[resolutionToLoad].toVariant().toStringList();

        convertFolderModelData();
    }
}

void Positioner::savePositionsConfig()
{
    if (m_applet && m_enabled && screenInUse() && !m_resolution.isEmpty()) {
        auto confdata = loadConfigData();
        auto doc = QJsonDocument::fromJson(confdata.toUtf8());
        QJsonObject root;
        // Iterate over the old data
        for (const auto &item : doc.toVariant().toMap().asKeyValueRange()) {
            if (item.first != m_resolution) {
                root.insert(item.first, QJsonValue::fromVariant(item.second));
            }
        }
        // Append our new item
        root.insert(m_resolution, QJsonArray::fromStringList(m_positions));

        const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Compact);
        auto config = m_applet->config().group(QStringLiteral("General"));
        config.writeEntry(QStringLiteral("lastResolution"), m_resolution);
        config.writeEntry(QStringLiteral("positions"), data);
        Q_EMIT m_applet->configNeedsSaving();
    }
}

bool Positioner::updateResolution()
{
    const QString prevResolution = m_resolution;
    QSize size;
    if (m_enabled && screenInUse()) {
        Q_ASSERT(m_folderModel);
        const QSizeF sizeF = m_folderModel->screenGeometry().size();
        size = QSize(std::floor(sizeF.width()), std::floor(sizeF.height()));
    }
    if (size.isEmpty()) {
        m_resolution.clear();
    } else {
        m_resolution = QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
    }
    return m_resolution != prevResolution;
}

bool Positioner::configurationHasResolution(const QString &resolution) const
{
    if (resolution.isEmpty()) {
        return false;
    }
    auto confdata = loadConfigData();
    if (confdata.isEmpty()) {
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(confdata.toUtf8());
    return doc.object().contains(resolution);
}

QString Positioner::loadConfigData() const
{
    QString confdata;
    if (m_applet) {
        confdata =
            m_applet->config().group(QStringLiteral("General")).readEntry(QStringLiteral("positions")).replace(QStringLiteral("\\,"), QStringLiteral(","));
    }
    return confdata;
}

bool Positioner::positionsEmpty() const
{
    // Make sure we have at least one position and the items per row have 3 items: filename, row, stripe-pos
    return m_positions.size() <= PositionsConstants::headerSize || (m_positions.size() - PositionsConstants::headerSize) % PositionsConstants::bucketSize != 0;
}

void Positioner::iteratePositions(auto &&callback)
{
    // This function requires that "positionsEmpty()" is already checked
    Q_ASSERT(!positionsEmpty());

    for (qsizetype i = PositionsConstants::headerSize; i < m_positions.size(); i += PositionsConstants::bucketSize) {
        if (!callback(i + PositionsConstants::filenameOffset)) {
            break;
        }
    }
}

qsizetype Positioner::findUrlInPositions(const QString &filename)
{
    qsizetype pos = -1;

    if (filename.isEmpty() || positionsEmpty()) {
        return pos;
    }

    iteratePositions([&](qsizetype offset) {
        if (m_positions.at(offset + PositionsConstants::filenameOffset) == filename) {
            pos = offset;
            return false;
        }
        return true;
    });

    return pos;
}

void Positioner::onItemAboutToRename(const QString &filename)
{
    // Store old item position
    if (const qsizetype i = findUrlInPositions(filename); i > -1) {
        // Copy 3 items into "m_toRename": filename, row, stripe-pos
        m_toRename = m_positions.mid(i, PositionsConstants::bucketSize);
    }
}

void Positioner::onItemRenamed(const QString &filename, const QString &newFilename)
{
    updatePositionsList();
    // "m_toRename" has 3 items: filename, row, stripe-pos
    if (m_toRename.size() == PositionsConstants::bucketSize && m_toRename.at(PositionsConstants::filenameOffset) == filename && filename != newFilename) {
        // Restore item position after rename
        if (const qsizetype i = findUrlInPositions(newFilename); i > -1) {
            // Assign two items: row, stripe-pos
            m_positions[i + PositionsConstants::stripeOffset] = m_toRename.at(PositionsConstants::stripeOffset);
            m_positions[i + PositionsConstants::posOffset] = m_toRename.at(PositionsConstants::posOffset);
            convertFolderModelData();
        }
    }
    m_toRename.clear();
    savePositionsConfig();
}

// Save positions only when listing is done, so we
// do not save them during they're being moved due to
// geometry changes
void Positioner::onListingCompleted()
{
    if (screenInUse()) {
        savePositionsConfig();
    }
}

void Positioner::connectSignals(FolderModel *model)
{
    connect(model, &QAbstractItemModel::dataChanged, this, &Positioner::sourceDataChanged, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &Positioner::sourceRowsAboutToBeInserted, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &Positioner::sourceRowsAboutToBeMoved, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &Positioner::sourceRowsAboutToBeRemoved, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &Positioner::sourceLayoutAboutToBeChanged, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::rowsInserted, this, &Positioner::sourceRowsInserted, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::rowsMoved, this, &Positioner::sourceRowsMoved, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &Positioner::sourceRowsRemoved, Qt::UniqueConnection);
    connect(model, &QAbstractItemModel::layoutChanged, this, &Positioner::sourceLayoutChanged, Qt::UniqueConnection);
    connect(m_folderModel, &FolderModel::urlChanged, this, &Positioner::reset, Qt::UniqueConnection);
    connect(m_folderModel, &FolderModel::statusChanged, this, &Positioner::sourceStatusChanged, Qt::UniqueConnection);
    connect(m_folderModel, &FolderModel::itemAboutToRename, this, &Positioner::onItemAboutToRename, Qt::UniqueConnection);
    connect(m_folderModel, &FolderModel::itemRenamed, this, &Positioner::onItemRenamed, Qt::UniqueConnection);
    connect(m_folderModel, &FolderModel::listingCompleted, this, &Positioner::onListingCompleted, Qt::UniqueConnection);
}

void Positioner::disconnectSignals(FolderModel *model)
{
    disconnect(model, &QAbstractItemModel::dataChanged, this, &Positioner::sourceDataChanged);
    disconnect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &Positioner::sourceRowsAboutToBeInserted);
    disconnect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &Positioner::sourceRowsAboutToBeMoved);
    disconnect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &Positioner::sourceRowsAboutToBeRemoved);
    disconnect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &Positioner::sourceLayoutAboutToBeChanged);
    disconnect(model, &QAbstractItemModel::rowsInserted, this, &Positioner::sourceRowsInserted);
    disconnect(model, &QAbstractItemModel::rowsMoved, this, &Positioner::sourceRowsMoved);
    disconnect(model, &QAbstractItemModel::rowsRemoved, this, &Positioner::sourceRowsRemoved);
    disconnect(model, &QAbstractItemModel::layoutChanged, this, &Positioner::sourceLayoutChanged);
    disconnect(m_folderModel, &FolderModel::urlChanged, this, &Positioner::reset);
    disconnect(m_folderModel, &FolderModel::statusChanged, this, &Positioner::sourceStatusChanged);
    disconnect(m_folderModel, &FolderModel::itemAboutToRename, this, &Positioner::onItemAboutToRename);
    disconnect(m_folderModel, &FolderModel::itemRenamed, this, &Positioner::onItemRenamed);
    disconnect(m_folderModel, &FolderModel::listingCompleted, this, &Positioner::onListingCompleted);
}

#include "moc_positioner.cpp"
