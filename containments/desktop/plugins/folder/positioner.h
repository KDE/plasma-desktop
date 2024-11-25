/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Plasma/Applet>
#include <QAbstractItemModel>
#include <qqmlregistration.h>

class FolderModel;

class QTimer;

class Positioner : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Plasma::Applet *applet READ applet WRITE setApplet NOTIFY appletChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(FolderModel *folderModel READ folderModel WRITE setFolderModel NOTIFY folderModelChanged)
    Q_PROPERTY(int perStripe READ perStripe WRITE setPerStripe NOTIFY perStripeChanged)

public:
    enum LoadAndApplyFlags {
        None,
        SkipPerStripeUpdate
    };

    explicit Positioner(QObject *parent = nullptr);
    ~Positioner() override;

    bool enabled() const;
    void setEnabled(bool enabled);

    FolderModel *folderModel() const;
    void setFolderModel(QObject *folderModel);

    int perStripe() const;
    void setPerStripe(int perStripe);

    QStringList positions() const; // Used in unit tests

    Q_INVOKABLE int map(int row) const;

    Q_INVOKABLE int nearestItem(int currentIndex, Qt::ArrowType direction);

    Q_INVOKABLE bool isBlank(int row) const;
    Q_INVOKABLE int indexForUrl(const QUrl &url) const;

    Q_INVOKABLE void setRangeSelected(int anchor, int to);

    Q_INVOKABLE void reset();

    /**
     * Loads the position configuration from a config file,
     * then calls convertFolderModelData to convert it to
     * proxyData which can be used later by updatePositions.
     * @param flags is used for handling if the PerStripe value is updated
     * on load.
     */
    void loadAndApplyPositionsConfig(const LoadAndApplyFlags flags = LoadAndApplyFlags::None);

    /**
     * Saves the positions in m_positions to a configuration file
     */
    void savePositionsConfig();

    /**
     * Performs the move operation in the underlying model.
     *
     * @param moves List of indexes that were moved. Two
     *              consecutive entries correspond to the
     *              from and to position.
     *
     * @return The lowest index that was moved. Used to
     *         determine the first selected item.
     */
    Q_INVOKABLE int move(const QVariantList &moves, bool save = true);

    QHash<int, QByteArray> roleNames() const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Plasma::Applet *applet() const;
    void setApplet(Plasma::Applet *applet);

    bool screenInUse() const;

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
    void appletChanged() const;

private Q_SLOTS:
    void updateResolution();
    void sourceStatusChanged();
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);
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
    void onItemRenamed();
    void onListingCompleted();

private:
    void initMaps(int size = -1);
    void updateMaps(int proxyIndex, int sourceIndex);
    int firstRow() const;
    int lastRow() const;
    int firstFreeRow() const;
    // Converts data from folderModel to proxy data
    void convertFolderModelData();
    // Turns proxyToSource data into positions QStringList
    void updatePositionsList();
    void flushPendingChanges();
    void connectSignals(FolderModel *model);
    void disconnectSignals(FolderModel *model);
    bool configurationHasResolution(const QString &resolution) const;
    QString loadConfigData() const;

    bool m_enabled;
    FolderModel *m_folderModel;

    int m_perStripe;

    QModelIndexList m_pendingChanges;
    bool m_ignoreNextTransaction;

    QStringList m_positions;
    bool m_deferApplyPositions;
    QVariantList m_deferMovePositions;

    QHash<int, int> m_proxyToSource;
    QHash<int, int> m_sourceToProxy;
    bool m_beginInsertRowsCalled = false; // used to sync the amount of begin/endInsertRows calls

    QString m_resolution;

    Plasma::Applet *m_applet = nullptr;

    friend class PositionerTest;
};
