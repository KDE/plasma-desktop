/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *   Copyright (C) 2016-2017 by Ivan Cukic <ivan.cukic@kde.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kastatsfavoritesmodel.h"
#include "appentry.h"
#include "contactentry.h"
#include "fileentry.h"
#include "actionlist.h"
#include "debug.h"

#include <QFileInfo>
#include <QTimer>
#include <QSortFilterProxyModel>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <KActivities/Consumer>
#include <KActivities/Stats/Terms>
#include <KActivities/Stats/Query>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

#define AGENT_APPLICATIONS QStringLiteral("org.kde.plasma.favorites.applications")
#define AGENT_CONTACTS     QStringLiteral("org.kde.plasma.favorites.contacts")
#define AGENT_DOCUMENTS    QStringLiteral("org.kde.plasma.favorites.documents")

QString agentForUrl(const QString &url)
{
    return url.startsWith(QLatin1String("ktp:"))
                ? AGENT_CONTACTS
         : url.startsWith(QLatin1String("preferred:"))
                ? AGENT_APPLICATIONS
         : url.startsWith(QLatin1String("applications:"))
                ? AGENT_APPLICATIONS
         : (url.startsWith(QLatin1Char('/')) && !url.endsWith(QLatin1String(".desktop")))
                ? AGENT_DOCUMENTS
         : (url.startsWith(QLatin1String("file:/")) && !url.endsWith(QLatin1String(".desktop")))
                ? AGENT_DOCUMENTS
         // use applications as the default
                : AGENT_APPLICATIONS;
}

class KAStatsFavoritesModel::Private: public QAbstractListModel {
public:
    class NormalizedId {
    public:
        NormalizedId()
        {
        }

        NormalizedId(const Private *parent, const QString &id)
        {
            if (id.isEmpty()) return;

            QSharedPointer<AbstractEntry> entry = nullptr;

            if (parent->m_itemEntries.contains(id)) {
                entry = parent->m_itemEntries[id];
            } else {
                // This entry is not cached - it is temporary,
                // so let's clean up when we exit this function
                entry = parent->entryForResource(id);
            }

            if (!entry || !entry->isValid()) {
                qWarning() << "Entry is not valid" << id << entry;
                m_id = id;
                return;
            }

            const auto url = entry->url();

            qCDebug(KICKER_DEBUG) << "Original id is: " << id << ", and the url is" << url;

            // Preferred applications need special handling
            if (entry->id().startsWith(QLatin1String("preferred:"))) {
                m_id = entry->id();
                return;
            }

            // If this is an application, use the applications:-format url
            auto appEntry = dynamic_cast<AppEntry*>(entry.data());
            if (appEntry && !appEntry->menuId().isEmpty()) {
                m_id = QStringLiteral("applications:") + appEntry->menuId();
                return;
            }

            // We want to resolve symbolic links not to have two paths
            // refer to the same .desktop file
            if (url.isLocalFile()) {
                QFileInfo file(url.toLocalFile());

                if (file.exists()) {
                    m_id = QUrl::fromLocalFile(file.canonicalFilePath()).toString();
                    return;
                }
            }

            // If this is a file, we should have already covered it
            if (url.scheme() == QLatin1String("file")) {
                return;
            }

            m_id = url.toString();
        }

        const QString& value() const
        {
            return m_id;
        }

        bool operator==(const NormalizedId &other) const
        {
            return m_id == other.m_id;
        }

    private:
        QString m_id;
    };

    NormalizedId normalizedId(const QString &id) const
    {
        return NormalizedId(this, id);
    }

    QSharedPointer<AbstractEntry> entryForResource(const QString &resource) const
    {
        using SP = QSharedPointer<AbstractEntry>;

        const auto agent =
            agentForUrl(resource);

        if (agent == AGENT_CONTACTS) {
            return SP(new ContactEntry(q, resource));

        } else if (agent == AGENT_DOCUMENTS) {
            if (resource.startsWith(QLatin1String("/"))) {
                return SP(new FileEntry(q, QUrl::fromLocalFile(resource)));
            } else {
                return SP(new FileEntry(q, QUrl(resource)));
            }

        } else if (agent == AGENT_APPLICATIONS) {
            if (resource.startsWith(QLatin1String("applications:"))) {
                return SP(new AppEntry(q, resource.mid(13)));
            } else {
                return SP(new AppEntry(q, resource));
            }

        } else {
            return {};
        }
    }

