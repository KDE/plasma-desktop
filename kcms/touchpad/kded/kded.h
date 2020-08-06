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

#ifndef KDED_H
#define KDED_H

#include <QVariantList>
#include <QTimer>
#include <QDBusServiceWatcher>
#include <QDBusPendingCallWatcher>
#include <QPointer>

#include <KDEDModule>
#include <KNotification>

#include "touchpadbackend.h"
#include "kdedsettings.h"

class TouchpadDisabler : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.touchpad")

public:
    TouchpadDisabler(QObject *, const QVariantList &);

Q_SIGNALS:
    Q_SCRIPTABLE void enabledChanged(bool);
    Q_SCRIPTABLE void mousePluggedInChanged(bool);
    Q_SCRIPTABLE void workingTouchpadFoundChanged(bool);

public Q_SLOTS:
    Q_SCRIPTABLE Q_NOREPLY void reloadSettings();
    Q_SCRIPTABLE Q_NOREPLY void toggle();
    Q_SCRIPTABLE Q_NOREPLY void disable();
    Q_SCRIPTABLE Q_NOREPLY void enable();
    Q_SCRIPTABLE bool isEnabled() const;
    Q_SCRIPTABLE bool workingTouchpadFound() const;
    Q_SCRIPTABLE bool isMousePluggedIn() const;

private Q_SLOTS:
    void keyboardActivityStarted();
    void keyboardActivityFinished();
    void timerElapsed();
    void mousePlugged();
    void updateCurrentState();
    void serviceRegistered(const QString &);
    void handleReset();
    void serviceNameFetchFinished(QDBusPendingCallWatcher *);
    void onPrepareForSleep(bool sleep);

private:
    void showNotification(const QString &name, const QString &text);
    void lateInit();
    void updateWorkingTouchpadFound();
    void showOsd();

    TouchpadBackend *m_backend;
    TouchpadDisablerSettings m_settings;
    QTimer m_keyboardActivityTimeout;
    QDBusServiceWatcher m_dependencies;

    TouchpadBackend::TouchpadOffState m_keyboardDisableState;
    bool m_userRequestedState, m_touchpadEnabled;
    bool m_workingTouchpadFound;
    bool m_keyboardActivity, m_mouse;

    QPointer<KNotification> m_notification;

    bool m_preparingForSleep = false;
};

#endif // KDED_H
