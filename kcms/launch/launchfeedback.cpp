/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
 *  Copyright (C) 2017 Eike Hein <hein@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#include "launchfeedback.h"

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include "kwin_interface.h"

#define STARTUP_DEFAULT_TIMEOUT 5

K_PLUGIN_FACTORY_WITH_JSON(LaunchFactory, "kcm_launchfeedback.json", registerPlugin<LaunchFeedback>();)

LaunchFeedback::LaunchFeedback(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_busyCursorCurrentIndex(3)
    , m_taskManagerNotification(true)
    , m_notificationTimeout(STARTUP_DEFAULT_TIMEOUT)
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_launchfeedback"),
        i18n("Configure application launch feedback"),
        QStringLiteral("0.2"), QString(), KAboutLicense::LGPL);
    setAboutData(about);

    setButtons(Apply | Default);
}

LaunchFeedback::~LaunchFeedback()
{
}

int LaunchFeedback::busyCursorCurrentIndex() const
{
    return m_busyCursorCurrentIndex;
}

void LaunchFeedback::setBusyCursorCurrentIndex(int index)
{
    if (m_busyCursorCurrentIndex != index) {
        m_busyCursorCurrentIndex = index;

        emit busyCursorCurrentIndexChanged();

        updateNeedsSave();
    }
}

bool LaunchFeedback::taskManagerNotification() const
{
    return m_taskManagerNotification;
}

void LaunchFeedback::setTaskManagerNotification(bool enabled)
{
    if (m_taskManagerNotification != enabled) {
        m_taskManagerNotification = enabled;

        emit taskManagerNotificationChanged();

        updateNeedsSave();
    }
}

int LaunchFeedback::notificationTimeout() const
{
    return m_notificationTimeout;
}

void LaunchFeedback::setNotificationTimeout(int duration)
{
    if (m_notificationTimeout != duration) {
        m_notificationTimeout = duration;

        emit notificationTimeoutChanged();

        updateNeedsSave();
    }
}

void LaunchFeedback::load()
{
    KConfig conf(QStringLiteral("klaunchrc"), KConfig::NoGlobals);
    KConfigGroup c = conf.group("FeedbackStyle");

    const bool busyCursor = c.readEntry("BusyCursor", true);

    setTaskManagerNotification(c.readEntry("TaskbarButton", true));

    c = conf.group("BusyCursorSettings");

    setNotificationTimeout(c.readEntry("Timeout", STARTUP_DEFAULT_TIMEOUT));

    bool busyBlinking = c.readEntry("Blinking", false);
    bool busyBouncing = c.readEntry("Bouncing", true);

    if (!busyCursor) {
        setBusyCursorCurrentIndex(0);
    } else if (busyBlinking) {
        setBusyCursorCurrentIndex(2);
    } else if (busyBouncing) {
        setBusyCursorCurrentIndex(3);
    } else {
        setBusyCursorCurrentIndex(1);
    }
}

void LaunchFeedback::save()
{
    KConfig conf(QStringLiteral("klaunchrc"), KConfig::NoGlobals);
    KConfigGroup  c = conf.group("FeedbackStyle");

    c.writeEntry("BusyCursor", m_busyCursorCurrentIndex != 0);
    c.writeEntry("TaskbarButton", m_taskManagerNotification);

    c = conf.group("BusyCursorSettings");
    c.writeEntry("Timeout", m_notificationTimeout);
    c.writeEntry("Blinking", m_busyCursorCurrentIndex == 2);
    c.writeEntry("Bouncing", m_busyCursorCurrentIndex == 3);

    c = conf.group("TaskbarButtonSettings");
    c.writeEntry( "Timeout", m_notificationTimeout);

    c.sync();

    setNeedsSave(false);

    org::kde::kwin::Effects kwin(QStringLiteral("org.kde.KWin"),
        QStringLiteral("/Effects"),
        QDBusConnection::sessionBus());
    kwin.reconfigureEffect(QStringLiteral("startupfeedback"));
}

void LaunchFeedback::defaults()
{
    setBusyCursorCurrentIndex(3); // Bouncing cursor.

    setTaskManagerNotification(true);

    setNotificationTimeout(STARTUP_DEFAULT_TIMEOUT);
}

void LaunchFeedback::updateNeedsSave()
{
    bool needsSave = false;

    KConfig conf(QStringLiteral("klaunchrc"), KConfig::NoGlobals);
    KConfigGroup c = conf.group("FeedbackStyle");

    const bool savedBusyCursor = c.readEntry("BusyCursor", true);

    if (m_taskManagerNotification != c.readEntry("TaskbarButton", true)) {
        needsSave = true;
    }

    c = conf.group("BusyCursorSettings");

    if (m_notificationTimeout != c.readEntry("Timeout", STARTUP_DEFAULT_TIMEOUT)) {
        needsSave = true;
    }

    int savedBusyCursorIndex = 3; // Bouncing cursor (default);

    bool savedBusyBlinking = c.readEntry("Blinking", false);
    bool savedBusyBouncing = c.readEntry("Bouncing", true);

    if (!savedBusyCursor) {
        savedBusyCursorIndex = 0;
    } else if (savedBusyBlinking) {
        savedBusyCursorIndex = 2;
    } else if (savedBusyBouncing) {
        savedBusyCursorIndex = 3;
    } else {
        savedBusyCursorIndex = 1;
    }

    if (m_busyCursorCurrentIndex != savedBusyCursorIndex) {
        needsSave = true;
    }

    setNeedsSave(needsSave);
}

#include "launchfeedback.moc"
