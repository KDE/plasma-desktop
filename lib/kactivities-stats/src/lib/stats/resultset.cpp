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

class ResultSet_ResultPrivate {
public:
    QString resource;
    QString title;
    QString mimetype;
    double  score;
    uint    lastUpdate;
    uint    firstUpdate;
    ResultSet::Result::LinkStatus linkStatus;

};

ResultSet::Result::Result()
    : d(new ResultSet_ResultPrivate())
{
}

ResultSet::Result::Result(Result &&result)
    : d(result.d)
{
    result.d = Q_NULLPTR;
}

ResultSet::Result::Result(const Result &result)
    : d(new ResultSet_ResultPrivate(*result.d))
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

#define CREATE_GETTER_AND_SETTER(Type, Name, Set)                              \
    Type ResultSet::Result::Name() const                                       \
    {                                                                          \
        return d->Name;                                                        \
    }                                                                          \
                                                                               \
    void ResultSet::Result::Set(Type Name)                                     \
    {                                                                          \
        d->Name = Name;                                                        \
    }

CREATE_GETTER_AND_SETTER(QString, resource, setResource)
CREATE_GETTER_AND_SETTER(QString, title, setTitle)
CREATE_GETTER_AND_SETTER(QString, mimetype, setMimetype)
CREATE_GETTER_AND_SETTER(double, score, setScore)
CREATE_GETTER_AND_SETTER(uint, lastUpdate, setLastUpdate)
CREATE_GETTER_AND_SETTER(uint, firstUpdate, setFirstUpdate)
CREATE_GETTER_AND_SETTER(ResultSet::Result::LinkStatus, linkStatus, setLinkStatus)

#undef CREATE_GETTER_AND_SETTER

class ResultSetPrivate {
public:
    Common::Database::Ptr database;
    QSqlQuery query;
    Query queryDefinition;

    mutable ActivitiesSync::ConsumerPtr activities;

    void initQuery()
    {
        if (!database || query.isActive()) {
            return;
        }

        auto selection = queryDefinition.selection();

        query = database->execQuery(replaceQueryParameters(
                selection == LinkedResources ? linkedResourcesQuery()
              : selection == UsedResources   ? usedResourcesQuery()
              : selection == AllResources    ? allResourcesQuery()
              : QString()));

        if (query.lastError().isValid()) {
            qDebug() << "Error: " << query.lastError();
        }

        Q_ASSERT_X(query.isActive(), "ResultSet initQuery", "Query is not valid");
    }

    QString agentClause(const QString &agent) const
    {
        if (agent == ":any") return "1";

        return "agent = '" + (
                agent == ":current" ? QCoreApplication::instance()->applicationName() :
                                      agent
            ) + "'";
    }

    QString activityClause(const QString &activity) const
    {
        if (activity == ":any") return "1";

        return "activity = '" + (
                activity == ":current" ? ActivitiesSync::currentActivity(activities) :
                                         activity
            ) + "'";
    }

    QString urlFilterClause(const QString &urlFilter) const
    {
        if (urlFilter == "*") return "1";

        return "resource GLOB '" + urlFilter + "'";
    }

    QString mimetypeClause(const QString &mimetype) const
    {
        if (mimetype == ":any" || mimetype == "*") return "1";

        return "mimetype GLOB '" + mimetype + "'";
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

    QString limitOffsetSuffix() const
    {
        QString result;

        const int limit = queryDefinition.limit();
        if (limit > 0) {
            result += " LIMIT " + QString::number(limit);

            const int offset = queryDefinition.offset();
            if (offset > 0) {
                result += " OFFSET " + QString::number(offset);
            }
        }

        return result;
    }

    inline QString replaceQueryParameters(const QString &_query) const
    {
        // ORDER BY column
        auto ordering = queryDefinition.ordering();
        QString orderingColumn = QStringLiteral("linkStatus DESC, ") + (
                ordering == HighScoredFirst      ? "score DESC,"
              : ordering == RecentlyCreatedFirst ? "firstUpdate DESC,"
              : ordering == RecentlyUsedFirst    ? "lastUpdate DESC,"
              : ordering == OrderByTitle         ? "title ASC,"
              : QString()
            );


        // WHERE clause for filtering on agents
        QStringList agentsFilter = transformedList(
                queryDefinition.agents(), &ResultSetPrivate::agentClause);

        // WHERE clause for filtering on activities
        QStringList activitiesFilter = transformedList(
                queryDefinition.activities(), &ResultSetPrivate::activityClause);

        // WHERE clause for filtering on resource URLs
        QStringList urlFilter = transformedList(
                queryDefinition.urlFilters(), &ResultSetPrivate::urlFilterClause);

        // WHERE clause for filtering on resource mime
        QStringList mimetypeFilter = transformedList(
                queryDefinition.types(), &ResultSetPrivate::mimetypeClause);

        auto query = _query
            + "\nORDER BY $orderingColumn resource ASC\n"
            + limitOffsetSuffix();


        return kamd::utils::debug_and_return("Query: ",
            query
                .replace("$orderingColumn", orderingColumn)
                .replace("$agentsFilter", agentsFilter.join(" OR "))
                .replace("$activitiesFilter", activitiesFilter.join(" OR "))
                .replace("$urlFilter", urlFilter.join(" OR "))
                .replace("$mimetypeFilter", mimetypeFilter.join(" OR "))
            );
    }

    const QString &linkedResourcesQuery() const
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated, although, for this query,
        //       scores are not that important.
        static const QString query =
            "\n"
            "SELECT \n"
            "    rl.targettedResource as resource \n"
            "  , SUM(rsc.cachedScore) as score \n"
            "  , MIN(rsc.firstUpdate) as firstUpdate \n"
            "  , MAX(rsc.lastUpdate)  as lastUpdate \n"
            "  , rl.usedActivity      as activity \n"
            "  , rl.initiatingAgent   as agent \n"
            "  , COALESCE(ri.title, rl.targettedResource) as title \n"
            "  , ri.mimetype as mimetype \n"
            "  , 2       as linkStatus \n"

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
            "    AND ($mimetypeFilter)\n"

            "GROUP BY resource, title \n"
            ;

        return query;
    }