    Private(KAStatsFavoritesModel *parent, QString clientId)
        : q(parent)
        , m_query(
              LinkedResources
                  | Agent {
                      AGENT_APPLICATIONS,
                      AGENT_CONTACTS,
                      AGENT_DOCUMENTS
                  }
                  | Type::any()
                  | Activity::current()
                  | Activity::global()
                  | Limit::all()
              )
        , m_watcher(m_query)
        , m_clientId(clientId)
    {
        // Connecting the watcher
        connect(&m_watcher, &ResultWatcher::resultLinked,
                [this] (const QString &resource) {
                    addResult(resource, -1);
                });

        connect(&m_watcher, &ResultWatcher::resultUnlinked,
                [this] (const QString &resource) {
                    removeResult(resource);
                });

        // Loading the items order
        const auto cfg = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerd-statsrc"));

        // We want first to check whether we have an ordering for this activity.
        // If not, we will try to get a global one for this applet

        const QString thisGroupName =
            QStringLiteral("Favorites-") + clientId + QStringLiteral("-") + m_activities.currentActivity();
        const QString globalGroupName =
            QStringLiteral("Favorites-") + clientId + QStringLiteral("-global");

        KConfigGroup thisCfgGroup(cfg, thisGroupName);
        KConfigGroup globalCfgGroup(cfg, globalGroupName);

        QStringList ordering =
            thisCfgGroup.readEntry("ordering", QStringList()) +
            globalCfgGroup.readEntry("ordering", QStringList());

        qCDebug(KICKER_DEBUG) << "Loading the ordering " << ordering;

        // Loading the results without emitting any model signals
        qCDebug(KICKER_DEBUG) << "Query is" << m_query;
        ResultSet results(m_query);

        for (const auto& result: results) {
            qCDebug(KICKER_DEBUG) << "Got " << result.resource() << " -->";
            addResult(result.resource(), -1, false);
        }

        // Normalizing all the ids
        std::transform(ordering.begin(), ordering.end(), ordering.begin(),
                       [&] (const QString &item) {
                          return normalizedId(item).value();
                       });

        // Sorting the items in the cache
        std::sort(m_items.begin(), m_items.end(),
                [&] (const NormalizedId &left, const NormalizedId &right) {
                    auto leftIndex = ordering.indexOf(left.value());
                    auto rightIndex = ordering.indexOf(right.value());

                    return (leftIndex == -1 && rightIndex == -1) ?
                               left.value() < right.value() :

                           (leftIndex == -1) ?
                               false :

                           (rightIndex == -1) ?
                               true :

                           // otherwise
                               leftIndex < rightIndex;
                });

        // Debugging:
        QVector<QString> itemStrings(m_items.size());
        std::transform(m_items.cbegin(), m_items.cend(), itemStrings.begin(),
                [] (const NormalizedId &item) {
                    return item.value();
                });
        qCDebug(KICKER_DEBUG) << "After ordering: " << itemStrings;
    }

    void addResult(const QString &_resource, int index, bool notifyModel = true)
    {
        // We want even files to have a proper URL
        const auto resource =
            _resource.startsWith(QLatin1Char('/')) ? QUrl::fromLocalFile(_resource).toString() : _resource;

        qCDebug(KICKER_DEBUG) << "Adding result" << resource << "already present?" << m_itemEntries.contains(resource);

        if (m_itemEntries.contains(resource)) return;

        auto entry = entryForResource(resource);

        if (!entry || !entry->isValid()) {
            qCDebug(KICKER_DEBUG) << "Entry is not valid!";
            return;
        }

        if (index == -1) {
            index = m_items.count();
        }

        if (notifyModel) {
            beginInsertRows(QModelIndex(), index, index);
        }

        auto url = entry->url();

        m_itemEntries[resource]
            = m_itemEntries[entry->id()]
            = m_itemEntries[url.toString()]
            = m_itemEntries[url.toLocalFile()]
            = entry;

        auto normalized = normalizedId(resource);
        m_items.insert(index, normalized);
        m_itemEntries[normalized.value()] = entry;

        if (notifyModel) {
            endInsertRows();
            saveOrdering();
        }
    }

