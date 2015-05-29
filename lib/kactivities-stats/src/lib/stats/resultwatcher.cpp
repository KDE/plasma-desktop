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

#include "resultwatcher.h"

// Qt
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QList>
#include <QRegExp>

// Local
#include <common/database/Database.h>
#include <utils/debug_and_return.h>

// Boost and STL
#include <boost/range/algorithm/transform.hpp>
#include <iterator>
#include <functional>
#include <mutex>
#include <limits>
#include <boost/optional.hpp>

// KActivities
#include <kactivities/consumer.h>

#include "resourceslinking_interface.h"
#include "resourcesscoring_interface.h"
#include "common/dbus/common.h"
#include "common/specialvalues.h"
#include "activitiessync_p.h"
#include "utils/lazy_val.h"
#include "utils/qsqlquery_iterator.h"

#include <algorithm>

#include <utils/debug_and_return.h>

#define QDBG qDebug() << "KActivitiesStats(" << (void*)this << ")"

namespace KActivities {
namespace Experimental {
namespace Stats {

// Main class

class ResultWatcher::Private {
public:
    mutable ActivitiesSync::ConsumerPtr activities;
    QList<QRegExp> urlFilters;

    Private(ResultWatcher *parent, Query query)
        : linking(new KAMD_DBUS_CLASS_INTERFACE(Resources/Linking, ResourcesLinking, Q_NULLPTR))
        , scoring(new KAMD_DBUS_CLASS_INTERFACE(Resources/Scoring, ResourcesScoring, Q_NULLPTR))
        , q(parent)
        , query(query)
    {
        for (const auto& urlFilter: query.urlFilters()) {
            urlFilters << QRegExp(urlFilter, Qt::CaseSensitive, QRegExp::WildcardUnix);
        }

        m_resultInvalidationTimer.setSingleShot(true);
        m_resultInvalidationTimer.setInterval(200);
        connect(&m_resultInvalidationTimer, &QTimer::timeout,
                q, emit &ResultWatcher::resultsInvalidated);
    }

    // Processing the list of activities as specified by the query.
    // If it contains :any, we are returning true, otherwise
    // we want to match a specific activity (be it the current
    // activity or not). The :global special value is not special here
    bool activityMatches(const QString &activity) const
    {
        return
            activity == ANY_ACTIVITY_TAG ||
            std::any_of(query.activities().cbegin(), query.activities().cend(),
            [&] (const QString &matcher) {
                return (matcher == ANY_ACTIVITY_TAG)     ? true :
                       (matcher == CURRENT_ACTIVITY_TAG) ? (matcher == activity || activity == ActivitiesSync::currentActivity(activities)) :
                                                           activity == matcher;
            }
        );
    }

    // Same as above, but for agents
    bool agentMatches(const QString &agent) const
    {
        return
            agent == ANY_AGENT_TAG ||
            std::any_of(query.agents().cbegin(), query.agents().cend(),
            [&] (const QString &matcher) {
                return (matcher == ANY_AGENT_TAG)     ? true :
                       (matcher == CURRENT_AGENT_TAG) ? (matcher == agent || agent == QCoreApplication::applicationName()) :
                                                        agent == matcher;
            }
        );
    }

    // Same as above, but for urls
    bool urlMatches(const QString &url) const
    {
        return std::any_of(urlFilters.cbegin(), urlFilters.cend(),
            [&] (const QRegExp &matcher) {
                return matcher.exactMatch(url);
            }
        );
    }

    bool typeMatches(const QString &resource) const
    {
        // We don't necessarily need to retrieve the type from
        // the database. If we do, get it only once
        auto type = kamd::utils::make_lazy_val([&] () -> QString {
            using Common::Database;

            auto query
                = Database::instance(Database::ResourcesDatabase,
                                     Database::ReadOnly)
                      ->execQuery("SELECT mimetype FROM ResourceInfo WHERE "
                                  "targettedResource = '" + resource + "'");

            for (const auto &item : query) {
                return item[0].toString();
            }

            return QString();
        });

        return std::any_of(query.types().cbegin(), query.types().cend(),
            [&] (const QString &matcher) {
                return matcher == ANY_TYPE_TAG || matcher == type;
            }
        );
    }

    bool eventMatches(const QString &agent, const QString &resource,
                      const QString &activity) const
    {
        // The order of checks is not arbitrary, it is sorted
        // from the cheapest, to the most expensive
        return agentMatches(agent)
               && activityMatches(activity)
               && urlMatches(resource)
               && typeMatches(resource)
               ;
    }

    void onResourceLinkedToActivity(const QString &agent,
                                    const QString &resource,
                                    const QString &activity)
    {
        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) return;

        if (!eventMatches(agent, resource, activity)) return;

        // TODO: See whether it makes sense to have lastUpdate/firstUpdate here as well
        emit q->resultAdded(resource, std::numeric_limits<double>::infinity(), 0, 0);
    }

