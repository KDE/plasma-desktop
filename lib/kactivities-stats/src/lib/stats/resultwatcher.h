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

#ifndef KACTIVITIES_STATS_RESULTWATCHER
#define KACTIVITIES_STATS_RESULTWATCHER

#include <QObject>

#include "query.h"
#include "resultset.h"

namespace KActivities {
namespace Experimental {
namespace Stats {

/**
 * A very thin class that sends signals when new resources matching
 * a predefined query are available.
 */
class KACTIVITIESSTATS_EXPORT ResultWatcher: public QObject {
    Q_OBJECT

public:
    ResultWatcher(Query query);
    ~ResultWatcher();

Q_SIGNALS:
    /**
     * Emitted when a result has been added or updated. This either means
     * a new resource has appeared in the result set, or that
     * a previously existing one has some of the attributes changed.
     * @param result new data for the resource defined by result.resource
     */
    void resultAdded(const QString &resource, double score, uint lastUpdate,
                     uint firstUpdate);

    /**
     * Emitted when a result has been added or updated. This either means
     * a new resource has appeared in the result set, or that
     * a previously existing one has some of the attributes changed.
     * @param result new data for the resource defined by result.resource
     */
    void resultRemoved(const QString &resource);

    /**
     * Emitted when the title of a resource has been changed.
     * @param resource URL of the resource that has a new title
     * @param title new title of the resource
     * @note This signal will be emitted even for the resources that
     * do not match the specified query. This is because the class is
     * lightweight, and it does not keep track of which resources match
     * the query to be able to filter this signal.
     */
    void resourceTitleChanged(const QString &resource, const QString &title);

    /**
     * Emitted when the mimetype of a resource has been changed.
     * @param resource URL of the resource that has a new mimetype
     * @param mimetype new mimetype of the resource
     * @note This signal will be emitted even for the resources that
     * do not match the specified query. This is because the class is
     * lightweight, and it does not keep track of which resources match
     * the query to be able to filter this signal.
     */
    void resourceMimetypeChanged(const QString &resource, const QString &mimetype);

    /**
     * Emitted when the client should forget about all the results it
     * knew about and reload them. This can happen when the user clears
     * the history, or when there are more significant changes to the data.
     */
    void resultsInvalidated();

public:
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


private:
    class Private;
    friend class Private;
    Private *const d;

};

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTWATCHER

