/*
 * Copyright 2015 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 * Copyright 2015 David Faure <david.faure@kdab.com>
 * Copyright 2020 David Redondo <kde@david-redondo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QAbstractItemModel>
#include <QScopedPointer>

/*
 * This class is based on KConcatenateRowsProxyModel adapted to handle trees with two levels.
 */

class ShortcutsModelPrivate;

class ShortcutsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /**
     * Creates a ShortcutsModel.
     * @param parent optional parent
     */
    explicit ShortcutsModel(QObject *parent = nullptr);
    /**
     * Destructor.
     */
    ~ShortcutsModel() override;

    /**
     * Adds a source model @p sourceModel, after all existing source models.
     * @param sourceModel the source model
     *
     * The ownership of @p sourceModel is not affected by this.
     * The same source model cannot be added more than once.
     */
    Q_SCRIPTABLE void addSourceModel(QAbstractItemModel *sourceModel);

    /**
     * Removes the source model @p sourceModel.
     * @param sourceModel a source model previously added to this proxy
     *
     * The ownership of @sourceModel is not affected by this.
     */
    Q_SCRIPTABLE void removeSourceModel(QAbstractItemModel *sourceModel);

    /**
     * The currently set source models
     */
    QList<QAbstractItemModel *> sources() const;

    /**
     * Returns the proxy index for a given source index
     * @param sourceIndex an index coming from any of the source models
     * @return a proxy index
     * Calling this method with an index not from a source model is undefined behavior.
     */
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    /**
     * Returns the source index for a given proxy index.
     * @param proxyIndex an index for this proxy model
     * @return a source index
     */
    Q_INVOKABLE QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

    /// @reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /// @reimp
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    /// @reimp
    QMap<int, QVariant> itemData(const QModelIndex &proxyIndex) const override;
    /// @reimp
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    /// @reimp
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    /// @reimp
    QModelIndex parent(const QModelIndex &index) const override;
    /// @reimp
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * The horizontal header data for the first source model is returned here.
     * @reimp
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    /**
     * The column count for the first source model is returned here.
     * @reimp
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * The roles names for the first source model is returned here
     * @reimp
     */
    QHash<int, QByteArray> roleNames() const override;

private:
    Q_PRIVATE_SLOT(d, void slotRowsAboutToBeInserted(const QModelIndex &, int start, int end))
    Q_PRIVATE_SLOT(d, void slotRowsInserted(const QModelIndex &, int start, int end))
    Q_PRIVATE_SLOT(d, void slotRowsAboutToBeRemoved(const QModelIndex &, int start, int end))
    Q_PRIVATE_SLOT(d, void slotRowsRemoved(const QModelIndex &, int start, int end))
    Q_PRIVATE_SLOT(d, void slotColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end))
    Q_PRIVATE_SLOT(d, void slotColumnsInserted(const QModelIndex &parent, int, int))
    Q_PRIVATE_SLOT(d, void slotColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end))
    Q_PRIVATE_SLOT(d, void slotColumnsRemoved(const QModelIndex &parent, int, int))
    Q_PRIVATE_SLOT(d, void slotDataChanged(const QModelIndex &from, const QModelIndex &to, const QVector<int> &roles))
    Q_PRIVATE_SLOT(d, void slotSourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint))
    Q_PRIVATE_SLOT(d, void slotSourceLayoutChanged(const QList<QPersistentModelIndex> &, QAbstractItemModel::LayoutChangeHint))
    Q_PRIVATE_SLOT(d, void slotModelAboutToBeReset())
    Q_PRIVATE_SLOT(d, void slotModelReset())

private:
    friend class ShortcutsModelPrivate;
    const QScopedPointer<ShortcutsModelPrivate> d;
};

#endif
