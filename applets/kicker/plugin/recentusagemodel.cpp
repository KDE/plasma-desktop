/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
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

#include "recentusagemodel.h"
#include "actionlist.h"
#include "appsmodel.h"
#include "appentry.h"
#include "kastatsfavoritesmodel.h"
#include <kio_version.h>

#include <config-X11.h>

#include <QIcon>
#include <QMimeDatabase>
#include <QQmlEngine>
#include <QTimer>
#if HAVE_X11
#include <QX11Info>
#endif

#include <KActivities/ResourceInstance>
#include <KFileItem>
#include <KLocalizedString>
#include <KMimeTypeTrader>
#include <KRun>
#include <KService>
#include <KStartupInfo>

#include <KActivities/Stats/Cleaning>
#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

GroupSortProxy::GroupSortProxy(AbstractModel *parentModel, QAbstractItemModel *sourceModel) : QSortFilterProxyModel(parentModel)
{
    sourceModel->setParent(this);
    setSourceModel(sourceModel);
    sort(0);
}

GroupSortProxy::~GroupSortProxy()
{
}

InvalidAppsFilterProxy::InvalidAppsFilterProxy(AbstractModel *parentModel, QAbstractItemModel *sourceModel) : QSortFilterProxyModel(parentModel)
, m_parentModel(parentModel)
{
    connect(parentModel, &AbstractModel::favoritesModelChanged, this, &InvalidAppsFilterProxy::connectNewFavoritesModel);
    connectNewFavoritesModel();

    sourceModel->setParent(this);
    setSourceModel(sourceModel);
}

InvalidAppsFilterProxy::~InvalidAppsFilterProxy()
{
}

void InvalidAppsFilterProxy::connectNewFavoritesModel()
{
    KAStatsFavoritesModel* favoritesModel = static_cast<KAStatsFavoritesModel *>(m_parentModel->favoritesModel());
    connect(favoritesModel, &KAStatsFavoritesModel::favoritesChanged, this, &QSortFilterProxyModel::invalidate);

    invalidate();
}

bool InvalidAppsFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);

    const QString resource = sourceModel()->index(source_row, 0).data(ResultModel::ResourceRole).toString();

    if (resource.startsWith(QLatin1String("applications:"))) {
        KService::Ptr service = KService::serviceByStorageId(resource.section(QLatin1Char(':'), 1));

        KAStatsFavoritesModel* favoritesModel = m_parentModel ? static_cast<KAStatsFavoritesModel *>(m_parentModel->favoritesModel()) : nullptr;

        return (service && (!favoritesModel || !favoritesModel->isFavorite(service->storageId())));
    }

    return true;
}

bool InvalidAppsFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return (left.row() < right.row());
}

bool GroupSortProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const QString &lResource = sourceModel()->data(left, ResultModel::ResourceRole).toString();
    const QString &rResource = sourceModel()->data(right, ResultModel::ResourceRole).toString();

    if (lResource.startsWith(QLatin1String("applications:"))
        && !rResource.startsWith(QLatin1String("applications:"))) {
        return true;
    } else if (!lResource.startsWith(QLatin1String("applications:"))
        && rResource.startsWith(QLatin1String("applications:"))) {
        return false;
    }

    return (left.row() < right.row());
}

RecentUsageModel::RecentUsageModel(QObject *parent, IncludeUsage usage, int ordering)
: ForwardingModel(parent)
, m_usage(usage)
, m_ordering((Ordering)ordering)
, m_complete(false)
{
    refresh();
}

RecentUsageModel::~RecentUsageModel()
{
}

RecentUsageModel::IncludeUsage RecentUsageModel::usage() const
{
    return m_usage;
}

QString RecentUsageModel::description() const
{
    switch (m_usage) {
        case AppsAndDocs:
            return i18n("Recently Used");
        case OnlyApps:
            return i18n("Applications");
        case OnlyDocs:
        default:
            return i18n("Documents");
    }
}

QString RecentUsageModel::resourceAt(int row) const
{
    QSortFilterProxyModel *sourceProxy = qobject_cast<QSortFilterProxyModel *>(sourceModel());

    if (sourceProxy) {
        return sourceProxy->sourceModel()->data(sourceProxy->mapToSource(sourceProxy->index(row, 0)),
            ResultModel::ResourceRole).toString();
    }

    return sourceModel()->data(index(row, 0), ResultModel::ResourceRole).toString();
}

QVariant RecentUsageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QString &resource = resourceAt(index.row());

    if (resource.startsWith(QLatin1String("applications:"))) {
        return appData(resource, role);
    } else {
        return docData(resource, role);
    }
}

