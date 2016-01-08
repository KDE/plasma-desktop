/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KACTIVITIES_STATS_RESULTMODEL_H
#define KACTIVITIES_STATS_RESULTMODEL_H

// Qt
#include <QObject>
#include <QAbstractListModel>

// Local
#include "query.h"

class QModelIndex;
class QDBusPendingCallWatcher;

class KConfigGroup;

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultModelPrivate;

/**
 * ResultModel
 */

class KACTIVITIESSTATS_EXPORT ResultModel : public QAbstractListModel {
    Q_OBJECT

public:
    ResultModel(Query query, QObject *parent = 0);
    ResultModel(Query query, const QString &clientId, QObject *parent = 0);
    virtual ~ResultModel();

    enum Roles {
        ResourceRole    = Qt::UserRole,
        TitleRole       = Qt::UserRole + 1,
        ScoreRole       = Qt::UserRole + 2,
        FirstUpdateRole = Qt::UserRole + 3,
        LastUpdateRole  = Qt::UserRole + 4,
        LinkStatusRole  = Qt::UserRole + 5
    };

    int rowCount(const QModelIndex &parent
                 = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &item,
                  int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    // TODO: BLOCKER Move these to libKActivities
    void linkToActivity(const QUrl &resource,
                        const Terms::Activity &activity
                            = Terms::Activity(QStringList()),
                        const Terms::Agent &agent
                            = Terms::Agent(QStringList()));

    void unlinkFromActivity(const QUrl &resource,
                            const Terms::Activity &activity
                                = Terms::Activity(QStringList()),
                            const Terms::Agent &agent
                                = Terms::Agent(QStringList()));

public Q_SLOTS:
    /**
     * Removes the specified resource from the history
     */
    void forgetResource(const QString &resource);

    /**
     * Removes the specified resource from the history
     */
    void forgetResource(int row);

    /**
     * Clears the history of all resources that match the current
     * model query
     */
    void forgetAllResources();

    /**
     * Moves the resource to the specified position.
     *
     * Note that this only applies to the linked resources
     * since the recently/frequently used ones have
     * their natural order.
     * This requires the clientId to be specified on construction.
     */
    void setResultPosition(const QString &resource, int position);

    /**
     * Sort the items by title.
     *
     * Note that this only affects the linked resources
     * since the recently/frequently used ones have
     * their natural order.
     * This requires the config to be specified on construction.
     */
    void sortItems(Qt::SortOrder sortOrder);

private:
    friend class ResultModelPrivate;
    ResultModelPrivate *const d;
};

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTMODEL_H

