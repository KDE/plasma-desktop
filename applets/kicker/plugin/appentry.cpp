/***************************************************************************
 *   Copyright (C) 201 by Eike Hein <hein@kde.org>                         *
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

#include "appentry.h"
#include "actionlist.h"
#include "appsmodel.h"
#include "containmentinterface.h"
#include "menuentryeditor.h"

#ifdef PackageKitQt5_FOUND
#include "findpackagenamejob.h"
#endif

#include <config-X11.h>

#include <QProcess>
#include <QQmlPropertyMap>
#include <QStandardPaths>
#if HAVE_X11
#include <QX11Info>
#endif

#include <KActivities/ResourceInstance>
#include <KConfigGroup>
#include <KJob>
#include <KLocalizedString>
#include <KMimeTypeTrader>
#include <KRun>
#include <KSycoca>
#include <KShell>
#include <KSharedConfig>
#include <KStartupInfo>

QObject *AppEntry::m_appletInterface = nullptr;
QQmlPropertyMap *AppEntry::m_appletConfig = nullptr;
MenuEntryEditor *AppEntry::m_menuEntryEditor = nullptr;

AppEntry::AppEntry(AbstractModel *owner, KService::Ptr service, NameFormat nameFormat)
: AbstractEntry(owner)
, m_service(service)
{
    if (m_service) {
        init(nameFormat);
    }
}

AppEntry::AppEntry(AbstractModel *owner, const QString &id) : AbstractEntry(owner)
{
    const QUrl url(id);

    if (url.scheme() == QStringLiteral("preferred")) {
        m_service = defaultAppByName(url.host());
        m_id = id;
    } else {
        m_service = KService::serviceByStorageId(id);
    }

    if (m_service) {
        init((NameFormat)qobject_cast<AppsModel* >(owner->rootModel())->appNameFormat());
    }
}

void AppEntry::init(NameFormat nameFormat)
{
    m_name = nameFromService(m_service, nameFormat);
    m_icon = QIcon::fromTheme(m_service->icon(), QIcon::fromTheme("unknown"));

    if (!m_appletInterface) {
        AbstractModel *rootModel = m_owner->rootModel();
        m_appletInterface = rootModel->property("appletInterface").value<QObject *>();
        m_appletConfig = qobject_cast<QQmlPropertyMap *>(m_appletInterface->property("configuration").value<QObject *>());
    }

    if (!m_menuEntryEditor) {
        m_menuEntryEditor = new MenuEntryEditor();
    }
}

bool AppEntry::isValid() const
{
    return m_service;
}

QIcon AppEntry::icon() const
{
    return m_icon;
}

QString AppEntry::name() const
{
    return m_name;
}

KService::Ptr AppEntry::service() const
{
    return m_service;
}

QString AppEntry::id() const
{
    if (!m_id.isEmpty()) {
        return m_id;
    }

    return m_service->storageId();
}

QUrl AppEntry::url() const
{
    return QUrl::fromLocalFile(m_service->entryPath());
}

bool AppEntry::hasActions() const
{
    return true;
}

QVariantList AppEntry::actions() const
{
    QVariantList actionList;

    actionList << Kicker::recentDocumentActions(m_service);

    if (actionList.count()) {
        actionList << Kicker::createSeparatorActionItem();
    }

    if (ContainmentInterface::mayAddLauncher(m_appletInterface, ContainmentInterface::Desktop)) {
        actionList << Kicker::createActionItem(i18n("Add to Desktop"), "addToDesktop");
    }

    if (ContainmentInterface::mayAddLauncher(m_appletInterface, ContainmentInterface::Panel)) {
        actionList << Kicker::createActionItem(i18n("Add to Panel"), "addToPanel");
    }

    if (ContainmentInterface::mayAddLauncher(m_appletInterface, ContainmentInterface::TaskManager, m_service->entryPath())) {
        actionList << Kicker::createActionItem(i18n("Add as Launcher"), "addToTaskManager");
    }

    if (m_menuEntryEditor->canEdit(m_service->entryPath())) {
        actionList << Kicker::createSeparatorActionItem();

        QVariantMap editAction = Kicker::createActionItem(i18n("Edit Application..."), "editApplication");
        editAction["icon"] = "kmenuedit"; // TODO: Using the KMenuEdit icon might be misleading.
        actionList << editAction;
    }

#ifdef PackageKitQt5_FOUND
    QStringList files(m_service->entryPath());

    if (m_service->isApplication()) {
        files += QStandardPaths::findExecutable(KShell::splitArgs(m_service->exec()).first());
    }

    FindPackageJob* job = new FindPackageJob(files); // TODO: Would be great to make this async.

    if (job->exec() && !job->packageNames().isEmpty()) {
        QString packageName = job->packageNames().first();

        QVariantMap removeAction = Kicker::createActionItem(i18n("Remove '%1'...", packageName), "removeApplication", packageName);
        removeAction["icon"] = "applications-other";
        actionList << removeAction;
    }
#endif

    if (m_appletConfig && m_appletConfig->contains("hiddenApplications") && qobject_cast<AppsModel *>(m_owner)) {
        const QStringList &hiddenApps = m_appletConfig->value("hiddenApplications").toStringList();

        if (!hiddenApps.contains(m_service->menuId())) {
            actionList << Kicker::createActionItem(i18n("Hide Application"), "hideApplication");
        }
    }

    return actionList;
}

bool AppEntry::run(const QString& actionId, const QVariant &argument)
{
    if (actionId.isEmpty()) {
        quint32 timeStamp = 0;

#if HAVE_X11
        if (QX11Info::isPlatformX11()) {
            timeStamp = QX11Info::appUserTime();
        }
#endif

        new KRun(QUrl::fromLocalFile(m_service->entryPath()), 0, true,
            KStartupInfo::createNewStartupIdForTimestamp(timeStamp));

        KActivities::ResourceInstance::notifyAccessed(QUrl("applications:" + m_service->storageId()),
            "org.kde.plasma.kicker");

        return true;
    } else if (actionId == "addToDesktop" && ContainmentInterface::mayAddLauncher(m_appletInterface, ContainmentInterface::Desktop)) {
        ContainmentInterface::addLauncher(m_appletInterface, ContainmentInterface::Desktop, m_service->entryPath());
    } else if (actionId == "addToPanel" && ContainmentInterface::mayAddLauncher(m_appletInterface, ContainmentInterface::Panel)) {
        ContainmentInterface::addLauncher(m_appletInterface, ContainmentInterface::Panel, m_service->entryPath());
    } else if (actionId == "addToTaskManager" && ContainmentInterface::mayAddLauncher(m_appletInterface, ContainmentInterface::TaskManager, m_service->entryPath())) {
        ContainmentInterface::addLauncher(m_appletInterface, ContainmentInterface::TaskManager, m_service->entryPath());
    } else if (actionId == "editApplication" && m_menuEntryEditor->canEdit(m_service->entryPath())) {
        QMetaObject::invokeMethod(m_menuEntryEditor, "edit", Qt::QueuedConnection,
            Q_ARG(QString, m_service->entryPath()),
            Q_ARG(QString, m_service->menuId()));

        return true;
    } else if (actionId == "removeApplication") {
        if (m_appletConfig && m_appletConfig->contains("removeApplicationCommand")) {
            const QStringList &removeAppCmd = KShell::splitArgs(m_appletConfig->value("removeApplicationCommand").toString());

            if (!removeAppCmd.isEmpty()) {
                return QProcess::startDetached(removeAppCmd.first(), removeAppCmd.mid(1) << argument.toString());
            }
        }
    }

    return Kicker::handleRecentDocumentAction(m_service, actionId, argument);
}

QString AppEntry::nameFromService(const KService::Ptr service, NameFormat nameFormat)
{
    const QString &name = service->name();
    QString genericName = service->genericName();

    if (genericName.isEmpty()) {
        genericName = service->comment();
    }

    if (nameFormat == NameOnly || genericName.isEmpty() || name == genericName) {
        return name;
    } else if (nameFormat == GenericNameOnly) {
        return genericName;
    } else if (nameFormat == NameAndGenericName) {
        return i18nc("App name (Generic name)", "%1 (%2)", name, genericName);
    } else {
        return i18nc("Generic name (App name)", "%1 (%2)", genericName, name);
    }
}

KService::Ptr AppEntry::defaultAppByName(const QString& name)
{
    if (name == QLatin1String("browser")) {
        KConfigGroup config(KSharedConfig::openConfig(), "General");
        QString browser = config.readPathEntry("BrowserApplication", QString());

        if (browser.isEmpty()) {
            return KMimeTypeTrader::self()->preferredService(QLatin1String("text/html"));
        } else if (browser.startsWith('!')) {
            browser = browser.mid(1);
        }

        return KService::serviceByStorageId(browser);
    }

    return KService::Ptr();
}

AppGroupEntry::AppGroupEntry(AppsModel *parentModel, KServiceGroup::Ptr group,
    bool flat, bool separators, int appNameFormat) : AbstractGroupEntry(parentModel)
{
    m_name = group->caption();
    m_icon = QIcon::fromTheme(group->icon(), QIcon::fromTheme("unknown"));
    AppsModel* model = new AppsModel(group->entryPath(), flat, separators, parentModel);
    model->setAppNameFormat(appNameFormat);
    m_childModel = model;

    QObject::connect(parentModel, &AppsModel::cleared, model, &AppsModel::deleteLater);

    QObject::connect(model, &AppsModel::countChanged,
        [parentModel, this] { if (parentModel) { parentModel->entryChanged(this); } }
    );

    QObject::connect(model, &AppsModel::hiddenEntriesChanged,
        [parentModel, this] { if (parentModel) { parentModel->entryChanged(this); } }
    );
}

QIcon AppGroupEntry::icon() const
{
    return m_icon;
}

QString AppGroupEntry::name() const
{
    return m_name;
}

bool AppGroupEntry::hasChildren() const
{
    return m_childModel && m_childModel->count() > 0;
}

AbstractModel *AppGroupEntry::childModel() const {
    return m_childModel;
}
