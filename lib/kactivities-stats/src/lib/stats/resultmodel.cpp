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
#include <QFile>

// STL and Boost
#include <functional>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/lower_bound.hpp>

// Local
#include <common/database/Database.h>
#include <utils/qsqlquery_iterator.h>
#include <utils/slide.h>
#include <utils/member_matcher.h>
#include "resultset.h"
#include "resultwatcher.h"
#include "cleaning.h"
#include "kactivities/consumer.h"

#include <common/specialvalues.h>

#define MAX_CHUNK_LOAD_SIZE 5
#define MAX_RELOAD_CACHE_SIZE 50

#define QDBG qDebug() << "KActivitiesStats(" << (void*)this << ")"

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultModel::Private {
public:
    Private(Query query, ResultModel *parent)
        : cache(this, query.limit())
        , query(query)
        , watcher(query)
        , hasMore(true)
        , q(parent)
    {
        using Common::Database;
        database = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);
    }

    enum Fetch {
        FetchReset,   // Remove old data and reload
        FetchReload,  // Update all data
        FetchMore     // Load more data if there is any
    };

    class Cache {
    public:
        typedef QList<ResultSet::Result> Items;

        Cache(Private *d, int limit)
            : m_countLimit(limit)
            , d(d)
        {
        }

        inline int size() const
        {
            return m_items.size();
        }

    private:

        QList<ResultSet::Result> m_items;
        int m_countLimit;
        Private *const d;

    public:
        //_ Fancy iterator

        struct FindCacheResult {
            Cache *const cache;
            Items::iterator iterator;
            int index;

            FindCacheResult(Cache *cache, Items::iterator iterator)
                : cache(cache)
                , iterator(iterator)
                , index(iterator == cache->m_items.end() ? -1 : iterator - cache->m_items.begin())
            {
            }

            operator bool() const
            {
                return iterator != cache->m_items.end();
            }
        };
        //^

        inline FindCacheResult find(const QString &resource)
        {
            using namespace kamd::utils::member_matcher;
            using boost::find_if;

            return FindCacheResult(
                this, find_if(m_items, member(&ResultSet::Result::resource)
                                           == resource));
        }

        template <typename What, typename Predicate>
        inline FindCacheResult lowerBound(What &&what, Predicate &&predicate)
        {
            return FindCacheResult(
                this,
                boost::lower_bound(m_items, std::forward<What>(what),
                                   std::forward<Predicate>(predicate)));
        }

        inline void insertAt(const FindCacheResult &at,
                             const ResultSet::Result &result)
        {
            m_items.insert(at.iterator, result);
        }

        inline void removeAt(const FindCacheResult &at)
        {
            m_items.removeAt(at.index);
        }

        inline const ResultSet::Result &operator[] (int index) const
        {
            return m_items[index];
        }

        inline void clear()
        {
            if (m_items.size() == 0) return;

            d->q->beginRemoveRows(QModelIndex(), 0, m_items.size());
            m_items.clear();
            d->q->endRemoveRows();
        }

        inline void replace(const Items &newItems, int from = 0)
        {
            using namespace kamd::utils::member_matcher;

            // Based on 'The string to string correction problem
            // with block moves' paper by Walter F. Tichy
            //
            // In essence, it goes like this:
            //
            // Take the first element from the new list, and try to find
            // it in the old one. If you can not find it, it is a new item
            // item - send the 'inserted' event.
            // If you did find it, test whether the following items also
            // match. This detects blocks of items that have moved.
            //
            // In this example, we find 'b', and then detect the rest of the
            // moved block 'b' 'c' 'd'
            //
            // Old items:  a[b c d]e f g
            //               ^
            //              /
            // New items: [b c d]a f g
            //
            // After processing one block, just repeat until the end of the
            // new list is reached.
            //
            // Then remove all remaining elements from the old list.
            //
            // The main addition here compared to the original papers is that
            // our 'strings' can not hold two instances of the same element,
            // and that we support updating from arbitrary position.

            auto newBlockStart = newItems.cbegin();

            // How many items should we add?
            // This should remove the need for post-replace-trimming
            // in the case where somebody called this with too much new items.
            const int maxToReplace = m_countLimit - from;

            if (maxToReplace <= 0) return;

            const auto newItemsEnd =
                newItems.size() <= maxToReplace ? newItems.cend() :
                                                  newItems.cbegin() + maxToReplace;


            // Finding the blocks until we reach the end of the newItems list
            //
            // from = 4
            // Old items: X Y Z U a b c d e f g
            //                      ^ oldBlockStart points to the first element
            //                        of the currently processed block in the old list
            //
            // New items: _ _ _ _ b c d a f g
            //                    ^ newBlockStartIndex is the index of the first
            //                      element of the block that is currently being
            //                      processed (with 'from' offset)

            while (newBlockStart != newItemsEnd) {

                const int newBlockStartIndex
                    = from + std::distance(newItems.cbegin(), newBlockStart);

                const auto oldBlockStart = std::find_if(
                    m_items.begin() + from, m_items.end(),
                    member(&ResultSet::Result::resource) == newBlockStart->resource());

                if (oldBlockStart == m_items.end()) {
                    // This item was not found in the old cache, so we are
                    // inserting a new item at the same position it had in
                    // the newItems array
                    d->q->beginInsertRows(QModelIndex(), newBlockStartIndex,
                                          newBlockStartIndex);

                    m_items.insert(newBlockStartIndex, *newBlockStart);
                    d->q->endInsertRows();

                    // This block contained only one item, move on to find
                    // the next block - it starts from the next item
                    ++newBlockStart;

                } else {
                    // We are searching for a block of matching items.
                    // This is a reimplementation of std::mismatch that
                    // accepts two complete ranges that is available only
                    // since C++14, so we can not use it.
                    auto newBlockEnd = newBlockStart;
                    auto oldBlockEnd = oldBlockStart;

                    while (newBlockEnd != newItemsEnd &&
                           oldBlockEnd != m_items.end() &&
                           newBlockEnd->resource() == oldBlockEnd->resource()) {
                        ++newBlockEnd;
                        ++oldBlockEnd;
                    }

                    // We have found matching blocks
                    // [newBlockStart, newBlockEnd) and [oldBlockStart, newBlockEnd)
                    const int oldBlockStartIndex
                        = std::distance(m_items.begin() + from, oldBlockStart);

                    const int blockSize
                        = std::distance(oldBlockStart, oldBlockEnd);

                    if (oldBlockStartIndex != newBlockStartIndex) {
                        // If these blocks do not have the same start,
                        // we need to send the move event.
                        d->q->beginMoveRows(QModelIndex(), oldBlockStartIndex,
                                            oldBlockStartIndex + blockSize - 1,
                                            QModelIndex(), newBlockStartIndex);

                        // Moving the items from the old location to the new one
                        kamd::utils::slide(
                                oldBlockStart, oldBlockEnd,
                                m_items.begin() + newBlockStartIndex);

                        d->q->endMoveRows();
                    }

                    // Skip all the items in this block, and continue with
                    // the search
                    newBlockStart = newBlockEnd;
                }
            }

            // We have avoided the need for trimming for the most part,
            // but if the newItems list was shorter than needed, we still
            // need to trim the rest.
            trim(from + newItems.size());

            // Check whether we got an item representing a non-existent file,
            // if so, schedule its removal from the database
            for (const auto &item: newItems) {
                if (item.resource().startsWith('/') && !QFile(item.resource()).exists()) {
                    d->q->forgetResource(item.resource());
                }
            }
        }

        inline void trim()
        {
            trim(m_countLimit);
        }

        inline void trim(int limit)
        {
            if (m_items.size() <= limit) return;

            // Example:
            //   limit is 5,
            //   current cache (0, 1, 2, 3, 4, 5, 6, 7), size = 8
            // We need to delete from 5 to 7

            d->q->beginRemoveRows(QModelIndex(), limit, m_items.size() - 1);
            m_items.erase(m_items.begin() + limit, m_items.end());
            d->q->endRemoveRows();
        }

    } cache;

    void reload()
    {
        fetch(FetchReload);
    }

    void init()
    {
        fetch(FetchReset);
    }

    void fetch(int from, int count)
    {
        using namespace Terms;

        if (from + count > query.limit()) {
            count = query.limit() - from;
        }

        if (count <= 0) return;

        // In order to see whether there are more results, we need to pass
        // the count increased by one
        ResultSet results(query | Offset(from) | Limit(count + 1));

        auto it = results.begin();

        Cache::Items newItems;

        while (count --> 0 && it != results.end()) {
            newItems << *it;
            ++it;
        }

        hasMore = (it != results.end());

        cache.replace(newItems, from);
    }

    void fetch(Fetch mode)
    {
        if (mode == FetchReset) {
            // Removing the previously cached data
            // and loading all from scratch
            cache.clear();

            fetch(0, MAX_CHUNK_LOAD_SIZE);

        } else if (mode == FetchReload) {
            if (cache.size() > MAX_RELOAD_CACHE_SIZE) {
                // If the cache is big, we are pretending
                // we were asked to reset the model
                fetch(FetchReset);

            } else {
                // We are only updating the currently
                // cached items, nothing more
                fetch(0, cache.size());

            }

        } else { // FetchMore
            // Load a new batch of data
            fetch(cache.size(), MAX_CHUNK_LOAD_SIZE);
        }
    }


    void onResultAdded(const QString &resource, double score, uint lastUpdate,
                       uint firstUpdate)
    {
        using namespace kamd::utils::member_matcher;
        using namespace Terms;
        using kamd::utils::slide_one;
        using boost::lower_bound;

        QDBG << "ResultModel::Private::onResultAdded "
             << "result added:" << resource
             << "score:" << score
             << "last:" << lastUpdate
             << "first:" << firstUpdate;

        // This can also be called when the resource score
        // has been updated, so we need to check whether
        // we already have it in the cache
        const auto result = cache.find(resource);

        // TODO: We should also sort by the resource, not only on a single field
        const auto destination =
            query.ordering() == HighScoredFirst ?
                cache.lowerBound(score, member(&ResultSet::Result::score) > _) :
            query.ordering() == RecentlyUsedFirst ?
                cache.lowerBound(lastUpdate, member(&ResultSet::Result::lastUpdate) > _) :
            query.ordering() == RecentlyCreatedFirst ?
                cache.lowerBound(firstUpdate, member(&ResultSet::Result::firstUpdate) > _) :
            // otherwise
                cache.lowerBound(resource, member(&ResultSet::Result::resource) > _)
            ;

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
                                   QModelIndex(), destination.index);

            slide_one(result.iterator, destination.iterator);

            if (moving) {
                q->endMoveRows();
            }

        } else {
            // We do not have the resource in the cache

            q->beginInsertRows(QModelIndex(), destination.index,
                               destination.index);

            ResultSet::Result result;
            result.setResource(resource);

            result.setTitle(" ");
            result.setMimetype(" ");
            fillTitleAndMimetype(result);

            result.setScore(score);
            result.setLastUpdate(lastUpdate);
            result.setFirstUpdate(firstUpdate);

            cache.insertAt(destination, result);

            q->endInsertRows();

            cache.trim();
        }
    }

    void onResultRemoved(const QString &resource)
    {
        const auto result = cache.find(resource);

        if (!result) return;

        q->beginRemoveRows(QModelIndex(), result.index, result.index);

        cache.removeAt(result);

        q->endRemoveRows();

        fetch(cache.size(), 1);
    }

    void onResourceTitleChanged(const QString &resource, const QString &title)
    {
        const auto result = cache.find(resource);

        if (!result) return;

        result.iterator->setTitle(title);

        q->dataChanged(q->index(result.index), q->index(result.index));
    }

    void onResourceMimetypeChanged(const QString &resource, const QString &mimetype)
    {
        // TODO: This can add or remove items from the model

        const auto result = cache.find(resource);

        if (!result) return;

        result.iterator->setMimetype(mimetype);

        q->dataChanged(q->index(result.index), q->index(result.index));
    }

    Query query;
    ResultWatcher watcher;
    bool hasMore;

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

    void onCurrentActivityChanged(const QString &activity)
    {
        Q_UNUSED(activity);
        // If the current activity has changed, and
        // the query lists items for the ':current' one,
        // reset the model (not a simple refresh this time)
        if (query.activities().contains(CURRENT_ACTIVITY_TAG)) {
            fetch(FetchReset);
        }
    }

private:
    ResultModel *const q;

};

ResultModel::ResultModel(Query query, QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(query, this))
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
            this, std::bind(&Private::reload, d));

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

QHash<int, QByteArray> ResultModel::roleNames() const
{
    return {
        { ResourceRole    , "resource" },
        { TitleRole       , "title" },
        { ScoreRole       , "score" },
        { FirstUpdateRole , "created" },
        { LastUpdateRole  , "modified" }
    };
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
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

int ResultModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->cache.size();
}

void ResultModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) return;
    d->fetch(Private::FetchMore);
}

bool ResultModel::canFetchMore(const QModelIndex &parent) const
{
    return parent.isValid()                    ? false
         : d->cache.size() >= d->query.limit() ? false
         : d->hasMore;
}

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

