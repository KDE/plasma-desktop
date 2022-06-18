/*
    SPDX-FileCopyrightText: 2008 Petri Damsten <damu@iki.fi>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "showdesktop.h"

#include <KWindowSystem>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>

ShowDesktop::ShowDesktop(QObject *parent)
    : QObject(parent)
{
    connect(KWindowSystem::self(), &KWindowSystem::showingDesktopChanged, this, &ShowDesktop::showingDesktopChanged);
}

ShowDesktop::~ShowDesktop() = default;

bool ShowDesktop::showingDesktop() const
{
    return KWindowSystem::showingDesktop();
}

void ShowDesktop::setShowingDesktop(bool showingDesktop)
{
    KWindowSystem::setShowingDesktop(showingDesktop);
    Q_EMIT showingDesktopChanged(showingDesktop);
}

void ShowDesktop::minimizeAll()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.interface()->isServiceRegistered(QStringLiteral("org.kde.KWin.Script.MinimizeAll1"))) {
        sessionBus.asyncCall(QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin.Script.MinimizeAll1"),
                                                            QStringLiteral("/org/kde/KWin/Script/MinimizeAll1"),
                                                            QStringLiteral("org.kde.KWin.Script.MinimizeAll1"),
                                                            QStringLiteral("toggle")));
    } else {
        // KWin is not running or the minimize all script is disabled, fallback to minimizing each window.
        const auto &windows = KWindowSystem::windows();
        for (WId wid : windows) {
            KWindowSystem::minimizeWindow(wid);
        }
    }
}

void ShowDesktop::toggleDesktop()
{
    const bool showingNow = KWindowSystem::showingDesktop();
    KWindowSystem::setShowingDesktop(!showingNow);
    Q_EMIT showingDesktopChanged(!showingNow);
}
