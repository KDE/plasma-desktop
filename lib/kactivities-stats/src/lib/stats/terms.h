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

#ifndef KACTIVITIES_STATS_TERMS_H
#define KACTIVITIES_STATS_TERMS_H

#include <qcompilerdetection.h>

#ifdef Q_COMPILER_INITIALIZER_LISTS
#include <initializer_list>
#endif

#include <QString>
#include <QStringList>

#include "kactivitiesstats_export.h"

namespace KActivities {
namespace Experimental {
namespace Stats {

namespace Terms {
    /**
     * Enumerator specifying the ordering in which the
     * results of the query should be listed
     */
    enum KACTIVITIESSTATS_EXPORT Order {
        HighScoredFirst,      ///< Resources with the highest scores first
        RecentlyUsedFirst,    ///< Recently used resources first
        RecentlyCreatedFirst, ///< Recently created resources first
        OrderByUrl,           ///< Order by uri, alphabetically
        OrderByTitle          ///< Order by uri, alphabetically
    };

    /**
     * Which resources should be returned
     */
    enum KACTIVITIESSTATS_EXPORT Select {
        LinkedResources, ///< Resources linked to an activity, or globally
        UsedResources,   ///< Resources that have been accessed
        AllResources     ///< Combined set of accessed and linked resources
    };

    /**
     * How many items do you need?
     */
    struct KACTIVITIESSTATS_EXPORT Limit {
        Limit(int value);
        static Limit all();
        int value;
    };

    /**
     * How many items to skip?
     */
    struct KACTIVITIESSTATS_EXPORT Offset {
        Offset(int value);
        int value;
    };

    /**
     * Term to filter the resources according to their types
     */
    struct KACTIVITIESSTATS_EXPORT Type {
        /**
         * Show resources of any type
         */
        static Type any();

        #ifdef Q_COMPILER_INITIALIZER_LISTS
        inline Type(std::initializer_list<QString> types)
            : values(types)
        {
        }
        #endif

        Type(QStringList types);
        Type(QString type);

        const QStringList values;
    };

    /**
     * Term to filter the resources according the agent (application) which
     * accessed it
     */
    struct KACTIVITIESSTATS_EXPORT Agent {
        /**
         * Show resources accessed/linked by any application
         */
        static Agent any();

        /**
         * Show resources not tied to a specific agent
         */
        static Agent global();

        /**
         * Show resources accessed/linked by the current application
         */
        static Agent current();

        #ifdef Q_COMPILER_INITIALIZER_LISTS
        inline Agent(std::initializer_list<QString> agents)
            : values(agents)
        {
        }
        #endif

        Agent(QStringList agents);
        Agent(QString agent);

        const QStringList values;
    };

    /**
     * Term to filter the resources according the activity in which they
     * were accessed
     */
    struct KACTIVITIESSTATS_EXPORT Activity {
        /**
         * Show resources accessed in / linked to any activity
         */
        static Activity any();

        /**
         * Show resources linked to all activities
         */
        static Activity global();

        /**
         * Show resources linked to all activities
         */
        static Activity current();

        #ifdef Q_COMPILER_INITIALIZER_LISTS
        inline Activity(std::initializer_list<QString> activities)
            : values(activities)
        {
        }
        #endif

        Activity(QStringList activities);
        Activity(QString activity);

        const QStringList values;
    };

    /**
     * Url filtering.
     */
    struct KACTIVITIESSTATS_EXPORT Url {
        /**
         * Show only resources that start with the specified prefix
         */
        static Url startsWith(const QString &prefix);

        /**
         * Show resources that contain the specified infix
         */
        static Url contains(const QString &infix);

        /**
         * Show local files
         */
        static Url localFile();

        /**
         * Show local files, smb, fish, ftp and stfp
         */
        static Url file();

        #ifdef Q_COMPILER_INITIALIZER_LISTS
        inline Url(std::initializer_list<QString> urlPatterns)
            : values(urlPatterns)
        {
        }
        #endif

        Url(QStringList urlPatterns);
        Url(QString urlPattern);

        const QStringList values;

    };
} // namespace Terms

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Order &order);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Select &select);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Type &type);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Agent &agent);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Activity &activity);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Url &url);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Limit &limit);

KACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Experimental::Stats::Terms::Offset &offset);

#endif // KACTIVITIES_STATS_TERMS_H

