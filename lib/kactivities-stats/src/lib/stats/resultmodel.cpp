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

// KDE
#include <KConfig>
#include <KConfigGroup>

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

#define MAX_CHUNK_LOAD_SIZE 50
#define MAX_RELOAD_CACHE_SIZE 50

#define QDBG qDebug() << "KActivitiesStats(" << (void*)this << ")"

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultModelPrivate {
public:
    ResultModelPrivate(Query query, const QString &clientId, ResultModel *parent)
        : cache(this, clientId, query.limit())
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

    class Cache { //_
    public:
        typedef QList<ResultSet::Result> Items;

        Cache(ResultModelPrivate *d, const QString &clientId, int limit)
            : d(d)
            , m_countLimit(limit)
            , m_clientId(clientId)
        {
            if (!m_clientId.isEmpty()) {
                m_configFile = new KConfig(
                        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                        + QStringLiteral("/kactivitymanagerd-statsrc"));

                m_config = KConfigGroup(m_configFile, "ResultModel-OrderingFor-" + clientId);

                if (m_config.isValid()) {
                    m_fixedItems = m_config.readEntry("kactivitiesLinkedItemsOrder",
                                                      QStringList());
                }

                qDebug() << "Configuration activated " << m_configFile->name();
            }
        }

        ~Cache()
        {
            delete m_configFile;
        }

        inline int size() const
        {
            return m_items.size();
        }

        inline void setLinkedResultPosition(const QString &resource,
                                            int position)
        {
            if (!m_config.isValid()) {
                qWarning() << "We can not reorder the results, no clientId was specified";
                return;
            }

            // Preconditions:
            //  - cache is ordered properly, first on the user's desired order,
            //    then on the query specified order
            //  - the resource that needs to be moved is already in the cache
            //  - the resource that needs to be moved is a linked resource, not
            //    one that comes from the stats (there are overly many
            //    corner-cases that need to be covered in order to support
            //    reordering of the statistics-based resources)
            //  - the new position for the resource is not outside of the cache

            qDebug() << "Searching for " << resource;
            auto resourcePosition = find(resource);
            qDebug() << "Was resource found? " << (bool)resourcePosition;
            if (resourcePosition) {
                qDebug() << "What is the status? " << resourcePosition.iterator->linkStatus();
            }
            if (!resourcePosition
                || resourcePosition.iterator->linkStatus() == ResultSet::Result::NotLinked) {
                qWarning("Trying to reposition a resource that we do not have, or is not linked");
                return;
            }

            // Lets make a list of linked items - we can only reorder them,
            // not others
            QStringList linkedItems;

            foreach (const ResultSet::Result &item, m_items) {
                if (item.linkStatus() == ResultSet::Result::NotLinked) break;
                linkedItems << item.resource();
            }

            // We can not accept the new position to be outside
            // of the linked items area
            if (position > linkedItems.size()) {
                position = linkedItems.size();
            }

            auto oldPosition = linkedItems.indexOf(resource);

            kamd::utils::slide_one(
                    linkedItems.begin() + oldPosition,
                    linkedItems.begin() + position);

            m_fixedItems = linkedItems;

            m_config.writeEntry("kactivitiesLinkedItemsOrder", m_fixedItems);
            m_config.sync();

            // We are prepared to reorder the cache
            d->repositionResult(resourcePosition,
                                d->destinationFor(*resourcePosition));
        }

    private:
        ResultModelPrivate *const d;

        QList<ResultSet::Result> m_items;
        int m_countLimit;

        QString m_clientId;
        KConfig *m_configFile;
        KConfigGroup m_config;
        QStringList m_fixedItems;

        friend QDebug operator<< (QDebug out, const Cache &cache)
        {
            for (const auto& item: cache.m_items) {
                out << "Cache item: " << item << "\n";
            }

            return out;
        }

    public:
        inline const QStringList &fixedItems() const
        {
            return m_fixedItems;
        }

        //_ Fancy iterator, find, lowerBound
        struct FindCacheResult {
            Cache *const cache;
            Items::iterator iterator;
            int index;

            FindCacheResult(Cache *cache, Items::iterator iterator)
                : cache(cache)
                , iterator(iterator)
                , index(std::distance(cache->m_items.begin(), iterator))
            {
            }

            operator bool() const
            {
                return iterator != cache->m_items.end();
            }

            ResultSet::Result &operator*() const
            {
                return *iterator;
            }

            ResultSet::Result *operator->() const
            {
                return &(*iterator);
            }

            // const ResultSet::Result &operator*() const
            // {
            //     return *iterator;
            // }
            //
            // const ResultSet::Result *operator->() const
            // {
            //     return &(*iterator);
            // }
        };

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
                this, boost::lower_bound(m_items, std::forward<What>(what),
                                         std::forward<Predicate>(predicate)));
        }

        template <typename Predicate>
        inline FindCacheResult lowerBound(Predicate &&predicate)
        {
            using namespace kamd::utils::member_matcher;
            return FindCacheResult(
                this, boost::lower_bound(m_items, _,
                                         std::forward<Predicate>(predicate)));
        }
        //^

        inline int indexOf(const FindCacheResult &result)
        {
            return std::distance(m_items.begin(), result.iterator);
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

        //  Algorithm to calculate the edit operations to allow
        //_ replaceing items without model reset
        inline void replace(const Items &newItems, int from = 0)
        {
            using namespace kamd::utils::member_matcher;

#if 0
            QDBG << "===\nOld items {";
            for (const auto& item: m_items) {
                QDBG << item;
            }
            QDBG << "}";

            QDBG << "New items to be added at " << from << " {";
            for (const auto& item: newItems) {
                QDBG << item;
            }
            QDBG << "}";
#endif


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

                        // Note: If there is a crash here, it means we
                        // are getting a bad query which has duplicate
                        // results

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
        //^

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

    } cache; //^

    struct FixedItemsLessThan {
        //_ Compartor that orders the linked items by user-specified order
        typedef kamd::utils::member_matcher::placeholder placeholder;

        FixedItemsLessThan(const Cache &cache,
                           const QString &matchResource = QString())
            : cache(cache), matchResource(matchResource)
        {
        }

        bool compare (const QString &leftResource, const QString &rightResource) const
        {
            const bool hasLeft  = cache.fixedItems().contains(leftResource);
            const bool hasRight = cache.fixedItems().contains(rightResource);

            return
                ( hasLeft && !hasRight) ? true :
                (!hasLeft &&  hasRight) ? false :
                ( hasLeft &&  hasRight) ? cache.fixedItems().indexOf(leftResource) <
                                          cache.fixedItems().indexOf(rightResource) :
                false;
        }

        template <typename T>
        bool operator() (const T &left, placeholder) const
        {
            return compare(left.resource(), matchResource);
        }

        template <typename T>
        bool operator() (placeholder, const T &right) const
        {
            return compare(matchResource, right.resource());
        }

        template <typename T, typename V>
        bool operator() (const T &left, const V &right) const
        {
            return compare(left.resource(), right.resource());
        }

        const Cache &cache;
        const QString matchResource;
        //^
    };

    inline Cache::FindCacheResult destinationFor(const ResultSet::Result &result)
    {
        using namespace kamd::utils::member_matcher;
        using namespace Terms;

        const auto resource    = result.resource();
        const auto score       = result.score();
        const auto firstUpdate = result.firstUpdate();
        const auto lastUpdate  = result.lastUpdate();
        const auto linkStatus  = result.linkStatus();

        #define ORDER_BY(Field) member(&ResultSet::Result::Field) > Field
        #define ORDER_BY_FULL(Field)                                           \
            cache.lowerBound(FixedItemsLessThan(cache, resource)               \
                             && ORDER_BY(linkStatus)                           \
                             && ORDER_BY(Field)                                \
                             && ORDER_BY(resource))

        const auto destination =
            query.ordering() == HighScoredFirst      ? ORDER_BY_FULL(score):
            query.ordering() == RecentlyUsedFirst    ? ORDER_BY_FULL(lastUpdate):
            query.ordering() == RecentlyCreatedFirst ? ORDER_BY_FULL(firstUpdate):
            /* otherwise */                            ORDER_BY_FULL(resource)
            ;
        #undef ORDER_BY
        #undef ORDER_BY_FULL

        return destination;
    }

    inline void removeResult(const Cache::FindCacheResult &result)
    {
        q->beginRemoveRows(QModelIndex(), result.index, result.index);
        cache.removeAt(result);
        q->endRemoveRows();

        fetch(cache.size(), 1);
    }

    inline void repositionResult(const Cache::FindCacheResult &result,
                                 const Cache::FindCacheResult &destination)
    {
        using kamd::utils::slide_one;

        // We already have the resource in the cache
        // So, it is the time for a reshuffle
        const int currentIndex = result.index;

        q->dataChanged(q->index(currentIndex), q->index(currentIndex));

        bool moving
            = q->beginMoveRows(QModelIndex(), currentIndex, currentIndex,
                               QModelIndex(), destination.index);

        slide_one(result.iterator, destination.iterator);

        if (moving) {
            q->endMoveRows();
        }
    }

    void reload()
    {
        fetch(FetchReload);
    }

    void init()
    {
        using namespace std::placeholders;

        QObject::connect(
            &watcher, &ResultWatcher::resultScoreUpdated,
            q, std::bind(&ResultModelPrivate::onResultScoreUpdated, this, _1, _2, _3, _4));
        QObject::connect(
            &watcher, &ResultWatcher::resultRemoved,
            q, std::bind(&ResultModelPrivate::onResultRemoved, this, _1));
        QObject::connect(
            &watcher, &ResultWatcher::resultLinked,
            q, std::bind(&ResultModelPrivate::onResultLinked, this, _1));
        QObject::connect(
            &watcher, &ResultWatcher::resultUnlinked,
            q, std::bind(&ResultModelPrivate::onResultUnlinked, this, _1));

        QObject::connect(
            &watcher, &ResultWatcher::resourceTitleChanged,
            q, std::bind(&ResultModelPrivate::onResourceTitleChanged, this, _1, _2));
        QObject::connect(
            &watcher, &ResultWatcher::resourceMimetypeChanged,
            q, std::bind(&ResultModelPrivate::onResourceMimetypeChanged, this, _1, _2));

        QObject::connect(
            &watcher, &ResultWatcher::resultsInvalidated,
            q, std::bind(&ResultModelPrivate::reload, this));

        if (query.activities().contains(CURRENT_ACTIVITY_TAG)) {
            QObject::connect(
                &activities, &KActivities::Consumer::currentActivityChanged, q,
                std::bind(&ResultModelPrivate::onCurrentActivityChanged, this, _1));
        }

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

        // We need to sort the new items for the linked resources
        // user-defined reordering
        if (query.selection() != Terms::UsedResources) {
            std::stable_sort(newItems.begin(), newItems.end(),
                             FixedItemsLessThan(cache));
        }

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

    void onResultScoreUpdated(const QString &resource, double score,
                              uint lastUpdate, uint firstUpdate)
    {
        using boost::lower_bound;

        QDBG << "ResultModelPrivate::onResultAdded "
             << "result added:" << resource
             << "score:" << score
             << "last:" << lastUpdate
             << "first:" << firstUpdate;

        // This can also be called when the resource score
        // has been updated, so we need to check whether
        // we already have it in the cache
        const auto result = cache.find(resource);

        ResultSet::Result::LinkStatus linkStatus
            = result ? result->linkStatus()
                     : ResultSet::Result::NotLinked;

        if (result) {
            // We are only updating a result we already had,
            // lets fill out the data and send the update signal.
            // Move it if necessary.

            auto &item = *result.iterator;

            item.setScore(score);
            item.setLinkStatus(linkStatus);
            item.setLastUpdate(lastUpdate);
            item.setFirstUpdate(firstUpdate);

            const auto destination = destinationFor(item);

            repositionResult(result, destination);

        } else {
            // We do not have the resource in the cache,
            // lets fill out the data and insert it
            // at the desired position

            ResultSet::Result result;
            result.setResource(resource);

            result.setTitle(" ");
            result.setMimetype(" ");
            fillTitleAndMimetype(result);

            result.setScore(score);
            result.setLinkStatus(linkStatus);
            result.setLastUpdate(lastUpdate);
            result.setFirstUpdate(firstUpdate);

            const auto destination = destinationFor(result);

            q->beginInsertRows(QModelIndex(), destination.index,
                               destination.index);

            cache.insertAt(destination, result);

            q->endInsertRows();

            cache.trim();
        }
    }

    void onResultRemoved(const QString &resource)
    {
        const auto result = cache.find(resource);

        if (!result) return;

        if (query.selection() == Terms::UsedResources
            || result->linkStatus() != ResultSet::Result::Linked) {
            removeResult(result);
        }
    }

    void onResultLinked(const QString &resource)
    {
        const auto result = cache.find(resource);

        if (!result) return;

        if (query.selection() == Terms::LinkedResources) {
            onResultScoreUpdated(resource, 0, 0, 0);

        } else if (query.selection() == Terms::AllResources) {
            result->setLinkStatus(ResultSet::Result::Linked);
            repositionResult(result, destinationFor(*result));

        }
    }

    void onResultUnlinked(const QString &resource)
    {
        const auto result = cache.find(resource);

        if (!result) return;

        if (query.selection() == Terms::LinkedResources) {
            removeResult(result);

        } else if (query.selection() == Terms::AllResources) {
            result->setLinkStatus(ResultSet::Result::NotLinked);
            repositionResult(result, destinationFor(*result));

        }
    }

    Query query;
    ResultWatcher watcher;
    bool hasMore;

    KActivities::Consumer activities;
    Common::Database::Ptr database;

    //_ Title and mimetype functions
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

    void onResourceTitleChanged(const QString &resource, const QString &title)
    {
        const auto result = cache.find(resource);

        if (!result) return;

        result->setTitle(title);

        q->dataChanged(q->index(result.index), q->index(result.index));
    }

    void onResourceMimetypeChanged(const QString &resource, const QString &mimetype)
    {
        // TODO: This can add or remove items from the model

        const auto result = cache.find(resource);

        if (!result) return;

        result->setMimetype(mimetype);

        q->dataChanged(q->index(result.index), q->index(result.index));
    }
    //^

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
    , d(new ResultModelPrivate(query, QString(), this))
{
    d->init();
}

ResultModel::ResultModel(Query query, const QString &clientId, QObject *parent)
    : QAbstractListModel(parent)
    , d(new ResultModelPrivate(query, clientId, this))
{
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
        { LastUpdateRole  , "modified" },
        { LinkStatusRole  , "linkStatus" }
    };
}

QVariant ResultModel::data(const QModelIndex &item, int role) const
{
    const auto row = item.row();

    if (row < 0 || row >= d->cache.size()) return QVariant();

    const auto &result = d->cache[row];

    return role == Qt::DisplayRole ? (
               result.title() + " " +
               result.resource() + " - " +
               QString::number(result.linkStatus()) + " - " +
               QString::number(result.score())
           )
         : role == ResourceRole    ? result.resource()
         : role == TitleRole       ? result.title()
         : role == ScoreRole       ? result.score()
         : role == FirstUpdateRole ? result.firstUpdate()
         : role == LastUpdateRole  ? result.lastUpdate()
         : role == LinkStatusRole  ? result.linkStatus()
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
    d->fetch(ResultModelPrivate::FetchMore);
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

void ResultModel::setResultPosition(const QString &resource, int position)
{
    d->cache.setLinkedResultPosition(resource, position);
}

void ResultModel::sortItems(Qt::SortOrder sortOrder)
{
    // TODO
}

void ResultModel::linkToActivity(const QUrl &resource,
                                 const Terms::Activity &activity,
                                 const Terms::Agent &agent)
{
    d->watcher.linkToActivity(resource, activity, agent);
}

void ResultModel::unlinkFromActivity(const QUrl &resource,
                                     const Terms::Activity &activity,
                                     const Terms::Agent &agent)
{
    d->watcher.unlinkFromActivity(resource, activity, agent);
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

// #include "resourcemodel.moc"

