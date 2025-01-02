/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kded.h"

#include <actions.h>

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>

using namespace Qt::Literals::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(TouchpadDisabler, "kded_touchpad.json")

void migrateConfig(TouchpadBackend *backend)
{
    if (!backend->isTouchpadAvailable()) {
        return;
    }

#if BUILD_KCM_TOUCHPAD_X11
    // on Wayland, KWin handles the on/off/toggle shortcuts, the kded is not used and X11 settings are not adopted
    if (backend->getMode() != TouchpadInputBackendMode::XLibinput) {
        return;
    }
#else
    return;
#endif

    KSharedConfig::Ptr oldConfig = KSharedConfig::openConfig(u"touchpadrc"_s);
    // TODO KF7 (or perhaps earlier): Remove Plasma 5->6 migration code and delete touchpadrc

    const KConfigGroup oldGroup = oldConfig->group(u"autodisable"_s);
    if (!oldGroup.exists()) {
        // avoid writing a new config file just with migration data
        return;
    }

    bool disableWhenMousePluggedIn = oldGroup.readEntry("DisableWhenMousePluggedIn", false);
    bool disableOnKeyboardActivity = oldGroup.readEntry("DisableOnKeyboardActivity", true);

    // touchpadxlibinputrc uses the device name in the config group - use backend API to write to the correct one
    backend->getConfig();

    for (LibinputCommon *device : backend->inputDevices()) { // only touchpads, because TouchpadBackend
        if (device->supportsDisableEventsOnExternalMouse()) {
            device->setDisableEventsOnExternalMouse(disableWhenMousePluggedIn);
        }
        // DisableWhileTyping has been configurable since early on, so keep the migration defensive
        // by only performing migrations from `true` (default) to `false` (manually changed in prior backend).
        // i.e. users who stuck with the default, or who changed it to `false` only for libinput, are unaffected
        if (!disableOnKeyboardActivity && device->supportsDisableWhileTyping()) {
            device->setDisableWhileTyping(false);
        }
    }

    backend->applyConfig();

    oldConfig->deleteGroup(u"autodisable"_s);
    oldConfig->sync();
}

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
    migrateConfig(m_backend);

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