    void onResourceUnlinkedFromActivity(const QString &agent,
                                        const QString &resource,
                                        const QString &activity)
    {
        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) return;

        if (!eventMatches(agent, resource, activity)) return;

        emit q->resultRemoved(resource);
    }

    void onResourceScoreUpdated(const QString &activity, const QString &agent,
                                const QString &resource, double score,
                                uint lastUpdate, uint firstUpdate)
    {
        Q_ASSERT_X(activity == "00000000-0000-0000-0000-000000000000" ||
                   !QUuid(activity).isNull(),
                   "ResultWatcher::onResourceScoreUpdated",
                   "The activity should be always specified here, no magic values");

        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        if (!eventMatches(agent, resource, activity)) return;

        emit q->resultAdded(resource, score, lastUpdate, firstUpdate);
    }


    void onEarlierStatsDeleted(QString, int)
    {
        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        scheduleResultsInvalidation();
    }

    void onRecentStatsDeleted(QString, int, QString)
    {
        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        scheduleResultsInvalidation();
    }

    void onStatsForResourceDeleted(const QString &activity,
                                   const QString &agent,
                                   const QString &resource)
    {
        if (query.selection() == Terms::LinkedResources) return;

        if (activityMatches(activity) && agentMatches(agent)) {
            if (resource.contains('*')) {
                scheduleResultsInvalidation();

            } else if (typeMatches(resource)) {
                if (!m_resultInvalidationTimer.isActive()) {
                    // Remove a result only if we haven't an invalidation
                    // request scheduled
                    q->resultRemoved(resource);
                }
            }
        }
    }

    // Lets not send a lot of invalidation events at once
    QTimer m_resultInvalidationTimer;
    void scheduleResultsInvalidation()
    {
        QDBG << "Scheduling invalidation";
        m_resultInvalidationTimer.start();
    }

    QScopedPointer<org::kde::ActivityManager::ResourcesLinking> linking;
    QScopedPointer<org::kde::ActivityManager::ResourcesScoring> scoring;

    ResultWatcher * const q;
    Query query;
};

ResultWatcher::ResultWatcher(Query query)
    : d(new Private(this, query))
{
    using namespace org::kde::ActivityManager;
    using namespace std::placeholders;

    // There is no need for private slots, when we have bind

    // Connecting the linking service
    QObject::connect(
        d->linking.data(), &ResourcesLinking::ResourceLinkedToActivity,
        this, std::bind(&Private::onResourceLinkedToActivity, d, _1, _2, _3));
    QObject::connect(
        d->linking.data(), &ResourcesLinking::ResourceUnlinkedFromActivity,
        this, std::bind(&Private::onResourceUnlinkedFromActivity, d, _1, _2, _3));

    // Connecting the scoring service
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::ResourceScoreUpdated,
        this, std::bind(&Private::onResourceScoreUpdated, d, _1, _2, _3, _4, _5, _6));
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::ResourceScoreDeleted,
        this, std::bind(&Private::onStatsForResourceDeleted, d, _1, _2, _3));
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::RecentStatsDeleted,
        this, std::bind(&Private::onRecentStatsDeleted, d, _1, _2, _3));
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::EarlierStatsDeleted,
        this, std::bind(&Private::onEarlierStatsDeleted, d, _1, _2));

}

ResultWatcher::~ResultWatcher()
{
    delete d;
}

void ResultWatcher::linkToActivity(const QUrl &resource,
                                   const Terms::Activity &activity,
                                   const Terms::Agent &agent)
{
    const auto activities =
        (!activity.values.isEmpty())       ? activity.values :
        (!d->query.activities().isEmpty()) ? d->query.activities() :
                                             Terms::Activity::current().values;
    const auto agents =
        (!agent.values.isEmpty())      ? agent.values :
        (!d->query.agents().isEmpty()) ? d->query.agents() :
                                         Terms::Agent::current().values;

    for (const auto &activity : activities) {
        for (const auto &agent : agents) {
            d->linking->LinkResourceToActivity(agent, resource.toString(),
                                               activity);
        }
    }
}

void ResultWatcher::unlinkFromActivity(const QUrl &resource,
                                       const Terms::Activity &activity,
                                       const Terms::Agent &agent)
{
    const auto activities =
        !activity.values.isEmpty()       ? activity.values :
        !d->query.activities().isEmpty() ? d->query.activities() :
                                           Terms::Activity::current().values;
    const auto agents =
        !agent.values.isEmpty()      ? agent.values :
        !d->query.agents().isEmpty() ? d->query.agents() :
                                       Terms::Agent::current().values;

    for (const auto &activity : activities) {
        for (const auto &agent : agents) {
            d->linking->UnlinkResourceFromActivity(agent, resource.toString(),
                                                   activity);
        }
    }
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

