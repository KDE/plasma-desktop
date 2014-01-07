/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "touchpadengine.h"

#include <QDBusConnection>
#include <QDBusInterface>

#include "touchpadinterface.h"
#include "touchpadservice.h"
#include "kdedinterface.h"

TouchpadEngine::TouchpadEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args), m_source("touchpad"), m_daemon(0)
{
}

void TouchpadEngine::init()
{
    OrgKdeKdedInterface kded(QLatin1String("org.kde.kded"),
                             QLatin1String("/kded"),
                             QDBusConnection::sessionBus());
    kded.loadModule("touchpad").waitForFinished();

    m_daemon = new OrgKdeTouchpadInterface("org.kde.kded", "/modules/touchpad",
                                           QDBusConnection::sessionBus(), this);
    if (!m_daemon->isValid()) {
        return;
    }

    QDBusPendingReply<bool> check(m_daemon->workingTouchpadFound());
    check.waitForFinished();
    if (!check.isValid() || !check.value()) {
        return;
    }

    connect(m_daemon, SIGNAL(enabledChanged(bool)), SLOT(enabledChanged(bool)));
    connect(m_daemon, SIGNAL(mousePluggedInChanged(bool)),
            SLOT(mousePluggedInChanged(bool)));

    enabledChanged(m_daemon->isEnabled());
    mousePluggedInChanged(m_daemon->isMousePluggedIn());
}

void TouchpadEngine::mousePluggedInChanged(bool value)
{
    setData(m_source, "mousePluggedIn", value);
}

void TouchpadEngine::enabledChanged(bool value)
{
    setData(m_source, "enabled", value);
}

Plasma::Service *TouchpadEngine::serviceForSource(const QString &source)
{
    if (source == m_source) {
        return new TouchpadService(m_daemon, source, this);
    }

    return Plasma::DataEngine::serviceForSource(source);
}

TouchpadEngine::~TouchpadEngine()
{
}

K_EXPORT_PLASMA_DATAENGINE(touchpad, TouchpadEngine)
