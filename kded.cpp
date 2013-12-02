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

#include <QIcon>

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

    m_enableTimer.setSingleShot(true);
    connect(&m_enableTimer, SIGNAL(timeout()), SLOT(timerElapsed()));

    updateCurrentState();
    reloadSettings();

    m_startup = false;

    m_confirmation.setWindowTitle(i18n("Touchpad"));
    m_confirmation.setWindowIcon(QIcon::fromTheme("input-touchpad"));
    m_confirmation.setText(i18n("No mouses are plugged in"));
    m_confirmation.setInformativeText(i18n("Are you sure you want to disable touchpad?"));

    m_confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_confirmation.setDefaultButton(QMessageBox::No);
    m_confirmation.setEscapeButton(QMessageBox::No);
    m_confirmation.setIcon(QMessageBox::Warning);
    connect(&m_confirmation, SIGNAL(finished(int)),
            SLOT(confirmationFinished(int)));
}

bool TouchpadDisabler::isEnabled() const
{
    return m_currentState == TouchpadBackend::TouchpadEnabled;
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

void TouchpadDisabler::safeToggle()
{
    if (isEnabled() && !m_backend->isMousePluggedIn()) {
        m_confirmation.show();
    } else {
        toggle();
    }
}

void TouchpadDisabler::confirmationFinished(int result)
{
    if (result == QMessageBox::Yes && isEnabled()) {
        toggle();
    }
}

void TouchpadDisabler::reloadSettings()
{
    m_settings.readConfig();
    m_enableTimer.setInterval(m_settings.keyboardActivityTimeoutMs());

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

    m_enableTimer.stop();
    m_keyboardActivity = true;
    m_oldKbState = m_currentState;
    if (isDeeper(m_keyboardDisableState, m_currentState)) {
        m_backend->setTouchpadState(m_keyboardDisableState);
        updateCurrentState();
    }
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
    m_backend->setTouchpadState(m_oldKbState);
    updateCurrentState();
}

void TouchpadDisabler::mousePlugged()
{
    bool prev = m_mouse;
    m_mouse = m_backend->isMousePluggedIn() &&
            m_settings.disableWhenMousePluggedIn();

    if (m_mouse == prev) {
        return;
    }

    TouchpadBackend::TouchpadState targetState = m_mouse ?
                TouchpadBackend::TouchpadFullyDisabled : m_oldState;
    if (m_mouse) {
        m_oldState = m_keyboardActivity ? m_currentState : m_oldKbState;
        targetState = deeper(m_currentState, targetState);
    } else if (isDeeper(targetState, m_currentState)) {
        targetState = m_currentState;
    }

    m_oldKbState = targetState;

    if (!m_mouse && m_keyboardActivity) {
        targetState = deeper(m_keyboardDisableState, targetState);
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
        QDBusInterface *klauncher = new QDBusInterface("org.kde.klauncher",
                                                       "/KLauncher",
                                                       QString(),
                                                       QDBusConnection::sessionBus(),
                                                       this);
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
