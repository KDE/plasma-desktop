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

inline bool isDeeper(TouchpadBackend::TouchpadState a,
                     TouchpadBackend::TouchpadState b)
{
    static const int deep[] = { 0, 2, 1 };
    return deep[a] > deep[b];
}

inline TouchpadBackend::TouchpadState deeper(TouchpadBackend::TouchpadState a,
                                             TouchpadBackend::TouchpadState b)
{
    return isDeeper(a, b) ? a : b;
}

static TouchpadBackend::TouchpadState getState(bool disable, bool onlyTaps)
{
    return !disable ? TouchpadBackend::TouchpadEnabled :
                      onlyTaps ? TouchpadBackend::TouchpadTapAndScrollDisabled :
                                 TouchpadBackend::TouchpadFullyDisabled;
}

TouchpadDisabler::TouchpadDisabler(QObject *parent, const QVariantList &)
    : KDEDModule(parent), m_backend(TouchpadBackend::self()),
            m_keyboardActivity(false), m_disabledByMe(false)
{
    if (!m_backend) {
        return;
    }

    m_mouse = m_backend->isMousePluggedIn();

    connect(m_backend, SIGNAL(mousesChanged()), SLOT(mousePlugged()));
    connect(m_backend, SIGNAL(keyboardActivityStarted()),
            SLOT(keyboardActivityStarted()));
    connect(m_backend, SIGNAL(keyboardActivityFinished()),
            SLOT(keyboardActivityFinished()));
    connect(m_backend, SIGNAL(touchpadStateChanged()),
            SLOT(updateCurrentState()));

    m_enableTimer.setSingleShot(true);
    connect(&m_enableTimer, SIGNAL(timeout()), SLOT(timerElapsed()));

    updateCurrentState();
    reloadSettings();
}

void TouchpadDisabler::updateCurrentState()
{
    m_currentState = m_backend->getTouchpadState();
}

void TouchpadDisabler::reloadSettings()
{
    m_settings.readConfig();
    m_enableTimer.setInterval(m_settings.keyboardActivityTimeoutMs());

    m_mouseDisableState = getState(m_settings.disableWhenMousePluggedIn(),
                                   m_settings.onlyDisableTapAndScrollWhenMousePluggedIn());
    m_keyboardDisableState = getState(m_settings.disableOnKeyboardActivity(),
                                      m_settings.onlyDisableTapAndScrollOnKeyboardActivity());

    updateState();

    m_backend->watchForEvents(m_settings.disableOnKeyboardActivity());
    updateCurrentState();

    if (!m_settings.disableOnKeyboardActivity()) {
        keyboardActivityFinished();
    }
}

void TouchpadDisabler::keyboardActivityStarted()
{
    if (m_keyboardActivity) {
        return;
    }

    m_enableTimer.stop();
    m_keyboardActivity = true;
    updateState();
}

void TouchpadDisabler::keyboardActivityFinished()
{
    if (!m_keyboardActivity) {
        keyboardActivityStarted();
    }
    m_enableTimer.start();
}

void TouchpadDisabler::timerElapsed()
{
    if (!m_keyboardActivity) {
        return;
    }

    m_keyboardActivity = false;
    updateState();
}

void TouchpadDisabler::mousePlugged()
{
    m_mouse = m_backend->isMousePluggedIn();

    updateState();
}

void TouchpadDisabler::updateState()
{
    TouchpadBackend::TouchpadState newState = deeper(
                m_mouse ? m_mouseDisableState :
                          TouchpadBackend::TouchpadEnabled,
                m_keyboardActivity ? m_keyboardDisableState :
                                     TouchpadBackend::TouchpadEnabled);

    if (newState == m_currentState) {
        if (newState == TouchpadBackend::TouchpadEnabled) {
            m_disabledByMe = false;
        }
        return;
    }
    if (!m_disabledByMe) {
        if (!deeper(newState, m_currentState)) {
            return;
        }
    }

    m_disabledByMe = (newState != TouchpadBackend::TouchpadEnabled);
    if (m_disabledByMe) {
        m_oldState = m_currentState;
    } else {
        newState = m_oldState;
    }
    m_backend->setTouchpadState(newState);
    m_currentState = newState;

    if (m_disabledByMe && m_mouse && m_currentState == m_mouseDisableState &&
            m_settings.showNotificationWhenDisabled())
    {
        KNotification::event("TouchpadDisabled",
                             i18n("Touchpad is disabled because mouse is "
                                  "detected"),
                             QPixmap(),
                             0,
                             KNotification::CloseOnTimeout,
                             TouchpadPluginFactory::componentData())
                ->sendEvent();
    }
}