    void removeResult(const QString &resource)
    {
        auto normalized = normalizedId(resource);

        // If we know this item will not really be removed,
        // but only that activities it is on have changed,
        // lets leave it
        if (m_ignoredItems.contains(normalized.value())) {
            m_ignoredItems.removeAll(normalized.value());
            return;
        }

        qCDebug(KICKER_DEBUG) << "Removing result" << resource;

        auto index = m_items.indexOf(normalizedId(resource));

        if (index == -1) return;

        beginRemoveRows(QModelIndex(), index, index);
        auto entry = m_itemEntries[resource];
        m_items.removeAt(index);

        // Removing the entry from the cache
        QMutableHashIterator<QString, QSharedPointer<AbstractEntry>> i(m_itemEntries);
        while (i.hasNext()) {
            i.next();
            if (i.value() == entry) {
                i.remove();
            }
        }

        endRemoveRows();
    }


    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;

        return m_items.count();
    }

    QVariant data(const QModelIndex &item,
                  int role = Qt::DisplayRole) const override
    {
        if (item.parent().isValid()) return QVariant();

        const auto index = item.row();

        const auto entry = m_itemEntries[m_items[index].value()];

        return entry == nullptr ? QVariant()
             : role == Qt::DisplayRole ? entry->name()
             : role == Qt::DecorationRole ? entry->icon()
             : role == Kicker::DescriptionRole ? entry->description()
             : role == Kicker::FavoriteIdRole ? entry->id()
             : role == Kicker::UrlRole ? entry->url()
             : role == Kicker::HasActionListRole ? entry->hasActions()
             : role == Kicker::ActionListRole ? entry->actions()
             : QVariant();
    }

    bool trigger(int row, const QString &actionId, const QVariant &argument)
    {
        if (row < 0 || row >= rowCount()) {
            return false;
        }

        const QString id = data(index(row, 0), Kicker::UrlRole).toString();

        return m_itemEntries.contains(id)
                    ? m_itemEntries[id]->run(actionId, argument)
                    : false;
    }

    void move(int from, int to)
    {
        if (from < 0) return;
        if (from >= m_items.count()) return;
        if (to < 0) return;
        if (to >= m_items.count()) return;

        if (from == to) return;

        const int modelTo = to + (to > from ? 1 : 0);

        if (q->beginMoveRows(QModelIndex(), from, from,
                             QModelIndex(), modelTo)) {
            m_items.move(from, to);
            q->endMoveRows();

            qCDebug(KICKER_DEBUG) << "Save ordering (from Private::move) -->";
            saveOrdering();
        }
    }

    void saveOrdering()
    {
        QStringList ids;

        for (const auto& item: m_items) {
            ids << item.value();
        }

        qCDebug(KICKER_DEBUG) << "Save ordering (from Private::saveOrdering) -->";
        saveOrdering(ids, m_clientId, m_activities.currentActivity());
    }

    static void saveOrdering(const QStringList &ids, const QString &clientId, const QString &currentActivity)
    {
        const auto cfg = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerd-statsrc"));

        QStringList activities {
            currentActivity,
            QStringLiteral("global")
        };

        qCDebug(KICKER_DEBUG) << "Saving ordering for" << currentActivity << "and global" << ids;

        for (const auto& activity: activities) {
            const QString groupName =
                QStringLiteral("Favorites-") + clientId + QStringLiteral("-") + activity;

            KConfigGroup cfgGroup(cfg, groupName);

            cfgGroup.writeEntry("ordering", ids);
        }

        cfg->sync();
    }

    KAStatsFavoritesModel *const q;
    KActivities::Consumer m_activities;
    Query m_query;
    ResultWatcher m_watcher;
    QString m_clientId;

    QVector<NormalizedId> m_items;
    QHash<QString, QSharedPointer<AbstractEntry>> m_itemEntries;
    QStringList m_ignoredItems;
};

