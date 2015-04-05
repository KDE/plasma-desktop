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

#include <QIcon>

#include <KFileItem>
#include <KLocalizedString>
#include <KRun>

#include <KActivitiesExperimentalStats/Cleaning>
#include <KActivitiesExperimentalStats/ResultModel>
#include <KActivitiesExperimentalStats/Terms>

namespace KAStats = KActivities::Experimental::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

RecentDocsModel::RecentDocsModel(QObject *parent) : ForwardingModel(parent)
{
    refresh();
}

RecentDocsModel::~RecentDocsModel()
{
}

QVariant RecentDocsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QUrl url(sourceModel()->data(index, ResultModel::ResourceRole).toString());
    const KFileItem fileItem(url);

    if (!url.isValid() || !fileItem.isFile()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return url.fileName();
    } else if (role == Qt::DecorationRole) {
        return QIcon::fromTheme(fileItem.iconName(), QIcon::fromTheme("unknown"));
    } else if (role == Kicker::HasActionListRole) {
        return true;
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList = Kicker::createActionListForFileItem(fileItem);

        actionList.prepend(Kicker::createSeparatorActionItem());

        const QVariantMap &forgetAction = Kicker::createActionItem(i18n("Forget Document"), "forget");
        actionList << forgetAction;

        const QVariantMap &forgetAllAction = Kicker::createActionItem(i18n("Forget All Documents"), "forgetAll");
        actionList << forgetAllAction;

        return actionList;
    }

    return QVariant();
}

bool RecentDocsModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= rowCount()) {
        return false;
    }

    QUrl url(sourceModel()->data(sourceModel()->index(row, 0), ResultModel::ResourceRole).toString());

    if (actionId.isEmpty()) {
        new KRun(url, 0);

        return true;
    } else if (actionId == "forget") {
        if (sourceModel()) {
            ResultModel *resultModel = static_cast<ResultModel *>(sourceModel());
            resultModel->forgetResource(row);
        }

        return false;
    } else if (actionId == "forgetAll") {
        if (sourceModel()) {
            ResultModel *resultModel = static_cast<ResultModel *>(sourceModel());
            resultModel->forgetAllResources();
        }

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

    auto query = UsedResources
                    | RecentlyUsedFirst
                    | Agent::any()
                    | Type::any()
                    | Activity::current()
                    | Url::file();

    ResultModel *model = new ResultModel(query);
    model->setItemCountLimit(15);

    setSourceModel(model);

    delete oldModel;
}