QVariant RecentUsageModel::appData(const QString &resource, int role) const
{
    const QString storageId = resource.section(QLatin1Char(':'), 1);
    KService::Ptr service = KService::serviceByStorageId(storageId);

    QStringList allowedTypes({ QLatin1String("Service"), QLatin1String("Application") });

    if (!service || !allowedTypes.contains(service->property(QLatin1String("Type")).toString())
            || service->exec().isEmpty()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        AppsModel *parentModel = qobject_cast<AppsModel *>(QObject::parent());

        if (parentModel)  {
            return AppEntry::nameFromService(service,
                (AppEntry::NameFormat)qobject_cast<AppsModel *>(QObject::parent())->appNameFormat());
        } else {
            return AppEntry::nameFromService(service, AppEntry::NameOnly);
        }
    } else if (role == Qt::DecorationRole) {
        return QIcon::fromTheme(service->icon(), QIcon::fromTheme(QStringLiteral("unknown")));
    } else if (role == Kicker::DescriptionRole) {
        return service->comment();
    } else if (role == Kicker::GroupRole) {
        return i18n("Applications");
    } else if (role == Kicker::FavoriteIdRole) {
        return service->storageId();
    } else if (role == Kicker::HasActionListRole) {
        return true;
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList;

        const QVariantList &jumpList = Kicker::jumpListActions(service);
        if (!jumpList.isEmpty()) {
            actionList << jumpList << Kicker::createSeparatorActionItem();
        }

        const QVariantList &recentDocuments = Kicker::recentDocumentActions(service);
        if (!recentDocuments.isEmpty()) {
            actionList << recentDocuments << Kicker::createSeparatorActionItem();
        }

        const QVariantMap &forgetAction = Kicker::createActionItem(i18n("Forget Application"), QStringLiteral("forget"));
        actionList << forgetAction;

        const QVariantMap &forgetAllAction = Kicker::createActionItem(forgetAllActionName(), QStringLiteral("forgetAll"));
        actionList << forgetAllAction;

        return actionList;
    }

    return QVariant();
}

QVariant RecentUsageModel::docData(const QString &resource, int role) const
{
    QUrl url(resource);

    if (url.scheme().isEmpty()) {
        url.setScheme(QStringLiteral("file"));
    }

#if KIO_VERSION >= QT_VERSION_CHECK(5,57,0)
    // Avoid calling QT_LSTAT and accessing recent documents
    const KFileItem fileItem(url, KFileItem::SkipMimeTypeFromContent);
#else
    const KFileItem fileItem(url);
#endif

    if (!url.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return fileItem.text();
    } else if (role == Qt::DecorationRole) {
        return QIcon::fromTheme(fileItem.iconName(), QIcon::fromTheme(QStringLiteral("unknown")));
    } else if (role == Kicker::GroupRole) {
        return i18n("Documents");
    } else if (role == Kicker::FavoriteIdRole || role == Kicker::UrlRole) {
        return url.toString();
    } else if (role == Kicker::UrlRole) {
        return url;
    } else if (role == Kicker::HasActionListRole) {
        return true;
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList = Kicker::createActionListForFileItem(fileItem);

        actionList << Kicker::createSeparatorActionItem();

        const QVariantMap &forgetAction = Kicker::createActionItem(i18n("Forget Document"), QStringLiteral("forget"));
        actionList << forgetAction;

        const QVariantMap &forgetAllAction = Kicker::createActionItem(forgetAllActionName(), QStringLiteral("forgetAll"));
        actionList << forgetAllAction;

        return actionList;
    }

    return QVariant();
}

