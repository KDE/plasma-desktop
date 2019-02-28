/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
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

#include "systementry.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>

#include <KAuthorized>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <kworkspace.h>
#include <Solid/PowerManagement>
#include "ksmserver_interface.h"
#include <kworkspace5/kdisplaymanager.h>

SystemEntry::SystemEntry(AbstractModel *owner, Action action) : AbstractEntry(owner)
, m_action(action)
, m_valid(false)
{
    init();
}

SystemEntry::SystemEntry(AbstractModel *owner, const QString &id) : AbstractEntry(owner)
, m_action(NoAction)
, m_valid(false)
{
    if (id == QLatin1String("lock-screen")) {
        m_action = LockSession;
    } else if (id == QLatin1String("logout")) {
        m_action = LogoutSession;
    } else if (id == QLatin1String("save-session")) {
        m_action = SaveSession;
    } else if (id == QLatin1String("switch-user")) {
        m_action = SwitchUser;
    } else if (id == QLatin1String("suspend")) {
        m_action = SuspendToRam;
    } else if (id == QLatin1String("hibernate")) {
        m_action = SuspendToDisk;
    } else if (id == QLatin1String("reboot")) {
        m_action = Reboot;
    } else if (id == QLatin1String("shutdown")) {
        m_action = Shutdown;
    }

    init();
}

void SystemEntry::init()
{
    switch (m_action) {
        case NoAction:
            m_valid = false;
            break;
        case LockSession:
            m_valid = KAuthorized::authorizeAction(QStringLiteral("lock_screen"));
            break;
        case LogoutSession:
        case SaveSession:
        {
            bool authorize = KAuthorized::authorizeAction(QStringLiteral("logout")) && KAuthorized::authorize(QStringLiteral("logout"));

            if (m_action == SaveSession) {
                const KConfigGroup c(KSharedConfig::openConfig(QStringLiteral("ksmserverrc"), KConfig::NoGlobals), "General");

                m_valid = authorize && c.readEntry("loginMode") == QLatin1String("restoreSavedSession");
            } else {
                m_valid = authorize;
            }

            break;
        }
        case SwitchUser:
            m_valid = (KAuthorized::authorizeAction(QStringLiteral("start_new_session")) || KAuthorized::authorizeAction(QStringLiteral("switch_user")))
                && KDisplayManager().isSwitchable();
            break;
        case SuspendToRam:
            m_valid = Solid::PowerManagement::supportedSleepStates().contains(Solid::PowerManagement::SuspendState);
            break;
        case SuspendToDisk:
            m_valid = Solid::PowerManagement::supportedSleepStates().contains(Solid::PowerManagement::HibernateState);
            break;
        case Reboot:
            m_valid = KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot);
            break;
        case Shutdown:
            m_valid = KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt);
            break;
        default:
            m_valid = true;
    }
}

bool SystemEntry::isValid() const
{
    return m_valid;
}

QIcon SystemEntry::icon() const
{
    const QString &name = iconName();

    if (!name.isEmpty()) {
        return QIcon::fromTheme(name, QIcon::fromTheme(QStringLiteral("unknown")));
    }

    return QIcon::fromTheme(QStringLiteral("unknown"));
}

QString SystemEntry::iconName() const
{
    switch (m_action) {
        case LockSession:
            return QStringLiteral("system-lock-screen");
            break;
        case LogoutSession:
            return QStringLiteral("system-log-out");
            break;
        case SaveSession:
            return QStringLiteral("system-save-session");
            break;
        case SwitchUser:
            return QStringLiteral("system-switch-user");
            break;
        case SuspendToRam:
            return QStringLiteral("system-suspend");
            break;
        case SuspendToDisk:
            return QStringLiteral("system-suspend-hibernate");
            break;
        case Reboot:
            return QStringLiteral("system-reboot");
            break;
        case Shutdown:
            return QStringLiteral("system-shutdown");
            break;
        default:
            break;
    }

    return QString();
}