KAStatsFavoritesModel::KAStatsFavoritesModel(QObject *parent)
: PlaceholderModel(parent)
, d(nullptr) // we have no client id yet
, m_enabled(true)
, m_maxFavorites(-1)
, m_activities(new KActivities::Consumer(this))
{
    connect(m_activities, &KActivities::Consumer::currentActivityChanged,
            this, [&] (const QString &currentActivity) {
                qCDebug(KICKER_DEBUG) << "Activity just got changed to" << currentActivity;
                Q_UNUSED(currentActivity);
                if (d) {
                    auto clientId = d->m_clientId;
                    initForClient(clientId);
                }
            });
}

KAStatsFavoritesModel::~KAStatsFavoritesModel()
{
    delete d;
}

void KAStatsFavoritesModel::initForClient(const QString &clientId)
{
    qCDebug(KICKER_DEBUG) << "initForClient" << clientId;

    setSourceModel(nullptr);
    delete d;
    d = new Private(
        this,
        clientId
        );

    setSourceModel(d);
}

QString KAStatsFavoritesModel::description() const
{
    return i18n("Favorites");
}

bool KAStatsFavoritesModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    return d && d->trigger(row, actionId, argument);
}

bool KAStatsFavoritesModel::enabled() const
{
    return m_enabled;
}

int KAStatsFavoritesModel::maxFavorites() const
{
    return m_maxFavorites;
}

void KAStatsFavoritesModel::setMaxFavorites(int max)
{
    Q_UNUSED(max);
}

void KAStatsFavoritesModel::setEnabled(bool enable)
{
    if (m_enabled != enable) {
        m_enabled = enable;

        emit enabledChanged();
    }
}

QStringList KAStatsFavoritesModel::favorites() const
{
    qWarning() << "KAStatsFavoritesModel::favorites returns nothing, it is here just to keep the API backwards-compatible";
    return QStringList();
}

void KAStatsFavoritesModel::setFavorites(const QStringList& favorites)
{
    Q_UNUSED(favorites);
    qWarning() << "KAStatsFavoritesModel::setFavorites is ignored";
}

bool KAStatsFavoritesModel::isFavorite(const QString &id) const
{
    return d && d->m_itemEntries.contains(id);
}

void KAStatsFavoritesModel::portOldFavorites(const QStringList &ids)
{
    if (!d) return;
    qCDebug(KICKER_DEBUG) << "portOldFavorites" << ids;

    const QString activityId = QStringLiteral(":global");
    std::for_each(ids.begin(), ids.end(), [&] (const QString &id) {
                addFavoriteTo(id, activityId);
            });

    // Resetting the model
    auto clientId = d->m_clientId;
    setSourceModel(nullptr);
    delete d;
    d = nullptr;

    qCDebug(KICKER_DEBUG) << "Save ordering (from portOldFavorites) -->";
    Private::saveOrdering(ids, clientId, m_activities->currentActivity());

    QTimer::singleShot(500,
        std::bind(&KAStatsFavoritesModel::initForClient, this, clientId));
}

void KAStatsFavoritesModel::addFavorite(const QString &id, int index)
{
    qCDebug(KICKER_DEBUG) << "addFavorite" << id << index << " -->";
    addFavoriteTo(id, QStringLiteral(":global"), index);
}

void KAStatsFavoritesModel::removeFavorite(const QString &id)
{
    qCDebug(KICKER_DEBUG) << "removeFavorite" << id << " -->";
    removeFavoriteFrom(id, QStringLiteral(":any"));
}

void KAStatsFavoritesModel::addFavoriteTo(const QString &id, const QString &activityId, int index)
{
    qCDebug(KICKER_DEBUG) << "addFavoriteTo" << id << activityId << index << " -->";
    addFavoriteTo(id, Activity(activityId), index);
}

void KAStatsFavoritesModel::removeFavoriteFrom(const QString &id, const QString &activityId)
{
    qCDebug(KICKER_DEBUG) << "removeFavoriteFrom" << id << activityId << " -->";
    removeFavoriteFrom(id, Activity(activityId));
}

