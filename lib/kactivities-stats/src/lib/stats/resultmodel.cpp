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

// Self
#include "resultmodel.h"

// Qt
#include <QDebug>
#include <QDateTime>
#include <QCoreApplication>

// STL and Boost
#include <functional>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/lower_bound.hpp>

// Local
#include <common/database/Database.h>
#include <utils/qsqlquery_iterator.h>
#include "resultset.h"
#include "resultwatcher.h"
#include "cleaning.h"
#include "kactivities/consumer.h"

#include <utils/member_matcher.h>
#include <utils/slide.h>
#include <common/specialvalues.h>

#define MAX_CHUNK_LOAD_SIZE 5

#define QDBG qDebug() << "KActivitiesStats(" << (void*)this << ")"

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultModel::Private {
public:
    Private(Query query, ResultModel *parent)
        : query(query)
        , watcher(query)
        , hasMore(true)
        , q(parent)
    {
        using Common::Database;
        database = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);
    }

    //_ findResource(...) -> (index, iterator)

    struct FindResult {
        FindResult(Private * const d, QList<ResultSet::Result>::iterator iterator)
            : d(d)
            , iterator(iterator)
            , index(iterator == d->cache.end() ? -1 : iterator - d->cache.begin())
        {
        }

        Private * const d;
        QList<ResultSet::Result>::iterator iterator;
        int index;

        operator bool() const
        {
            return iterator != d->cache.end();
        }
    };

    inline FindResult findResource(const QString &resource)
    {
        using namespace kamd::utils::member_matcher;
        using boost::find_if;

        return FindResult(
            this,
            find_if(cache, member(&ResultSet::Result::resource) == resource));
    }

    //^

    void fetchMore(bool emitChanges)
    {
        using namespace Terms;

        if (!hasMore) return;

        const int previousSize = cache.size();

        // We want to load itemCountLimit - previousSize new items,
        // but not more than chunkSize. We are skipping
        // the first previousSize elements from the result set

        int wantToInsertCount = qMin(MAX_CHUNK_LOAD_SIZE,
                                     query.limit() - previousSize);

        if (wantToInsertCount == 0) return;

        int insertedCount = 0;

        // In order to see whether there are more results, we need to pass
        // the count increased by one
        ResultSet results(query | Offset(previousSize) | Limit(wantToInsertCount + 1));
        auto it = results.begin();

        while ((wantToInsertCount --> 0) && (it != results.end())) {
            cache.append(*it);
            ++insertedCount;
            ++it;
        }

        hasMore = (it != results.end());

        if (emitChanges && insertedCount) {
            q->beginInsertRows(QModelIndex(), previousSize,
                               cache.size() + 1);
            q->endInsertRows();
        }
    }

    void onResultAdded(const QString &resource, double score, uint lastUpdate,
                       uint firstUpdate)
    {
        using namespace kamd::utils::member_matcher;
        using namespace Terms;
        using kamd::utils::slide_one;
        using boost::lower_bound;

        QDBG << "result added:" << resource
             << "score:" << score
             << "last:" << lastUpdate
             << "first:" << firstUpdate;

        // This can also be called when the resource score
        // has been updated, so we need to check whether
        // we already have it in the cache
        const auto result = findResource(resource);

        // TODO: We should also sort by the resource, not only on a single field
        const auto destination =
            query.ordering() == HighScoredFirst ?
                lower_bound(cache, score, member(&ResultSet::Result::score) > _) :
            query.ordering() == RecentlyUsedFirst ?
                lower_bound(cache, lastUpdate, member(&ResultSet::Result::lastUpdate) > _) :
            query.ordering() == RecentlyCreatedFirst ?
                lower_bound(cache, firstUpdate, member(&ResultSet::Result::firstUpdate) > _) :
            // otherwise
                lower_bound(cache, resource, member(&ResultSet::Result::resource) > _)
            ;

        const int destinationIndex = destination - cache.begin();

        if (result) {
            // We already have the resource in the cache
            // So, it is the time for a reshuffle
            const int currentIndex = result.index;

            result.iterator->setScore(score);
            result.iterator->setLastUpdate(lastUpdate);
            result.iterator->setFirstUpdate(firstUpdate);

            q->dataChanged(q->index(currentIndex), q->index(currentIndex));

            bool moving
                = q->beginMoveRows(QModelIndex(), currentIndex, currentIndex,
                                   QModelIndex(), destinationIndex);

            slide_one(result.iterator, destination);

            if (moving) {
                q->endMoveRows();
            }

        } else {
            // We do not have the resource in the cache

            q->beginInsertRows(QModelIndex(), destinationIndex,
                               destinationIndex);

            QDBG << "inserting" << resource
                 << "score:" << score
                 << "last:" << lastUpdate
                 << "first:" << firstUpdate;

            ResultSet::Result result;
            result.setResource(resource);

            result.setTitle(" ");
            result.setMimetype(" ");
            fillTitleAndMimetype(result);

            result.setScore(score);
            result.setLastUpdate(lastUpdate);
            result.setFirstUpdate(firstUpdate);

            cache.insert(destinationIndex, result);

            q->endInsertRows();

            trim();
        }
    }

    void onResultRemoved(const QString &resource)
    {
        const auto result = findResource(resource);

        if (!result) return;

        QDBG << "removing row " << result.index << " of " << cache.size();

        q->beginRemoveRows(QModelIndex(), result.index, result.index);

        cache.removeAt(result.index);

        q->endRemoveRows();
    }

    void onResourceTitleChanged(const QString &resource, const QString &title)
    {
        const auto result = findResource(resource);

        if (!result) return;

        result.iterator->setTitle(title);

        q->dataChanged(q->index(result.index), q->index(result.index));
    }

    void onResourceMimetypeChanged(const QString &resource, const QString &mimetype)
    {
        // TODO: This can add or remove items from the model

        const auto result = findResource(resource);

        if (!result) return;

        result.iterator->setMimetype(mimetype);

        q->dataChanged(q->index(result.index), q->index(result.index));
    }

    void reset()
    {
        q->beginResetModel();

        QDBG << "Model reset";
        // TODO: Make this a little bit smarter
        //       - there is a possibility that the new list of
        //       items will not differ significantly to the old one
        //       in which case it does not need to be a full model reset

        cache.clear();

        init();

        q->endResetModel();

    }

    void trim()
    {
        const int limit = query.limit();
        if (limit >= cache.size()) return;

        q->beginRemoveRows(QModelIndex(), limit, cache.size() - 1);
        cache.erase(cache.begin() + limit, cache.end());
        q->endRemoveRows();
    }

    void init()
    {
        hasMore = true;
        fetchMore(false);
    }

    void onCurrentActivityChanged(const QString &activity)
    {
        reset();
    }

    Query query;
    ResultWatcher watcher;

    bool hasMore;

    QList<ResultSet::Result> cache;
    KActivities::Consumer activities;
    Common::Database::Ptr database;

    void fillTitleAndMimetype(ResultSet::Result &result)
    {
        auto query = database->execQuery(
                "SELECT "
                "title, mimetype "
                "FROM "
                "ResourceInfo "
                "WHERE "
                "targettedResource = '" + result.resource() + "'"
                );

        // Only one item at most
        for (const auto &item: query) {
            result.setTitle(item["title"].toString());
            result.setMimetype(item["mimetype"].toString());
        }
    }

