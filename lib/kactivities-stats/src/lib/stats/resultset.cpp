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

#include "resultset.h"

// Qt
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>

// Local
#include <common/database/Database.h>
#include <utils/debug_and_return.h>

// Boost and STL
#include <boost/range/algorithm/transform.hpp>
#include <iterator>
#include <functional>
#include <mutex>
#include <boost/optional.hpp>

// KActivities
#include "activitiessync_p.h"

namespace KActivities {
namespace Experimental {
namespace Stats {

using namespace Terms;

class ResultSet::Result::Private {
public:
    QString resource;
    QString title;
    QString mimetype;
    double  score;
    uint    lastUpdate;
    uint    firstUpdate;

};

ResultSet::Result::Result()
    : d(new Private())
{
}

ResultSet::Result::Result(Result &&result)
    : d(result.d)
{
    result.d = Q_NULLPTR;
}

ResultSet::Result::Result(const Result &result)
    : d(new Private(*result.d))
{
}

ResultSet::Result &ResultSet::Result::operator=(Result result)
{
    std::swap(d, result.d);

    return *this;
}

ResultSet::Result::~Result()
{
    delete d;
}

QString ResultSet::Result::resource() const
{
    return d->resource;
}

QString ResultSet::Result::title() const
{
    return d->title;
}

QString ResultSet::Result::mimetype() const
{
    return d->mimetype;
}

double ResultSet::Result::score() const
{
    return d->score;
}

uint ResultSet::Result::lastUpdate() const
{
    return d->lastUpdate;
}

uint ResultSet::Result::firstUpdate() const
{
    return d->firstUpdate;
}

void ResultSet::Result::setResource(const QString &resource)
{
    d->resource = resource;
}

void ResultSet::Result::setTitle(const QString &title)
{
    d->title = title;
}

void ResultSet::Result::setMimetype(const QString &mimetype)
{
    d->mimetype = mimetype;
}

void ResultSet::Result::setScore(double score)
{
    d->score = score;
}

void ResultSet::Result::setLastUpdate(uint lastUpdate)
{
    d->lastUpdate = lastUpdate;
}

void ResultSet::Result::setFirstUpdate(uint firstUpdate)
{
    d->firstUpdate = firstUpdate;
}


class ResultSet::Private {
public:
    Common::Database::Ptr database;
    QSqlQuery query;
    Query queryDefinition;

    mutable ActivitiesSync::ConsumerPtr activities;

    void initQuery()
    {
        if (query.isActive()) {
            return;
        }

        auto selection = queryDefinition.selection();

        query = database->execQuery(
                selection == LinkedResources ? linkedResourcesQuery()
              : selection == UsedResources   ? usedResourcesQuery()
              : selection == AllResources    ? allResourcesQuery()
              : QString());

        Q_ASSERT_X(query.isActive(), "ResultSet initQuery", "Query is not valid");

        // TODO: Implement types
        // QStringList types() const;

    }

    QString agentClause(const QString &agent) const
    {
        if (agent == ":any") return "1";

        return "agent = '" + (
                agent == ":global"  ? "" :
                agent == ":current" ? QCoreApplication::instance()->applicationName() :
                                      agent
            ) + "'";
    }

    QString activityClause(const QString &activity) const
    {
        if (activity == ":any") return "1";

        return "activity = '" + (
                activity == ":global"  ? "" :
                activity == ":current" ? ActivitiesSync::currentActivity(activities) :
                                         activity
            ) + "'";
    }

    QString urlFilterClause(const QString &urlFilter) const
    {
        if (urlFilter == "*") return "1";

        return "resource GLOB '" + urlFilter + "'";
    }

    /**
     * Transforms the input list's elements with the f member method,
     * and returns the resulting list
     */
    template <typename F>
    inline
    QStringList transformedList(const QStringList &input, F f) const
    {
        using namespace std::placeholders;

        QStringList result;
        boost::transform(input,
                         std::back_inserter(result),
                         std::bind(f, this, _1));

        return result;
    }

    QString linkedResourcesQuery() const
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated, although, for this query,
        //       scores are not that important.
        static const QString _query =
            "\n"
            "SELECT \n"
            "    rl.targettedResource as resource \n"
            "  , SUM(rsc.cachedScore) as score \n"
            "  , MIN(rsc.firstUpdate) as firstUpdate \n"
            "  , MAX(rsc.lastUpdate)  as lastUpdate \n"
            "  , rl.usedActivity      as activity \n"
            "  , rl.initiatingAgent   as agent \n"
            "  , COALESCE(ri.title, rl.targettedResource) as title \n"

            "FROM \n"
            "    ResourceLink rl \n"
            "LEFT JOIN \n"
            "    ResourceScoreCache rsc \n"
            "    ON rl.targettedResource = rsc.targettedResource \n"
            "    AND rl.usedActivity     = rsc.usedActivity \n"
            "    AND rl.initiatingAgent  = rsc.initiatingAgent \n"
            "LEFT JOIN \n"
            "    ResourceInfo ri \n"
            "    ON rl.targettedResource = ri.targettedResource \n"

            "WHERE \n"
            "    ($agentsFilter) \n"
            "    AND ($activitiesFilter) \n"
            "    AND ($urlFilter)\n"