void KAStatsFavoritesModel::addFavoriteTo(const QString &id, const Activity &activity, int index)
{
    if (!d || id.isEmpty()) return;

    Q_ASSERT(!activity.values.isEmpty());

    setDropPlaceholderIndex(-1);

    QStringList matchers { d->m_activities.currentActivity(), QStringLiteral(":global"), QStringLiteral(":current") };
    if (std::find_first_of(activity.values.cbegin(), activity.values.cend(),
                           matchers.cbegin(), matchers.cend()) != activity.values.cend()) {
        d->addResult(id, index);
    }

    const auto url = d->normalizedId(id).value();

    qCDebug(KICKER_DEBUG) << "addFavoriteTo" << id << activity << index << url << " (actual)";

    if (url.isEmpty()) return;

    d->m_watcher.linkToActivity(QUrl(url), activity,
                                Agent(agentForUrl(url)));
}

void KAStatsFavoritesModel::removeFavoriteFrom(const QString &id, const Activity &activity)
{
    if (!d || id.isEmpty()) return;

    const auto url = d->normalizedId(id).value();

    Q_ASSERT(!activity.values.isEmpty());

    qCDebug(KICKER_DEBUG) << "addFavoriteTo" << id << activity << url << " (actual)";

    if (url.isEmpty()) return;

    d->m_watcher.unlinkFromActivity(QUrl(url), activity,
                                    Agent(agentForUrl(url)));
}

void KAStatsFavoritesModel::setFavoriteOn(const QString &id, const QString &activityId)
{
    if (!d || id.isEmpty()) return;

    const auto url = d->normalizedId(id).value();

    qCDebug(KICKER_DEBUG) << "setFavoriteOn" << id << activityId << url << " (actual)";

    qCDebug(KICKER_DEBUG) << "%%%%%%%%%%% Activity is" << activityId;
    if (activityId.isEmpty() || activityId == QLatin1String(":any") ||
            activityId == QStringLiteral(":global") ||
            activityId == m_activities->currentActivity()) {
        d->m_ignoredItems << url;
    }

    d->m_watcher.unlinkFromActivity(QUrl(url), Activity::any(),
                                    Agent(agentForUrl(url)));
    d->m_watcher.linkToActivity(QUrl(url), activityId,
                                Agent(agentForUrl(url)));
}

void KAStatsFavoritesModel::moveRow(int from, int to)
{
    if (!d) return;

    d->move(from, to);
}

AbstractModel *KAStatsFavoritesModel::favoritesModel()
{
    return this;
}

void KAStatsFavoritesModel::refresh()
{
}

QObject *KAStatsFavoritesModel::activities() const
{
    return m_activities;
}

QString KAStatsFavoritesModel::activityNameForId(const QString &activityId) const
{
    // It is safe to use a short-lived object here,
    // we are always synced with KAMD in plasma
    KActivities::Info info(activityId);
    return info.name();
}

QStringList KAStatsFavoritesModel::linkedActivitiesFor(const QString &id) const
{
    if (!d) {
        qCDebug(KICKER_DEBUG) << "Linked for" << id << "is empty, no Private instance";
        return {};
    }

    auto url = d->normalizedId(id).value();

    if (url.startsWith(QLatin1String("file:"))) {
        url = QUrl(url).toLocalFile();
    }

    if (url.isEmpty()) {
        qCDebug(KICKER_DEBUG) << "The url for" << id << "is empty";
        return {};
    }

    auto query = LinkedResources
                    | Agent {
                        AGENT_APPLICATIONS,
                        AGENT_CONTACTS,
                        AGENT_DOCUMENTS
                      }
                    | Type::any()
                    | Activity::any()
                    | Url(url)
                    | Limit::all();

    ResultSet results(query);

    for (const auto &result: results) {
        qCDebug(KICKER_DEBUG) << "Returning" << result.linkedActivities() << "for" << id << url;
        return result.linkedActivities();
    }

    qCDebug(KICKER_DEBUG) << "Returning empty list of activities for" << id << url;
    return {};
}

