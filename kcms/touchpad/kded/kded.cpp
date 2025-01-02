/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kded.h"

#include <KPluginFactory>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>

#include "actions.h"

K_PLUGIN_CLASS_WITH_JSON(TouchpadDisabler, "kded_touchpad.json")

bool TouchpadDisabler::workingTouchpadFound() const
{
    return m_workingTouchpadFound;
}

void TouchpadDisabler::serviceRegistered(const QString &service)
{
    if (!m_dependencies.removeWatchedService(service)) {
        return;
    }

    if (m_dependencies.watchedServices().isEmpty()) {
        lateInit();
    }
}

TouchpadDisabler::TouchpadDisabler(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
    , m_backend(TouchpadBackend::implementation())
    , m_userRequestedState(true)
    , m_touchpadEnabled(true)
    , m_workingTouchpadFound(false)
{
    if (!m_backend) {
        return;
    }

    m_dependencies.addWatchedService("org.kde.plasmashell");
    m_dependencies.addWatchedService("org.kde.kglobalaccel");
    connect(&m_dependencies, SIGNAL(serviceRegistered(QString)), SLOT(serviceRegistered(QString)));

    connect(m_backend, SIGNAL(touchpadStateChanged()), SLOT(updateCurrentState()));
    connect(m_backend, SIGNAL(touchpadReset()), SLOT(handleReset()));

    updateCurrentState();
    m_userRequestedState = m_touchpadEnabled;

    m_dependencies.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    m_dependencies.setConnection(QDBusConnection::sessionBus());
    QDBusPendingCall async = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("ListNames"));
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, SLOT(serviceNameFetchFinished(QDBusPendingCallWatcher *)));

    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(onPrepareForSleep(bool)));
}

void TouchpadDisabler::serviceNameFetchFinished(QDBusPendingCallWatcher *callWatcher)
{
    QDBusPendingReply<QStringList> reply = *callWatcher;
    callWatcher->deleteLater();

    if (reply.isError() || reply.value().isEmpty()) {
        qWarning() << "Error: Couldn't get registered services list from session bus";
        return;
    }

    const QStringList allServices = reply.value();
    const QStringList watchedList = m_dependencies.watchedServices();
    for (const QString &service : watchedList) {
        if (allServices.contains(service)) {
            serviceRegistered(service);
        }
    }
}

bool TouchpadDisabler::isEnabled() const
{
    return m_touchpadEnabled;
}

void TouchpadDisabler::updateCurrentState()
{
    updateWorkingTouchpadFound();
    if (!m_backend->isTouchpadAvailable()) {
        return;
    }
    bool newEnabled = m_backend->isTouchpadEnabled();
    if (newEnabled != m_touchpadEnabled) {
        m_touchpadEnabled = newEnabled;
        Q_EMIT enabledChanged(m_touchpadEnabled);
    }
}

void TouchpadDisabler::toggle()
{
    m_userRequestedState = !m_touchpadEnabled;
    m_backend->setTouchpadEnabled(m_userRequestedState);
}

void TouchpadDisabler::disable()
{
    m_userRequestedState = false;
    m_backend->setTouchpadEnabled(false);
}

void TouchpadDisabler::enable()
{
    m_userRequestedState = true;
    m_backend->setTouchpadEnabled(true);
}

void TouchpadDisabler::lateInit()
{
    TouchpadGlobalActions *actions = new TouchpadGlobalActions(false, this);
    connect(actions, &TouchpadGlobalActions::enableTriggered, this, [this] {
        enable();
        showOsd();
    });
    connect(actions, &TouchpadGlobalActions::disableTriggered, this, [this] {
        disable();
        showOsd();
    });
    connect(actions, &TouchpadGlobalActions::toggleTriggered, this, [this] {
        toggle();
        showOsd();
    });

    updateCurrentState();
}

void TouchpadDisabler::handleReset()
{
    updateCurrentState();
    if (!m_workingTouchpadFound) {
        return;
    }
    touchpadApplySavedConfig();
    m_backend->setTouchpadEnabled(m_userRequestedState);
}

void TouchpadDisabler::updateWorkingTouchpadFound()
{
    bool newWorkingTouchpadFound = m_backend && m_backend->isTouchpadAvailable();
    if (newWorkingTouchpadFound != m_workingTouchpadFound) {
        m_workingTouchpadFound = newWorkingTouchpadFound;
        Q_EMIT workingTouchpadFoundChanged(m_workingTouchpadFound);
    }
}

void TouchpadDisabler::onPrepareForSleep(bool sleep)
{
    m_preparingForSleep = sleep;
}

void TouchpadDisabler::showOsd()
{
    if (m_preparingForSleep) {
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                      QStringLiteral("/org/kde/osdService"),
                                                      QStringLiteral("org.kde.osdService"),
                                                      QStringLiteral("touchpadEnabledChanged"));

    msg.setArguments({m_backend->isTouchpadEnabled()});

    QDBusConnection::sessionBus().asyncCall(msg);
}

#include "kded.moc"

#include "moc_kded.cpp"
