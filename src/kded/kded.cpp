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
#include <klauncher_iface.h>

#include "plugins.h"

bool TouchpadDisabler::workingTouchpadFound() const
{
    return m_backend && !(m_backend->supportedParameters().isEmpty());
}

TouchpadDisabler::TouchpadDisabler(QObject *parent, const QVariantList &)
    : KDEDModule(parent), m_backend(TouchpadBackend::self()),
      m_currentState(TouchpadBackend::TouchpadEnabled),
      m_oldState(m_currentState), m_oldKbState(m_currentState),
      m_keyboardActivity(false), m_mouse(false), m_startup(true)
{
    if (!workingTouchpadFound()) {
        return;
    }

    connect(m_backend, SIGNAL(mousesChanged()), SLOT(mousePlugged()));
    connect(m_backend, SIGNAL(keyboardActivityStarted()),
            SLOT(keyboardActivityStarted()));
    connect(m_backend, SIGNAL(keyboardActivityFinished()),
            SLOT(keyboardActivityFinished()));
    connect(m_backend, SIGNAL(touchpadStateChanged()),
            SLOT(updateCurrentState()));

    m_keyboardActivityTimeout.setSingleShot(true);
    connect(&m_keyboardActivityTimeout, SIGNAL(timeout()),
            SLOT(timerElapsed()));

    updateCurrentState();
    reloadSettings();

    m_startup = false;
}

bool TouchpadDisabler::isEnabled() const
{
    return m_currentState != TouchpadBackend::TouchpadFullyDisabled;
}

void TouchpadDisabler::updateCurrentState()
{
    bool prevEnabled = isEnabled();
    m_currentState = m_backend->getTouchpadState();
    if (prevEnabled != isEnabled()) {
        Q_EMIT enabledChanged(isEnabled());
    }
}

void TouchpadDisabler::toggle()
{
    m_backend->setTouchpadState(isEnabled() ? TouchpadBackend::TouchpadFullyDisabled :
                                              TouchpadBackend::TouchpadEnabled
                                              );
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
    m_oldKbState = m_currentState;
    if (m_keyboardDisableState > m_currentState) {
        m_backend->setTouchpadState(m_keyboardDisableState);
        updateCurrentState();
    }
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
    m_backend->setTouchpadState(m_oldKbState);
    updateCurrentState();
}

void TouchpadDisabler::mousePlugged()
{
    bool pluggedIn = isMousePluggedIn();
    Q_EMIT mousePluggedInChanged(pluggedIn);

    bool prev = m_mouse;
    m_mouse = pluggedIn && m_settings.disableWhenMousePluggedIn();

    if (m_mouse == prev) {
        return;
    }

    TouchpadBackend::TouchpadState targetState = m_mouse ?
                TouchpadBackend::TouchpadFullyDisabled : m_oldState;
    if (m_mouse) {
        m_oldState = m_keyboardActivity ? m_currentState : m_oldKbState;
        targetState = qMax(m_currentState, targetState);
    } else if (targetState > m_currentState) {
        targetState = m_currentState;
    }

    m_oldKbState = targetState;

    if (!m_mouse && m_keyboardActivity) {
        targetState = qMax(m_keyboardDisableState, targetState);
    }

    if (m_currentState != targetState) {
        m_backend->setTouchpadState(targetState);
        updateCurrentState();

        if (m_mouse) {
            showNotification();
        }
    }
}

void TouchpadDisabler::showNotification()
{
    if (!m_settings.showNotificationWhenDisabled()) {
        return;
    }

    if (m_startup) {
        //Don't show ugly notification on top of splash screen
        //Delay it until startup is finished
        org::kde::KLauncher *klauncher =
                new org::kde::KLauncher("org.kde.klauncher", "/KLauncher",
                                        QDBusConnection::sessionBus(), this);
        if (klauncher->isValid()) {
            if (connect(klauncher, SIGNAL(autoStart2Done()), SLOT(showNotification()))) {
                return;
            }
        }
    }

    KNotification::event("TouchpadDisabled",
                         i18n("Touchpad is disabled because mouse is "
                              "detected"),
                         QPixmap(),
                         0,
                         KNotification::CloseOnTimeout,
                         TouchpadPluginFactory::componentData());
}

bool TouchpadDisabler::isMousePluggedIn() const
{
    return !m_backend->listMouses(m_settings.mouseBlacklist()).isEmpty();
}