private:
    ResultModel *const q;

};

ResultModel::ResultModel(Query query, QObject *parent)
    : d(new Private(query, this))
{
    using namespace std::placeholders;

    connect(&d->watcher, &ResultWatcher::resultAdded,
            this, std::bind(&Private::onResultAdded, d, _1, _2, _3, _4));
    connect(&d->watcher, &ResultWatcher::resultRemoved,
            this, std::bind(&Private::onResultRemoved, d, _1));

    connect(&d->watcher, &ResultWatcher::resourceTitleChanged,
            this, std::bind(&Private::onResourceTitleChanged, d, _1, _2));
    connect(&d->watcher, &ResultWatcher::resourceMimetypeChanged,
            this, std::bind(&Private::onResourceMimetypeChanged, d, _1, _2));

    connect(&d->watcher, &ResultWatcher::resultsInvalidated,
            this, std::bind(&Private::reset, d));

    if (query.activities().contains(CURRENT_ACTIVITY_TAG)) {
        connect(&d->activities, &KActivities::Consumer::currentActivityChanged,
                this, std::bind(&Private::onCurrentActivityChanged, d, _1));
    }

    d->init();
}

ResultModel::~ResultModel()
{
    delete d;
}

QVariant ResultModel::data(const QModelIndex &item, int role) const
{
    const auto row = item.row();

    if (row < 0 || row >= d->cache.size()) return QVariant();

    const auto &result = d->cache[row];

    return role == Qt::DisplayRole ? (result.title() + " " + result.resource())
         : role == ResourceRole    ? result.resource()
         : role == TitleRole       ? result.title()
         : role == ScoreRole       ? result.score()
         : role == FirstUpdateRole ? result.firstUpdate()
         : role == LastUpdateRole  ? result.lastUpdate()
         : QVariant()
         ;
}

QVariant ResultModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    return QVariant();
}

int ResultModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->cache.size();
}

void ResultModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) return;
    d->fetchMore(true);
}

bool ResultModel::canFetchMore(const QModelIndex &parent) const
{
    return parent.isValid()                    ? false
         : d->cache.size() >= d->query.limit() ? false
         : d->hasMore;
}

// void ResultModel::setItemCountLimit(int count)
// {
//     d->itemCountLimit = count;
//
//     const int oldSize = d->cache.size();
//
//     if (oldSize > count) {
//         // We need to remove all items from the tail if the new
//         // size is less than the current number of loaded items
//         d->trim();
//
//     } else if (oldSize < count) {
//         // If the requested size is bigger, we are resetting the
//         // model
//         d->reset();
//     }
// }
//
// int ResultModel::itemCountLimit() const
// {
//     return d->itemCountLimit;
// }

void ResultModel::forgetResource(const QString &resource)
{
    foreach (const QString &activity, d->query.activities()) {
        foreach (const QString &agent, d->query.agents()) {
            Stats::forgetResource(
                    activity,
                    agent == CURRENT_AGENT_TAG ?
                        QCoreApplication::applicationName() : agent,
                    resource);
        }
    }
}

void ResultModel::forgetResource(int row)
{
    if (row >= d->cache.size()) return;

    foreach (const QString &activity, d->query.activities()) {
        foreach (const QString &agent, d->query.agents()) {
            Stats::forgetResource(
                    activity,
                    agent == CURRENT_AGENT_TAG ?
                        QCoreApplication::applicationName() : agent,
                    d->cache[row].resource());
        }
    }
}

void ResultModel::forgetAllResources()
{
    Stats::forgetResources(d->query);
}


} // namespace Stats
} // namespace Experimental
} // namespace KActivities

// #include "resourcemodel.moc"

