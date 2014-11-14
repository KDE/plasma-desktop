/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "itemhandlers.h"

// Qt
#include <QTimer>

// KDE
#include <KAuthorized>
#include <QDebug>
#include <KJob>
#include <KService>
#include <KToolInvocation>
#include <KUrl>
#include <Solid/PowerManagement>

// KDE Base
#include <kworkspace.h>

// Local
#include "recentapplications.h"

// DBus
#include "krunner_interface.h"
#include "screensaver_interface.h"
#include "ksmserver_interface.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

using namespace Kickoff;

bool ServiceItemHandler::openUrl(const KUrl& url)
{
    int result = KToolInvocation::startServiceByDesktopPath(url.pathOrUrl(), QStringList(), 0, 0, 0, "", true);

    if (result == 0) {
        KService::Ptr service = KService::serviceByDesktopPath(url.pathOrUrl());

        if (service) {
            RecentApplications::self()->add(service);
        } else {
            qWarning() << "Failed to find service for" << url;
            return false;
        }
    }

    return result == 0;
}
LeaveItemHandler::LeaveItemHandler(QObject *parent)
    : QObject (parent)
{
}

bool LeaveItemHandler::openUrl(const KUrl& url)
{
    m_logoutAction = url.path().remove('/');

    if (m_logoutAction == "sleep") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(suspendRAM()));
            return true;
    } else if (m_logoutAction == "hibernate") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(suspendDisk()));
            return true;
    } else if (m_logoutAction == "lock") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(lock()));
        return true;
    } else if (m_logoutAction == "switch") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(switchUser()));
        return true;
    } else if (m_logoutAction == "logout" || m_logoutAction == "logoutonly" ||
               m_logoutAction == "restart" || m_logoutAction == "shutdown") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(logout()));
        return true;
    } else if (m_logoutAction == "savesession") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(saveSession()));
        return true;
    } else if (m_logoutAction == "standby") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(standby()));
        return true;
    } else if (m_logoutAction == "suspendram") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(suspendRAM()));
        return true;
    } else if (m_logoutAction == "suspenddisk") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(suspendDisk()));
        return true;
    } else if (m_logoutAction == "run") {
        // decouple dbus call, otherwise we'll run into a dead-lock
        QTimer::singleShot(0, this, SLOT(runCommand()));
        return true;
    }

    return false;
}

void LeaveItemHandler::runCommand()
{
    if (KAuthorized::authorize("run_command")) {
        QString interface("org.kde.krunner");
        org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
        krunner.display();
    }
}

void LeaveItemHandler::logout()
{
    KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
    KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeNone;

    if (m_logoutAction == "logout" || m_logoutAction == "logoutonly") {
        type = KWorkSpace::ShutdownTypeNone;
    } else if (m_logoutAction == "lock") {
        qDebug() << "Locking screen";
    } else if (m_logoutAction == "switch") {
        qDebug() << "Switching user";
    } else if (m_logoutAction == "restart") {
        type = KWorkSpace::ShutdownTypeReboot;
    } else if (m_logoutAction == "shutdown") {
        type = KWorkSpace::ShutdownTypeHalt;
    }

    KWorkSpace::requestShutDown(confirm, type);
}

void LeaveItemHandler::lock()
{
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver", QDBusConnection::sessionBus());
    screensaver.Lock();
}

void LeaveItemHandler::switchUser()
{
    QString interface("org.kde.krunner");
    org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
    krunner.switchUser();
}

void LeaveItemHandler::saveSession()
{
    QString interface("org.kde.ksmserver");

    org::kde::KSMServerInterface ksmserver(interface, "/KSMServer", QDBusConnection::sessionBus());
    if (ksmserver.isValid()) {
        ksmserver.saveCurrentSession();
    }
}

void LeaveItemHandler::standby()
{
    Solid::PowerManagement::requestSleep(Solid::PowerManagement::StandbyState, 0, 0);
}

void LeaveItemHandler::suspendRAM()
{
    Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, 0, 0);
}

void LeaveItemHandler::suspendDisk()
{
    Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState, 0, 0);
}

#include "itemhandlers.moc"