QString SystemEntry::name() const
{
    switch (m_action) {
        case LockSession:
            return i18n("Lock");
            break;
        case LogoutSession:
            return i18n("Log Out");
            break;
        case SaveSession:
            return i18n("Save Session");
            break;
        case SwitchUser:
            return i18n("Switch User");
            break;
        case SuspendToRam:
            return i18nc("Suspend to RAM", "Sleep");
            break;
        case SuspendToDisk:
            return i18n("Hibernate");
            break;
        case Reboot:
            return i18n("Restart");
            break;
        case Shutdown:
            return i18n("Shut Down");
            break;
        default:
            break;
    }

    return QString();
}

QString SystemEntry::group() const
{
    switch (m_action) {
        case LockSession:
            return i18n("Session");
            break;
        case LogoutSession:
            return i18n("Session");
            break;
        case SaveSession:
            return i18n("Session");
            break;
        case SwitchUser:
            return i18n("Session");
            break;
        case SuspendToRam:
            return i18n("System");
            break;
        case SuspendToDisk:
            return i18n("System");
            break;
        case Reboot:
            return i18n("System");
            break;
        case Shutdown:
            return i18n("System");
            break;
        default:
            break;
    }

    return QString();
}

QString SystemEntry::description() const
{
    switch (m_action) {
        case LockSession:
            return i18n("Lock screen");
            break;
        case LogoutSession:
            return i18n("End session");
            break;
        case SaveSession:
            return i18n("Save Session");
            break;
        case SwitchUser:
            return i18n("Start a parallel session as a different user");
            break;
        case SuspendToRam:
            return i18n("Suspend to RAM");
            break;
        case SuspendToDisk:
            return i18n("Suspend to disk");
            break;
        case Reboot:
            return i18n("Restart computer");
            break;
        case Shutdown:
            return i18n("Turn off computer");
            break;
        default:
            break;
    }

    return QString();
}

QString SystemEntry::id() const
{
    switch (m_action) {
        case LockSession:
            return QStringLiteral("lock-screen");
            break;
        case LogoutSession:
            return QStringLiteral("logout");
            break;
        case SaveSession:
            return QStringLiteral("save-session");
            break;
        case SwitchUser:
            return QStringLiteral("switch-user");
            break;
        case SuspendToRam:
            return QStringLiteral("suspend");
            break;
        case SuspendToDisk:
            return QStringLiteral("hibernate");
            break;
        case Reboot:
            return QStringLiteral("reboot");
            break;
        case Shutdown:
            return QStringLiteral("shutdown");
            break;

        default:
            break;
    }

    return QString();
}

bool SystemEntry::run(const QString& actionId, const QVariant &argument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(argument)

    switch (m_action) {
        case LockSession:
        {
            QDBusConnection bus = QDBusConnection::sessionBus();
            QDBusInterface interface(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"), QStringLiteral("org.freedesktop.ScreenSaver"), bus);
            interface.asyncCall(QStringLiteral("Lock"));
            break;
        }
        case LogoutSession:
            KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeNone);
            break;
        case SaveSession:
        {
            org::kde::KSMServerInterface ksmserver(QStringLiteral("org.kde.ksmserver"),
                QStringLiteral("/KSMServer"), QDBusConnection::sessionBus());

            if (ksmserver.isValid()) {
                ksmserver.saveCurrentSession();
            }

            break;
        }
        case SwitchUser:
        {
            QDBusConnection bus = QDBusConnection::sessionBus();
            QDBusInterface interface(QStringLiteral("org.kde.ksmserver"), QStringLiteral("/KSMServer"), QStringLiteral("org.kde.KSMServerInterface"), bus);
            interface.asyncCall(QStringLiteral("openSwitchUserDialog"));
            break;
        };
        case SuspendToRam:
            Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, nullptr, nullptr);
            break;
        case SuspendToDisk:
            Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState, nullptr, nullptr);
            break;
        case Reboot:
            KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot);
            break;
        case Shutdown:
            KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt);
            break;
        default:
            return false;
    }

    return true;
}
