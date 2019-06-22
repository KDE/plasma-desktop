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
#include "menuentryeditor.h"

#include <config-appstream.h>

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KMimeTypeTrader>
#include <KPropertiesDialog>
#include <KProtocolInfo>
#include <KRun>

#include <KActivities/Stats/Cleaning>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

#include "containmentinterface.h"

#ifdef HAVE_APPSTREAMQT
#include <AppStreamQt/pool.h>
#endif

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

namespace Kicker
{

QVariantMap createActionItem(const QString &label, const QString &actionId, const QVariant &argument)
{
    QVariantMap map;

    map[QStringLiteral("text")] = label;
    map[QStringLiteral("actionId")] = actionId;

    if (argument.isValid()) {
        map[QStringLiteral("actionArgument")] = argument;
    }

    return map;
}

QVariantMap createTitleActionItem(const QString &label)
{
    QVariantMap map;

    map[QStringLiteral("text")] = label;
    map[QStringLiteral("type")] = QStringLiteral("title");

    return map;
}

QVariantMap createSeparatorActionItem()
{
    QVariantMap map;

    map[QStringLiteral("type")] = QStringLiteral("separator");

    return map;
}

QVariantList createActionListForFileItem(const KFileItem &fileItem)
{
    QVariantList list;

    KService::List services = KMimeTypeTrader::self()->query(fileItem.mimetype(), QStringLiteral("Application"));

    if (!services.isEmpty()) {
        list << createTitleActionItem(i18n("Open with:"));

        foreach (const KService::Ptr service, services) {
            const QString text = service->name().replace(QLatin1Char('&'), QStringLiteral("&&"));
            QVariantMap item = createActionItem(text, QStringLiteral("_kicker_fileItem_openWith"), service->entryPath());
            item[QStringLiteral("icon")] = service->icon();

            list << item;
        }

        list << createSeparatorActionItem();
    }

    QVariantMap propertiesItem = createActionItem(i18n("Properties"), QStringLiteral("_kicker_fileItem_properties"));
    propertiesItem[QStringLiteral("icon")] = QStringLiteral("document-properties");
    list << propertiesItem;

    return list;
}

bool handleFileItemAction(const KFileItem &fileItem, const QString &actionId, const QVariant &argument, bool *close)
{
    if (actionId == QLatin1String("_kicker_fileItem_properties")) {
        KPropertiesDialog *dlg = new KPropertiesDialog(fileItem, QApplication::activeWindow());
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();

        *close = false;

        return true;
    }

    if (actionId == QLatin1String("_kicker_fileItem_openWith")) {
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

QVariantList createAddLauncherActionList(QObject *appletInterface, const KService::Ptr &service)
{
    QVariantList actionList;
    if (!service) {
        return actionList;
    }

    if (ContainmentInterface::mayAddLauncher(appletInterface, ContainmentInterface::Desktop)) {
        actionList << Kicker::createActionItem(i18n("Add to Desktop"), QStringLiteral("addToDesktop"));
    }

    if (ContainmentInterface::mayAddLauncher(appletInterface, ContainmentInterface::Panel)) {
        actionList << Kicker::createActionItem(i18n("Add to Panel (Widget)"), QStringLiteral("addToPanel"));
    }

    if (service && ContainmentInterface::mayAddLauncher(appletInterface, ContainmentInterface::TaskManager, Kicker::resolvedServiceEntryPath(service))) {
        actionList << Kicker::createActionItem(i18n("Pin to Task Manager"), QStringLiteral("addToTaskManager"));
    }

    return actionList;
}

bool handleAddLauncherAction(const QString &actionId, QObject *appletInterface, const KService::Ptr &service)
{
    if (!service) {
        return false;
    }

    if (actionId == QLatin1String("addToDesktop")) {
        if (ContainmentInterface::mayAddLauncher(appletInterface, ContainmentInterface::Desktop)) {
            ContainmentInterface::addLauncher(appletInterface, ContainmentInterface::Desktop, Kicker::resolvedServiceEntryPath(service));
        }
        return true;
    } else if (actionId == QLatin1String("addToPanel")) {
        if (ContainmentInterface::mayAddLauncher(appletInterface, ContainmentInterface::Panel)) {
            ContainmentInterface::addLauncher(appletInterface, ContainmentInterface::Panel, Kicker::resolvedServiceEntryPath(service));
        }
        return true;
    } else if (actionId == QLatin1String("addToTaskManager")) {
        if (ContainmentInterface::mayAddLauncher(appletInterface, ContainmentInterface::TaskManager, Kicker::resolvedServiceEntryPath(service))) {
            ContainmentInterface::addLauncher(appletInterface, ContainmentInterface::TaskManager, Kicker::resolvedServiceEntryPath(service));
        }
        return true;
    }

    return false;
}

QString storageIdFromService(KService::Ptr service)
{
    QString storageId = service->storageId();

    if (storageId.endsWith(QLatin1String(".desktop"))) {
        storageId = storageId.left(storageId.length() - 8);
    }

    return storageId;
}

QVariantList jumpListActions(KService::Ptr service)
{
    QVariantList list;

    if (!service) {
        return list;
    }

    const auto &actions = service->actions();
    foreach (const KServiceAction &action, actions) {
        if (action.text().isEmpty() || action.exec().isEmpty()) {
            continue;
        }

        QVariantMap item = createActionItem(action.text(), QStringLiteral("_kicker_jumpListAction"), action.exec());
        item[QStringLiteral("icon")] = action.icon();

        list << item;
    }

    return list;
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
        ++resultIt;

        const QUrl url(resource);

        if (!url.isValid()) {
            continue;
        }

        const KFileItem fileItem(url);

        if (!fileItem.isFile()) {
            continue;
        }

        if (list.isEmpty()) {
            list << createTitleActionItem(i18n("Recent Documents"));
        }

        QVariantMap item = createActionItem(url.fileName(), QStringLiteral("_kicker_recentDocument"), resource);
        item[QStringLiteral("icon")] = fileItem.iconName();

        list << item;
    }

    if (!list.isEmpty()) {
        list << createActionItem(i18n("Forget Recent Documents"), QStringLiteral("_kicker_forgetRecentDocuments"));
    }

    return list;
}

bool handleRecentDocumentAction(KService::Ptr service, const QString &actionId, const QVariant &_argument)
{
    if (!service) {
        return false;
    }

    if (actionId == QLatin1String("_kicker_forgetRecentDocuments")) {
        const QString storageId = storageIdFromService(service);

        if (storageId.isEmpty()) {
            return false;
        }

        auto query = UsedResources
            | Agent(storageId)
            | Type::any()
            | Activity::current()
            | Url::file();

        KAStats::forgetResources(query);

        return false;
    }

    QString argument = _argument.toString();

    if (argument.isEmpty()) {
        return false;
    }

    return (KRun::runService(*service, QList<QUrl>() << QUrl(argument), QApplication::activeWindow()) != 0);
}

Q_GLOBAL_STATIC(MenuEntryEditor, menuEntryEditor)

bool canEditApplication(const KService::Ptr &service)
{
    return (service->isApplication() && menuEntryEditor->canEdit(service->entryPath()));
}

void editApplication(const QString &entryPath, const QString &menuId)
{
    menuEntryEditor->edit(entryPath, menuId);
}

QVariantList editApplicationAction(const KService::Ptr &service)
{
    QVariantList actionList;

    if (canEditApplication(service)) {
        QVariantMap editAction = Kicker::createActionItem(i18n("Edit Application..."), QStringLiteral("editApplication"));
        editAction[QStringLiteral("icon")] = QStringLiteral("kmenuedit"); // TODO: Using the KMenuEdit icon might be misleading.
        actionList << editAction;
    }

    return actionList;
}

bool handleEditApplicationAction(const QString &actionId, const KService::Ptr &service)
{

    if (service && actionId ==QLatin1String("editApplication") && canEditApplication(service)) {
        Kicker::editApplication(service->entryPath(), service->menuId());

        return true;
    }

    return false;
}

#ifdef HAVE_APPSTREAMQT
Q_GLOBAL_STATIC(AppStream::Pool, appstreamPool)
#endif

QVariantList appstreamActions(const KService::Ptr &service)
{
    QVariantList ret;

#ifdef HAVE_APPSTREAMQT
    const KService::Ptr appStreamHandler = KMimeTypeTrader::self()->preferredService(QStringLiteral("x-scheme-handler/appstream"));

    // Don't show action if we can't find any app to handle appstream:// URLs.
    if (!appStreamHandler) {
        if (!KProtocolInfo::isHelperProtocol(QStringLiteral("appstream"))
            || KProtocolInfo::exec(QStringLiteral("appstream")).isEmpty()) {
            return ret;
        }
    }

    if (!appstreamPool.exists()) {
        appstreamPool->load();
    }

    const auto components = appstreamPool->componentsById(service->desktopEntryName()+QLatin1String(".desktop"));
    for(const auto &component: components) {
        const QString componentId = component.id();

        QVariantMap appstreamAction = Kicker::createActionItem(i18nc("@action opens a software center with the application", "Manage '%1'...", component.name()), "manageApplication", QVariant(QStringLiteral("appstream://") + componentId));
        appstreamAction[QStringLiteral("icon")] = QStringLiteral("applications-other");
        ret << appstreamAction;
    }
#else
    Q_UNUSED(service)
#endif

    return ret;
}

bool handleAppstreamActions(const QString &actionId, const QVariant &argument)
{
    if (actionId == QLatin1String("manageApplication")) {
        return QDesktopServices::openUrl(QUrl(argument.toString()));
    }

    return false;
}

QString resolvedServiceEntryPath(const KService::Ptr &service)
{
    QString path = service->entryPath();
    if (!QDir::isAbsolutePath(path)) {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kservices5/") + path);
    }
    return path;
}

}
