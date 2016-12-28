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

#include <config-workspace.h>
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
#include <QDesktopServices>
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

#include <Plasma/Plasma>
#include <AppStreamQt/pool.h>

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
        init((NameFormat)owner->rootModel()->property("appNameFormat").toInt());
    }
}

void AppEntry::init(NameFormat nameFormat)
{
    m_name = nameFromService(m_service, nameFormat);

    if (nameFormat == GenericNameOnly) {
        m_description = nameFromService(m_service, NameOnly);
    } else {
        m_description = nameFromService(m_service, GenericNameOnly);
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
    if (m_icon.isNull()) {
        m_icon = QIcon::fromTheme(m_service->icon(), QIcon::fromTheme("unknown"));
    }
    return m_icon;
}

QString AppEntry::name() const
{
    return m_name;
}

QString AppEntry::description() const
{
    return m_description;
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

QVariantList appstreamActions(const KService::Ptr &service)
{
    static AppStream::Pool pool;
    if (!pool.load())
        return {};

    QVariantList ret;
    const auto components = pool.componentsById(service->desktopEntryName()+QStringLiteral(".desktop"));
    for(const auto &component: components) {
        const QString componentId = component.id();

        QVariantMap appstreamAction = Kicker::createActionItem(i18nc("@action opens a software center with the application", "Manage '%1'...", component.name()), "manageApplication", QVariant(QStringLiteral("appstream://") + componentId));
        appstreamAction["icon"] = "applications-other";
        ret << appstreamAction;
    }
    return ret;
}

QVariantList AppEntry::actions() const
{
    QVariantList actionList;

    actionList << Kicker::jumpListActions(m_service);
    if (!actionList.isEmpty()) {
        actionList << Kicker::createSeparatorActionItem();
    }

    QObject *appletInterface = m_owner->rootModel()->property("appletInterface").value<QObject *>();

    const bool systemImmutable = appletInterface->property("immutability").toInt() == Plasma::Types::SystemImmutable;

    const QVariantList &addLauncherActions = Kicker::createAddLauncherActionList(appletInterface, m_service);
    if (!systemImmutable && !addLauncherActions.isEmpty()) {
        actionList << addLauncherActions
                   << Kicker::createSeparatorActionItem();
    }

    const QVariantList &recentDocuments = Kicker::recentDocumentActions(m_service);
    if (!recentDocuments.isEmpty()) {
        actionList << recentDocuments << Kicker::createSeparatorActionItem();
    }

    // Don't allow adding launchers, editing, hiding, or uninstalling applications
    // when system is immutable.
    if (systemImmutable) {
        return actionList;
    }

    if (m_menuEntryEditor->canEdit(m_service->entryPath())) {
        actionList << Kicker::createSeparatorActionItem();

        QVariantMap editAction = Kicker::createActionItem(i18n("Edit Application..."), "editApplication");
        editAction["icon"] = "kmenuedit"; // TODO: Using the KMenuEdit icon might be misleading.
        actionList << editAction;
    }

    if (m_service->isApplication())
        actionList << appstreamActions(m_service);

    QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

    if (appletConfig && appletConfig->contains("hiddenApplications") && qobject_cast<AppsModel *>(m_owner)) {
        const QStringList &hiddenApps = appletConfig->value("hiddenApplications").toStringList();

        if (!hiddenApps.contains(m_service->menuId())) {
            actionList << Kicker::createActionItem(i18n("Hide Application"), "hideApplication");
        }
    }

    return actionList;
}

bool AppEntry::run(const QString& actionId, const QVariant &argument)
{
    if (!m_service->isValid()) {
        return false;
    }

    if (actionId.isEmpty()) {
        quint32 timeStamp = 0;

#if HAVE_X11
        if (QX11Info::isPlatformX11()) {
            timeStamp = QX11Info::appUserTime();
        }
#endif

        // TODO Once we depend on KDE Frameworks 5.24 and D1902 is merged, use KRun::runApplication instead
        KRun::runService(*m_service, {}, nullptr, true, {}, KStartupInfo::createNewStartupIdForTimestamp(timeStamp));

        KActivities::ResourceInstance::notifyAccessed(QUrl("applications:" + m_service->storageId()),
            "org.kde.plasma.kicker");

        return true;
    }

    QObject *appletInterface = m_owner->rootModel()->property("appletInterface").value<QObject *>();

    if (Kicker::handleAddLauncherAction(actionId, appletInterface, m_service)) {
        return true;
    } else if (actionId == "editApplication" && m_menuEntryEditor->canEdit(m_service->entryPath())) {
        m_menuEntryEditor->edit(m_service->entryPath(), m_service->menuId());

        return true;
    } else if (actionId == "manageApplication") {
        return QDesktopServices::openUrl(QUrl(argument.toString()));
    } else if (actionId == "_kicker_jumpListAction") {
        return KRun::run(argument.toString(), {}, nullptr, m_service->name(), m_service->icon());
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
    bool paginate, int pageSize, bool flat, bool sorted, bool separators, int appNameFormat) : AbstractGroupEntry(parentModel),
    m_group(group)
{
    AppsModel* model = new AppsModel(group->entryPath(), paginate, pageSize, flat,
        sorted, separators, parentModel);
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
    if (m_icon.isNull()) {
        m_icon = QIcon::fromTheme(m_group->icon(), QIcon::fromTheme("unknown"));
    }
    return m_icon;
}

QString AppGroupEntry::name() const
{
    return m_group->caption();
}

bool AppGroupEntry::hasChildren() const
{
    return m_childModel && m_childModel->count() > 0;
}

AbstractModel *AppGroupEntry::childModel() const {
    return m_childModel;
}
