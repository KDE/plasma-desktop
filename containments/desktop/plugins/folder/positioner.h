/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POSITIONER_H
#define POSITIONER_H

#include <QAbstractItemModel>

#include "folderplugin_private_export.h"

class FolderModel;

class QTimer;

class FOLDERPLUGIN_TESTS_EXPORT Positioner : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(FolderModel *folderModel READ folderModel WRITE setFolderModel NOTIFY folderModelChanged)
    Q_PROPERTY(int perStripe READ perStripe WRITE setPerStripe NOTIFY perStripeChanged)
    Q_PROPERTY(QStringList positions READ positions WRITE setPositions NOTIFY positionsChanged)

public:
    explicit Positioner(QObject *parent = nullptr);
    ~Positioner() override;

    bool enabled() const;
    void setEnabled(bool enabled);

    FolderModel *folderModel() const;
    void setFolderModel(QObject *folderModel);

    int perStripe() const;
    void setPerStripe(int perStripe);

    QStringList positions() const;
    void setPositions(const QStringList &positions);

    Q_INVOKABLE int map(int row) const;

    Q_INVOKABLE int nearestItem(int currentIndex, Qt::ArrowType direction);

    Q_INVOKABLE bool isBlank(int row) const;
    Q_INVOKABLE int indexForUrl(const QUrl &url) const;

    Q_INVOKABLE void setRangeSelected(int anchor, int to);

    Q_INVOKABLE void reset();

    Q_INVOKABLE void move(const QVariantList &moves);

    QHash<int, QByteArray> roleNames() const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

#ifdef BUILD_TESTING
    QHash<int, int> proxyToSourceMapping() const
    {
        return m_proxyToSource;
    }
    QHash<int, int> sourceToProxyMapping() const
    {
        return m_sourceToProxy;
    }
#endif

Q_SIGNALS:
    void enabledChanged() const;
    void folderModelChanged() const;
    void perStripeChanged() const;
    void positionsChanged() const;

private Q_SLOTS:
    void updatePositions();
    void sourceStatusChanged();
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void sourceModelAboutToBeReset();
    void sourceModelReset();
    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceRowsInserted(const QModelIndex &parent, int first, int last);
    void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void sourceRowsRemoved(const QModelIndex &parent, int first, int last);
    void sourceLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);

private:
    void initMaps(int size = -1);
    void updateMaps(int proxyIndex, int sourceIndex);
    int firstRow() const;
    int lastRow() const;
    int firstFreeRow() const;
    void applyPositions();
    void flushPendingChanges();
    void connectSignals(FolderModel *model);
    void disconnectSignals(FolderModel *model);

    bool m_enabled;
    FolderModel *m_folderModel;

    int m_perStripe;

    int m_lastRow;

    QModelIndexList m_pendingChanges;
    bool m_ignoreNextTransaction;

    QStringList m_positions;
    bool m_deferApplyPositions;
    QVariantList m_deferMovePositions;
    QTimer *m_updatePositionsTimer;

    QHash<int, int> m_proxyToSource;
    QHash<int, int> m_sourceToProxy;
    bool m_beginInsertRowsCalled = false; // used to sync the amount of begin/endInsertRows calls
};

#endif
