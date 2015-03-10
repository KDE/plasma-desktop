/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
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

#include "recentdocsmodel.h"
#include "actionlist.h"

#include <QDebug>
#include <QIcon>
#include <QMimeType>
#include <QMimeDatabase>

#include <KFileItem>
#include <KRun>

#include <kactivitiesstats/resultmodel.h>
#include <kactivitiesstats/terms.h>

namespace KAStats = KActivities::Experimental::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

RecentDocsModel::RecentDocsModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    connect(this, &QSortFilterProxyModel::rowsInserted, this, &RecentDocsModel::countChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this, &RecentDocsModel::countChanged);

    refresh();

    // FIXME TODO: Duplication from AbstractModel.
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(Kicker::IsParentRole, "isParent");
    roles.insert(Kicker::HasChildrenRole, "hasChildren");
    roles.insert(Kicker::FavoriteIdRole, "favoriteId");
    roles.insert(Kicker::HasActionListRole, "hasActionList");
    roles.insert(Kicker::ActionListRole, "actionList");
    roles.insert(Kicker::UrlRole, "url");

    setRoleNames(roles);
}

RecentDocsModel::~RecentDocsModel()
{
}

QVariant RecentDocsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QUrl url(QSortFilterProxyModel::data(index, ResultModel::ResourceRole).toString());
    const KFileItem fileItem(url);

    if (!url.isValid() || !fileItem.isFile()) {
        return QLatin1String("Resource not a file!");
    }

    if (role == Qt::DisplayRole) {
        return url.fileName();
    } else if (role == Qt::DecorationRole) {
        return QIcon::fromTheme(fileItem.iconName());
    } else if (role == Kicker::HasActionListRole) {
        return true;
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList = Kicker::createActionListForFileItem(fileItem);

        /* FIXME TODO: No support in KActivities.
        actionList.prepend(Kicker::createSeparatorActionItem());

        const QVariantMap &forgetAllAction = Kicker::createActionItem(i18n("Forget All Documents"), "forgetAll");
        actionList.prepend(forgetAllAction);

        const QVariantMap &forgetAction = Kicker::createActionItem(i18n("Forget Document"), "forget");
        actionList.prepend(forgetAction);
        */

        return actionList;
    }

    return QVariant();
}

bool RecentDocsModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= rowCount()) {
        return false;
    }

    QUrl url(QSortFilterProxyModel::data(index(row, 0), ResultModel::ResourceRole).toString());

    if (actionId.isEmpty()) {
        new KRun(url, 0);

        return true;
    } else if (actionId == "forget") {
        forget(row);

        return false;
    } else if (actionId == "forgetAll") {
        forgetAll();

        return true;
    }

    bool close = false;

    KFileItem item(url);

    if (Kicker::handleFileItemAction(item, actionId, argument, &close)) {
        return close;
    }

    return false;
}

void RecentDocsModel::refresh()
{
    QObject *oldModel = sourceModel();

    auto query = (LinkedResources | RecentlyUsedFirst | Agent(":any") | Type(":any") | Activity(":current"));

    ResultModel *model = new ResultModel(query);
    model->setItemCountLimit(15);

    setSourceModel(model);

    delete oldModel;

    emit countChanged();
}

void RecentDocsModel::forget(int row)
{
    Q_UNUSED(row)

    // FIXME TODO: No support in KActivities.
}

void RecentDocsModel::forgetAll()
{
    // FIXME TODO: FIXME TODO: No support in KActivities.
}
