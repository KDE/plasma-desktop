/***************************************************************************
 *   Copyright (C) 2013 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#include "actionlist.h"

#include <QApplication>

#include <KLocalizedString>
#include <KMimeTypeTrader>
#include <KPropertiesDialog>
#include <KRun>

#include <KActivitiesExperimentalStats/Cleaning>
#include <KActivitiesExperimentalStats/ResultSet>
#include <KActivitiesExperimentalStats/Terms>

namespace KAStats = KActivities::Experimental::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

namespace Kicker
{

QVariantMap createActionItem(const QString &label, const QString &actionId, const QVariant &argument)
{
    QVariantMap map;

    map["text"] = label;
    map["actionId"] = actionId;

    if (argument.isValid()) {
        map["actionArgument"] = argument;
    }

    return map;
}

QVariantMap createTitleActionItem(const QString &label)
{
    QVariantMap map;

    map["text"] = label;
    map["type"] = "title";

    return map;
}

QVariantMap createSeparatorActionItem()
{
    QVariantMap map;

    map["type"] = "separator";

    return map;
}

QVariantList createActionListForFileItem(const KFileItem &fileItem)
{
    QVariantList list;

    KService::List services = KMimeTypeTrader::self()->query(fileItem.mimetype(), "Application");

    if (!services.isEmpty()) {
        list << createTitleActionItem(i18n("Open with:"));

        foreach (const KService::Ptr service, services) {
            const QString text = service->name().replace('&', "&&");
            QVariantMap item = createActionItem(text, "_kicker_fileItem_openWith", service->entryPath());
            item["icon"] = service->icon();

            list << item;
        }

        list << createSeparatorActionItem();
    }

    list << createActionItem(i18n("Properties"), "_kicker_fileItem_properties");

    return list;
}

bool handleFileItemAction(const KFileItem &fileItem, const QString &actionId, const QVariant &argument, bool *close)
{
    if (actionId == "_kicker_fileItem_properties") {
        KPropertiesDialog *dlg = new KPropertiesDialog(fileItem, QApplication::activeWindow());
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();

        *close = false;

        return true;
    }

    if (actionId == "_kicker_fileItem_openWith") {
        const QString path = argument.toString();
        const KService::Ptr service = KService::serviceByDesktopPath(path);

        if (!service) {
            return false;
        }

        KRun::runService(*service, QList<QUrl>() << fileItem.url(), QApplication::activeWindow());

        *close = true;

        return true;
    }

    return false;
}

// HACK TEMP FIXME TODO IVAN
QString storageIdFromService(KService::Ptr service)
{
    QString storageId = service->storageId();

    if (storageId.startsWith("org.kde.")) {
        storageId = storageId.right(storageId.length() - 8);
    }

    if (storageId.endsWith(".desktop")) {
        storageId = storageId.left(storageId.length() - 8);
    }

    return storageId;
}

QVariantList recentDocumentActions(KService::Ptr service)
{
    QVariantList list;

    if (!service) {
        return list;
    }

    const QString storageId = storageIdFromService(service);

    if (storageId.isEmpty()) {
        return list;
    }

    auto query = UsedResources
                    | RecentlyUsedFirst
                    | Agent(storageId)
                    | Type::any()
                    | Activity::current()
                    | Url::file();

    ResultSet results(query);

    ResultSet::const_iterator resultIt;
    resultIt = results.begin();

    while (list.count() < 6 && resultIt != results.end()) {
        const QString resource = (*resultIt).resource();
        const QUrl url(resource);

        if (!url.isValid()) {
            continue;
        }

        const KFileItem fileItem(url);

        if (!fileItem.isFile()) {
            continue;
        }

        if (list.count() == 0) {
            list << createTitleActionItem(i18n("Recent documents:"));
        }

        QVariantMap item = createActionItem(url.fileName(), "_kicker_recentDocument", resource);
        item["icon"] = fileItem.iconName();

        list << item;

        ++resultIt;
    }

    if (list.count()) {
        list << createActionItem(i18n("Forget Recent Documents"), "_kicker_forgetRecentDocuments");
    }

    return list;
}

bool handleRecentDocumentAction(KService::Ptr service, const QString &actionId, const QVariant &_argument)
{
    if (!service) {
        return false;
    }

    if (actionId == "_kicker_forgetRecentDocuments") {
        const QString storageId = storageIdFromService(service);

        if (storageId.isEmpty()) {
            return false;
        }

        // IVAN
        // forgetResource(Activity::current(), Agent(storageId), "*");

        return true;
    }

    QString argument = _argument.toString();

    if (argument.isEmpty()) {
        return false;
    }

    return (KRun::runService(*service, QList<QUrl>() << QUrl(argument), QApplication::activeWindow()) != 0);
}

}
