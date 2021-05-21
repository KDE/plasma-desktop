/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "touchpadengine.h"

#include <QDBusConnection>
#include <QDBusInterface>

#include "kded5interface.h"
#include "touchpadinterface.h"
#include "touchpadservice.h"

TouchpadEngine::TouchpadEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
    , m_source("touchpad")
    , m_daemon(nullptr)
{
    init();
}

void TouchpadEngine::init()
{
    OrgKdeKded5Interface kded(QLatin1String("org.kde.kded5"), QLatin1String("/kded"), QDBusConnection::sessionBus());
    kded.loadModule("touchpad").waitForFinished();

    m_daemon = new OrgKdeTouchpadInterface("org.kde.kded5", "/modules/touchpad", QDBusConnection::sessionBus(), this);
    if (!m_daemon->isValid()) {
        return;
    }

    connect(m_daemon, SIGNAL(workingTouchpadFoundChanged(bool)), SLOT(workingTouchpadFoundChanged(bool)));
    connect(m_daemon, SIGNAL(enabledChanged(bool)), SLOT(enabledChanged(bool)));
    connect(m_daemon, SIGNAL(mousePluggedInChanged(bool)), SLOT(mousePluggedInChanged(bool)));

    workingTouchpadFoundChanged(m_daemon->workingTouchpadFound());
    enabledChanged(m_daemon->isEnabled());
    mousePluggedInChanged(m_daemon->isMousePluggedIn());
}

void TouchpadEngine::workingTouchpadFoundChanged(bool value)
{
    setData(m_source, "workingTouchpadFound", value);
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

K_PLUGIN_CLASS_WITH_JSON(TouchpadEngine, "plasma-dataengine-touchpad.json")

#include "touchpadengine.moc"
