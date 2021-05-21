/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kded.h"

#include <KPluginFactory>
#include <KLocalizedString>
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
    , m_keyboardActivity(false)
    , m_mouse(false)
{
    if (!m_backend) {
        return;
    }

    m_dependencies.addWatchedService("org.kde.plasmashell");
    m_dependencies.addWatchedService("org.kde.kglobalaccel");
    connect(&m_dependencies, SIGNAL(serviceRegistered(QString)), SLOT(serviceRegistered(QString)));

    connect(m_backend, SIGNAL(mousesChanged()), SLOT(mousePlugged()));
    connect(m_backend, SIGNAL(keyboardActivityStarted()), SLOT(keyboardActivityStarted()));
    connect(m_backend, SIGNAL(keyboardActivityFinished()), SLOT(keyboardActivityFinished()));
    connect(m_backend, SIGNAL(touchpadStateChanged()), SLOT(updateCurrentState()));

    connect(m_backend, SIGNAL(touchpadReset()), SLOT(handleReset()));

    m_keyboardActivityTimeout.setSingleShot(true);
    connect(&m_keyboardActivityTimeout, SIGNAL(timeout()), SLOT(timerElapsed()));

    updateCurrentState();
    m_userRequestedState = m_touchpadEnabled;
    reloadSettings();

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

void TouchpadDisabler::reloadSettings()
{
    m_settings.load();
    m_keyboardActivityTimeout.setInterval(m_settings.keyboardActivityTimeoutMs());

    m_keyboardDisableState =
        m_settings.onlyDisableTapAndScrollOnKeyboardActivity() ? TouchpadBackend::TouchpadTapAndScrollDisabled : TouchpadBackend::TouchpadFullyDisabled;

    mousePlugged();

    m_backend->watchForEvents(m_settings.disableOnKeyboardActivity());
}

void TouchpadDisabler::keyboardActivityStarted()
{
    if (m_keyboardActivity || !m_settings.disableOnKeyboardActivity()) {
        return;
    }

    m_keyboardActivityTimeout.stop();
    m_keyboardActivity = true;
    m_backend->setTouchpadOff(m_keyboardDisableState);
}

void TouchpadDisabler::keyboardActivityFinished()
{
    if (!m_keyboardActivity) {
        keyboardActivityStarted();
    }
    m_keyboardActivityTimeout.start();
}

void TouchpadDisabler::timerElapsed()
{
    if (!m_keyboardActivity) {
        return;
    }

    m_keyboardActivity = false;
    m_backend->setTouchpadOff(TouchpadBackend::TouchpadEnabled);
}

void TouchpadDisabler::mousePlugged()
{
    if (!m_dependencies.watchedServices().isEmpty()) {
        return;
    }

    bool pluggedIn = isMousePluggedIn();
    Q_EMIT mousePluggedInChanged(pluggedIn);

    bool disable = pluggedIn && m_settings.disableWhenMousePluggedIn();
    if (m_mouse == disable) {
        return;
    }
    m_mouse = disable;

    bool newState = disable ? false : m_userRequestedState;
    if (newState == m_touchpadEnabled) {
        return;
    }

    // If the disable is caused by plugin mouse, show the message, otherwise it might
    // be user already disables touchpad themselves.
    if (!newState && disable) {
        showNotification("TouchpadDisabled", i18n("Touchpad was disabled because a mouse was plugged in"));
    }
    if (newState) {
        showNotification("TouchpadEnabled", i18n("Touchpad was enabled because the mouse was unplugged"));
    }

    m_backend->setTouchpadEnabled(newState);
}

void TouchpadDisabler::showNotification(const QString &name, const QString &text)
{
    if (m_notification) {
        m_notification->close();
    }

    m_notification = KNotification::event(name,
                                          text,
                                          QPixmap(), // Icon is specified in .notifyrc
                                          nullptr,
                                          KNotification::CloseOnTimeout,
                                          "kcm_touchpad"); // this has to match the name of the .notifyrc file
    // TouchpadPluginFactory::componentData());
}

bool TouchpadDisabler::isMousePluggedIn() const
{
    return !m_backend->listMouses(m_settings.mouseBlacklist()).isEmpty();
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
    mousePlugged();
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
