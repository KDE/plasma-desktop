/*
    SPDX-FileCopyrightText: 2015 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileCopyrightText: 2015 David Faure <david.faure@kdab.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "shortcutsmodel.h"

class ShortcutsModelPrivate
{
public:
    ShortcutsModelPrivate(ShortcutsModel *model)
        : q(model)
    {
    }

    int computeRowsPrior(const QAbstractItemModel *sourceModel) const;
    QAbstractItemModel *sourceModelForRow(int row, int *sourceRow) const;

    void slotRowsAboutToBeInserted(const QModelIndex &, int start, int end);
    void slotRowsInserted(const QModelIndex &, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex &, int start, int end);
    void slotRowsRemoved(const QModelIndex &, int start, int end);
    void slotColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void slotColumnsInserted(const QModelIndex &parent, int, int);
    void slotColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void slotColumnsRemoved(const QModelIndex &parent, int, int);
    void slotDataChanged(const QModelIndex &from, const QModelIndex &to, const QVector<int> &roles);
    void slotSourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void slotSourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void slotModelAboutToBeReset();
    void slotModelReset();

    ShortcutsModel *q;
    QList<QAbstractItemModel *> m_models;
    int m_rowCount = 0; // have to maintain it here since we can't compute during model destruction

    // for layoutAboutToBeChanged/layoutChanged
    QVector<QPersistentModelIndex> layoutChangePersistentIndexes;
    QModelIndexList proxyIndexes;
};

ShortcutsModel::ShortcutsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new ShortcutsModelPrivate(this))
{
}

ShortcutsModel::~ShortcutsModel()
{
}

QModelIndex ShortcutsModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    const QAbstractItemModel *sourceModel = sourceIndex.model();
    if (!sourceModel) {
        return {};
    }
    int rowsPrior = d->computeRowsPrior(sourceModel);

    if (sourceIndex.parent().isValid()) {
        return createIndex(sourceIndex.row(), sourceIndex.column(), rowsPrior + sourceIndex.parent().row() + 1);
    }
    return createIndex(rowsPrior + sourceIndex.row(), sourceIndex.column());
}

QModelIndex ShortcutsModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    int sourceRow;
    int topLevelRow = proxyIndex.internalId() ? proxyIndex.internalId() - 1 : proxyIndex.row();
    QAbstractItemModel *sourceModel = d->sourceModelForRow(topLevelRow, &sourceRow);
    if (!sourceModel) {
        return QModelIndex();
    }

    if (proxyIndex.internalId()) {
        return sourceModel->index(proxyIndex.row(), proxyIndex.column(), sourceModel->index(sourceRow, proxyIndex.column()));
    }
    return sourceModel->index(sourceRow, proxyIndex.column());
}

QVariant ShortcutsModel::data(const QModelIndex &index, int role) const
{
    const QModelIndex sourceIndex = mapToSource(index);
    if (!sourceIndex.isValid()) {
        return QVariant();
    }
    return sourceIndex.model()->data(sourceIndex, role);
}

bool ShortcutsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const QModelIndex sourceIndex = mapToSource(index);
    if (!sourceIndex.isValid()) {
        return false;
    }
    QAbstractItemModel *sourceModel = const_cast<QAbstractItemModel *>(sourceIndex.model());
    return sourceModel->setData(sourceIndex, value, role);
}

QMap<int, QVariant> ShortcutsModel::itemData(const QModelIndex &proxyIndex) const
{
    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) {
        return {};
    }
    return sourceIndex.model()->itemData(sourceIndex);
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex &index) const
{
    const QModelIndex sourceIndex = mapToSource(index);
    return sourceIndex.isValid() ? sourceIndex.model()->flags(sourceIndex) : Qt::ItemFlags();
}

QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (d->m_models.isEmpty()) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        return d->m_models.at(0)->headerData(section, orientation, role);
    } else {
        int sourceRow;
        QAbstractItemModel *sourceModel = d->sourceModelForRow(section, &sourceRow);
        if (!sourceModel) {
            return QVariant();
        }
        return sourceModel->headerData(sourceRow, orientation, role);
    }
}

int ShortcutsModel::columnCount(const QModelIndex &parent) const
{
    if (d->m_models.isEmpty()) {
        return 0;
    }
    if (parent.isValid()) {
        const QModelIndex sourceParent = mapToSource(parent);
        return sourceParent.model()->columnCount(sourceParent);
    }
    return d->m_models.at(0)->columnCount(QModelIndex());
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    if (d->m_models.isEmpty()) {
        return {};
    }
    return d->m_models.at(0)->roleNames();
}

QModelIndex ShortcutsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0) {
        return {};
    }
    if (column < 0) {
        return {};
    }

    if (parent.isValid()) {
        const QModelIndex sourceParent = mapToSource(parent);
        return mapFromSource(sourceParent.model()->index(row, column, sourceParent));
    }

    int sourceRow;
    QAbstractItemModel *sourceModel = d->sourceModelForRow(row, &sourceRow);
    if (!sourceModel) {
        return QModelIndex();
    }
    return mapFromSource(sourceModel->index(sourceRow, column, parent));
}

QModelIndex ShortcutsModel::parent(const QModelIndex &child) const
{
    return mapFromSource(mapToSource(child).parent());
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        const QModelIndex sourceParent = mapToSource(parent);
        return sourceParent.model()->rowCount(sourceParent);
    }

    return d->m_rowCount;
}

void ShortcutsModel::addSourceModel(QAbstractItemModel *sourceModel)
{
    Q_ASSERT(sourceModel);
    Q_ASSERT(!d->m_models.contains(sourceModel));
    // clang-format off
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(slotDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(slotRowsInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotRowsRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(slotRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(columnsInserted(QModelIndex,int,int)), this, SLOT(slotColumnsInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsRemoved(QModelIndex,int,int)), this, SLOT(slotColumnsRemoved(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(slotColumnsAboutToBeInserted(QModelIndex,int,int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(slotColumnsAboutToBeRemoved(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            this, SLOT(slotSourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
    connect(sourceModel, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            this, SLOT(slotSourceLayoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
    connect(sourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(slotModelAboutToBeReset()));
    connect(sourceModel, SIGNAL(modelReset()), this, SLOT(slotModelReset()));
    // clang-format on

    const int newRows = sourceModel->rowCount();
    if (newRows > 0) {
        beginInsertRows(QModelIndex(), d->m_rowCount, d->m_rowCount + newRows - 1);
    }
    d->m_rowCount += newRows;
    d->m_models.append(sourceModel);
    if (newRows > 0) {
        endInsertRows();
    }
}

QList<QAbstractItemModel *> ShortcutsModel::sources() const
{
    return d->m_models;
}

void ShortcutsModel::removeSourceModel(QAbstractItemModel *sourceModel)
{
    Q_ASSERT(d->m_models.contains(sourceModel));
    disconnect(sourceModel, nullptr, this, nullptr);

    const int rowsRemoved = sourceModel->rowCount();
    const int rowsPrior = d->computeRowsPrior(sourceModel); // location of removed section

    if (rowsRemoved > 0) {
        beginRemoveRows(QModelIndex(), rowsPrior, rowsPrior + rowsRemoved - 1);
    }
    d->m_models.removeOne(sourceModel);
    d->m_rowCount -= rowsRemoved;
    if (rowsRemoved > 0) {
        endRemoveRows();
    }
}

void ShortcutsModelPrivate::slotRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
    const QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(q->sender());
    if (sourceParent.isValid()) {
        q->beginInsertRows(q->mapFromSource(sourceParent), start, end);
    } else {
        const int rowsPrior = computeRowsPrior(model);
        q->beginInsertRows(QModelIndex(), rowsPrior + start, rowsPrior + end);
    }
}

void ShortcutsModelPrivate::slotRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    if (!sourceParent.isValid()) {
        m_rowCount += end - start + 1;
    }
    q->endInsertRows();
}

void ShortcutsModelPrivate::slotRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    if (sourceParent.isValid()) {
        q->beginRemoveRows(q->mapFromSource(sourceParent), start, end);
    } else {
        const QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(q->sender());
        const int rowsPrior = computeRowsPrior(model);
        q->beginRemoveRows(QModelIndex(), rowsPrior + start, rowsPrior + end);
    }
}

void ShortcutsModelPrivate::slotRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
    if (!sourceParent.isValid()) {
        m_rowCount -= end - start + 1;
    }
    q->endRemoveRows();
}

void ShortcutsModelPrivate::slotColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) { // we are flat
        q->beginInsertColumns(q->mapFromSource(parent), start, end);
    }
    const QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(q->sender());
    if (m_models.at(0) == model) {
        q->beginInsertColumns(QModelIndex(), start, end);
    }
}

void ShortcutsModelPrivate::slotColumnsInserted(const QModelIndex &parent, int, int)
{
    const QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(q->sender());
    if (m_models.at(0) == model || parent.isValid()) {
        q->endInsertColumns();
    }
}

void ShortcutsModelPrivate::slotColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) {
        q->beginRemoveColumns(q->mapFromSource(parent), start, end);
    }
    const QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(q->sender());
    if (m_models.at(0) == model) {
        q->beginRemoveColumns(QModelIndex(), start, end);
    }
}

void ShortcutsModelPrivate::slotColumnsRemoved(const QModelIndex &parent, int, int)
{
    const QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(q->sender());
    if (m_models.at(0) == model || parent.isValid()) {
        q->endRemoveColumns();
    }
}

void ShortcutsModelPrivate::slotDataChanged(const QModelIndex &from, const QModelIndex &to, const QVector<int> &roles)
{
    if (!from.isValid()) { // QSFPM bug, it emits dataChanged(invalid, invalid) if a cell in a hidden column changes
        return;
    }
    const QModelIndex myFrom = q->mapFromSource(from);
    const QModelIndex myTo = q->mapFromSource(to);
    Q_EMIT q->dataChanged(myFrom, myTo, roles);
}

void ShortcutsModelPrivate::slotSourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> parents;
    parents.reserve(sourceParents.size());
    for (const QPersistentModelIndex &parent : sourceParents) {
        if (!parent.isValid()) {
            parents << QPersistentModelIndex();
            continue;
        }
        const QModelIndex mappedParent = q->mapFromSource(parent);
        Q_ASSERT(mappedParent.isValid());
        parents << mappedParent;
    }

    Q_EMIT q->layoutAboutToBeChanged(parents, hint);

    const QModelIndexList persistentIndexList = q->persistentIndexList();
    layoutChangePersistentIndexes.reserve(persistentIndexList.size());

    for (const QPersistentModelIndex &proxyPersistentIndex : persistentIndexList) {
        proxyIndexes << proxyPersistentIndex;
        Q_ASSERT(proxyPersistentIndex.isValid());
        const QPersistentModelIndex srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
        Q_ASSERT(srcPersistentIndex.isValid());
        layoutChangePersistentIndexes << srcPersistentIndex;
    }
}

void ShortcutsModelPrivate::slotSourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    for (int i = 0; i < proxyIndexes.size(); ++i) {
        const QModelIndex proxyIdx = proxyIndexes.at(i);
        QModelIndex newProxyIdx = q->mapFromSource(layoutChangePersistentIndexes.at(i));
        q->changePersistentIndex(proxyIdx, newProxyIdx);
    }

    layoutChangePersistentIndexes.clear();
    proxyIndexes.clear();

    QList<QPersistentModelIndex> parents;
    parents.reserve(sourceParents.size());
    for (const QPersistentModelIndex &parent : sourceParents) {
        if (!parent.isValid()) {
            parents << QPersistentModelIndex();
            continue;
        }
        const QModelIndex mappedParent = q->mapFromSource(parent);
        Q_ASSERT(mappedParent.isValid());
        parents << mappedParent;
    }
    Q_EMIT q->layoutChanged(parents, hint);
}

void ShortcutsModelPrivate::slotModelAboutToBeReset()
{
    const QAbstractItemModel *sourceModel = qobject_cast<const QAbstractItemModel *>(q->sender());
    Q_ASSERT(m_models.contains(const_cast<QAbstractItemModel *>(sourceModel)));
    q->beginResetModel();
}

void ShortcutsModelPrivate::slotModelReset()
{
    m_rowCount = computeRowsPrior(nullptr);
    q->endResetModel();
}

int ShortcutsModelPrivate::computeRowsPrior(const QAbstractItemModel *sourceModel) const
{
    int rowsPrior = 0;
    for (const QAbstractItemModel *model : qAsConst(m_models)) {
        if (model == sourceModel) {
            break;
        }
        rowsPrior += model->rowCount();
    }
    return rowsPrior;
}

QAbstractItemModel *ShortcutsModelPrivate::sourceModelForRow(int row, int *sourceRow) const
{
    int rowCount = 0;
    QAbstractItemModel *selection = nullptr;
    for (QAbstractItemModel *model : qAsConst(m_models)) {
        const int subRowCount = model->rowCount();
        if (rowCount + subRowCount > row) {
            selection = model;
            break;
        }
        rowCount += subRowCount;
    }
    *sourceRow = row - rowCount;
    return selection;
}

#include "moc_shortcutsmodel.cpp"
