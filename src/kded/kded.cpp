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

#include "kded.h"

#include <KNotification>
#include <KLocale>

#include "plugins.h"
#include "kdedactions.h"

bool TouchpadDisabler::workingTouchpadFound() const
{
    return m_backend && !(m_backend->supportedParameters().isEmpty());
}

void TouchpadDisabler::serviceRegistered(const QString &service)
{
    if (!m_dependecies.removeWatchedService(service)) {
        return;
    }

    if (m_dependecies.watchedServices().isEmpty()) {
        lateInit();
    }
}

TouchpadDisabler::TouchpadDisabler(QObject *parent, const QVariantList &)
    : KDEDModule(parent), m_backend(TouchpadBackend::implementation()),
      m_enabled(true), m_keyboardActivity(false), m_mouse(false)
{
    if (!workingTouchpadFound()) {
        return;
    }

    m_dependecies.addWatchedService("org.kde.plasma-desktop");
    m_dependecies.addWatchedService("org.kde.kglobalaccel");
    connect(&m_dependecies, SIGNAL(serviceRegistered(QString)),
            SLOT(serviceRegistered(QString)));

    connect(m_backend, SIGNAL(mousesChanged()), SLOT(mousePlugged()));
    connect(m_backend, SIGNAL(keyboardActivityStarted()),
            SLOT(keyboardActivityStarted()));
    connect(m_backend, SIGNAL(keyboardActivityFinished()),
            SLOT(keyboardActivityFinished()));
    connect(m_backend, SIGNAL(touchpadStateChanged()),
            SLOT(updateCurrentState()));

    connect(m_backend, SIGNAL(touchpadReset()), SLOT(handleReset()));

    m_keyboardActivityTimeout.setSingleShot(true);
    connect(&m_keyboardActivityTimeout, SIGNAL(timeout()),
            SLOT(timerElapsed()));

    updateCurrentState();
    reloadSettings();

    m_dependecies.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    m_dependecies.setConnection(QDBusConnection::sessionBus());
    Q_FOREACH (const QString &service, m_dependecies.watchedServices()) {
        QDBusReply<bool> registered = QDBusConnection::sessionBus().interface()
                ->isServiceRegistered(service);
        if (!registered.isValid() || registered.value()) {
            serviceRegistered(service);
        }
    }
}

bool TouchpadDisabler::isEnabled() const
{
    return m_enabled;
}

void TouchpadDisabler::updateCurrentState()
{
    bool newEnabled = m_backend->isTouchpadEnabled();
    if (newEnabled != m_enabled) {
        m_enabled = newEnabled;
        Q_EMIT enabledChanged(m_enabled);
    }
}

void TouchpadDisabler::toggle()
{
    m_backend->setTouchpadEnabled(!isEnabled());
}

void TouchpadDisabler::disable()
{
    m_backend->setTouchpadEnabled(false);
}

void TouchpadDisabler::enable()
{
    m_backend->setTouchpadEnabled(true);
}

void TouchpadDisabler::reloadSettings()
{
    m_settings.readConfig();
    m_keyboardActivityTimeout.setInterval(
                m_settings.keyboardActivityTimeoutMs());

    m_keyboardDisableState =
            m_settings.onlyDisableTapAndScrollOnKeyboardActivity() ?
                TouchpadBackend::TouchpadTapAndScrollDisabled :
                TouchpadBackend::TouchpadFullyDisabled;

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
    if (!m_dependecies.watchedServices().isEmpty()) {
        return;
    }

    bool pluggedIn = isMousePluggedIn();
    Q_EMIT mousePluggedInChanged(pluggedIn);

    bool disable = pluggedIn && m_settings.disableWhenMousePluggedIn();
    if (m_mouse == disable) {
        return;
    }
    m_mouse = disable;

    if (m_enabled == !disable) {
        return;
    }

    if (disable) {
        showNotification("TouchpadDisabled",
                         i18n("Touchpad was disabled because a mouse was plugged in"));
    } else {
        showNotification("TouchpadEnabled",
                         i18n("Touchpad was enabled because the mouse was unplugged"));
    }

    m_backend->setTouchpadEnabled(!disable);
}

void TouchpadDisabler::showNotification(const QString &name, const QString &text)
{
    KNotification::event(name, text, QPixmap(), //Icon is specified in .notifyrc
                         0, KNotification::CloseOnTimeout,
                         TouchpadPluginFactory::componentData());
}

bool TouchpadDisabler::isMousePluggedIn() const
{
    return !m_backend->listMouses(m_settings.mouseBlacklist()).isEmpty();
}

void TouchpadDisabler::lateInit()
{
    TouchpadGlobalActions *actions = new TouchpadGlobalActions(this);
    connect(actions, SIGNAL(enableTriggered()), SLOT(enable()));
    connect(actions, SIGNAL(disableTriggered()), SLOT(disable()));
    connect(actions, SIGNAL(toggleTriggered()), SLOT(toggle()));

    updateCurrentState();
    mousePlugged();
}

void touchpadApplySavedConfig();

void TouchpadDisabler::handleReset()
{
    m_backend->setTouchpadEnabled(m_enabled);
    touchpadApplySavedConfig();
}