            "GROUP BY resource, title \n"
            "ORDER BY $orderingColumn resource ASC\n";

        // ORDER BY column
        auto ordering = queryDefinition.ordering();
        QString orderingColumn =
                ordering == HighScoredFirst      ? "score DESC,"
              : ordering == RecentlyCreatedFirst ? "firstUpdate DESC,"
              : ordering == RecentlyUsedFirst    ? "lastUpdate DESC,"
              : ordering == OrderByTitle         ? "title ASC,"
              : QString();


        // WHERE clause for filtering on agents
        QStringList agentsFilter = transformedList(
                queryDefinition.agents(), &Private::agentClause);

        // WHERE clause for filtering on activities
        QStringList activitiesFilter = transformedList(
                queryDefinition.activities(), &Private::activityClause);

        // WHERE clause for filtering on resource URLs
        QStringList urlFilter = transformedList(
                queryDefinition.urlFilters(), &Private::urlFilterClause);

        auto query = _query;

        return
            query
                .replace("$orderingColumn", orderingColumn)
                .replace("$agentsFilter", agentsFilter.join(" OR "))
                .replace("$activitiesFilter", activitiesFilter.join(" OR "))
                .replace("$urlFilter", urlFilter.join(" OR "))
            ;
    }

    QString usedResourcesQuery() const
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated
        static const QString _query =
            "\n"
            "SELECT \n"
            "    rsc.targettedResource as resource \n"
            "  , SUM(rsc.cachedScore)  as score \n"
            "  , MIN(rsc.firstUpdate)  as firstUpdate \n"
            "  , MAX(rsc.lastUpdate)   as lastUpdate \n"
            "  , rsc.usedActivity      as activity \n"
            "  , rsc.initiatingAgent   as agent \n"
            "  , COALESCE(ri.title, rsc.targettedResource) as title \n"

            "FROM \n"
            "    ResourceScoreCache rsc \n"
            "LEFT JOIN \n"
            "    ResourceInfo ri \n"
            "    ON rsc.targettedResource = ri.targettedResource \n"

            "WHERE \n"
            "    ($agentsFilter) \n"
            "    AND ($activitiesFilter) \n"
            "    AND ($urlFilter)\n"

            "GROUP BY resource, title \n"
            "ORDER BY $orderingColumn resource ASC\n";

        // ORDER BY column
        auto ordering = queryDefinition.ordering();
        QString orderingColumn =
                ordering == HighScoredFirst      ? "score DESC,"
              : ordering == RecentlyCreatedFirst ? "firstUpdate DESC,"
              : ordering == RecentlyUsedFirst    ? "lastUpdate DESC,"
              : ordering == OrderByTitle         ? "title ASC,"
              : QString();


        // WHERE clause for filtering on agents
        QStringList agentsFilter = transformedList(
                queryDefinition.agents(), &Private::agentClause);

        // WHERE clause for filtering on activities
        QStringList activitiesFilter = transformedList(
                queryDefinition.activities(), &Private::activityClause);

        // WHERE clause for filtering on resource URLs
        QStringList urlFilter = transformedList(
                queryDefinition.urlFilters(), &Private::urlFilterClause);

        auto query = _query;

        return kamd::utils::debug_and_return("Query: ",
            query
                .replace("$orderingColumn", orderingColumn)
                .replace("$agentsFilter", agentsFilter.join(" OR "))
                .replace("$activitiesFilter", activitiesFilter.join(" OR "))
                .replace("$urlFilter", urlFilter.join(" OR "))
            );
    }

    QString allResourcesQuery() const
    {
        // TODO: Get this to work
        static QString query =
            "SELECT * FROM ResourceLink ";

        return query;
    }

    Result currentResult() const
    {
        Result result;
        result.setResource(query.value("resource").toString());
        result.setTitle(query.value("title").toString());
        result.setMimetype(query.value("mimetype").toString());
        result.setScore(query.value("score").toDouble());
        result.setLastUpdate(query.value("lastUpdate").toInt());
        result.setFirstUpdate(query.value("firstUpdate").toInt());
        return result;
    }
};

ResultSet::ResultSet(Query query)
    : d(new Private())
{
    using namespace Common;

    d->database = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);
    Q_ASSERT_X(d->database, "ResultSet constructor", "Database is NULL");

    d->queryDefinition = query;

    d->initQuery();
}

ResultSet::ResultSet(ResultSet &&source)
    : d(nullptr)
{
    std::swap(d, source.d);
}

ResultSet::ResultSet(const ResultSet &source)
    : d(new Private(*source.d))
{
}

ResultSet &ResultSet::operator= (ResultSet source)
{
    std::swap(d, source.d);
    return *this;
}

ResultSet::~ResultSet()
{
    delete d;
}

ResultSet::Result ResultSet::at(int index) const
{
    Q_ASSERT_X(d->query.isActive(), "ResultSet::at", "Query is not active");
    // Q_ASSERT_X(d->query.size() >= 0, "ResultSet::at", "This query is not countable?!");
    // Q_ASSERT_X(index < d->query.size(), "ResultSet::at", "This index does not exist");

    d->query.seek(index);

    return d->currentResult();
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#include "resultset_iterator.cpp"