bool RecentUsageModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    Q_UNUSED(argument)

    bool withinBounds = row >= 0 && row < rowCount();

    if (actionId.isEmpty() && withinBounds) {
        const QString &resource = resourceAt(row);

        if (!resource.startsWith(QLatin1String("applications:"))) {
            const QUrl resourceUrl = docData(resource, Kicker::UrlRole).toUrl();
            const QList<QUrl> urlsList{resourceUrl};

            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForUrl(resourceUrl);
            KService::Ptr service = KMimeTypeTrader::self()->preferredService(mime.name());
            if (service) {
                KRun::runApplication(*service, urlsList, nullptr);
            } else {
                QTimer::singleShot(0, [urlsList] {
                    KRun::displayOpenWithDialog(urlsList, nullptr);
                });
            }

            return true;
        }

        const QString storageId = resource.section(QLatin1Char(':'), 1);
        KService::Ptr service = KService::serviceByStorageId(storageId);

        if (!service) {
            return false;
        }

        quint32 timeStamp = 0;

#if HAVE_X11
        if (QX11Info::isPlatformX11()) {
            timeStamp = QX11Info::appUserTime();
        }
#endif

        // TODO Once we depend on KDE Frameworks 5.24 and D1902 is merged, use KRun::runApplication instead
        KRun::runService(*service, {}, nullptr, true, {}, KStartupInfo::createNewStartupIdForTimestamp(timeStamp));

        KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + storageId),
            QStringLiteral("org.kde.plasma.kicker"));

        return true;
    } else if (actionId == QLatin1String("forget") && withinBounds) {
        if (m_activitiesModel) {
            QModelIndex idx = sourceModel()->index(row, 0);
            QSortFilterProxyModel *sourceProxy = qobject_cast<QSortFilterProxyModel *>(sourceModel());

            while (sourceProxy) {
                idx = sourceProxy->mapToSource(idx);
                sourceProxy = qobject_cast<QSortFilterProxyModel *>(sourceProxy->sourceModel());
            }

            static_cast<ResultModel *>(m_activitiesModel.data())->forgetResource(idx.row());
        }

        return false;
    } else if (actionId == QLatin1String("forgetAll")) {
        if (m_activitiesModel) {
            static_cast<ResultModel *>(m_activitiesModel.data())->forgetAllResources();
        }

        return false;
    } else if (withinBounds) {
        const QString &resource = resourceAt(row);

        if (resource.startsWith(QLatin1String("applications:"))) {
            const QString storageId = sourceModel()->data(sourceModel()->index(row, 0),
                ResultModel::ResourceRole).toString().section(QLatin1Char(':'), 1);
            KService::Ptr service = KService::serviceByStorageId(storageId);

            if (service) {
                return Kicker::handleRecentDocumentAction(service, actionId, argument);
            }
        } else {
            bool close = false;

            QUrl url(sourceModel()->data(sourceModel()->index(row, 0), ResultModel::ResourceRole).toString());

            KFileItem item(url);

            if (Kicker::handleFileItemAction(item, actionId, argument, &close)) {
                return close;
            }
        }
    }

    return false;
}

bool RecentUsageModel::hasActions() const
{
    return rowCount();
}

QVariantList RecentUsageModel::actions() const
{
    QVariantList actionList;

    if (rowCount()) {
        actionList << Kicker::createActionItem(forgetAllActionName(), QStringLiteral("forgetAll"));
    }

    return actionList;
}

QString RecentUsageModel::forgetAllActionName() const
{
    switch (m_usage) {
        case AppsAndDocs:
            return i18n("Forget All");
        case OnlyApps:
            return i18n("Forget All Applications");
        case OnlyDocs:
        default:
            return i18n("Forget All Documents");
    }
}

void RecentUsageModel::setOrdering(int ordering)
{
    if (ordering == m_ordering) return;

    m_ordering = (Ordering)ordering;
    refresh();

    emit orderingChanged(ordering);
}

int RecentUsageModel::ordering() const
{
    return m_ordering;
}

void RecentUsageModel::classBegin()
{
}

void RecentUsageModel::componentComplete()
{
    m_complete = true;

    refresh();
}

void RecentUsageModel::refresh()
{
    if (qmlEngine(this) && !m_complete) {
        return;
    }

    QAbstractItemModel *oldModel = sourceModel();
    disconnectSignals();
    setSourceModel(nullptr);
    delete oldModel;

    auto query = UsedResources
                    | (m_ordering == Recent ? RecentlyUsedFirst : HighScoredFirst)
                    | Agent::any()
                    | Type::any()
                    | Activity::current();

    switch (m_usage) {
        case AppsAndDocs:
        {
            query = query | Url::startsWith(QStringLiteral("applications:")) | Url::file() | Limit(30);
            break;
        }
        case OnlyApps:
        {
            query = query | Url::startsWith(QStringLiteral("applications:")) | Limit(15);
            break;
        }
        case OnlyDocs:
        default:
        {
            query = query | Url::file() | Limit(15);
        }
    }

    m_activitiesModel = new ResultModel(query);
    QAbstractItemModel *model = m_activitiesModel;

    QModelIndex index;

    if (model->canFetchMore(index)) {
        model->fetchMore(index);
    }

    if (m_usage != OnlyDocs) {
        model = new InvalidAppsFilterProxy(this, model);
    }

    if (m_usage == AppsAndDocs) {
        model = new GroupSortProxy(this, model);
    }

    setSourceModel(model);
}