    const QString &usedResourcesQuery() const
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated
        static const QString query =
            "\n"
            "SELECT \n"
            "    rsc.targettedResource as resource \n"
            "  , SUM(rsc.cachedScore)  as score \n"
            "  , MIN(rsc.firstUpdate)  as firstUpdate \n"
            "  , MAX(rsc.lastUpdate)   as lastUpdate \n"
            "  , rsc.usedActivity      as activity \n"
            "  , rsc.initiatingAgent   as agent \n"
            "  , COALESCE(ri.title, rsc.targettedResource) as title \n"
            "  , ri.mimetype as mimetype \n"
            "  , 1 as linkStatus \n" // Note: this is replaced by allResourcesQuery

            "FROM \n"
            "    ResourceScoreCache rsc \n"
            "LEFT JOIN \n"
            "    ResourceInfo ri \n"
            "    ON rsc.targettedResource = ri.targettedResource \n"

            "WHERE \n"
            "    ($agentsFilter) \n"
            "    AND ($activitiesFilter) \n"
            "    AND ($urlFilter)\n"
            "    AND ($mimetypeFilter)\n"

            "GROUP BY resource, title \n"
            ;

        return query;
    }

    const QString &allResourcesQuery() const
    {
        // TODO: Implement counting of the linked items
        // int linkedItemsCount = 0; // ...;
        //
        // if (linkedItemsCount >= limit) {
        //     return linkedResourcesQuery();
        //
        // } else if (linkedItemsCount == 0) {
        //     return usedResourcesQuery();
        //
        // } else {
            static QString usedResourcesQuery_ = usedResourcesQuery();

            static const QString query =
                "WITH LinkedResourcesResults as (\n" +
                linkedResourcesQuery() +
                "\n)\n" +
                "SELECT * FROM LinkedResourcesResults \n" +
                "UNION \n" +
                usedResourcesQuery_
                    .replace("WHERE ",
                        "WHERE rsc.targettedResource NOT IN "
                        "(SELECT resource FROM LinkedResourcesResults) AND ")
                    .replace("1 as linkStatus", "0 as linkStatus");
        // }

        return query;
    }

    ResultSet::Result currentResult() const
    {
        ResultSet::Result result;
        result.setResource(query.value("resource").toString());
        result.setTitle(query.value("title").toString());
        result.setMimetype(query.value("mimetype").toString());
        result.setScore(query.value("score").toDouble());
        result.setLastUpdate(query.value("lastUpdate").toInt());
        result.setFirstUpdate(query.value("firstUpdate").toInt());

        result.setLinkStatus(
            (ResultSet::Result::LinkStatus)query.value("linkStatus").toInt());

        return result;
    }
};

ResultSet::ResultSet(Query query)
    : d(new ResultSetPrivate())
{
    using namespace Common;

    d->database = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);

    if (!(d->database)) {
        qWarning() << "KActivities ERROR: There is no database. This probably means "
                      "that you do not have the Activity Manager running, or that "
                      "something else is broken on your system. Recent documents and "
                      "alike will not work!";
        Q_ASSERT_X((bool)d->database, "ResultSet constructor", "Database is NULL");
    }

    d->queryDefinition = query;

    d->initQuery();
}

ResultSet::ResultSet(ResultSet &&source)
    : d(nullptr)
{
    std::swap(d, source.d);
}

ResultSet::ResultSet(const ResultSet &source)
    : d(new ResultSetPrivate(*source.d))
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

    d->query.seek(index);

    return d->currentResult();
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#include "resultset_iterator.cpp"

